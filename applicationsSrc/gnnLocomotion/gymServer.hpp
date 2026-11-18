#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "utils/tDefs.h"
#include "math/cell3DPosition.h"

namespace GNNLocomotion {

static constexpr int MAX_ACTIONS_PER_BLOCK = 13; // 0=stay, 1..12=move

struct BlockObs {
    bID id;
    Cell3DPosition pos;
    bool inTarget;
    int neighbors[6]; // neighbor blockId or -1 per direction (PlusX..MinusZ)
    std::vector<Cell3DPosition> moves; // valid destination positions
};

struct GymObs {
    int step = 0;
    int maxSteps = 500;
    int gridSize[3] = {0, 0, 0};
    double reward = 0.0;
    bool done = false;
    std::vector<BlockObs> blocks;
    std::vector<Cell3DPosition> target; // all target cell positions
};

class GymServer {
public:
    static GymServer* getInstance();

    void start(int port = 9999);
    void stop();

    // --- Called from scheduler thread (non-blocking) ---

    // Returns true if Python sent a step request
    bool hasStep();

    // Retrieves and clears the pending step request
    std::map<bID, int> getAndClearStep();

    // Sends an observation to Python (wakes socket thread)
    void setObs(const GymObs& obs);

    // --- Move tracking ---

    // Register callback invoked (in scheduler thread) when a notifyMoveDone() arrives
    void setMoveCompleteCallback(std::function<void()> cb);

    // Called by any block's onMotionEnd (scheduler thread)
    void notifyMoveDone();

private:
    GymServer() = default;
    ~GymServer();
    GymServer(const GymServer&) = delete;
    GymServer& operator=(const GymServer&) = delete;

    static GymServer* instance;

    int serverFd = -1;
    int clientFd = -1;
    std::thread socketThread;
    std::atomic<bool> running{false};

    std::mutex mtx;

    // socket→scheduler direction
    bool stepReady = false;
    std::map<bID, int> pendingActions;

    // scheduler→socket direction
    bool obsReady = false;
    GymObs pendingObs;
    std::condition_variable cv_obs;

    std::function<void()> moveCompleteCb;

    void socketLoop(int port);
    bool sendLine(const std::string& line);
    std::string recvLine();
    std::string buildObsJson(const GymObs& obs);
    std::map<bID, int> parseStepJson(const std::string& json);
};

} // namespace GNNLocomotion
