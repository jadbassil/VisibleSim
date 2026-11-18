/**
 * @file AI4PMCode_CNN_3D.cpp
 * 3D Shape Recognition via Height-Map Projection + 2D CNN
 *
 * KEY DIFFERENCE from 2D version:
 * - Blocks at z > 0 do NOTHING (they don't participate in CNN)
 * - Blocks at z = 0 query the lattice to count how many blocks exist
 *   above them in the Z column, then normalize: height / gridZ
 * - This continuous [0,1] value replaces the binary pixel input
 * - The entire CNN pipeline (Conv1 -> Conv2 -> FC -> Softmax) is identical
 */

#include "AI4PMCode_CNN_3D.hpp"
#include <iomanip>
#include <sstream>

double cnn3DResults[3] = {-1, -1, -1};
int cnn3DNbMessages = 0;

AI4PMCode_CNN_3D::AI4PMCode_CNN_3D(BlinkyBlocksBlock *host)
    : BlinkyBlocksBlockCode(host), module(host) {
    if (not host) return;

    addMessageEventFunc2(PIXEL_MSG_ID,
        std::bind(&AI4PMCode_CNN_3D::handlePixelMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CONV1_MSG_ID,
        std::bind(&AI4PMCode_CNN_3D::handleConv1Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CONV2_MSG_ID,
        std::bind(&AI4PMCode_CNN_3D::handleConv2Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(SOFTMAX_MSG_ID,
        std::bind(&AI4PMCode_CNN_3D::handleSoftmaxMsg, this,
                  std::placeholders::_1, std::placeholders::_2));

    conv1_received = 0;
    conv2_received = 0;
    fc_received = 0;
    fc_output = 0.0;
    softmax_sum = 0.0;
    softmax_received = 0;
    finalResult = -1.0;
    memset(conv1_input, 0, sizeof(conv1_input));
    memset(conv1_output, 0, sizeof(conv1_output));
    memset(conv2_input, 0, sizeof(conv2_input));
    memset(conv2_output, 0, sizeof(conv2_output));
    memset(fc_accum, 0, sizeof(fc_accum));
}

/**
 * @brief Query the lattice to count blocks in the Z-column at (mx, my).
 *        Returns normalized value: count / gridZ  (range [0, 1])
 *
 * This uses direct lattice query, zero extra messages!
 */
double AI4PMCode_CNN_3D::computeHeightMapValue(int mx, int my) {
    auto* lattice = BaseSimulator::getWorld()->lattice;
    int gridZ = lattice->gridSize[2];

    int count = 0;
    for (int z = 0; z < gridZ; z++) {
        Cell3DPosition pos(mx, my, z);
        if (lattice->cellHasBlock(pos)) {
            // Only count "shape" blocks (white), not background (grey)
            auto* bb = (BlinkyBlocksBlock*)lattice->getBlock(pos);
            if (bb && bb->color[0] == 255) {
                count++;
            }
        }
    }

    return (double)count / (double)gridZ;
}

// ── Startup ──
void AI4PMCode_CNN_3D::startup() {
    int mx = module->position[0];
    int my = module->position[1];
    int mz = module->position[2];

    // ═══════════════════════════════════════════════════════════
    // CRITICAL: Blocks above z=0 do NOT participate in the CNN.
    // They exist in the 3D world but all computation happens at z=0.
    // ═══════════════════════════════════════════════════════════
    if (mz > 0) {
        role = ROLE_UPPER_BLOCK;
        return;  // Do nothing
    }

    // ── z=0 blocks: determine CNN role ──
    if (isConv2Block(mx, my)) {
        role = ROLE_CONV2;
    } else if (isConv1Block(mx, my)) {
        role = ROLE_CONV1;
    } else if (isFCBlock(mx, my)) {
        role = ROLE_FC_OUTPUT;
    } else {
        role = ROLE_INPUT_ONLY;
    }

    double heightVal = computeHeightMapValue(mx, my);

    // ── VISUAL HEAT MAP ──
    // Color floor blocks based on height: 0 = Grey, >0 = Blue to Cyan gradient
    if (heightVal > 0) {
        // Deep Blue (0, 0.2, 0.5) for low height -> Vibrant Cyan (0, 1.0, 1.0) for high height
        module->setColor(Color(0.0f, (float)heightVal, 1.0f)); 
    } else {
        module->setColor(Color(0.4f, 0.4f, 0.4f)); // Keep empty floor dark grey
    }

    // ── DEBUG COLORS (Layer Visualization) ──
    // These colors overwrite the heat map for specific filter blocks so you can see the brain
    if (role == ROLE_CONV2) {
        module->setColor(Color(1.0f, 0.5f, 0.0f)); // Orange for Conv2 Regional Hubs
    } else if (role == ROLE_CONV1) {
        module->setColor(Color(0.0f, 1.0f, 0.0f)); // Green for Conv1 Local Filters
    } else if (role == ROLE_FC_OUTPUT) {
        module->setColor(Color(1.0f, 0.0f, 0.0f)); // Red for the 3 Final Decision Blocks
    }

    // Quantize using the model's input scale/zero_point
    double inputVal = (double)((int)(heightVal * 255.0 - 128) - input_zero_point) * input_scale;

    console << "height(" << mx << "," << my << ") = " << heightVal << "\n";

    // ── Route pixel to Conv1 filter blocks (same as 2D) ──
    std::vector<Cell3DPosition> conv1Targets = getConv1Targets(mx, my);

    for (const auto& target : conv1Targets) {
        int fx = target[0];
        int fy = target[1];

        if (mx == fx && my == fy) {
            uint8_t lx, ly;
            getLocalCoords(mx, my, fx, fy, lx, ly);
            conv1_input[ly][lx] = inputVal;
            conv1_received++;

            if (conv1_received >= 9) {
                computeConv1();
            }
        } else {
            uint8_t lx, ly;
            getLocalCoords(mx, my, fx, fy, lx, ly);
            PixelMsgData data(target, lx, ly, inputVal);
            routeMessage(PIXEL_MSG_ID, data, target);
        }
    }
}


// ── Pixel message handler ──
void AI4PMCode_CNN_3D::handlePixelMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender) {
    MessageOf<PixelMsgData>* msg = static_cast<MessageOf<PixelMsgData>*>(_msg.get());
    PixelMsgData data = *msg->getData();
    cnn3DNbMessages++;

    if (data.destPosition == module->position) {
        if (role == ROLE_UPPER_BLOCK) return; // Safety: consume and ignore
        
        conv1_input[data.localY][data.localX] = data.value;
        conv1_received++;
        if (conv1_received >= 9) {
            computeConv1();
        }
    } else {
        routeMessage(PIXEL_MSG_ID, data, data.destPosition);
    }
}


// ── Conv1 computation (identical to 2D) ──
void AI4PMCode_CNN_3D::computeConv1() {
    if (role == ROLE_UPPER_BLOCK) return; // Logic safety

    for (int f = 0; f < CONV1_FILTERS; f++) {
        double sum = 0.0;
        for (int ky = 0; ky < 3; ky++) {
            for (int kx = 0; kx < 3; kx++) {
                sum += conv1_input[ky][kx] * (double)conv1_kernel[f][0][ky][kx] * conv1_weight_scales[f];
            }
        }
        sum += conv1_bias[f];
        conv1_output[f] = (sum > 0.0) ? sum : 0.0;
    }
    sendConv1Results();
}


void AI4PMCode_CNN_3D::sendConv1Results() {
    int cx = module->position[0];
    int cy = module->position[1];

    std::vector<Cell3DPosition> conv2Targets = getConv2Targets(cx, cy);

    for (const auto& target : conv2Targets) {
        int fx = target[0];
        int fy = target[1];

        if (cx == fx && cy == fy) {
            uint8_t lx, ly;
            getConv2LocalCoords(cx, cy, fx, fy, lx, ly);
            for (int ch = 0; ch < CONV1_FILTERS; ch++) {
                conv2_input[ly][lx][ch] = conv1_output[ch];
            }
            conv2_received++;
            if (conv2_received >= 9) {
                computeConv2();
            }
        } else {
            uint8_t lx, ly;
            getConv2LocalCoords(cx, cy, fx, fy, lx, ly);
            Conv1MsgData data(target, lx, ly, conv1_output);
            routeMessage(CONV1_MSG_ID, data, target);
        }
    }
}


// ── Conv1 output handler ──
void AI4PMCode_CNN_3D::handleConv1Msg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender) {
    MessageOf<Conv1MsgData>* msg = static_cast<MessageOf<Conv1MsgData>*>(_msg.get());
    Conv1MsgData data = *msg->getData();
    cnn3DNbMessages++;

    if (data.destPosition == module->position) {
        if (role == ROLE_UPPER_BLOCK) return; // Safety

        for (int ch = 0; ch < CONV1_FILTERS; ch++) {
            conv2_input[data.localY][data.localX][ch] = data.values[ch];
        }
        conv2_received++;
        if (conv2_received >= 9) {
            computeConv2();
        }
    } else {
        routeMessage(CONV1_MSG_ID, data, data.destPosition);
    }
}


// ── Conv2 computation (identical to 2D) ──
void AI4PMCode_CNN_3D::computeConv2() {
    if (role == ROLE_UPPER_BLOCK) return; // Safety

    for (int f = 0; f < CONV2_FILTERS; f++) {
        double sum = 0.0;
        for (int ic = 0; ic < CONV1_FILTERS; ic++) {
            for (int ky = 0; ky < 3; ky++) {
                for (int kx = 0; kx < 3; kx++) {
                    sum += conv2_input[ky][kx][ic] * (double)conv2_kernel[f][ic][ky][kx] * conv2_weight_scales[f];
                }
            }
        }
        sum += conv2_bias[f];
        conv2_output[f] = (sum > 0.0) ? sum : 0.0;
    }
    sendConv2Results();
}

void AI4PMCode_CNN_3D::sendConv2Results() {
    int cx = module->position[0];
    int cy = module->position[1];

    int ix = (cx - 3) / 4;
    int iy = (cy - 3) / 4;
    uint8_t flatIndex = (uint8_t)(iy * 3 + ix);

    for (int fc = 0; fc < NUM_CLASSES; fc++) {
        Cell3DPosition fcPos;
        fcPos.set(fc, 0, 0);

        if (module->position == fcPos) {
            computeFC(flatIndex, conv2_output);
        } else {
            Conv2MsgData data(fcPos, flatIndex, conv2_output);
            routeMessage(CONV2_MSG_ID, data, fcPos);
        }
    }
}


// ── Conv2 output handler ──
void AI4PMCode_CNN_3D::handleConv2Msg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender) {
    MessageOf<Conv2MsgData>* msg = static_cast<MessageOf<Conv2MsgData>*>(_msg.get());
    Conv2MsgData data = *msg->getData();
    cnn3DNbMessages++;

    if (data.destPosition == module->position) {
        if (role == ROLE_UPPER_BLOCK) return; // Safety
        computeFC(data.srcIndex, data.values);
    } else {
        routeMessage(CONV2_MSG_ID, data, data.destPosition);
    }
}


// ── FC computation (identical to 2D) ──
void AI4PMCode_CNN_3D::computeFC(uint8_t srcIndex, const double* values) {
    if (role == ROLE_UPPER_BLOCK) return; // Safety

    int classId = module->position[0];

    for (int ch = 0; ch < CONV2_FILTERS; ch++) {
        int fcIdx = srcIndex * CONV2_FILTERS + ch;
        fc_accum[classId] += values[ch] * (double)fc_weight[fcIdx][classId] * fc_weight_scale;
    }

    fc_received++;

    if (fc_received >= 9) {
        finishFC();
    }
}

void AI4PMCode_CNN_3D::finishFC() {
    int classId = module->position[0];
    double val = fc_accum[classId] + fc_bias[classId];
    fc_output = exp(val);

    Cell3DPosition sumPos;
    sumPos.set(0, 0, 0);

    if (module->position == sumPos) {
        softmax_sum += fc_output;
        softmax_received++;

        if (softmax_received >= 3) {
            for (int i = 0; i < 3; i++) {
                Cell3DPosition fcPos;
                fcPos.set(i, 0, 0);
                if (i == 0) {
                    finishSoftmax(softmax_sum);
                } else {
                    SoftmaxMsgData data(fcPos, softmax_sum);
                    routeMessage(SOFTMAX_MSG_ID, data, fcPos);
                }
            }
        }
    } else {
        SoftmaxMsgData data(sumPos, fc_output);
        routeMessage(SOFTMAX_MSG_ID, data, sumPos);
    }
}


// ── Softmax handler ──
void AI4PMCode_CNN_3D::handleSoftmaxMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender) {
    MessageOf<SoftmaxMsgData>* msg = static_cast<MessageOf<SoftmaxMsgData>*>(_msg.get());
    SoftmaxMsgData data = *msg->getData();
    cnn3DNbMessages++;

    if (data.destPosition == module->position) {
        if (role == ROLE_UPPER_BLOCK) return; // Safety

        int mx = module->position[0];
        if (mx == 0 && softmax_received < 3) {
            softmax_sum += data.sumExp;
            softmax_received++;

            if (softmax_received >= 3) {
                for (int i = 0; i < 3; i++) {
                    Cell3DPosition fcPos;
                    fcPos.set(i, 0, 0);
                    if (i == 0) {
                        finishSoftmax(softmax_sum);
                    } else {
                        SoftmaxMsgData outData(fcPos, softmax_sum);
                        routeMessage(SOFTMAX_MSG_ID, outData, fcPos);
                    }
                }
            }
        } else {
            finishSoftmax(data.sumExp);
        }
    } else {
        routeMessage(SOFTMAX_MSG_ID, data, data.destPosition);
    }
}

void AI4PMCode_CNN_3D::finishSoftmax(double sumExp) {
    int classId = module->position[0];
    double probability = fc_output / sumExp;
    cnn3DResults[classId] = probability;

    if (probability > 0.8) {
        module->setColor(module->blockId - 1);
    } else {
        module->setColor(Color(0.5f, 0.5f, 0.5f));
    }

    cout << "Result(" << classId << ") = " << probability * 100.0 << "%" << endl;
}


void AI4PMCode_CNN_3D::parseUserBlockElements(TiXmlElement *config) {}

void AI4PMCode_CNN_3D::onTap(int face) {
    cout << "Block tapped: " << getId() << endl;
}

void AI4PMCode_CNN_3D::onEndOfSimulation() {
    cout << "CNN 3D Messages: " << cnn3DNbMessages << endl;
}

string AI4PMCode_CNN_3D::onInterfaceDraw() {
    string res("3D CNN Height-Map Recognition");
    if (cnn3DResults[0] > 0) {
        stringstream stream;
        stream << "Cube: " << std::fixed << std::setprecision(2) << cnn3DResults[0]*100.0 << "%\n";
        stream << "Sphere: " << std::fixed << std::setprecision(2) << cnn3DResults[1]*100.0 << "%\n";
        stream << "Cylinder: " << std::fixed << std::setprecision(2) << cnn3DResults[2]*100.0 << "%";
        res = stream.str();
    }
    return res;
}
