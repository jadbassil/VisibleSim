"""
Proximal Policy Optimisation (PPO) for multi-agent GNN policies.

Each "step" from the environment contains per-node (per-module) actions,
so transitions store full graph tensors.  The value estimate is global
(one scalar per graph), and advantages are broadcast to all nodes.
"""
from __future__ import annotations

import random
from dataclasses import dataclass
from typing import List

import torch
import torch.nn.functional as F


@dataclass
class PPOConfig:
    lr:            float = 3e-4
    gamma:         float = 0.99
    gae_lambda:    float = 0.95
    clip_eps:      float = 0.2
    value_coef:    float = 0.5
    entropy_coef:  float = 0.01
    max_grad_norm: float = 0.5
    n_epochs:      int   = 4
    batch_size:    int   = 32   # number of transitions per mini-batch
    n_steps:       int   = 256  # rollout length before an update


@dataclass
class Transition:
    x:            torch.Tensor   # [N, node_dim]
    edge_index:   torch.Tensor   # [2, E]
    edge_attr:    torch.Tensor   # [E, edge_dim]
    action_masks: torch.Tensor   # [N, n_actions] bool
    actions:      torch.Tensor   # [N] long
    log_probs:    torch.Tensor   # [N] float
    value:        torch.Tensor   # scalar (global value)
    reward:       float
    done:         bool


class PPOTrainer:
    def __init__(self, policy, config: PPOConfig, device: str = "cpu"):
        if device == "cpu" and torch.backends.mps.is_available():
            device = "mps"
        print(f"[PPOTrainer] Using device: {device}")
        self.policy   = policy.to(device)
        self.config   = config
        self.device   = device
        self.optim    = torch.optim.Adam(policy.parameters(), lr=config.lr,
                                         eps=1e-5)
        self.rollout: List[Transition] = []

    # ------------------------------------------------------------------
    # Action sampling
    # ------------------------------------------------------------------

    @torch.no_grad()
    def select_actions(self, x, edge_index, edge_attr, action_masks, block_ids):
        """
        Sample actions for the current observation.
        Returns {block_id: action_index}.
        """
        x           = x.to(self.device)
        edge_index  = edge_index.to(self.device)
        edge_attr   = edge_attr.to(self.device)
        action_masks = action_masks.to(self.device)

        actions, log_probs, _, value = self.policy.get_action_and_value(
            x, edge_index, edge_attr, action_masks)

        return (
            {block_ids[i]: actions[i].item() for i in range(len(block_ids))},
            actions.cpu(), log_probs.cpu(), value.cpu(),
        )

    def store(self, tensors, actions, log_probs, value, reward, done):
        """Append one transition to the rollout buffer."""
        x, edge_index, edge_attr, masks = tensors
        self.rollout.append(Transition(
            x=x.cpu(), edge_index=edge_index.cpu(), edge_attr=edge_attr.cpu(),
            action_masks=masks.cpu(), actions=actions.cpu(),
            log_probs=log_probs.cpu(), value=value.cpu(),
            reward=float(reward), done=bool(done),
        ))

    def ready(self) -> bool:
        return len(self.rollout) >= self.config.n_steps

    # ------------------------------------------------------------------
    # PPO update
    # ------------------------------------------------------------------

    def update(self) -> dict:
        if len(self.rollout) < 2:
            return {}

        cfg = self.config
        T   = len(self.rollout)

        # ---- GAE returns & advantages ----
        returns    = [0.0] * T
        advantages = [0.0] * T
        gae = 0.0

        for t in reversed(range(T)):
            tr = self.rollout[t]
            if t == T - 1:
                next_val      = 0.0 if tr.done else tr.value.item()
                next_nonterm  = 0.0 if tr.done else 1.0
            else:
                next_val     = self.rollout[t + 1].value.item()
                next_nonterm = 0.0 if self.rollout[t].done else 1.0

            delta      = tr.reward + cfg.gamma * next_val * next_nonterm - tr.value.item()
            gae        = delta + cfg.gamma * cfg.gae_lambda * next_nonterm * gae
            advantages[t] = gae
            returns[t]    = gae + tr.value.item()

        adv_t = torch.tensor(advantages, dtype=torch.float32, device=self.device)
        ret_t = torch.tensor(returns,    dtype=torch.float32, device=self.device)

        # Normalize advantages globally
        if T > 1:
            adv_t = (adv_t - adv_t.mean()) / (adv_t.std() + 1e-8)

        # ---- Mini-batch PPO ----
        indices = list(range(T))
        stats   = dict(loss=0.0, pg=0.0, vf=0.0, ent=0.0, n=0)

        for _ in range(cfg.n_epochs):
            random.shuffle(indices)
            for start in range(0, T, cfg.batch_size):
                batch = indices[start: start + cfg.batch_size]
                for i in batch:
                    tr  = self.rollout[i]
                    adv = adv_t[i]
                    ret = ret_t[i]

                    x          = tr.x.to(self.device)
                    ei         = tr.edge_index.to(self.device)
                    ea         = tr.edge_attr.to(self.device)
                    masks      = tr.action_masks.to(self.device)
                    old_acts   = tr.actions.to(self.device)
                    old_lp     = tr.log_probs.to(self.device)

                    _, new_lp, entropy, new_val = self.policy.get_action_and_value(
                        x, ei, ea, masks, old_acts)

                    # Per-node ratio → mean over nodes
                    ratio    = (new_lp - old_lp).exp()
                    pg1      = -adv * ratio
                    pg2      = -adv * ratio.clamp(1 - cfg.clip_eps, 1 + cfg.clip_eps)
                    pg_loss  = torch.max(pg1, pg2).mean()

                    # Value loss (global scalar)
                    vf_loss  = F.mse_loss(new_val.squeeze(), ret)

                    ent_loss = -entropy.mean()

                    loss = pg_loss + cfg.value_coef * vf_loss + cfg.entropy_coef * ent_loss

                    self.optim.zero_grad()
                    loss.backward()
                    torch.nn.utils.clip_grad_norm_(
                        self.policy.parameters(), cfg.max_grad_norm)
                    self.optim.step()

                    stats["loss"] += loss.item()
                    stats["pg"]   += pg_loss.item()
                    stats["vf"]   += vf_loss.item()
                    stats["ent"]  += entropy.mean().item()
                    stats["n"]    += 1

        self.rollout.clear()

        n = max(stats.pop("n"), 1)
        return {k: v / n for k, v in stats.items()}
