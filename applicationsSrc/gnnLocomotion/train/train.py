"""
Main training script.

Usage
-----
  cd applicationsSrc/gnnLocomotion/train
  python train.py [--binary PATH] [--config PATH] [--port PORT]
                                    [--episodes N] [--device cpu|cuda]
                                    [--render] [--realtime] [--step-delay SEC]

The VisibleSim binary is built by make inside applicationsSrc/gnnLocomotion/.
Checkpoints are saved to ./checkpoints/ every SAVE_INTERVAL episodes.
"""
import argparse
import os
import sys
import time

import numpy as np
import torch

from env   import VisibleSimEnv
from model import GNNPolicy
from ppo   import PPOConfig, PPOTrainer

# ---- Default paths (relative to this file) ----
_HERE        = os.path.dirname(os.path.abspath(__file__))
_BIN_DEFAULT = os.path.join(_HERE, "../../../applicationsBin/gnnLocomotion/gnnLocomotion")
_CFG_DEFAULT = os.path.join(_HERE, "../../../applicationsBin/gnnLocomotion/config.xml")

SAVE_INTERVAL = 50
LOG_INTERVAL  = 10
CKPT_DIR      = os.path.join(_HERE, "checkpoints")


def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--binary",   default=_BIN_DEFAULT)
    p.add_argument("--config",   default=_CFG_DEFAULT)
    p.add_argument("--port",     type=int, default=9999)
    p.add_argument("--episodes", type=int, default=2000)
    p.add_argument("--device",   default="cuda" if torch.cuda.is_available() else "cpu")
    p.add_argument("--backbone", default="gat", choices=["gat", "gcn"],
                   help="GNN encoder: 'gat' (Graph Attention) or 'gcn' (Graph Convolution)")
    p.add_argument(
        "--render",
        action="store_true",
        help="launch VisibleSim with GUI (disabled by default for speed)",
    )
    p.add_argument(
        "--realtime",
        action="store_true",
        help="with --render, use realtime scheduler for visible motion",
    )
    p.add_argument(
        "--step-delay",
        type=float,
        default=0.0,
        help="optional sleep (seconds) after each env step to slow playback",
    )
    return p.parse_args()


def train(args):
    os.makedirs(CKPT_DIR, exist_ok=True)

    print(f"[train] binary : {args.binary}")
    print(f"[train] config : {args.config}")
    print(f"[train] device : {args.device}")
    print(f"[train] backbone : {args.backbone}")
    print(f"[train] render : {args.render}")
    print(f"[train] realtime : {args.realtime}")
    print(f"[train] step_delay : {args.step_delay}")

    if not os.path.isfile(args.binary):
        sys.exit(f"[train] Binary not found: {args.binary}\n"
                 "  Run: cd applicationsSrc/gnnLocomotion && make")

    ppo_cfg = PPOConfig(
        lr           = 3e-4,
        n_steps      = 20,
        n_epochs     = 4,
        batch_size   = 32,
        clip_eps     = 0.2,
        value_coef   = 0.5,
        entropy_coef = 0.01,
    )

    policy  = GNNPolicy(backbone=args.backbone)
    trainer = PPOTrainer(policy, ppo_cfg, device=args.device)
    env     = VisibleSimEnv(
        args.binary,
        args.config,
        port=args.port,
        render=args.render,
        realtime=args.realtime,
    )

    ep_rewards: list = []
    ep_lengths: list = []

    try:
        for episode in range(args.episodes):
            obs     = env.reset()
            tensors = env.obs_to_tensors(obs)

            ep_reward = 0.0
            ep_steps  = 0
            done      = False
            target_reached = False
            step_rewards: list = []

            while not done:
                # ---- sample actions ----
                actions_dict, actions_t, log_probs_t, value_t = \
                    trainer.select_actions(*tensors, env.block_ids)

                # ---- environment step ----
                next_obs, reward, done, info = env.step(actions_dict)
                ep_reward += reward
                ep_steps  += 1
                step_rewards.append(reward)

                target_cells = len(next_obs.get("target", []))
                in_target = sum(1 for b in next_obs.get("blocks", []) if b.get("in_target", False))
                if target_cells > 0 and in_target >= target_cells:
                    target_reached = True

                if args.step_delay > 0:
                    time.sleep(args.step_delay)

                # ---- store transition ----
                trainer.store(tensors, actions_t, log_probs_t, value_t,
                              reward, done)

                tensors = env.obs_to_tensors(next_obs)

                # ---- update if rollout is full ----
                if trainer.ready():
                    metrics = trainer.update()
                    if metrics and episode % LOG_INTERVAL == 0:
                        recent_step_rewards = step_rewards[-LOG_INTERVAL:]
                        mean_step_reward = np.mean(recent_step_rewards)
                        print(f"  [update] loss={metrics['loss']:.4f}  "
                              f"pg={metrics['pg']:.4f}  "
                              f"vf={metrics['vf']:.4f}  "
                              f"ent={metrics['ent']:.4f}  "
                              f"r_step={reward:.3f}  "
                              f"r_step_mean={mean_step_reward:.3f}  "
                              f"ep_r={ep_reward:.3f}")

                if done and target_reached:
                    print(f"  [target] reached at episode={episode} step={ep_steps}  "
                          f"in_target={in_target}/{target_cells}  ep_r={ep_reward:.3f}")

            # Final update at episode end
            if trainer.rollout:
                trainer.update()

            ep_rewards.append(ep_reward)
            ep_lengths.append(ep_steps)

            if episode % LOG_INTERVAL == 0:
                recent_r = ep_rewards[-LOG_INTERVAL:]
                recent_l = ep_lengths[-LOG_INTERVAL:]
                print(f"Episode {episode:5d}  "
                      f"reward={ep_reward:7.2f}  "
                      f"mean_r={np.mean(recent_r):7.2f}  "
                      f"steps={ep_steps:4d}  "
                      f"mean_steps={np.mean(recent_l):.1f}")

            if episode > 0 and episode % SAVE_INTERVAL == 0:
                ckpt = os.path.join(CKPT_DIR, f"policy_ep{episode:05d}.pt")
                torch.save({
                    "episode":     episode,
                    "model_state": policy.state_dict(),
                    "optim_state": trainer.optim.state_dict(),
                }, ckpt)
                print(f"  [ckpt] Saved {ckpt}")

    except KeyboardInterrupt:
        print("\n[train] Interrupted.")

    finally:
        env.close()
        final_ckpt = os.path.join(CKPT_DIR, "policy_final.pt")
        torch.save({
            "episode":     args.episodes,
            "model_state": policy.state_dict(),
            "optim_state": trainer.optim.state_dict(),
        }, final_ckpt)
        print(f"[train] Saved final checkpoint: {final_ckpt}")


if __name__ == "__main__":
    train(parse_args())
