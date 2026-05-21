"""
GNN actor-critic for modular robot self-reconfiguration.

Architecture
------------
  Encoder : 3-layer GNN (GAT or GCN) with edge features
  Actor   : per-node MLP → action logits  [N, MAX_ACTIONS]   ← distributed
  Critic  : global mean-pool MLP → value  scalar             ← centralized (training only)

Backbone choice
---------------
  backbone='gat'  Graph Attention Network — attention weights over neighbours
  backbone='gcn'  Graph Convolutional Network — uniform mean aggregation,
                  lower parameter count, straightforward to analyse

Distributed inference
---------------------
Both backbones satisfy the distributed-inference property: after L message-
passing layers a node's hidden state depends only on its L-hop neighbourhood.
For action selection, call distributed_act() — it runs the encoder + actor
head only, with no global pooling, so each physical module can run it
independently using only the information available over its local links.
"""
import torch
import torch.nn as nn
import torch.nn.functional as F
from typing import Literal, Optional

MAX_ACTIONS = 13   # 0=stay + up to 12 moves
NODE_DIM    = 12   # must match env.NODE_FEAT_DIM
EDGE_DIM    = 6    # one-hot connector direction
HIDDEN      = 64

try:
    from torch_geometric.nn import GATConv, MessagePassing
    from torch_geometric.nn import global_mean_pool as _pyg_global_mean
    _HAS_PYG = True
except ImportError:
    _HAS_PYG = False


# ---------------------------------------------------------------------------
# GCN layer with edge features
# ---------------------------------------------------------------------------

if _HAS_PYG:
    class GCNEdgeConv(MessagePassing):
        """
        GCN-style convolution that incorporates directed edge features.

        For each node i:
            msg_{j→i}  = W_msg · [h_j ‖ e_{ji}]
            h_i'       = mean_{j∈N(i)}(msg_{j→i}) + W_self · h_i + b

        Using mean aggregation (no learned attention weights) keeps the layer
        symmetric and easy to analyse; the self-loop term preserves the node's
        own features.  Each layer is one communication round, making the hop-
        count / information-radius relationship explicit.
        """

        def __init__(self, in_channels: int, out_channels: int, edge_dim: int):
            super().__init__(aggr="mean")
            self.msg_lin  = nn.Linear(in_channels + edge_dim, out_channels, bias=False)
            self.self_lin = nn.Linear(in_channels,            out_channels, bias=False)
            self.bias     = nn.Parameter(torch.zeros(out_channels))

        def forward(
            self,
            x:          torch.Tensor,   # [N, in_channels]
            edge_index: torch.Tensor,   # [2, E]
            edge_attr:  torch.Tensor,   # [E, edge_dim]
        ) -> torch.Tensor:              # [N, out_channels]
            agg = self.propagate(edge_index, x=x, edge_attr=edge_attr)
            return agg + self.self_lin(x) + self.bias

        def message(self, x_j: torch.Tensor, edge_attr: torch.Tensor) -> torch.Tensor:
            return self.msg_lin(torch.cat([x_j, edge_attr], dim=-1))


# ---------------------------------------------------------------------------
# Policy
# ---------------------------------------------------------------------------

class GNNPolicy(nn.Module):
    """
    Actor-critic policy operating on the communication graph of modular robots.

    Parameters
    ----------
    backbone : 'gat' | 'gcn'
        'gat' — Graph Attention Network (richer, recommended for training)
        'gcn' — Graph Convolutional Network (simpler, recommended for analysis
                and on-robot deployment)
    """

    def __init__(
        self,
        node_dim:   int = NODE_DIM,
        edge_dim:   int = EDGE_DIM,
        hidden_dim: int = HIDDEN,
        n_actions:  int = MAX_ACTIONS,
        backbone:   Literal["gat", "gcn"] = "gat",
    ):
        super().__init__()
        self.n_actions  = n_actions
        self.hidden_dim = hidden_dim
        self.backbone   = backbone

        if _HAS_PYG:
            if backbone == "gat":
                self.conv1 = GATConv(node_dim,   hidden_dim, edge_dim=edge_dim,
                                     heads=4, concat=False, dropout=0.0)
                self.conv2 = GATConv(hidden_dim, hidden_dim, edge_dim=edge_dim,
                                     heads=4, concat=False, dropout=0.0)
                self.conv3 = GATConv(hidden_dim, hidden_dim, edge_dim=edge_dim,
                                     heads=1, concat=False, dropout=0.0)
            elif backbone == "gcn":
                self.conv1 = GCNEdgeConv(node_dim,   hidden_dim, edge_dim)
                self.conv2 = GCNEdgeConv(hidden_dim, hidden_dim, edge_dim)
                self.conv3 = GCNEdgeConv(hidden_dim, hidden_dim, edge_dim)
            else:
                raise ValueError(f"backbone must be 'gat' or 'gcn', got {backbone!r}")
        else:
            # No PyG: fall back to an MLP that ignores graph structure.
            # Both backbone choices degrade to the same MLP fallback.
            self.fallback_enc = nn.Sequential(
                nn.Linear(node_dim,   hidden_dim), nn.ELU(),
                nn.Linear(hidden_dim, hidden_dim), nn.ELU(),
                nn.Linear(hidden_dim, hidden_dim),
            )

        # ---- Actor head (per-node → distributed) ----
        self.actor_head = nn.Sequential(
            nn.Linear(hidden_dim, hidden_dim), nn.ELU(),
            nn.Linear(hidden_dim, n_actions),
        )

        # ---- Critic head (global pool → centralized, training only) ----
        self.critic_head = nn.Sequential(
            nn.Linear(hidden_dim, hidden_dim), nn.ELU(),
            nn.Linear(hidden_dim, 1),
        )

        self._init_weights()
        print(f"[GNNPolicy] backbone={backbone}  "
              f"({'PyG' if _HAS_PYG else 'MLP fallback'})")

    def _init_weights(self):
        for m in self.modules():
            if isinstance(m, nn.Linear):
                nn.init.orthogonal_(m.weight, gain=1.0)
                if m.bias is not None:
                    nn.init.zeros_(m.bias)

    # ------------------------------------------------------------------
    # Encoder — shared by actor and critic
    # ------------------------------------------------------------------

    def encode(
        self,
        x:          torch.Tensor,   # [N, node_dim]
        edge_index: torch.Tensor,   # [2, E]
        edge_attr:  torch.Tensor,   # [E, edge_dim]
    ) -> torch.Tensor:              # [N, hidden_dim]
        """3-layer GNN encoder.  Output is L-hop neighbourhood representation."""
        if _HAS_PYG:
            h = F.elu(self.conv1(x, edge_index, edge_attr))
            h = F.elu(self.conv2(h, edge_index, edge_attr))
            h = F.elu(self.conv3(h, edge_index, edge_attr))
        else:
            h = self.fallback_enc(x)
        return h

    # ------------------------------------------------------------------
    # Distributed actor — no global pooling
    # ------------------------------------------------------------------

    @torch.no_grad()
    def distributed_act(
        self,
        x:            torch.Tensor,
        edge_index:   torch.Tensor,
        edge_attr:    torch.Tensor,
        action_masks: Optional[torch.Tensor] = None,
    ) -> torch.Tensor:
        """
        Distributed inference: each node computes its own action distribution
        using only its L-hop neighbourhood.  No global aggregation is performed,
        so every physical module can execute this independently after exchanging
        L rounds of messages with its neighbours.

        Returns
        -------
        probs : [N, n_actions]  action probability for each node
        """
        h      = self.encode(x, edge_index, edge_attr)
        logits = self.actor_head(h)
        if action_masks is not None:
            logits = logits.masked_fill(~action_masks, float("-inf"))
        return torch.softmax(logits, dim=-1)

    # ------------------------------------------------------------------
    # Centralised forward — actor + critic (used during training)
    # ------------------------------------------------------------------

    def forward(
        self,
        x:          torch.Tensor,
        edge_index: torch.Tensor,
        edge_attr:  torch.Tensor,
        batch:      Optional[torch.Tensor] = None,
    ):
        """
        Returns
        -------
        logits : [N, n_actions]   unmasked per-node action logits
        value  : []               global scalar value estimate
        """
        h      = self.encode(x, edge_index, edge_attr)
        logits = self.actor_head(h)

        if _HAS_PYG and batch is not None:
            global_h = _pyg_global_mean(h, batch)
        else:
            global_h = h.mean(dim=0, keepdim=True)

        value = self.critic_head(global_h).squeeze()
        return logits, value

    # ------------------------------------------------------------------
    # PPO interface
    # ------------------------------------------------------------------

    def get_action_and_value(
        self,
        x:            torch.Tensor,
        edge_index:   torch.Tensor,
        edge_attr:    torch.Tensor,
        action_masks: Optional[torch.Tensor] = None,
        actions:      Optional[torch.Tensor] = None,
    ):
        """
        Sample or evaluate actions.

        Parameters
        ----------
        action_masks : [N, n_actions] bool — True = valid action
        actions      : [N] long — if given, evaluate log-prob instead of sampling

        Returns
        -------
        actions, log_probs, entropy, value
        """
        logits, value = self.forward(x, edge_index, edge_attr)

        if action_masks is not None:
            logits = logits.masked_fill(~action_masks, float("-inf"))

        dist = torch.distributions.Categorical(logits=logits)

        if actions is None:
            actions = dist.sample()

        return actions, dist.log_prob(actions), dist.entropy(), value
