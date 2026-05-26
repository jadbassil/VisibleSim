# GNN Shape Reconfiguration — wandb Metrics Reference

This document describes every metric logged to Weights & Biases (wandb) by `train.py`. All metrics are scoped by `train/global_step` (the cumulative environment step counter across all episodes) on the wandb step axis.

To enable logging, pass `--wandb` to `train.py`:

```sh
python train.py --wandb --wandb-project marl-shape --backbone gcn
```

---

## Run Configuration (wandb `config`)

These values are written once at `wandb.init()` and visible in the run's **Overview / Config** panel. They are not time series.

| Key | Source | Meaning |
|---|---|---|
| `binary` | `--binary` | Absolute path to the VisibleSim binary launched as the env. |
| `config` | `--config` | Absolute path to the `config.xml` used. Determines initial shape and target shape. |
| `port` | `--port` | TCP port for the gym socket. |
| `episodes` | `--episodes` | Maximum number of episodes the training loop will run. |
| `device` | `--device` | Torch device used for policy/critic forward and backward (`cpu` or `cuda`). |
| `backbone` | `--backbone` | GNN encoder type — `gat` (default, richer) or `gcn` (deployable). |
| `render` | `--render` | Whether VisibleSim was launched with GUI. |
| `realtime` | `--realtime` | With `--render`, whether the realtime scheduler is used. |
| `step_delay` | `--step-delay` | Per-step sleep in seconds for visual debugging. |
| `ppo.lr` | `PPOConfig` | Adam learning rate (default `3e-4`). |
| `ppo.gamma` | `PPOConfig` | Discount factor for returns (default `0.99`). |
| `ppo.gae_lambda` | `PPOConfig` | GAE λ — bias/variance trade-off for advantage estimation (default `0.95`). |
| `ppo.clip_eps` | `PPOConfig` | PPO clipping range ε (default `0.2`). |
| `ppo.value_coef` | `PPOConfig` | Weight of value-function loss in the total loss (default `0.5`). |
| `ppo.entropy_coef` | `PPOConfig` | Weight of entropy bonus (default `0.01`). |
| `ppo.max_grad_norm` | `PPOConfig` | Gradient-norm clip applied before `optim.step()` (default `0.5`). |
| `ppo.n_epochs` | `PPOConfig` | Number of passes over the rollout per update (default `4`). |
| `ppo.batch_size` | `PPOConfig` | Mini-batch size in transitions (default `32`). |
| `ppo.n_steps` | `PPOConfig` | Rollout length collected before each PPO update (default `256`). |

---

## Time-Series Namespaces

Logged time-series keys are grouped by the prefix before the first `/`. There are four namespaces:

- `episode/…` — per-episode summary, logged at episode boundary.
- `update/…` — per-PPO-update training diagnostics, logged when `trainer.ready()` triggers an update.
- `train/…` — bookkeeping counters logged alongside the others.
- `checkpoint/…` — checkpoint events.

---

## `episode/…` — End-of-Episode Summary

Logged once per episode in the main loop (`train.py:211`). Reflects the trajectory just completed.

| Metric | Definition | Computation |
|---|---|---|
| `episode/reward` | Total episode reward. | `Σ_t R_total(t)` over all steps of the episode, where `R_total = R_sim + 0.1·(Φ(s')−Φ(s))`. |
| `episode/steps` | Number of gym steps taken. | Increments by 1 each env step; ranges `[1, MAX_STEPS]`. Lower is better once the shape is achievable. |
| `episode/mean_reward_window` | Mean of `episode/reward` over the last `LOG_INTERVAL=10` episodes. | Rolling mean — smooths over single-episode variance. |
| `episode/mean_steps_window` | Mean of `episode/steps` over the last `LOG_INTERVAL=10` episodes. | Falling values across training indicate the policy is solving the task faster. |
| `episode/target_reached` | `1` if all target cells were filled before episode end, else `0`. | Computed step-by-step (`target_reached = True` once `in_target ≥ target_cells`). |
| `episode/target_fill_ratio` | Fraction of target cells occupied at episode end. | `in_target / target_cells` (computed on the final `next_obs`). Ranges `[0, 1]`. |
| `episode/solve_rate` | Rolling success rate over the last `SUCCESS_WINDOW=200` episodes. | `np.mean(ep_solved[-SUCCESS_WINDOW:])`. Used for early-stopping (training halts at `≥ 0.95`). |

### Mid-episode success log

When the shape is reached **during** an episode (not only at its end), an additional log line is emitted at the moment of success (`train.py:169`):

| Metric | Definition |
|---|---|
| `episode/target_reached` | Always `1` at this firing — confirms the success event. |
| `episode/target_fill_ratio` | Fill ratio at the moment of completion (typically `1.0`). |

This means `episode/target_reached` can appear twice in a single episode: once at the success step and once at the end-of-episode summary. When plotting, prefer the end-of-episode line (or filter by step boundary) to avoid double-counting.

---

## `update/…` — Per-PPO-Update Diagnostics

Logged once per PPO update inside the rollout loop (`train.py:183`). An update is triggered every `n_steps = 256` env transitions. Each value is the mean over `n_epochs × (n_steps / batch_size)` mini-batch passes inside `PPOTrainer.update()`.

| Metric | Definition | What to look for |
|---|---|---|
| `update/loss` | Total scalar loss minimised by Adam. | `pg + value_coef · vf + entropy_coef · ent_loss`. Should trend downward then stabilise. |
| `update/pg` | Clipped policy-gradient loss. | `mean(max(−adv·ratio, −adv·clamp(ratio, 1−ε, 1+ε)))` where `ratio = exp(new_logp − old_logp)`. Negative `pg` = policy is improving in the right direction; small positive bursts are normal when the clip activates. |
| `update/vf` | Value-function MSE loss. | `mean((V(s) − return)²)`. Should decrease as the critic learns. |
| `update/ent` | Mean per-step entropy of the action distribution (positive, ≤ `log(N_ACTIONS)=log 13≈2.56`). | High early in training (exploration), decreases as the policy commits. A stuck high value means the policy isn't differentiating actions; a collapse to 0 too early means premature exploitation. |
| `update/r_step` | The single last step reward seen before the update fired. | Snapshot value — noisy. Use `r_step_mean` instead for trend analysis. |
| `update/r_step_mean` | Mean of the most recent `LOG_INTERVAL=10` step rewards. | Smoother view of the per-step reward signal. |
| `update/ep_r_partial` | Cumulative reward of the episode in progress when the update fired. | Mid-episode snapshot; resets to `0` at every `env.reset()`. Useful for spotting whether updates correlate with reward trajectories within an episode. |

> **Note on `update/loss` sign**: total loss combines a (potentially negative) policy gradient term, a positive value loss, and a negative entropy bonus weighted by `entropy_coef`. Values can be small in magnitude (often `< 0.1`) and the absolute number matters less than the trend.

---

## `train/…` — Bookkeeping Counters

Attached to every wandb log call so each panel can plot against either training time scale.

| Metric | Definition |
|---|---|
| `train/global_step` | Total environment step count across all episodes so far. Monotonically increasing. Used as the wandb step axis (`step=global_step`). |
| `train/episode` | Index of the current episode (0-based). Monotonically non-decreasing — flat during a single episode, then jumps by 1. |

---

## `checkpoint/…` — Checkpoint Events

Sparse events; absent from the time series most of the time.

| Metric | Definition | When logged |
|---|---|---|
| `checkpoint/episode` | Episode number at which the periodic checkpoint was saved. | Every `SAVE_INTERVAL=50` episodes (skipping episode 0). |
| `checkpoint/final_saved` | Always `1`. | Logged exactly once on training exit (normal completion, early stop, or `KeyboardInterrupt`). Indicates `policy_final.pt` was written. |

---

## How to read the dashboards

1. **Solve curve** — primary success signal: `episode/solve_rate` vs. `train/episode`. Early stop fires when this reaches `0.95` over the last 200 episodes.
2. **Reward curve** — `episode/mean_reward_window` vs. `train/episode`. Should climb monotonically once the policy starts solving the task; flat lines near zero indicate the policy is mostly stalling.
3. **Efficiency curve** — `episode/mean_steps_window` vs. `train/episode`. Should drop towards a small number (≪ `MAX_STEPS = 200`) once the policy can solve consistently.
4. **Optimisation health** — `update/loss`, `update/pg`, `update/vf` vs. `train/global_step`. Look for smooth decay; large spikes typically correlate with on-policy distribution shift and should subside within a few updates.
5. **Exploration** — `update/ent` should decrease gradually. A premature drop to near-zero before `solve_rate` rises suggests entropy bonus is too low.
6. **Fill quality without success** — when `episode/target_reached = 0`, `episode/target_fill_ratio` reveals how close the policy came; values stuck near the initial overlap (e.g., `1/6 ≈ 0.167` for the default config) mean no progress was made beyond starting placement.

---

## Reproducibility tips

- Always log `--backbone` and the `config.xml` content (uploaded by wandb as `config`/`binary` references) — the same code with different XML targets will produce wildly different reward curves.
- Pin `--wandb-run-name` per experiment so you can group runs by hyperparameter.
- If running multiple seeds, the only argument that varies environment behaviour is the `<deploy seed="…">` attribute (deploy mode only) and PyTorch's global RNG — there is no `--seed` flag on `train.py` today; vary the PyTorch seed manually if you need controlled seeding.
