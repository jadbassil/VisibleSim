#include "gymServer.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>

namespace GNNShapeReconfiguration {

GymServer* GymServer::instance = nullptr;

GymServer* GymServer::getInstance() {
    if (!instance) instance = new GymServer();
    return instance;
}

GymServer::~GymServer() {
    stop();
}

void GymServer::start(int port) {
    running = true;
    socketThread = std::thread(&GymServer::socketLoop, this, port);
}

void GymServer::stop() {
    running = false;
    cv_obs.notify_all();
    if (clientFd >= 0) { ::close(clientFd); clientFd = -1; }
    if (serverFd >= 0) { ::close(serverFd); serverFd = -1; }
    if (socketThread.joinable()) socketThread.join();
}

bool GymServer::hasStep() {
    std::unique_lock<std::mutex> lk(mtx);
    return stepReady;
}

std::map<bID, int> GymServer::getAndClearStep() {
    std::unique_lock<std::mutex> lk(mtx);
    stepReady = false;
    return pendingActions;
}

void GymServer::setObs(const GymObs& obs) {
    {
        std::unique_lock<std::mutex> lk(mtx);
        pendingObs = obs;
        obsReady = true;
    }
    cv_obs.notify_one();
}

void GymServer::setMoveCompleteCallback(std::function<void()> cb) {
    moveCompleteCb = std::move(cb);
}

void GymServer::notifyMoveDone() {
    if (moveCompleteCb) moveCompleteCb();
}

// ---------------------------------------------------------------------------
// Socket thread
// ---------------------------------------------------------------------------

void GymServer::socketLoop(int port) {
    serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "[GymServer] socket() failed: " << strerror(errno) << "\n";
        return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[GymServer] bind() failed on port " << port
                  << ": " << strerror(errno) << "\n";
        return;
    }
    ::listen(serverFd, 1);
    std::cout << "[GymServer] Listening on port " << port << "\n";

    while (running) {
        clientFd = ::accept(serverFd, nullptr, nullptr);
        if (clientFd < 0) break;
        std::cout << "[GymServer] Client connected\n";

        // Wait for block code to set initial observation, then send it
        {
            std::unique_lock<std::mutex> lk(mtx);
            cv_obs.wait(lk, [this] { return obsReady || !running; });
            if (!running) { lk.unlock(); break; }
            std::string json = buildObsJson(pendingObs);
            obsReady = false;
            lk.unlock();
            if (!sendLine(json)) goto client_done;
        }

        // Step loop
        while (running) {
            std::string line = recvLine();
            if (line.empty()) break;

            auto actions = parseStepJson(line);
            {
                std::unique_lock<std::mutex> lk(mtx);
                pendingActions = std::move(actions);
                stepReady = true;
            }

            // Wait for the block code to finish the step and post an obs
            {
                std::unique_lock<std::mutex> lk(mtx);
                cv_obs.wait(lk, [this] { return obsReady || !running; });
                if (!running) { lk.unlock(); break; }
                std::string json = buildObsJson(pendingObs);
                bool done = pendingObs.done;
                obsReady = false;
                lk.unlock();
                if (!sendLine(json)) goto client_done;
                if (done) goto client_done; // episode over; Python will reconnect
            }
        }

    client_done:
        ::close(clientFd);
        clientFd = -1;
        std::cout << "[GymServer] Client disconnected\n";
    }

    if (serverFd >= 0) { ::close(serverFd); serverFd = -1; }
}

bool GymServer::sendLine(const std::string& line) {
    if (clientFd < 0) return false;
    std::string msg = line + "\n";
    ssize_t sent = ::send(clientFd, msg.c_str(), msg.size(), 0);
    return sent == static_cast<ssize_t>(msg.size());
}

std::string GymServer::recvLine() {
    std::string result;
    char c;
    while (true) {
        ssize_t n = ::recv(clientFd, &c, 1, 0);
        if (n <= 0) return "";
        if (c == '\n') break;
        result += c;
    }
    return result;
}

// ---------------------------------------------------------------------------
// JSON serialization
// ---------------------------------------------------------------------------

std::string GymServer::buildObsJson(const GymObs& obs) {
    std::ostringstream ss;
    ss << "{\"type\":\"obs\""
       << ",\"step\":" << obs.step
       << ",\"max_steps\":" << obs.maxSteps
       << ",\"reward\":" << obs.reward
       << ",\"done\":" << (obs.done ? "true" : "false")
       << ",\"grid_size\":[" << obs.gridSize[0] << ","
                             << obs.gridSize[1] << ","
                             << obs.gridSize[2] << "]"
       << ",\"blocks\":[";

    for (size_t i = 0; i < obs.blocks.size(); i++) {
        const auto& b = obs.blocks[i];
        if (i > 0) ss << ",";
        ss << "{\"id\":" << b.id
           << ",\"pos\":[" << b.pos[0] << "," << b.pos[1] << "," << b.pos[2] << "]"
           << ",\"in_target\":" << (b.inTarget ? "true" : "false")
           << ",\"neighbors\":[";
        for (int d = 0; d < 6; d++) {
            if (d > 0) ss << ",";
            ss << b.neighbors[d];
        }
        ss << "],\"moves\":[";
        for (size_t m = 0; m < b.moves.size(); m++) {
            if (m > 0) ss << ",";
            ss << "[" << b.moves[m][0] << "," << b.moves[m][1] << "," << b.moves[m][2] << "]";
        }
        ss << "]}";
    }

    ss << "],\"target\":[";
    for (size_t i = 0; i < obs.target.size(); i++) {
        if (i > 0) ss << ",";
        ss << "[" << obs.target[i][0] << "," << obs.target[i][1] << "," << obs.target[i][2] << "]";
    }
    ss << "]}";

    return ss.str();
}

// ---------------------------------------------------------------------------
// JSON parsing  — expected format: {"type":"step","actions":{"1":0,"2":2,...}}
// ---------------------------------------------------------------------------

std::map<bID, int> GymServer::parseStepJson(const std::string& json) {
    std::map<bID, int> actions;

    auto aPos = json.find("\"actions\"");
    if (aPos == std::string::npos) return actions;

    auto braceOpen = json.find('{', aPos + 9);
    if (braceOpen == std::string::npos) return actions;

    auto braceClose = json.find('}', braceOpen + 1);
    if (braceClose == std::string::npos) return actions;

    size_t pos = braceOpen + 1;
    while (pos < braceClose) {
        // Skip commas and spaces
        while (pos < braceClose && (json[pos] == ',' || json[pos] == ' ')) ++pos;
        if (pos >= braceClose) break;

        // Key must be a quoted integer string
        if (json[pos] != '"') break;
        auto keyEnd = json.find('"', pos + 1);
        if (keyEnd == std::string::npos || keyEnd >= braceClose) break;
        std::string keyStr = json.substr(pos + 1, keyEnd - pos - 1);

        auto colon = json.find(':', keyEnd + 1);
        if (colon == std::string::npos || colon >= braceClose) break;

        pos = colon + 1;
        while (pos < braceClose && json[pos] == ' ') ++pos;

        auto valEnd = json.find_first_of(",}", pos);
        if (valEnd == std::string::npos) valEnd = braceClose;
        std::string valStr = json.substr(pos, valEnd - pos);

        try {
            auto id = static_cast<bID>(std::stoul(keyStr));
            int action = std::stoi(valStr);
            actions[id] = action;
        } catch (...) {}

        pos = valEnd;
    }

    return actions;
}

} // namespace GNNShapeReconfiguration
