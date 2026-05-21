"""
Main training script for gnnLocomotion.

Usage
-----
  cd applicationsSrc/gnnLocomotion/train
  python train.py [--binary PATH] [--config PATH] [--port PORT]
                  [--episodes N] [--device cpu|cuda]
                  [--direction DX,DY,DZ]
                  [--render] [--realtime] [--step-delay SEC]

Objective: maximise CoM displacement along --direction per episode.
Reward = ΔCoM · direction at each step (potential-based shaping).
"""
import argparse
import os
import sys
import time
from typing import Optional

import numpy as np
import torch

from env   import VisibleSimEnv
from model import GNNPolicy
from ppo   import PPOConfig, PPOTrainer

_HERE        = os.path.dirname(os.path.abspath(__file__))
_BIN_DEFAULT = os.path.join(_HERE, "../../../applicationsBin/gnnLocomotion/gnnLocomotion")
_CFG_DEFAULT = os.path.join(_HERE, "../../../applicationsBin/gnnLocomotion/config.xml")

SAVE_INTERVAL = 50
LOG_INTERVAL  = 10
CKPT_DIR      = os.path.join(_HERE, "checkpoints")


def _init_wandb(args, ppo_cfg) -> Optional[object]:
    if not args.wandb:
        return None
    try:
        import wandb  # type: ignore
    except ImportError:
        print("[wandb] not installed; continuing without wandb logging.")
        return None
    run = wandb.init(
        project=args.wandb_project,
        entity=args.wandb_entity,
        name=args.wandb_run_name,
        mode=args.wandb_mode,
        config={
            "binary":     args.binary,
            "config":     args.config,
            "direction":  args.direction,
            "episodes":   args.episodes,
            "device":     args.device,
            "backbone":   args.backbone,
            "ppo": {
                "lr":            ppo_cfg.lr,
                "gamma":         ppo_cfg.gamma,
                "gae_lambda":    ppo_cfg.gae_lambda,
                "clip_eps":      ppo_cfg.clip_eps,
                "value_coef":    ppo_cfg.value_coef,
                "entropy_coef":  ppo_cfg.entropy_coef,
                "n_epochs":      ppo_cfg.n_epochs,
                "batch_size":    ppo_cfg.batch_size,
                "n_steps":       ppo_cfg.n_steps,
            },
        },
    )
    print(f"[wandb] run initialized: {run.name}")
    return run


def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--binary",    default=_BIN_DEFAULT)
    p.add_argument("--config",    default=_CFG_DEFAULT)
    p.add_argument("--port",      type=int, default=9999)
    p.add_argument("--episodes",  type=int, default=2000)
    p.add_argument("--device",    default="cuda" if torch.cuda.is_available() else "cpu")
    p.add_argument("--backbone",  default="gat", choices=["gat", "gcn"])
    p.add_argument("--direction", default="1,0,0",
                   help="locomotion direction as dx,dy,dz (will be normalised)")
    p.add_argument("--render",    action="store_true")
    p.add_argument("--realtime",  action="store_true")
    p.add_argument("--step-delay", type=float, default=0.0)
    p.add_argument("--wandb",     action="store_true")
    p.add_argument("--wandb-project",  default="visible-sim-gnn-locomotion")
    p.add_argument("--wandb-entity",   default=None)
    p.add_argument("--wandb-run-name", default=None)
    p.add_argument("--wandb-mode",     default="online",
                   choices=["online", "offline", "disabled"])
    return p.parse_args()


def train(args):
    os.makedirs(CKPT_DIR, exist_ok=True)

    # Parse direction vector
    try:
        dx, dy, dz = [float(v) for v in args.direction.split(",")]
    except ValueError:
        sys.exit(f"[train] --direction must be 'dx,dy,dz', got: {args.direction}")
    direction = (dx, dy, dz)

    print(f"[train] binary    : {args.binary}")
    print(f"[train] config    : {args.config}")
    print(f"[train] direction : {direction}")
    print(f"[train] device    : {args.device}")
    print(f"[train] backbone  : {args.backbone}")

    if not os.path.isfile(args.binary):
        sys.exit(f"[train] Binary not found: {args.binary}\n"
                 "  Run: cd applicationsSrc/gnnLocomotion && make")

    ppo_cfg = PPOConfig(
        lr           = 3e-4,
        n_steps      = 256,
        n_epochs     = 4,
        batch_size   = 32,
        clip_eps     = 0.2,
        value_coef   = 0.5,
        entropy_coef = 0.01,
    )

    policy    = GNNPolicy(backbone=args.backbone)
    trainer   = PPOTrainer(policy, ppo_cfg, device=args.device)
    wandb_run = _init_wandb(args, ppo_cfg)
    env       = VisibleSimEnv(
        args.binary, args.config,
        port=args.port, render=args.render, realtime=args.realtime,
        direction=direction,
    )

    ep_rewards: list = []
    ep_lengths: list = []
    global_step = 0

    try:
        for episode in range(args.episodes):
            obs     = env.reset()
            tensors = env.obs_to_tensors(obs)

            ep_reward = 0.0
            ep_steps  = 0
            done      = False

            while not done:
                actions_dict, actions_t, log_probs_t, value_t = \
                    trainer.select_actions(*tensors, env.block_ids)

                next_obs, reward, done, info = env.step(actions_dict)
                ep_reward   += reward
                ep_steps    += 1
                global_step += 1

                if args.step_delay > 0:
                    time.sleep(args.step_delay)

                trainer.store(tensors, actions_t, log_probs_t, value_t, reward, done)
                tensors = env.obs_to_tensors(next_obs)

                if trainer.ready():
                    metrics = trainer.update()
                    if wandb_run and metrics:
                        wandb_run.log({
                            "update/loss": metrics["loss"],
                            "update/pg":   metrics["pg"],
                            "update/vf":   metrics["vf"],
                            "update/ent":  metrics["ent"],
                            "train/global_step": global_step,
                        }, step=global_step)

            ep_rewards.append(ep_reward)
            ep_lengths.append(ep_steps)

            if episode % LOG_INTERVAL == 0:
                recent_r = ep_rewards[-LOG_INTERVAL:]
                recent_l = ep_lengths[-LOG_INTERVAL:]
                print(f"Episode {episode:5d}  reward={ep_reward:7.2f}  "
                      f"mean_r={np.mean(recent_r):7.2f}  steps={ep_steps:4d}  "
                      f"mean_steps={np.mean(recent_l):.1f}")

            if wandb_run:
                wandb_run.log({
                    "episode/reward":            ep_reward,
                    "episode/steps":             ep_steps,
                    "episode/mean_reward_window": np.mean(ep_rewards[-LOG_INTERVAL:]),
                    "train/global_step":         global_step,
                    "train/episode":             episode,
                }, step=global_step)

            if episode > 0 and episode % SAVE_INTERVAL == 0:
                ckpt = os.path.join(CKPT_DIR, f"policy_ep{episode:05d}.pt")
                torch.save({"episode": episode, "model_state": policy.state_dict(),
                            "optim_state": trainer.optim.state_dict()}, ckpt)
                print(f"  [ckpt] Saved {ckpt}")

    except KeyboardInterrupt:
        print("\n[train] Interrupted.")

    finally:
        env.close()
        final_ckpt = os.path.join(CKPT_DIR, "policy_final.pt")
        torch.save({"episode": args.episodes, "model_state": policy.state_dict(),
                    "optim_state": trainer.optim.state_dict()}, final_ckpt)
        print(f"[train] Saved final checkpoint: {final_ckpt}")
        if wandb_run:
            wandb_run.finish()


if __name__ == "__main__":
    train(parse_args())
