#include "gcnPolicy.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace GNNLocomotion {

namespace {

constexpr char     MAGIC[4] = {'G', 'C', 'N', '1'};
constexpr uint32_t VERSION  = 1;

// CRC-32 (IEEE 802.3) — same polynomial Python's zlib.crc32 uses.
uint32_t crc32(const uint8_t* data, size_t n) {
    static uint32_t table[256];
    static bool     ready = false;
    if (!ready) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) c = (c >> 1) ^ ((c & 1) ? 0xEDB88320u : 0);
            table[i] = c;
        }
        ready = true;
    }
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++) c = table[(c ^ data[i]) & 0xFFu] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

void readMatrix(std::ifstream& f, Eigen::MatrixXf& m, int rows, int cols) {
    m.resize(rows, cols);
    // Stored row-major; Eigen default is column-major. Read into a temp buffer
    // then transpose into m.
    std::vector<float> buf(rows * cols);
    f.read(reinterpret_cast<char*>(buf.data()), buf.size() * sizeof(float));
    if (!f) throw std::runtime_error("gcnPolicy: short read on matrix");
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            m(i, j) = buf[i * cols + j];
}

void readVector(std::ifstream& f, Eigen::VectorXf& v, int n) {
    v.resize(n);
    f.read(reinterpret_cast<char*>(v.data()), n * sizeof(float));
    if (!f) throw std::runtime_error("gcnPolicy: short read on vector");
}

inline Eigen::VectorXf elu(const Eigen::VectorXf& x) {
    Eigen::VectorXf y(x.size());
    for (int i = 0; i < x.size(); i++)
        y(i) = (x(i) >= 0.0f) ? x(i) : (std::exp(x(i)) - 1.0f);
    return y;
}

// xorshift64*
inline uint64_t xorshift64s(uint64_t& s) {
    s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
    return s * 2685821657736338717ull;
}
inline float uniform01(uint64_t& s) {
    return static_cast<float>(xorshift64s(s) >> 40) / static_cast<float>(1u << 24);
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

GCNWeights& GCNWeights::instance() {
    static GCNWeights w;
    return w;
}

void GCNWeights::loadFromFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("gcnPolicy: cannot open " + path);

    // Read entire file for CRC verification
    f.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(f.tellg());
    f.seekg(0, std::ios::beg);

    if (fileSize < 8 + 20 + 4) throw std::runtime_error("gcnPolicy: file too small");

    std::vector<uint8_t> all(fileSize);
    f.read(reinterpret_cast<char*>(all.data()), fileSize);

    if (std::memcmp(all.data(), MAGIC, 4) != 0)
        throw std::runtime_error("gcnPolicy: bad magic");

    uint32_t crcStored;
    std::memcpy(&crcStored, all.data() + fileSize - 4, 4);
    uint32_t crcActual = crc32(all.data(), fileSize - 4);
    if (crcStored != crcActual)
        throw std::runtime_error("gcnPolicy: crc32 mismatch");

    // Re-open as a stream for sequential reads
    f.clear();
    f.seekg(4, std::ios::beg);  // skip magic

    uint32_t version, hidden, nodeDim, edgeDim, nActions, nLayers;
    f.read(reinterpret_cast<char*>(&version),  4);
    f.read(reinterpret_cast<char*>(&hidden),   4);
    f.read(reinterpret_cast<char*>(&nodeDim),  4);
    f.read(reinterpret_cast<char*>(&edgeDim),  4);
    f.read(reinterpret_cast<char*>(&nActions), 4);
    f.read(reinterpret_cast<char*>(&nLayers),  4);

    if (version != VERSION
        || hidden != HIDDEN
        || nodeDim != NODE_DIM
        || edgeDim != EDGE_DIM
        || nActions != N_ACTIONS
        || nLayers != N_LAYERS) {
        throw std::runtime_error("gcnPolicy: header mismatch with compiled constants");
    }

    int inDim = NODE_DIM;
    for (int i = 0; i < N_LAYERS; i++) {
        readMatrix(f, layers[i].msgW,  HIDDEN, inDim + EDGE_DIM);
        readMatrix(f, layers[i].selfW, HIDDEN, inDim);
        readVector(f, layers[i].bias,  HIDDEN);
        inDim = HIDDEN;
    }

    readMatrix(f, actor_fc1_W, HIDDEN,    HIDDEN);
    readVector(f, actor_fc1_b, HIDDEN);
    readMatrix(f, actor_fc2_W, N_ACTIONS, HIDDEN);
    readVector(f, actor_fc2_b, N_ACTIONS);

    loaded = true;
}

// ---------------------------------------------------------------------------
// Feature builders
// ---------------------------------------------------------------------------

Eigen::VectorXf edgeFeature(int direction) {
    Eigen::VectorXf e = Eigen::VectorXf::Zero(EDGE_DIM);
    if (direction >= 0 && direction < EDGE_DIM) e(direction) = 1.0f;
    return e;
}

Eigen::VectorXf buildNodeFeature(const Cell3DPosition& gridSize,
                                 const Cell3DPosition& pos,
                                 bool inTarget,
                                 uint8_t neighborMask6,
                                 int nMoves,
                                 bool isAP) {
    Eigen::VectorXf x(NODE_DIM);
    const float gx = static_cast<float>(std::max<int>(1, gridSize[0]));
    const float gy = static_cast<float>(std::max<int>(1, gridSize[1]));
    const float gz = static_cast<float>(std::max<int>(1, gridSize[2]));
    x(0) = static_cast<float>(pos[0]) / gx;
    x(1) = static_cast<float>(pos[1]) / gy;
    x(2) = static_cast<float>(pos[2]) / gz;
    x(3) = inTarget ? 1.0f : 0.0f;
    for (int d = 0; d < 6; d++)
        x(4 + d) = (neighborMask6 & (1u << d)) ? 1.0f : 0.0f;
    int capped = std::min(nMoves, N_ACTIONS - 1);
    x(10) = static_cast<float>(capped) / static_cast<float>(N_ACTIONS - 1);
    x(11) = isAP ? 1.0f : 0.0f;
    return x;
}

// ---------------------------------------------------------------------------
// Forward pass primitives
// ---------------------------------------------------------------------------

Eigen::VectorXf gcnEmitMsg(const GCNLayer& L,
                           const Eigen::VectorXf& selfH,
                           const Eigen::VectorXf& edgeFeat) {
    Eigen::VectorXf cat(selfH.size() + edgeFeat.size());
    cat.head(selfH.size())     = selfH;
    cat.tail(edgeFeat.size())  = edgeFeat;
    return L.msgW * cat;
}

Eigen::VectorXf gcnLayerForward(const GCNLayer& L,
                                const Eigen::VectorXf& selfH,
                                const std::vector<Eigen::VectorXf>& incomingMsgs) {
    Eigen::VectorXf agg = Eigen::VectorXf::Zero(L.bias.size());
    if (!incomingMsgs.empty()) {
        for (const auto& m : incomingMsgs) agg += m;
        agg /= static_cast<float>(incomingMsgs.size());
    }
    Eigen::VectorXf out = agg + L.selfW * selfH + L.bias;
    return elu(out);
}

Eigen::VectorXf actorLogits(const GCNWeights& W, const Eigen::VectorXf& finalH) {
    Eigen::VectorXf a1 = elu(W.actor_fc1_W * finalH + W.actor_fc1_b);
    return W.actor_fc2_W * a1 + W.actor_fc2_b;
}

int argmaxMasked(const Eigen::VectorXf& logits,
                 const std::array<bool, N_ACTIONS>& mask) {
    int   best = 0;
    float bv   = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < N_ACTIONS; i++) {
        if (!mask[i]) continue;
        if (logits(i) > bv) { bv = logits(i); best = i; }
    }
    return best;
}

int sampleMaskedSoftmax(const Eigen::VectorXf& logits,
                        const std::array<bool, N_ACTIONS>& mask,
                        uint64_t& rngState) {
    float maxLogit = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < N_ACTIONS; i++)
        if (mask[i] && logits(i) > maxLogit) maxLogit = logits(i);
    if (!std::isfinite(maxLogit)) return 0;  // no valid action -> stay

    float sum = 0.0f;
    std::array<float, N_ACTIONS> p{};
    for (int i = 0; i < N_ACTIONS; i++) {
        if (mask[i]) {
            p[i] = std::exp(logits(i) - maxLogit);
            sum += p[i];
        } else p[i] = 0.0f;
    }
    if (sum <= 0.0f) return 0;

    float u = uniform01(rngState) * sum;
    float c = 0.0f;
    for (int i = 0; i < N_ACTIONS; i++) {
        c += p[i];
        if (u <= c) return i;
    }
    return 0;
}

} // namespace GNNLocomotion
