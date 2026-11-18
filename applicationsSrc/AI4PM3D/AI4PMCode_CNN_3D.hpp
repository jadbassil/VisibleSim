#ifndef AI4PMCode_CNN_3D_H_
#define AI4PMCode_CNN_3D_H_

/**
 * @file AI4PMCode_CNN_3D.hpp
 * 3D Shape Recognition via Height-Map Projection + 2D CNN
 *
 * Each z=0 block queries the lattice to count blocks stacked above it,
 * creating a normalized height value [0, 1]. This 16x16 "height map"
 * is then processed by the same 2D CNN architecture as AI4PM_CNN.
 *
 * Classes: 0=Cube, 1=Sphere, 2=Cylinder
 */

#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"
#include "robots/blinkyBlocks/blinkyBlocksWorld.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include "neuralNetworkData_cnn_3d.h"
#include <vector>
#include <cmath>
#include <cstring>

using namespace BlinkyBlocks;

static const int PIXEL_MSG_ID   = 2001;
static const int CONV1_MSG_ID   = 2002;
static const int CONV2_MSG_ID   = 2003;
static const int SOFTMAX_MSG_ID = 2004;

// ── Message Data Structures (identical to 2D CNN) ──

struct PixelMsgData {
    Cell3DPosition destPosition;
    uint8_t localX;
    uint8_t localY;
    double value;
    PixelMsgData() : localX(0), localY(0), value(0.0) {}
    PixelMsgData(const Cell3DPosition& dest, uint8_t lx, uint8_t ly, double v)
        : destPosition(dest), localX(lx), localY(ly), value(v) {}
};

struct Conv1MsgData {
    Cell3DPosition destPosition;
    uint8_t localX;
    uint8_t localY;
    double values[CONV1_FILTERS];
    Conv1MsgData() : localX(0), localY(0) { memset(values, 0, sizeof(values)); }
    Conv1MsgData(const Cell3DPosition& dest, uint8_t lx, uint8_t ly, const double* v)
        : destPosition(dest), localX(lx), localY(ly) {
        for (int i = 0; i < CONV1_FILTERS; i++) values[i] = v[i];
    }
};

struct Conv2MsgData {
    Cell3DPosition destPosition;
    uint8_t srcIndex;
    double values[CONV2_FILTERS];
    Conv2MsgData() : srcIndex(0) { memset(values, 0, sizeof(values)); }
    Conv2MsgData(const Cell3DPosition& dest, uint8_t idx, const double* v)
        : destPosition(dest), srcIndex(idx) {
        for (int i = 0; i < CONV2_FILTERS; i++) values[i] = v[i];
    }
};

struct SoftmaxMsgData {
    Cell3DPosition destPosition;
    double sumExp;
    SoftmaxMsgData() : sumExp(0.0) {}
    SoftmaxMsgData(const Cell3DPosition& dest, double s)
        : destPosition(dest), sumExp(s) {}
};

enum BlockRole {
    ROLE_INPUT_ONLY,
    ROLE_CONV1,
    ROLE_CONV2,
    ROLE_FC_OUTPUT,
    ROLE_SOFTMAX_SUM,
    ROLE_UPPER_BLOCK   // z > 0: does nothing
};

class AI4PMCode_CNN_3D : public BlinkyBlocksBlockCode {
private:
    BlinkyBlocksBlock *module = nullptr;
    BlockRole role;

    // Conv1 state
    double conv1_input[3][3];
    int conv1_received;
    double conv1_output[CONV1_FILTERS];

    // Conv2 state
    double conv2_input[3][3][CONV1_FILTERS];
    int conv2_received;
    double conv2_output[CONV2_FILTERS];

    // FC state
    double fc_accum[3];
    int fc_received;
    double fc_output;

    // Softmax
    double softmax_sum;
    int softmax_received;

    // Result
    double finalResult;

    // ── Grid position helpers (identical to 2D) ──
    bool isConv1Block(int x, int y) {
        return (x >= 1 && x <= 13 && y >= 1 && y <= 13 && x % 2 == 1 && y % 2 == 1);
    }

    bool isConv2Block(int x, int y) {
        return (x == 3 || x == 7 || x == 11) && (y == 3 || y == 7 || y == 11);
    }

    bool isFCBlock(int x, int y) {
        return (y == 0 && x < 3);
    }

    bool isSoftmaxBlock(int x, int y) {
        return (x == 0 && y == 0);
    }

    std::vector<Cell3DPosition> getConv1Targets(int px, int py) {
        std::vector<Cell3DPosition> targets;
        for (int fx = 1; fx <= 13; fx += 2) {
            for (int fy = 1; fy <= 13; fy += 2) {
                if (abs(px - fx) <= 1 && abs(py - fy) <= 1) {
                    targets.push_back(Cell3DPosition(fx, fy, 0));
                }
            }
        }
        return targets;
    }

    std::vector<Cell3DPosition> getConv2Targets(int cx, int cy) {
        std::vector<Cell3DPosition> targets;
        for (int fx : {3, 7, 11}) {
            for (int fy : {3, 7, 11}) {
                if (abs(cx - fx) <= 2 && abs(cy - fy) <= 2) {
                    targets.push_back(Cell3DPosition(fx, fy, 0));
                }
            }
        }
        return targets;
    }

    void getLocalCoords(int px, int py, int fx, int fy, uint8_t &lx, uint8_t &ly) {
        lx = (uint8_t)(px - (fx - 1));
        ly = (uint8_t)(py - (fy - 1));
    }

    void getConv2LocalCoords(int cx, int cy, int fx, int fy, uint8_t &lx, uint8_t &ly) {
        int ix = (cx - 1) / 2;
        int iy = (cy - 1) / 2;
        int fix = (fx - 1) / 2;
        int fiy = (fy - 1) / 2;
        lx = (uint8_t)(ix - (fix - 1));
        ly = (uint8_t)(iy - (fiy - 1));
    }

    P2PNetworkInterface* getNextInterface(int8_t dx, int8_t dy, int8_t dz) {
        std::vector<P2PNetworkInterface*> greedyItfs;
        if (dx != 0) {
            auto itf = module->getInterface(dx > 0 ? SCLattice::Direction::South : SCLattice::Direction::North);
            if (itf && itf->connectedInterface) greedyItfs.push_back(itf);
        }
        if (dy != 0) {
            auto itf = module->getInterface(dy > 0 ? SCLattice::Direction::East : SCLattice::Direction::West);
            if (itf && itf->connectedInterface) greedyItfs.push_back(itf);
        }
        if (dz != 0) {
            auto itf = module->getInterface(dz > 0 ? SCLattice::Direction::Top : SCLattice::Direction::Bottom);
            if (itf && itf->connectedInterface) greedyItfs.push_back(itf);
        }

        if (!greedyItfs.empty()) {
            return greedyItfs[rand() % greedyItfs.size()];
        }

        // Fallback: search all interfaces if greedy move is blocked (for complex hollow shapes)
        for (int i = 0; i < 6; i++) {
            auto itf = module->getInterface((SCLattice::Direction)i);
            if (itf && itf->connectedInterface) return itf;
        }
        return nullptr;
    }

    template<typename MsgData>
    void routeMessage(int msgId, const MsgData& data, const Cell3DPosition& dest) {
        int dx = dest[0] - module->position[0];
        int dy = dest[1] - module->position[1];
        int dz = dest[2] - module->position[2];
        
        if (dx == 0 && dy == 0 && dz == 0) return;

        P2PNetworkInterface *nextItf = getNextInterface((int8_t)dx, (int8_t)dy, (int8_t)dz);
        if (nextItf) {
            uint64_t commTime = uint64_t(89.350 * sizeof(MsgData) + 1516.0);
            sendMessage(new MessageOf<MsgData>(msgId, data), nextItf, commTime, 0);
        }
    }

    /**
     * @brief Query the lattice to count blocks stacked above this (x, y) column.
     *        Returns normalized height in [0, 1].
     */
    double computeHeightMapValue(int mx, int my);

public:
    AI4PMCode_CNN_3D(BlinkyBlocksBlock *host);
    ~AI4PMCode_CNN_3D() {};

    void startup() override;
    void parseUserBlockElements(TiXmlElement *config) override;
    void onTap(int face) override;
    void onEndOfSimulation() override;
    string onInterfaceDraw() override;

    // Message handlers
    void handlePixelMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleConv1Msg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleConv2Msg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleSoftmaxMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);

    // Computation
    void computeConv1();
    void sendConv1Results();
    void computeConv2();
    void sendConv2Results();
    void computeFC(uint8_t srcIndex, const double* values);
    void finishFC();
    void finishSoftmax(double sumExp);

    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new AI4PMCode_CNN_3D((BlinkyBlocksBlock*)host));
    }
};

#endif /* AI4PMCode_CNN_3D_H_ */
