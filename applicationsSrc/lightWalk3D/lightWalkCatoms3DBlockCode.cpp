#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "robots/catoms3D/catoms3DWorld.h"
#include "events/scheduler.h"
#include "events/events.h"
#include "utils/trace.h"
#include "utils/tDefs.h"

#include "motion/teleportationEvents.h"
#include "robots/catoms3D/catoms3DRotationEvents.h"
#include "robots/catoms3D/catoms3DMotionEngine.h"

#include "lightWalkCatoms3DBlockCode.hpp"

namespace
{
    const int DP_GOAL_REWARD = 100000;
    const int DP_STEP_COST = 100;
    const int DP_HEIGHT_COST = 5;

    bool readCell3DAttribute(TiXmlElement *element,
                             const char *name,
                             Cell3DPosition &pos)
    {
        if (!element or !name)
            return false;

        std::string value;
        if (element->QueryStringAttribute(name, &value) != TIXML_SUCCESS)
            return false;

        std::replace(value.begin(), value.end(), ',', ' ');
        std::stringstream parser(value);
        int x = 0;
        int y = 0;
        int z = 0;
        if (!(parser >> x >> y >> z))
            return false;

        pos = Cell3DPosition(x, y, z);
        return true;
    }

    bool parseCell3DText(const std::string &text, Cell3DPosition &pos)
    {
        std::string value = text;
        std::replace(value.begin(), value.end(), ',', ' ');
        std::stringstream parser(value);
        int x = 0;
        int y = 0;
        int z = 0;
        if (!(parser >> x >> y >> z))
            return false;

        pos = Cell3DPosition(x, y, z);
        return true;
    }

    bool parseGoalBoxText(const std::string &text,
                          Cell3DPosition &minCorner,
                          Cell3DPosition &maxCorner)
    {
        const size_t separator = text.find(':');
        if (separator == std::string::npos)
            return false;

        return parseCell3DText(text.substr(0, separator), minCorner)
            and parseCell3DText(text.substr(separator + 1), maxCorner);
    }

    bool readDpGoalAttributes(TiXmlElement *element)
    {
        if (!element)
            return false;

        bool found = false;
        Cell3DPosition minCorner;
        Cell3DPosition maxCorner;
        const bool hasMin =
            readCell3DAttribute(element, "goalMin", minCorner)
            or readCell3DAttribute(element, "dpGoalMin", minCorner)
            or readCell3DAttribute(element, "targetMin", minCorner);
        const bool hasMax =
            readCell3DAttribute(element, "goalMax", maxCorner)
            or readCell3DAttribute(element, "dpGoalMax", maxCorner)
            or readCell3DAttribute(element, "targetMax", maxCorner);
        if (!LightWalkCatoms3DBlockCode::dpGoalBoxSetFromCommandLine
            and hasMin and hasMax)
        {
            LightWalkCatoms3DBlockCode::setDpGoalBox(minCorner, maxCorner);
            found = true;
        }

        double fillPercent = 0.0;
        if (!LightWalkCatoms3DBlockCode::dpGoalFillPercentSetFromCommandLine
            and (element->QueryDoubleAttribute("goalFillPercent", &fillPercent) == TIXML_SUCCESS
                 or element->QueryDoubleAttribute("dpGoalFillPercent", &fillPercent) == TIXML_SUCCESS
                 or element->QueryDoubleAttribute("nPercent", &fillPercent) == TIXML_SUCCESS))
        {
            LightWalkCatoms3DBlockCode::setDpGoalFillPercent(fillPercent);
            found = true;
        }

        return found;
    }

    bool readMovingRadiusAttribute(TiXmlElement *element, int &radius)
    {
        if (!element)
            return false;
        return element->QueryIntAttribute("movingCatomRadius", &radius) == TIXML_SUCCESS or element->QueryIntAttribute("freeCatomRadius", &radius) == TIXML_SUCCESS or element->QueryIntAttribute("radius", &radius) == TIXML_SUCCESS;
    }

    bool readMetricsAttributes(TiXmlElement *element)
    {
        if (!element)
            return false;

        bool found = false;
        bool enabled = true;
        if (element->QueryBoolAttribute("metricsEnabled", &enabled) == TIXML_SUCCESS)
        {
            LightWalkCatoms3DBlockCode::setMetricsEnabled(enabled);
            found = true;
        }

        std::string path;
        if (!LightWalkCatoms3DBlockCode::metricsCsvPathSetFromCommandLine and element->QueryStringAttribute("metricsCsv", &path) == TIXML_SUCCESS)
        {
            LightWalkCatoms3DBlockCode::setMetricsCsvPath(path);
            found = true;
        }

        int period = 0;
        if (!LightWalkCatoms3DBlockCode::metricsSamplePeriodSetFromCommandLine and element->QueryIntAttribute("metricsSamplePeriodUs", &period) == TIXML_SUCCESS)
        {
            LightWalkCatoms3DBlockCode::setMetricsSamplePeriod(
                static_cast<Time>(std::max(1, period)));
            found = true;
        }

        return found;
    }

    int intervalFreeCells(int minA, int maxA, int minB, int maxB)
    {
        if (maxA < minB)
            return minB - maxA - 1;
        if (maxB < minA)
            return minA - maxB - 1;
        return 0;
    }

    bool movementReservationsConflict(const Cell3DPosition &mobileA,const Cell3DPosition &targetA,const Cell3DPosition &mobileB,const Cell3DPosition &targetB,int radius)
    {
        if (radius <= 0)
            return false;

        const int minLineA = std::min(mobileA[1], targetA[1]);
        const int maxLineA = std::max(mobileA[1], targetA[1]);
        const int minLineB = std::min(mobileB[1], targetB[1]);
        const int maxLineB = std::max(mobileB[1], targetB[1]);
        if (intervalFreeCells(minLineA, maxLineA, minLineB, maxLineB) >= 1)
        {
            return false;
        }

        const int minZA = std::min(mobileA[2], targetA[2]);
        const int maxZA = std::max(mobileA[2], targetA[2]);
        const int minZB = std::min(mobileB[2], targetB[2]);
        const int maxZB = std::max(mobileB[2], targetB[2]);
        if (intervalFreeCells(minZA, maxZA, minZB, maxZB) >= 1)
        {
            return false;
        }

        const int minXA = std::min(mobileA[0], targetA[0]);
        const int maxXA = std::max(mobileA[0], targetA[0]);
        const int minXB = std::min(mobileB[0], targetB[0]);
        const int maxXB = std::max(mobileB[0], targetB[0]);
        return intervalFreeCells(minXA, maxXA, minXB, maxXB) < radius;
    }

    struct ConfigurationGraphMetrics
    {
        int nodes = 0;
        int edges = 0;
        int connectedComponents = 0;
        int diameterCells = 0;
        int bottleneckCandidates = 0;
    };

    ConfigurationGraphMetrics collectConfigurationGraphMetrics()
    {
        ConfigurationGraphMetrics metrics;

        std::map<bID, Catoms3DBlock *> modules;
        std::map<bID, std::set<bID>> adjacency;
        auto &blocks = BaseSimulator::getWorld()->getMap();

        for (const auto &entry : blocks)
        {
            auto *bb = static_cast<Catoms3DBlock *>(entry.second);
            if (!bb)
                continue;
            modules[bb->blockId] = bb;
            adjacency[bb->blockId];
        }

        metrics.nodes = static_cast<int>(modules.size());
        for (const auto &entry : modules)
        {
            Catoms3DBlock *bb = entry.second;
            for (int i = 0; i < bb->getNbInterfaces(); ++i)
            {
                P2PNetworkInterface *p2p = bb->getInterface(i);
                if (!p2p or !p2p->isConnected())
                    continue;

                const bID otherId = p2p->getConnectedBlockId();
                if (!modules.count(otherId))
                    continue;

                adjacency[bb->blockId].insert(otherId);
                adjacency[otherId].insert(bb->blockId);
                if (bb->blockId < otherId)
                {
                    metrics.edges++;
                }
            }
        }

        std::set<bID> visited;
        for (const auto &entry : modules)
        {
            if (!visited.insert(entry.first).second)
                continue;

            metrics.connectedComponents++;
            std::vector<bID> stack{entry.first};
            while (!stack.empty())
            {
                const bID current = stack.back();
                stack.pop_back();

                for (const bID neighbor : adjacency[current])
                {
                    if (visited.insert(neighbor).second)
                    {
                        stack.push_back(neighbor);
                    }
                }
            }
        }

        for (auto lhs = modules.begin(); lhs != modules.end(); ++lhs)
        {
            auto rhs = lhs;
            ++rhs;
            for (; rhs != modules.end(); ++rhs)
            {
                metrics.diameterCells =
                    std::max(metrics.diameterCells,
                             lhs->second->position.dist_taxi(rhs->second->position));
            }
        }

        std::map<bID, int> discovery;
        std::map<bID, int> low;
        std::map<bID, bID> parent;
        std::set<bID> articulationPoints;
        int discoveryTime = 0;

        std::function<void(bID)> dfs = [&](bID current)
        {
            discovery[current] = low[current] = ++discoveryTime;
            int children = 0;

            for (const bID neighbor : adjacency[current])
            {
                if (!discovery.count(neighbor))
                {
                    parent[neighbor] = current;
                    children++;
                    dfs(neighbor);

                    low[current] = std::min(low[current], low[neighbor]);
                    if (!parent.count(current) and children > 1)
                    {
                        articulationPoints.insert(current);
                    }
                    if (parent.count(current) and low[neighbor] >= discovery[current])
                    {
                        articulationPoints.insert(current);
                    }
                }
                else if (!parent.count(current) or neighbor != parent[current])
                {
                    low[current] = std::min(low[current], discovery[neighbor]);
                }
            }
        };

        for (const auto &entry : modules)
        {
            if (!discovery.count(entry.first))
            {
                dfs(entry.first);
            }
        }

        metrics.bottleneckCandidates = static_cast<int>(articulationPoints.size());
        return metrics;
    }

    struct CellSetGraphMetrics
    {
        int nodes = 0;
        int edges = 0;
        int connectedComponents = 0;
        int diameterCells = 0;
    };

    CellSetGraphMetrics collectCellSetGraphMetrics(const std::vector<Cell3DPosition> &cells)
    {
        CellSetGraphMetrics metrics;
        metrics.nodes = static_cast<int>(cells.size());
        if (cells.empty())
            return metrics;

        Lattice *lattice = BaseSimulator::getWorld()->lattice;
        std::vector<std::set<int>> adjacency(cells.size());

        for (size_t i = 0; i < cells.size(); ++i)
        {
            for (size_t j = i + 1; j < cells.size(); ++j)
            {
                metrics.diameterCells =
                    std::max(metrics.diameterCells, cells[i].dist_taxi(cells[j]));
                if (lattice and lattice->cellsAreAdjacent(cells[i], cells[j]))
                {
                    adjacency[i].insert(static_cast<int>(j));
                    adjacency[j].insert(static_cast<int>(i));
                    metrics.edges++;
                }
            }
        }

        std::vector<bool> visited(cells.size(), false);
        for (size_t i = 0; i < cells.size(); ++i)
        {
            if (visited[i])
                continue;

            metrics.connectedComponents++;
            std::vector<int> stack{static_cast<int>(i)};
            visited[i] = true;
            while (!stack.empty())
            {
                const int current = stack.back();
                stack.pop_back();

                for (const int neighbor : adjacency[current])
                {
                    if (!visited[neighbor])
                    {
                        visited[neighbor] = true;
                        stack.push_back(neighbor);
                    }
                }
            }
        }

        return metrics;
    }

    std::vector<Cell3DPosition> collectCurrentCells()
    {
        std::vector<Cell3DPosition> cells;
        auto &blocks = BaseSimulator::getWorld()->getMap();
        cells.reserve(blocks.size());
        for (const auto &entry : blocks)
        {
            auto *bb = static_cast<Catoms3DBlock *>(entry.second);
            if (bb)
                cells.push_back(bb->position);
        }
        return cells;
    }

    std::vector<Cell3DPosition> uniqueCells(std::vector<Cell3DPosition> cells)
    {
        std::sort(cells.begin(), cells.end());
        cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
        return cells;
    }

}

const int LightWalkCatoms3DBlockCode::ZLINE = 2;
const int LightWalkCatoms3DBlockCode::SAME_LINE_GAP_CATOMS = 2;
const int LightWalkCatoms3DBlockCode::TARGET_TRANSIT_Z = ZLINE + 1;
const int LightWalkCatoms3DBlockCode::METRICS_SAMPLE_EVENT = -900001;
bool LightWalkCatoms3DBlockCode::targetPlanInitialized = false;
bool LightWalkCatoms3DBlockCode::targetPlanAvailable = false;
bool LightWalkCatoms3DBlockCode::targetPlanUsesLineMapping = false;
bool LightWalkCatoms3DBlockCode::targetFillHighXFirst = false;
bool LightWalkCatoms3DBlockCode::dpGoalBoxConfigured = false;
Cell3DPosition LightWalkCatoms3DBlockCode::dpGoalMin =
    Cell3DPosition(-1, -1, -1);
Cell3DPosition LightWalkCatoms3DBlockCode::dpGoalMax =
    Cell3DPosition(-1, -1, -1);
bool LightWalkCatoms3DBlockCode::dpGoalBoxSetFromCommandLine = false;
double LightWalkCatoms3DBlockCode::dpGoalFillPercent = 80.0;
bool LightWalkCatoms3DBlockCode::dpGoalFillPercentSetFromCommandLine = false;
bool LightWalkCatoms3DBlockCode::dpConsoleEnabled = false;
std::map<int, int> LightWalkCatoms3DBlockCode::targetLineForSourceLine;
std::map<int, std::vector<Cell3DPosition>>
    LightWalkCatoms3DBlockCode::targetCellsByLine;
std::vector<Cell3DPosition> LightWalkCatoms3DBlockCode::targetCellsByGlobalOrder;
std::map<Cell3DPosition, long long> LightWalkCatoms3DBlockCode::reservedTargetCells;
std::map<Cell3DPosition, int> LightWalkCatoms3DBlockCode::claimedTargetCells;
std::set<Cell3DPosition> LightWalkCatoms3DBlockCode::filledTargetCells;
int LightWalkCatoms3DBlockCode::movingCatomRadius = 0;
bool LightWalkCatoms3DBlockCode::movingCatomRadiusSetFromCommandLine = false;
long long LightWalkCatoms3DBlockCode::nextSpacingWaitingOrder = 1;
std::map<long long, LightWalkCatoms3DBlockCode::MovementReservation>
    LightWalkCatoms3DBlockCode::movementReservations;
std::map<int, LightWalkCatoms3DBlockCode::MovementReservation>
    LightWalkCatoms3DBlockCode::spacingWaitingRequests;
bool LightWalkCatoms3DBlockCode::metricsEnabled = true;
bool LightWalkCatoms3DBlockCode::metricsCsvPathSetFromCommandLine = false;
bool LightWalkCatoms3DBlockCode::metricsSamplePeriodSetFromCommandLine = false;
std::string LightWalkCatoms3DBlockCode::metricsCsvPath =
    "lightWalkCatoms3DmultiLines2_metrics.csv";
Time LightWalkCatoms3DBlockCode::metricsSamplePeriodUs = 0;
bool LightWalkCatoms3DBlockCode::metricsSamplingStarted = false;
bool LightWalkCatoms3DBlockCode::metricsCompleted = false;
bID LightWalkCatoms3DBlockCode::metricsOwnerId = 0;
Time LightWalkCatoms3DBlockCode::metricsStartTime = 0;
long long LightWalkCatoms3DBlockCode::metricsMessagesSent = 0;
long long LightWalkCatoms3DBlockCode::metricsMessagesReceived = 0;
long long LightWalkCatoms3DBlockCode::metricsMotionRequests = 0;
long long LightWalkCatoms3DBlockCode::metricsMotionsStarted = 0;
long long LightWalkCatoms3DBlockCode::metricsMotionsCompleted = 0;
long long LightWalkCatoms3DBlockCode::metricsMotionDistance = 0;
long long LightWalkCatoms3DBlockCode::metricsMaxMotionDistance = 0;
long long LightWalkCatoms3DBlockCode::metricsSpacingDenials = 0;
long long LightWalkCatoms3DBlockCode::metricsPivotBusyDenials = 0;
long long LightWalkCatoms3DBlockCode::metricsTargetCellDenials = 0;
long long LightWalkCatoms3DBlockCode::metricsReservationTimeouts = 0;
long long LightWalkCatoms3DBlockCode::metricsReleaseAckTimeouts = 0;
long long LightWalkCatoms3DBlockCode::metricsReleaseRetries = 0;
long long LightWalkCatoms3DBlockCode::metricsTargetPlanningWallTimeUs = -1;
int LightWalkCatoms3DBlockCode::metricsMaxMovingModules = 0;
int LightWalkCatoms3DBlockCode::metricsMaxWaitingModules = 0;
bool LightWalkCatoms3DBlockCode::metricsInitialGraphCaptured = false;
int LightWalkCatoms3DBlockCode::metricsInitialGraphNodes = 0;
int LightWalkCatoms3DBlockCode::metricsInitialGraphEdges = 0;
int LightWalkCatoms3DBlockCode::metricsInitialConnectedComponents = 0;
int LightWalkCatoms3DBlockCode::metricsInitialConfigurationDiameterCells = 0;
int LightWalkCatoms3DBlockCode::metricsInitialBottleneckCandidates = 0;
std::vector<Cell3DPosition> LightWalkCatoms3DBlockCode::metricsInitialCells;
std::ofstream LightWalkCatoms3DBlockCode::metricsCsv;

bool LightWalkCatoms3DBlockCode::flatteningPhaseActive    = false;
bool LightWalkCatoms3DBlockCode::flatteningPhaseComplete  = false;
int  LightWalkCatoms3DBlockCode::flatteningTotalLayers    = 0;
int  LightWalkCatoms3DBlockCode::flatteningCompletedLayers = 0;
std::unordered_set<int> LightWalkCatoms3DBlockCode::flatteningDoneLayers;
std::unordered_set<long long> LightWalkCatoms3DBlockCode::seenFlattenLaunchMessages;
std::unordered_set<long long> LightWalkCatoms3DBlockCode::seenFlattenDoneMessages;

LightWalkCatoms3DBlockCode::LightWalkCatoms3DBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host)
{
    catom = host;
    if (not host)
        return;

    scheduler = getScheduler();
    world = BaseSimulator::getWorld();
    lattice = world->lattice;
}

LightWalkCatoms3DBlockCode::~LightWalkCatoms3DBlockCode()
{
}

void LightWalkCatoms3DBlockCode::onBlockSelected()
{

}

std::string LightWalkCatoms3DBlockCode::onInterfaceDraw()
{
    int moduleCount = 0;
    int movingModules = 0;
    int waitingModules = 0;
    int spacingBlockedModules = static_cast<int>(spacingWaitingRequests.size());

    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        auto *bc = bb ? dynamic_cast<LightWalkCatoms3DBlockCode *>(bb->blockCode)
                      : nullptr;
        if (!bc)
            continue;

        moduleCount++;
        if (bc->rotating)
            movingModules++;
        if (bc->isWaitingForMetrics())
            waitingModules++;
    }

    std::stringstream trace;
    trace << "#Modules: " << moduleCount << '\n';
    trace << "Moving radius: " << movingCatomRadius;
    if (movingCatomRadius <= 0)
        trace << " (spacing disabled)";
    trace << '\n';
    trace << "Radius effect: " << movementReservations.size()
          << " active, " << spacingBlockedModules
          << " waiting, " << metricsSpacingDenials << " denials\n";
    trace << "Moving/waiting: " << movingModules << '/' << waitingModules;
    if (targetPlanAvailable)
    {
        const int inside = countModulesInsideGoal();
        const int required = requiredGoalOccupancyCount();
        trace << "\nDP goal box: " << dpGoalMin << " -> " << dpGoalMax;
        trace << "\nGoal occupancy: " << inside << '/' << required
              << " catoms (" << dpGoalFillPercent << "% target)";
    }

    if (catom)
    {
        trace << "\nSelected #" << catom->blockId;
        if (targetPlanAvailable)
        {
            trace << " Dp=" << dpValueForCell(catom->position)
                  << " dist=" << distanceToGoalRegion(catom->position);
        }
        auto waiter = spacingWaitingRequests.find(catom->blockId);
        if (waiter != spacingWaitingRequests.end())
        {
            trace << " blocked by radius toward " << waiter->second.targetPos;
        }
        else if (rotating)
        {
            trace << " moving toward " << targetPos;
        }
        else if (awaitingPivotReservation)
        {
            trace << " waiting for pivot " << requestedPivotPos;
        }
        else if (awaitingReleaseAck)
        {
            trace << " waiting release ack";
        }
        else if (relocated)
        {
            trace << " relocated";
        }
    }

    return trace.str();
}

void LightWalkCatoms3DBlockCode::setMovingCatomRadius(int radius)
{
    movingCatomRadius = std::max(0, radius);
}

void LightWalkCatoms3DBlockCode::setDpGoalBox(
    const Cell3DPosition &minCorner, const Cell3DPosition &maxCorner)
{
    dpGoalMin = Cell3DPosition(std::min(minCorner[0], maxCorner[0]),
                               std::min(minCorner[1], maxCorner[1]),
                               std::min(minCorner[2], maxCorner[2]));
    dpGoalMax = Cell3DPosition(std::max(minCorner[0], maxCorner[0]),
                               std::max(minCorner[1], maxCorner[1]),
                               std::max(minCorner[2], maxCorner[2]));
    dpGoalBoxConfigured = true;
}

void LightWalkCatoms3DBlockCode::setDpGoalFillPercent(double percent)
{
    if (percent <= 0.0)
        percent = 1.0;
    if (percent > 100.0)
        percent = 100.0;
    dpGoalFillPercent = percent;
}

void LightWalkCatoms3DBlockCode::setMetricsCsvPath(const std::string &path)
{
    if (!path.empty())
    {
        metricsCsvPath = path;
    }
}

void LightWalkCatoms3DBlockCode::setMetricsSamplePeriod(Time period)
{
    metricsSamplePeriodUs = std::max<Time>(1, period);
}

void LightWalkCatoms3DBlockCode::setMetricsEnabled(bool enabled)
{
    metricsEnabled = enabled;
}

void LightWalkCatoms3DBlockCode::parseUserElements(TiXmlDocument *config)
{
    if (!config)
        return;

    TiXmlElement *root = config->RootElement();
    int parsedRadius = 0;
    if (!movingCatomRadiusSetFromCommandLine and readMovingRadiusAttribute(root, parsedRadius))
    {
        setMovingCatomRadius(parsedRadius);
    }

    TiXmlElement *lightWalk = root ? root->FirstChildElement("lightWalk") : nullptr;
    if (!movingCatomRadiusSetFromCommandLine and readMovingRadiusAttribute(lightWalk, parsedRadius))
    {
        setMovingCatomRadius(parsedRadius);
    }

    readMetricsAttributes(root);
    readMetricsAttributes(lightWalk);
    readDpGoalAttributes(root);
    readDpGoalAttributes(lightWalk);
}

bool LightWalkCatoms3DBlockCode::parseUserCommandLineArgument(int &argc,
                                                              char **argv[])
{
    if (argc <= 0 or !argv or !*argv or !(*argv)[0])
        return false;

    const std::string arg((*argv)[0]);
    if (arg == "-d")
    {
        setMetricsEnabled(false);
        return true;
    }

    if (arg == "-v")
    {
        dpConsoleEnabled = true;
        return true;
    }

    if (arg == "-y")
    {
        if (argc < 2 or !(*argv)[1])
        {
            throw CLIParsingError(arg + " requires a positive motion-delay multiplier");
        }

        try
        {
            const double multiplier = std::stod((*argv)[1]);
            if (multiplier <= 0.0)
                throw std::logic_error("non-positive multiplier");
            BaseSimulator::motionDelayMultiplier =
                static_cast<float>(std::max(0.01, multiplier));
        }
        catch (const std::logic_error &)
        {
            throw CLIParsingError(arg + " must be a positive motion-delay multiplier");
        }

        argc--;
        (*argv)++;
        return true;
    }

    if (arg == "-m")
    {
        if (argc < 2 or !(*argv)[1])
        {
            throw CLIParsingError(arg + " requires a CSV file path");
        }

        setMetricsCsvPath((*argv)[1]);
        metricsCsvPathSetFromCommandLine = true;
        argc--;
        (*argv)++;
        return true;
    }

    if (arg == "-u")
    {
        if (argc < 2 or !(*argv)[1])
        {
            throw CLIParsingError(arg + " requires an integer period in microseconds");
        }

        try
        {
            setMetricsSamplePeriod(static_cast<Time>(std::stoull((*argv)[1])));
        }
        catch (const std::logic_error &)
        {
            throw CLIParsingError(arg + " must be a positive integer period in microseconds");
        }

        metricsSamplePeriodSetFromCommandLine = true;
        argc--;
        (*argv)++;
        return true;
    }

    if (arg == "-q")
    {
        if (argc < 2 or !(*argv)[1])
        {
            throw CLIParsingError(arg + " requires a goal-fill percentage");
        }

        try
        {
            setDpGoalFillPercent(std::stod((*argv)[1]));
        }
        catch (const std::logic_error &)
        {
            throw CLIParsingError(arg + " must be a numeric goal-fill percentage");
        }

        dpGoalFillPercentSetFromCommandLine = true;
        argc--;
        (*argv)++;
        return true;
    }

    if (arg == "-b")
    {
        if (argc < 2 or !(*argv)[1])
        {
            throw CLIParsingError(arg + " requires a goal box: minX,minY,minZ:maxX,maxY,maxZ");
        }

        Cell3DPosition minCorner;
        Cell3DPosition maxCorner;
        const std::string value((*argv)[1]);
        if (parseGoalBoxText(value, minCorner, maxCorner))
        {
            argc--;
            (*argv)++;
        }
        else
        {
            if (argc < 3 or !(*argv)[2]
                or !parseCell3DText((*argv)[1], minCorner)
                or !parseCell3DText((*argv)[2], maxCorner))
            {
                throw CLIParsingError(arg + " requires a goal box: minX,minY,minZ:maxX,maxY,maxZ");
            }

            argc -= 2;
            (*argv) += 2;
        }

        setDpGoalBox(minCorner, maxCorner);
        dpGoalBoxSetFromCommandLine = true;
        return true;
    }

    if (arg != "-p" and arg != "-n")
        return false;
    if (argc < 2 or !(*argv)[1])
    {
        throw CLIParsingError(arg + " requires an integer radius value");
    }

    try
    {
        setMovingCatomRadius(std::stoi((*argv)[1]));
    }
    catch (const std::logic_error &)
    {
        throw CLIParsingError(arg + " must be an integer radius value");
    }

    movingCatomRadiusSetFromCommandLine = true;
    argc--;
    (*argv)++;
    return true;
}

int LightWalkCatoms3DBlockCode::sendMessage(HandleableMessage *msg,
                                            P2PNetworkInterface *dest,
                                            Time t0,
                                            Time dt)
{
    if (metricsEnabled and msg and dest)
    {
        metricsMessagesSent++;
    }

    return Catoms3DBlockCode::sendMessage(msg, dest, t0, dt);
}

void LightWalkCatoms3DBlockCode::startMetricsSamplingIfNeeded(
    Catoms3DBlock *owner, Scheduler *scheduler)
{
    if (!metricsEnabled or metricsSamplingStarted or !owner or !scheduler)
        return;

    if (metricsSamplePeriodUs == 0)
    {
        const double defaultPeriod =
            Catoms3DRotation::ANIMATION_DELAY * BaseSimulator::motionDelayMultiplier;
        setMetricsSamplePeriod(static_cast<Time>(std::max(1.0, defaultPeriod)));
    }

    metricsCsv.open(metricsCsvPath.c_str(), std::ios::out | std::ios::trunc);
    if (!metricsCsv.is_open())
    {
        cerr << "Could not open metrics CSV file: " << metricsCsvPath << endl;
        metricsEnabled = false;
        return;
    }

    metricsCsv
        << "row_kind,time_us,elapsed_us,time_step_index,sample_period_us,"
        << "paper_self_reconfiguration_time_us,paper_total_time_steps,"
        << "module_count,initialized_modules,active_modules,relocated_modules,"
        << "catom_count,meta_module_count,initial_configuration_cells,"
        << "goal_configuration_cells,initial_goal_union_cells,"
        << "paper_graph_nodes_v,paper_graph_edges_e,paper_graph_diameter_dg,"
        << "moving_modules,waiting_modules,awaiting_pivot_reservation_modules,"
        << "awaiting_release_ack_modules,pivot_waiting_modules,"
        << "spacing_waiting_requests,active_spacing_reservations,"
        << "reserved_target_cells,claimed_target_cells,filled_target_cells,"
        << "unfilled_target_cells,total_target_cells,completion_ratio,"
        << "target_plan_available,planner_wall_time_us,max_flow_planner_time_us,"
        << "max_flow_value,flow_path_count,flow_total_path_length_cells,"
        << "flow_max_path_length_cells,round_count,"
        << "total_motion_requests,total_motions_started,total_motions_completed,"
        << "total_motion_distance_cells,avg_motion_distance_cells,"
        << "max_motion_distance_cells,energy_proxy_motion_count,"
        << "energy_proxy_distance_cells,motion_success_ratio,"
        << "moving_modules_ratio,total_messages_sent,total_messages_received,"
        << "avg_sent_messages_per_module,avg_received_messages_per_module,"
        << "avg_total_messages_per_module,messages_per_completed_motion,"
        << "communication_per_motion,denials_per_motion,"
        << "spacing_denials,pivot_busy_denials,target_cell_denials,"
        << "reservation_timeouts,release_ack_timeouts,release_retries,"
        << "max_moving_modules,max_waiting_modules,initial_graph_nodes,"
        << "initial_graph_edges,initial_connected_components,"
        << "initial_configuration_diameter_cells,initial_bottleneck_candidates,"
        << "goal_graph_edges,goal_connected_components,"
        << "goal_configuration_diameter_cells,initial_goal_union_edges,"
        << "initial_goal_union_connected_components,"
        << "initial_goal_union_diameter_cells,"
        << "current_graph_edges,connected_components,configuration_diameter_cells,"
        << "bottleneck_candidates,max_local_waiting_modules,avg_waiting_ratio,"
        << "simulation_complete\n";

    metricsSamplingStarted = true;
    metricsCompleted = false;
    metricsOwnerId = owner->blockId;
    metricsStartTime = scheduler->now();

    if (!metricsInitialGraphCaptured)
    {
        const ConfigurationGraphMetrics graphMetrics =
            collectConfigurationGraphMetrics();
        metricsInitialCells = collectCurrentCells();
        metricsInitialGraphNodes = graphMetrics.nodes;
        metricsInitialGraphEdges = graphMetrics.edges;
        metricsInitialConnectedComponents = graphMetrics.connectedComponents;
        metricsInitialConfigurationDiameterCells = graphMetrics.diameterCells;
        metricsInitialBottleneckCandidates = graphMetrics.bottleneckCandidates;
        metricsInitialGraphCaptured = true;
    }

    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + metricsSamplePeriodUs, owner, METRICS_SAMPLE_EVENT));
}

void LightWalkCatoms3DBlockCode::sampleMetrics(Scheduler *scheduler,
                                               const std::string &rowKind)
{
    if (!metricsEnabled or !metricsCsv.is_open() or !scheduler)
        return;

    int moduleCount = 0;
    int initializedModules = 0;
    int activeModules = 0;
    int relocatedModules = 0;
    int movingModules = 0;
    int waitingModules = 0;
    int awaitingPivotReservationModules = 0;
    int awaitingReleaseAckModules = 0;
    int pivotWaitingModules = 0;
    std::map<Cell3DPosition, int> localWaitingCounts;

    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        auto *bc = bb ? dynamic_cast<LightWalkCatoms3DBlockCode *>(bb->blockCode)
                      : nullptr;
        if (!bc)
            continue;

        moduleCount++;
        if (!bc->initialized)
        {
            activeModules++;
            continue;
        }

        initializedModules++;
        if (bc->relocated)
            relocatedModules++;
        if (bc->rotating)
            movingModules++;
        if (bc->isWaitingForMetrics())
            waitingModules++;
        if (bc->awaitingPivotReservation)
        {
            awaitingPivotReservationModules++;
            localWaitingCounts[bc->requestedPivotPos]++;
        }
        if (bc->awaitingReleaseAck)
        {
            awaitingReleaseAckModules++;
            localWaitingCounts[bc->releaseAckPivotPos]++;
        }
        if (bc->moduleAwaitingGo)
        {
            pivotWaitingModules++;
            localWaitingCounts[bc->catom->position]++;
        }

        if (!bc->relocated or bc->rotating or bc->awaitingPivotReservation or bc->awaitingReleaseAck or bc->moduleAwaitingGo)
        {
            activeModules++;
        }
    }

    for (const auto &entry : spacingWaitingRequests)
    {
        localWaitingCounts[entry.second.targetPos]++;
    }

    metricsMaxMovingModules = std::max(metricsMaxMovingModules, movingModules);
    metricsMaxWaitingModules = std::max(metricsMaxWaitingModules, waitingModules);

    if (targetPlanAvailable)
    {
        updateFilledTargetCellsFromOccupancy();
    }
    const int totalTargetCells = static_cast<int>(targetCellsByGlobalOrder.size());
    const int filledTargetCellsCount = static_cast<int>(filledTargetCells.size());
    const int goalInsideCount = targetPlanAvailable ? countModulesInsideGoal() : 0;
    const int goalRequiredCount = targetPlanAvailable ? requiredGoalOccupancyCount() : 0;
    const int unfilledTargetCells =
        targetPlanAvailable
        ? std::max(0, goalRequiredCount - goalInsideCount)
        : std::max(0, moduleCount - relocatedModules);

    double completionRatio = 0.0;
    if (targetPlanAvailable and goalRequiredCount > 0)
    {
        completionRatio =
            std::min(1.0, static_cast<double>(goalInsideCount) / goalRequiredCount);
    }
    else if (moduleCount > 0)
    {
        completionRatio =
            static_cast<double>(relocatedModules) / moduleCount;
    }

    const Time now = scheduler->now();
    const Time elapsed = now >= metricsStartTime ? now - metricsStartTime : 0;
    const Time period = std::max<Time>(1, metricsSamplePeriodUs);
    const Time timeStepIndex = elapsed / period;
    const double avgMotionDistance = metricsMotionsStarted > 0 ? static_cast<double>(metricsMotionDistance) / metricsMotionsStarted : 0.0;
    const double avgSentMessagesPerModule = moduleCount > 0 ? static_cast<double>(metricsMessagesSent) / moduleCount : 0.0;
    const double avgReceivedMessagesPerModule = moduleCount > 0 ? static_cast<double>(metricsMessagesReceived) / moduleCount : 0.0;
    const double avgTotalMessagesPerModule = moduleCount > 0 ? static_cast<double>(metricsMessagesSent + metricsMessagesReceived) / moduleCount : 0.0;
    const double messagesPerCompletedMotion = metricsMotionsCompleted > 0 ? static_cast<double>(metricsMessagesSent + metricsMessagesReceived) / metricsMotionsCompleted : 0.0;
    const double avgWaitingRatio = moduleCount > 0 ? static_cast<double>(waitingModules) / moduleCount : 0.0;
    int maxLocalWaitingModules = 0;
    for (const auto &entry : localWaitingCounts)
    {
        maxLocalWaitingModules = std::max(maxLocalWaitingModules, entry.second);
    }
    const ConfigurationGraphMetrics graphMetrics = collectConfigurationGraphMetrics();
    const CellSetGraphMetrics goalGraphMetrics =
        collectCellSetGraphMetrics(targetCellsByGlobalOrder);
    std::vector<Cell3DPosition> initialGoalUnionCells = metricsInitialCells;
    initialGoalUnionCells.insert(initialGoalUnionCells.end(),
                                 targetCellsByGlobalOrder.begin(),
                                 targetCellsByGlobalOrder.end());
    const CellSetGraphMetrics initialGoalUnionMetrics =
        collectCellSetGraphMetrics(uniqueCells(initialGoalUnionCells));
    const int paperGraphNodes = initialGoalUnionMetrics.nodes;
    const int paperGraphEdges = initialGoalUnionMetrics.edges;
    const int paperGraphDiameter = initialGoalUnionMetrics.diameterCells;
    const double motionSuccessRatio = metricsMotionRequests > 0 ? static_cast<double>(metricsMotionsCompleted) / metricsMotionRequests : 0.0;
    const double movingModulesRatio = moduleCount > 0 ? static_cast<double>(movingModules) / moduleCount : 0.0;
    const long long totalDenials =
        metricsSpacingDenials + metricsPivotBusyDenials + metricsTargetCellDenials;
    const double communicationPerMotion = messagesPerCompletedMotion;
    const double denialsPerMotion = metricsMotionsCompleted > 0 ? static_cast<double>(totalDenials) / metricsMotionsCompleted : 0.0;
    const int notApplicable = -1;

    metricsCsv
        << rowKind << ','
        << now << ','
        << elapsed << ','
        << timeStepIndex << ','
        << period << ','
        << elapsed << ','
        << timeStepIndex << ','
        << moduleCount << ','
        << initializedModules << ','
        << activeModules << ','
        << relocatedModules << ','
        << moduleCount << ','
        << notApplicable << ','
        << metricsInitialGraphNodes << ','
        << totalTargetCells << ','
        << initialGoalUnionMetrics.nodes << ','
        << paperGraphNodes << ','
        << paperGraphEdges << ','
        << paperGraphDiameter << ','
        << movingModules << ','
        << waitingModules << ','
        << awaitingPivotReservationModules << ','
        << awaitingReleaseAckModules << ','
        << pivotWaitingModules << ','
        << spacingWaitingRequests.size() << ','
        << movementReservations.size() << ','
        << reservedTargetCells.size() << ','
        << claimedTargetCells.size() << ','
        << filledTargetCellsCount << ','
        << unfilledTargetCells << ','
        << totalTargetCells << ','
        << completionRatio << ','
        << (targetPlanAvailable ? 1 : 0) << ','
        << metricsTargetPlanningWallTimeUs << ','
        << notApplicable << ','
        << notApplicable << ','
        << notApplicable << ','
        << notApplicable << ','
        << notApplicable << ','
        << notApplicable << ','
        << metricsMotionRequests << ','
        << metricsMotionsStarted << ','
        << metricsMotionsCompleted << ','
        << metricsMotionDistance << ','
        << avgMotionDistance << ','
        << metricsMaxMotionDistance << ','
        << metricsMotionsCompleted << ','
        << metricsMotionDistance << ','
        << motionSuccessRatio << ','
        << movingModulesRatio << ','
        << metricsMessagesSent << ','
        << metricsMessagesReceived << ','
        << avgSentMessagesPerModule << ','
        << avgReceivedMessagesPerModule << ','
        << avgTotalMessagesPerModule << ','
        << communicationPerMotion << ','
        << messagesPerCompletedMotion << ','
        << denialsPerMotion << ','
        << metricsSpacingDenials << ','
        << metricsPivotBusyDenials << ','
        << metricsTargetCellDenials << ','
        << metricsReservationTimeouts << ','
        << metricsReleaseAckTimeouts << ','
        << metricsReleaseRetries << ','
        << metricsMaxMovingModules << ','
        << metricsMaxWaitingModules << ','
        << metricsInitialGraphNodes << ','
        << metricsInitialGraphEdges << ','
        << metricsInitialConnectedComponents << ','
        << metricsInitialConfigurationDiameterCells << ','
        << metricsInitialBottleneckCandidates << ','
        << goalGraphMetrics.edges << ','
        << goalGraphMetrics.connectedComponents << ','
        << goalGraphMetrics.diameterCells << ','
        << initialGoalUnionMetrics.edges << ','
        << initialGoalUnionMetrics.connectedComponents << ','
        << initialGoalUnionMetrics.diameterCells << ','
        << graphMetrics.edges << ','
        << graphMetrics.connectedComponents << ','
        << graphMetrics.diameterCells << ','
        << graphMetrics.bottleneckCandidates << ','
        << maxLocalWaitingModules << ','
        << avgWaitingRatio << ','
        << (rowKind == "final" ? 1 : 0) << '\n';
    metricsCsv.flush();
}

void LightWalkCatoms3DBlockCode::finalizeMetrics()
{
    if (!metricsEnabled or !metricsCsv.is_open())
        return;

    metricsCompleted = true;
    metricsCsv.flush();
    metricsCsv.close();
    cout << "Metrics CSV written to " << metricsCsvPath << endl;
}

void LightWalkCatoms3DBlockCode::recordMotionRequest()
{
    if (metricsEnabled)
        metricsMotionRequests++;
}

void LightWalkCatoms3DBlockCode::recordMotionStarted(
    const Cell3DPosition &from, const Cell3DPosition &to)
{
    if (!metricsEnabled)
        return;

    metricsMotionsStarted++;
    const int distance = from.dist_taxi(to);
    metricsMotionDistance += distance;
    metricsMaxMotionDistance = std::max<long long>(metricsMaxMotionDistance, distance);
}

void LightWalkCatoms3DBlockCode::recordMotionCompleted()
{
    if (metricsEnabled)
        metricsMotionsCompleted++;
}

void LightWalkCatoms3DBlockCode::recordReservationDenied(
    bool targetCellAvailable, bool spacingDenied)
{
    if (!metricsEnabled)
        return;

    if (spacingDenied)
    {
        metricsSpacingDenials++;
    }
    else if (!targetCellAvailable)
    {
        metricsTargetCellDenials++;
    }
    else
    {
        metricsPivotBusyDenials++;
    }
}

void LightWalkCatoms3DBlockCode::recordMessageReceived()
{
    if (metricsEnabled)
        metricsMessagesReceived++;
}

bool LightWalkCatoms3DBlockCode::metricsSimulationComplete()
{
    if (!metricsSamplingStarted)
        return false;
    if (!movementReservations.empty() or !spacingWaitingRequests.empty() or !reservedTargetCells.empty())
    {
        return false;
    }

    auto &blocks = BaseSimulator::getWorld()->getMap();
    bool allRelocated = true;
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        auto *bc = bb ? dynamic_cast<LightWalkCatoms3DBlockCode *>(bb->blockCode)
                      : nullptr;
        if (!bc or !bc->initialized)
            return false;

        if (bc->rotating or bc->awaitingPivotReservation or bc->awaitingReleaseAck or bc->moduleAwaitingGo or bc->reservedForToken != 0)
        {
            return false;
        }
        if (!bc->relocated)
        {
            allRelocated = false;
        }
    }

    if (targetPlanAvailable and !targetCellsByGlobalOrder.empty())
    {
        return goalCompletionReached();
    }

    return allRelocated;
}

void LightWalkCatoms3DBlockCode::startup()
{
    stringstream info;
    info << "Starting ";

    relocated = false;
    rotating = false;
    initialPosition = catom->position;
    lineNumber = catom->position[1];
    initialLineOrder = computeInitialLineOrder();
    initialGlobalOrder = computeInitialGlobalOrder();
    initializeTargetPlanIfNeeded(lattice);
    assignTargetFromPlan();
    initialLineHead = isLineHead();

    if (targetPlanAvailable)
    {
        initialized = true;
        lineStarted = true;
        updateFilledTargetCellsFromOccupancy();
        seedLocalDpValues();

        if (goalCompletionReached())
        {
            relocated = true;
            catom->setColor(GREEN);
            logDpStatus("complete", true);
        }
        else
        {
            relocated = false;
            catom->setColor(YELLOW);
            const Time dpStartupSlot = std::max<Time>(
                MSG_DELAY_MC,
                static_cast<Time>(Catoms3DRotation::ANIMATION_DELAY
                                  * BaseSimulator::motionDelayMultiplier / 20.0));
            scheduler->schedule(new InterruptionEvent<int>(
                scheduler->now() + MSG_DELAY_MC
                    + static_cast<Time>(catom->blockId % 24) * dpStartupSlot,
                catom, 1));
        }

        startMetricsSamplingIfNeeded(catom, scheduler);
        return;
    }

    if (!flatteningPhaseComplete and flatteningTotalLayers == 0)
    {
        int maxZ = ZLINE;
        auto &allBlocks = BaseSimulator::getWorld()->getMap();
        for (const auto &entry : allBlocks)
        {
            auto *bb = static_cast<Catoms3DBlock *>(entry.second);
            if (bb)
                maxZ = std::max(maxZ, static_cast<int>(bb->position[2]));
        }
        const int layersAbove = maxZ - ZLINE;
        if (layersAbove > 0)
        {
            flatteningTotalLayers = layersAbove;
            flatteningPhaseActive = true;
        }
    }

    if (flatteningPhaseActive)
    {

        if (static_cast<int>(catom->position[2]) == ZLINE)
        {
            catom->setColor(GREEN);
        }
        else
        {
            catom->setColor(CYAN);

            if (static_cast<int>(catom->position[2]) == ZLINE + 1 &&
                isFlatteningLeftHead())
            {
                startFlatteningLocomotion();
            }
        }
        initialized = true;
        startMetricsSamplingIfNeeded(catom, scheduler);
        return;
    }

    if (relocated)
    {
        lineStarted = true;
        catom->setColor(GREEN);
    }
    else if (isTopLineHead())
    {
        lineStarted = true;
        startLineLocomotion();
    }
    else if (isLineHead())
    {
        catom->setColor(MAGENTA);
    }
    else
    {
        catom->setColor(GREEN);

    }

    initialized = true;
    startMetricsSamplingIfNeeded(catom, scheduler);
}

void LightWalkCatoms3DBlockCode::processReceivedMessage(MessagePtr msg,
                                                        P2PNetworkInterface *sender)
{
    stringstream info;

    switch (msg->type)
    {

    default:
        cout << "Unknown Generic Message Type" << endl;
        assert(false);
        break;
    }
}

void LightWalkCatoms3DBlockCode::processLocalEvent(EventPtr pev)
{
    MessagePtr message;
    stringstream info;

    switch (pev->eventType)
    {
    case EVENT_RECEIVE_MESSAGE:
    {
        message =
            (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;

        if (message->isMessageHandleable())
        {
            std::shared_ptr<HandleableMessage> hMsg =
                (std::static_pointer_cast<HandleableMessage>(message));

            recordMessageReceived();
            if (dpConsoleEnabled)
            {
                console << " received " << hMsg->getName() << " from "
                        << message->sourceInterface->hostBlock->blockId
                        << " at " << getScheduler()->now() << "\n";
            }
            hMsg->handle(this);
        }
        else
        {
            P2PNetworkInterface *recv_interface = message->destinationInterface;

            processReceivedMessage(message, recv_interface);
        }
    }
    break;

    case EVENT_ADD_NEIGHBOR:
    {
        break;
    }

    case EVENT_REMOVE_NEIGHBOR:
    {
        break;
    }

    case EVENT_PIVOT_ACTUATION_START:
    {
        std::shared_ptr<PivotActuationStartEvent> pase = std::static_pointer_cast<PivotActuationStartEvent>(pev);

        if (greenLightIsOn)
            greenLightIsOn = false;

        catom->getNeighborPos(pase->toConP, actuationTargetPos);

        if (dpConsoleEnabled)
        {
            cout << "Dp rotation started mobile #" << pase->mobile->blockId
                 << " pivot #" << catom->blockId
                 << " pivotPos=" << catom->position
                 << " target=" << actuationTargetPos << endl;
            console << " started actuating for module #"
                    << pase->mobile->blockId << "\n";
        }
    }
    break;

    case EVENT_PIVOT_ACTUATION_END:
    {
        std::shared_ptr<PivotActuationEndEvent> paee = std::static_pointer_cast<PivotActuationEndEvent>(pev);

        if (dpConsoleEnabled)
        {
            cout << "Dp rotation pivot finished mobile #" << paee->mobile->blockId
                 << " pivot #" << catom->blockId
                 << " pivotPos=" << catom->position << endl;
            console << " finished actuating for module #"
                    << paee->mobile->blockId << "\n";
        }
    }
    break;

    case EVENT_ROTATION3D_END:
    {
        rotating = false;
        recordMotionCompleted();
        if (targetPlanAvailable and dpConsoleEnabled)
        {
            cout << "Dp rotation ended catom #" << catom->blockId
                 << " pos=" << catom->position
                 << " Dp=" << dpValueForCell(catom->position)
                 << endl;
        }

        if (flatteningPhaseActive)
        {

            resumeMotionAfterRelease   = false;
            finalizeRelocationAfterRelease = false;
            if (releaseReservedPivot(true))
            {

                resumeMotionAfterRelease = true;
            }
            else
            {
                continueFlatteningStep();
            }
            break;
        }

        if (targetPlanAvailable)
        {
            updateFilledTargetCellsFromOccupancy();
            broadcastCurrentDpValue("post-move");
            const bool assignedReached =
                hasAssignedTarget and isAssignedTargetReached();
            resumeMotionAfterRelease =
                !assignedReached and !shouldSettleAtTargetCell()
                and !goalCompletionReached();
        }
        else
        {
            resumeMotionAfterRelease = catom->position[2] != ZLINE;
        }
        finalizeRelocationAfterRelease = !resumeMotionAfterRelease;

        if (releaseReservedPivot(true))
        {
            break;
        }

        acknowledgeCompletedMotion();
        continueAfterReleaseAck();
    }
    break;

    case EVENT_TAP:
    {
    }
    break;

    case EVENT_INTERRUPTION:
    {
        auto itev = std::static_pointer_cast<InterruptionEvent<int>>(pev);
        if (itev->data == METRICS_SAMPLE_EVENT)
        {
            const bool complete = metricsSimulationComplete();
            sampleMetrics(scheduler, complete ? "final" : "sample");
            if (complete)
            {
                metricsCompleted = true;
                if (metricsCsv.is_open())
                    metricsCsv.close();
                cout << "Metrics CSV written to " << metricsCsvPath << endl;
            }
            else
            {
                scheduler->schedule(new InterruptionEvent<int>(
                    scheduler->now() + metricsSamplePeriodUs, catom,
                    METRICS_SAMPLE_EVENT));
            }
            break;
        }

        if (itev->data >= 400000 and itev->data < 500000)
        {
            const int token = itev->data - 400000;
            if (awaitingReleaseAck and token == activeReservationToken)
            {
                if (releaseAckRetryCount < 5)
                {
                    metricsReleaseRetries++;
                    releaseAckRetryCount++;
                    sendReleaseToPivot(releaseAckPivotPos, token,
                                       releaseAckCompletedMotion);
                    scheduler->schedule(new InterruptionEvent<int>(
                        scheduler->now() + (20 * MSG_DELAY_MC), catom,
                        400000 + token));
                }
                else
                {
                    awaitingReleaseAck = false;
                    releaseMovementSpacing(catom->blockId, activeReservationToken);
                    clearMovementSpacingWait(catom->blockId);
                    activeReservationToken = 0;
                    releaseAckRetryCount = 0;
                    releaseAckPivotPos = Cell3DPosition(-1, -1, -1);
                    metricsReleaseAckTimeouts++;
                    if (releaseAckCompletedMotion)
                    {
                        acknowledgeCompletedMotion();
                    }
                    releaseAckCompletedMotion = false;
                    continueAfterReleaseAck();
                }
            }
            break;
        }

        if (itev->data >= 300000 and itev->data < 400000)
        {
            const int token = itev->data - 300000;
            if (awaitingPivotReservation and token == activeReservationToken)
            {
                sendReleaseToPivot(requestedPivotPos, token, false);
                awaitingPivotReservation = false;
                releaseMovementSpacing(catom->blockId, activeReservationToken);
                clearMovementSpacingWait(catom->blockId);
                activeReservationToken = 0;
                requestedPivotPos = Cell3DPosition(-1, -1, -1);
                pendingMotionPivotPos = Cell3DPosition(-1, -1, -1);
                metricsReservationTimeouts++;
                retryMotionRequest(activeReservationRetryEvent);
            }
            break;
        }

        switch (itev->data)
        {
        case 10:
        {

            if (flatteningPhaseActive and flatteningLocomotionStarted and !flatteningLanded)
            {
                continueFlatteningStep();
            }
        }
        break;
        case 1:
        {
            if (targetPlanAvailable and goalCompletionReached())
            {
                clearMovementSpacingWait(catom->blockId);
                relocated = true;
                lineStarted = true;
                catom->setColor(GREEN);
                logDpStatus("goal-threshold-reached", true);
                break;
            }

            if (targetPlanAvailable and hasAssignedTarget and isAssignedTargetReached())
            {
                clearMovementSpacingWait(catom->blockId);
                markAssignedTargetFilled();
                relocated = true;
                lineStarted = true;
                catom->setColor(GREEN);
                logDpStatus("assigned-target-reached", true);
                break;
            }

            if (targetPlanAvailable and shouldSettleAtTargetCell())
            {
                clearMovementSpacingWait(catom->blockId);
                relocated = true;
                lineStarted = true;
                catom->setColor(GREEN);
                logDpStatus("settled-target-cell", true);
                break;
            }

            if (rotating or awaitingPivotReservation or awaitingReleaseAck or relocated or (not isUsingTargetPlan() and catom->position[2] == ZLINE))
            {
                clearMovementSpacingWait(catom->blockId);
                break;
            }

            Cell3DPosition nextTarget;
            if (!chooseNextMotionTarget(nextTarget))
            {
                clearMovementSpacingWait(catom->blockId);
                if (targetPlanAvailable and goalCompletionReached())
                {
                    relocated = true;
                    lineStarted = true;
                    catom->setColor(GREEN);
                    break;
                }
                retryMotionRequest(1);
                break;
            }
            requestMotionToTarget(nextTarget, 1);
        }
        break;
        case 2:
        {
            if (isLineHead() and not rotating and not relocated)
            {
                startLineLocomotion();
            }
        }
        break;
        default:
            break;
        }
    }
    break;
    }
}

void LightWalkCatoms3DBlockCode::initializeTargetPlanIfNeeded(Lattice *lattice)
{
    if (targetPlanInitialized)
        return;

    const auto planningStart = std::chrono::steady_clock::now();
    auto finishTargetPlanning = [&]()
    {
        metricsTargetPlanningWallTimeUs =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - planningStart)
                .count();
    };

    targetPlanInitialized = true;
    targetPlanAvailable = false;
    targetPlanUsesLineMapping = false;
    targetFillHighXFirst = false;
    targetLineForSourceLine.clear();
    targetCellsByLine.clear();
    targetCellsByGlobalOrder.clear();
    claimedTargetCells.clear();
    filledTargetCells.clear();

    if (!lattice)
    {
        finishTargetPlanning();
        return;
    }

    auto addGoalCell = [&](const Cell3DPosition &pos)
    {
        if (!lattice->isInGrid(pos))
            return;
        targetCellsByLine[pos[1]].push_back(pos);
        targetCellsByGlobalOrder.push_back(pos);
    };

    if (!dpGoalBoxConfigured and target)
    {
        std::vector<Cell3DPosition> rawTargets;
        for (int z = 0; z < lattice->gridSize[2]; ++z)
        {
            for (int y = 0; y < lattice->gridSize[1]; ++y)
            {
                for (int x = 0; x < lattice->gridSize[0]; ++x)
                {
                    const Cell3DPosition pos(x, y, z);
                    if (target->isInTarget(pos))
                    {
                        rawTargets.push_back(pos);
                    }
                }
            }
        }

        if (!rawTargets.empty())
        {
            Cell3DPosition minCorner = rawTargets.front();
            Cell3DPosition maxCorner = rawTargets.front();
            int minTargetZ = rawTargets.front()[2];
            for (const auto &pos : rawTargets)
            {
                minCorner = Cell3DPosition(std::min(minCorner[0], pos[0]),
                                           std::min(minCorner[1], pos[1]),
                                           std::min(minCorner[2], pos[2]));
                maxCorner = Cell3DPosition(std::max(maxCorner[0], pos[0]),
                                           std::max(maxCorner[1], pos[1]),
                                           std::max(maxCorner[2], pos[2]));
                minTargetZ = std::min(minTargetZ, static_cast<int>(pos[2]));
            }

            const int zOffset = minTargetZ < ZLINE ? ZLINE - minTargetZ : 0;
            for (const auto &pos : rawTargets)
            {
                addGoalCell(Cell3DPosition(pos[0], pos[1], pos[2] + zOffset));
            }
            setDpGoalBox(Cell3DPosition(minCorner[0], minCorner[1],
                                        minCorner[2] + zOffset),
                         Cell3DPosition(maxCorner[0], maxCorner[1],
                                        maxCorner[2] + zOffset));
        }
    }

    if (dpGoalBoxConfigured and targetCellsByGlobalOrder.empty())
    {
        for (int z = dpGoalMin[2]; z <= dpGoalMax[2]; ++z)
        {
            for (int y = dpGoalMin[1]; y <= dpGoalMax[1]; ++y)
            {
                for (int x = dpGoalMin[0]; x <= dpGoalMax[0]; ++x)
                {
                    addGoalCell(Cell3DPosition(x, y, z));
                }
            }
        }
    }

    if (!dpGoalBoxConfigured and targetCellsByGlobalOrder.empty())
    {
        auto &blocks = BaseSimulator::getWorld()->getMap();
        bool foundBlock = false;
        Cell3DPosition minCorner;
        Cell3DPosition maxCorner;
        for (const auto &entry : blocks)
        {
            auto *bb = static_cast<Catoms3DBlock *>(entry.second);
            if (!bb)
                continue;
            if (!foundBlock)
            {
                minCorner = bb->position;
                maxCorner = bb->position;
                foundBlock = true;
            }
            else
            {
                minCorner = Cell3DPosition(std::min(minCorner[0], bb->position[0]),
                                           std::min(minCorner[1], bb->position[1]),
                                           std::min(minCorner[2], bb->position[2]));
                maxCorner = Cell3DPosition(std::max(maxCorner[0], bb->position[0]),
                                           std::max(maxCorner[1], bb->position[1]),
                                           std::max(maxCorner[2], bb->position[2]));
            }
        }

        if (foundBlock)
        {
            const int width = maxCorner[0] - minCorner[0] + 1;
            const Cell3DPosition autoMin(maxCorner[0] + std::max(3, width / 2),
                                         minCorner[1], minCorner[2]);
            const Cell3DPosition autoMax(autoMin[0] + width - 1,
                                         maxCorner[1], maxCorner[2]);
            setDpGoalBox(autoMin, autoMax);
            for (int z = dpGoalMin[2]; z <= dpGoalMax[2]; ++z)
            {
                for (int y = dpGoalMin[1]; y <= dpGoalMax[1]; ++y)
                {
                    for (int x = dpGoalMin[0]; x <= dpGoalMax[0]; ++x)
                    {
                        addGoalCell(Cell3DPosition(x, y, z));
                    }
                }
            }
        }
    }

    if (targetCellsByGlobalOrder.empty())
    {
        finishTargetPlanning();
        return;
    }

    bool foundSourceBlock = false;
    int minSourceX = 0;
    int maxSourceX = 0;
    auto &sourceBlocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : sourceBlocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb)
            continue;

        if (!foundSourceBlock)
        {
            minSourceX = bb->position[0];
            maxSourceX = bb->position[0];
            foundSourceBlock = true;
        }
        else
        {
            minSourceX = std::min(minSourceX, static_cast<int>(bb->position[0]));
            maxSourceX = std::max(maxSourceX, static_cast<int>(bb->position[0]));
        }
    }
    if (foundSourceBlock)
    {
        const int sourceCenter2x = minSourceX + maxSourceX;
        const int goalCenter2x = dpGoalMin[0] + dpGoalMax[0];
        targetFillHighXFirst = goalCenter2x >= sourceCenter2x;
    }

    for (auto &entry : targetCellsByLine)
    {
        std::sort(entry.second.begin(), entry.second.end(), compareTargetFillOrder);
    }
    std::sort(targetCellsByGlobalOrder.begin(), targetCellsByGlobalOrder.end(),
              compareTargetFillOrder);
    targetCellsByGlobalOrder.erase(
        std::unique(targetCellsByGlobalOrder.begin(), targetCellsByGlobalOrder.end()),
        targetCellsByGlobalOrder.end());

    targetPlanUsesLineMapping = false;
    targetPlanAvailable = true;
    updateFilledTargetCellsFromOccupancy();

    for (const auto &pos : targetCellsByGlobalOrder)
    {
        lattice->highlightCell(pos, BLUE);
    }

    cout << "DP goal box " << dpGoalMin << " -> " << dpGoalMax
         << ", capacity=" << targetCellsByGlobalOrder.size()
         << ", fillPercent=" << dpGoalFillPercent
         << ", requiredCatoms=" << requiredGoalOccupancyCount() << endl;
    finishTargetPlanning();
}

bool LightWalkCatoms3DBlockCode::
    compareTargetFillOrder(const Cell3DPosition &lhs, const Cell3DPosition &rhs)
{
    if (lhs[2] != rhs[2])
        return lhs[2] < rhs[2];
    if (lhs[0] != rhs[0])
    {
        return targetFillHighXFirst ? lhs[0] > rhs[0] : lhs[0] < rhs[0];
    }
    if (lhs[1] != rhs[1])
        return lhs[1] > rhs[1];
    return false;
}

void LightWalkCatoms3DBlockCode::updateFilledTargetCellsFromOccupancy()
{
    filledTargetCells.clear();
    if (!targetPlanAvailable)
        return;

    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (bb and std::find(targetCellsByGlobalOrder.begin(),
                             targetCellsByGlobalOrder.end(),
                             bb->position) != targetCellsByGlobalOrder.end())
        {
            filledTargetCells.insert(bb->position);
        }
    }
}

int LightWalkCatoms3DBlockCode::countInitializedModules()
{
    int count = 0;
    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        auto *bc = bb ? dynamic_cast<LightWalkCatoms3DBlockCode *>(bb->blockCode)
                      : nullptr;
        if (bc and bc->initialized)
        {
            ++count;
        }
        else if (bb)
        {
            ++count;
        }
    }
    return count;
}

int LightWalkCatoms3DBlockCode::countModulesInsideGoal()
{
    int count = 0;
    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (bb and std::find(targetCellsByGlobalOrder.begin(),
                             targetCellsByGlobalOrder.end(),
                             bb->position) != targetCellsByGlobalOrder.end())
        {
            ++count;
        }
    }
    return count;
}

int LightWalkCatoms3DBlockCode::requiredGoalOccupancyCount()
{
    const int moduleCount = countInitializedModules();
    if (moduleCount <= 0)
        return 0;

    const int requested =
        static_cast<int>(std::ceil(moduleCount * dpGoalFillPercent / 100.0));
    const int capacity = static_cast<int>(targetCellsByGlobalOrder.size());
    return std::max(1, std::min(requested, capacity > 0 ? capacity : requested));
}

bool LightWalkCatoms3DBlockCode::goalCompletionReached()
{
    if (!targetPlanAvailable)
        return false;

    updateFilledTargetCellsFromOccupancy();
    const int required = requiredGoalOccupancyCount();
    return required > 0 and countModulesInsideGoal() >= required;
}

int LightWalkCatoms3DBlockCode::computeInitialLineOrder() const
{
    int order = 0;
    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb or bb->position[2] != ZLINE or bb->position[1] != lineNumber)
        {
            continue;
        }
        if (bb->position[0] < initialPosition[0] or (bb->position[0] == initialPosition[0] and bb->blockId < catom->blockId))
        {
            ++order;
        }
    }
    return order;
}

int LightWalkCatoms3DBlockCode::computeInitialGlobalOrder() const
{
    int order = 0;
    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb or bb->position[2] != ZLINE)
            continue;
        if (bb->position[1] > initialPosition[1] or (bb->position[1] == initialPosition[1] and (bb->position[0] < initialPosition[0] or (bb->position[0] == initialPosition[0] and bb->blockId < catom->blockId))))
        {
            ++order;
        }
    }
    return order;
}

void LightWalkCatoms3DBlockCode::assignTargetFromPlan()
{
    hasAssignedTarget = false;
    assignedTargetPos = Cell3DPosition(-1, -1, -1);
}

bool LightWalkCatoms3DBlockCode::isUsingTargetPlan() const
{
    return targetPlanAvailable;
}

bool LightWalkCatoms3DBlockCode::isAssignedTargetReached() const
{
    return hasAssignedTarget and catom->position == assignedTargetPos;
}

int LightWalkCatoms3DBlockCode::
    distanceToAssignedTarget(const Cell3DPosition &pos) const
{
    return std::abs(pos[0] - assignedTargetPos[0]) + std::abs(pos[1] - assignedTargetPos[1]) + std::abs(pos[2] - assignedTargetPos[2]);
}

bool LightWalkCatoms3DBlockCode::
    chooseNextMotionTarget(Cell3DPosition &nextTarget)
{
    if (targetPlanAvailable)
    {
        updateFilledTargetCellsFromOccupancy();
        if (goalCompletionReached())
        {
            relocated = true;
            catom->setColor(GREEN);
            return false;
        }

        if (shouldSettleAtTargetCell())
        {
            relocated = true;
            catom->setColor(GREEN);
            return false;
        }

        if (!isTargetCell(catom->position) and !isDpFrontierCandidate())
            return false;

        return chooseNextDpStep(nextTarget);
    }
    return chooseNextStraightWalkStep(nextTarget);
}

bool LightWalkCatoms3DBlockCode::isTargetCell(const Cell3DPosition &pos) const
{
    return std::find(targetCellsByGlobalOrder.begin(), targetCellsByGlobalOrder.end(),
                     pos) != targetCellsByGlobalOrder.end();
}

bool LightWalkCatoms3DBlockCode::isTargetFilled(const Cell3DPosition &pos) const
{
    return filledTargetCells.find(pos) != filledTargetCells.end();
}

bool LightWalkCatoms3DBlockCode::shouldSettleAtTargetCell() const
{
    if (!targetPlanAvailable or !isTargetCell(catom->position))
        return false;

    const int direction = targetFillHighXFirst ? 1 : -1;
    for (int x = catom->position[0] + direction;
         targetFillHighXFirst ? x <= dpGoalMax[0] : x >= dpGoalMin[0];
         x += direction)
    {
        const Cell3DPosition fartherCell(x, catom->position[1],
                                         catom->position[2]);
        if (isTargetCell(fartherCell) and !isTargetFilled(fartherCell))
            return false;
    }

    return true;
}

bool LightWalkCatoms3DBlockCode::
    configurationStaysConnectedAfterMove(const Cell3DPosition &targetPos) const
{
    if (!targetPlanAvailable or !lattice or !catom
        or targetPos == catom->position)
    {
        return true;
    }

    auto &blocks = BaseSimulator::getWorld()->getMap();
    const int stationaryCount = std::max(
        0, static_cast<int>(blocks.size()) - 1);
    if (lattice->getBlock(targetPos) != nullptr)
        return false;

    int currentNeighborCount = 0;
    for (const auto &neighbor : lattice->getActiveNeighborCells(catom->position))
    {
        auto *bb = static_cast<Catoms3DBlock *>(lattice->getBlock(neighbor));
        if (bb and bb->blockId != catom->blockId)
            ++currentNeighborCount;
    }

    bool targetTouchesStructure = false;
    for (const auto &neighbor : lattice->getActiveNeighborCells(targetPos))
    {
        auto *bb = static_cast<Catoms3DBlock *>(lattice->getBlock(neighbor));
        if (bb and bb->blockId != catom->blockId)
        {
            targetTouchesStructure = true;
            break;
        }
    }

    if (stationaryCount == 0)
        return true;
    // The destination must re-attach to the rest of the body, otherwise the
    // moved module strands itself even if leaving was locally safe.
    if (!targetTouchesStructure)
        return false;
    if (currentNeighborCount <= 1)
        return true;

    // Paper's connectivity guarantee (Section 4): the module may leave iff its
    // neighbours remain mutually connected without it -- a bounded *local*
    // connecting-cycle search, instead of an O(N) flood over the whole body.
    // Combined with targetTouchesStructure above, a safe departure plus a
    // re-attaching arrival keeps the structure connected.
    return isMobileByConnectingCycle();
}

// Bounds for the local connecting-cycle search (paper, Section 4, Figure 6).
// Dense lattices yield short cycles, so a small constant depth/expansion finds
// them; the search is conservative (reports immobile) if the bound is hit,
// which never breaks connectivity, only forgoes a legal move.
static const int CC_MAX_DEPTH = 8;
static const int CC_MAX_EXPAND = 80;

bool LightWalkCatoms3DBlockCode::isMobileByConnectingCycle() const
{
    if (!lattice or !catom)
        return true;

    // Robot neighbours = adjacent occupied cells (every block here is part of
    // the robot; lightWalk3D has no terrain/obstacle modules).
    std::vector<Cell3DPosition> robotNb;
    for (const auto &n : lattice->getActiveNeighborCells(catom->position))
    {
        if (lattice->getBlock(n) != nullptr)
            robotNb.push_back(n);
    }

    if (robotNb.size() <= 1)
        return true; // a leaf is always mobile

    // Bounded BFS for a connecting cycle joining all neighbours through other
    // modules, forbidding our own cell. If every neighbour is reached, we are
    // not an articulation point and may safely move.
    const Cell3DPosition self = catom->position;
    std::set<Cell3DPosition> targets(robotNb.begin() + 1, robotNb.end());
    std::set<Cell3DPosition> visited;
    std::queue<std::pair<Cell3DPosition, int>> q;

    visited.insert(robotNb[0]);
    targets.erase(robotNb[0]);
    q.push({robotNb[0], 0});

    int expand = 0;
    while (!q.empty() and expand < CC_MAX_EXPAND)
    {
        const Cell3DPosition cell = q.front().first;
        const int depth = q.front().second;
        q.pop();
        ++expand;
        if (depth >= CC_MAX_DEPTH)
            continue;

        for (const auto &nb : lattice->getActiveNeighborCells(cell))
        {
            if (nb == self or visited.count(nb))
                continue;
            if (lattice->getBlock(nb) == nullptr)
                continue;

            visited.insert(nb);
            targets.erase(nb);
            if (targets.empty())
                return true; // all neighbours reconnected
            q.push({nb, depth + 1});
        }
    }
    return targets.empty(); // conservative: immobile if the bound was hit
}

bool LightWalkCatoms3DBlockCode::hasUnfilledTargetCell() const
{
    for (const auto &pos : targetCellsByGlobalOrder)
    {
        if (!isTargetFilled(pos))
            return true;
    }

    return false;
}

bool LightWalkCatoms3DBlockCode::targetHasSupport(const Cell3DPosition &pos) const
{
    if (pos[2] <= ZLINE)
        return true;

    const Cell3DPosition below(pos[0], pos[1], pos[2] - 1);
    return isTargetFilled(below);
}

bool LightWalkCatoms3DBlockCode::claimNextTargetCell()
{
    if (!targetPlanAvailable)
        return false;

    const bool previousHasTarget = hasAssignedTarget;
    const Cell3DPosition previousTarget = assignedTargetPos;
    updateFilledTargetCellsFromOccupancy();
    for (auto claim = claimedTargetCells.begin();
         claim != claimedTargetCells.end();)
    {
        if (isTargetFilled(claim->first))
            claim = claimedTargetCells.erase(claim);
        else
            ++claim;
    }

    const int required = requiredGoalOccupancyCount();
    if (!previousHasTarget and required > 0
        and countModulesInsideGoal()
                + static_cast<int>(claimedTargetCells.size())
            >= required)
    {
        return false;
    }

    std::vector<Cell3DPosition> candidates;
    std::set<Cell3DPosition> seen;

    auto addCandidate = [&](const Cell3DPosition &candidate)
    {
        if (seen.insert(candidate).second)
            candidates.push_back(candidate);
    };

    bool hasLineScopedTargets = false;
    if (targetPlanUsesLineMapping)
    {
        const auto mappedLine = targetLineForSourceLine.find(lineNumber);
        if (mappedLine != targetLineForSourceLine.end())
        {
            const auto cells = targetCellsByLine.find(mappedLine->second);
            if (cells != targetCellsByLine.end())
            {
                for (const auto &candidate : cells->second)
                    addCandidate(candidate);
                hasLineScopedTargets = true;
            }
        }
    }
    else
    {
        const auto cells = targetCellsByLine.find(lineNumber);
        if (cells != targetCellsByLine.end())
        {
            for (const auto &candidate : cells->second)
                addCandidate(candidate);
            hasLineScopedTargets = true;
        }
    }

    if (!hasLineScopedTargets)
        return false;

    int buildLayer = std::numeric_limits<int>::max();
    for (const auto &candidate : targetCellsByGlobalOrder)
    {
        if (!isTargetFilled(candidate))
        {
            buildLayer = std::min(buildLayer, static_cast<int>(candidate[2]));
        }
    }
    if (buildLayer == std::numeric_limits<int>::max())
        return false;

    for (const auto &candidate : candidates)
    {
        if (candidate[2] != buildLayer)
            continue;
        if (isTargetFilled(candidate))
            continue;
        if (!targetHasSupport(candidate))
            continue;

        bool earlierTargetInLineOpen = false;
        const auto lineCells = targetCellsByLine.find(candidate[1]);
        if (lineCells != targetCellsByLine.end())
        {
            for (const auto &lineCandidate : lineCells->second)
            {
                if (lineCandidate[2] != buildLayer)
                    continue;
                if (lineCandidate == candidate)
                    break;
                if (!isTargetFilled(lineCandidate))
                {
                    earlierTargetInLineOpen = true;
                    break;
                }
            }
        }
        if (earlierTargetInLineOpen)
            continue;

        const auto claim = claimedTargetCells.find(candidate);
        if (claim != claimedTargetCells.end() and claim->second != static_cast<int>(catom->blockId))
        {
            continue;
        }

        hasAssignedTarget = true;
        assignedTargetPos = candidate;
        Cell3DPosition previewTarget;
        if (!chooseNextTargetPlanStep(previewTarget))
            continue;

        claimedTargetCells[candidate] = catom->blockId;
        return true;
    }

    hasAssignedTarget = previousHasTarget;
    assignedTargetPos = previousTarget;
    return false;
}

void LightWalkCatoms3DBlockCode::releaseAssignedTargetClaim()
{
    if (hasAssignedTarget)
    {
        auto claim = claimedTargetCells.find(assignedTargetPos);
        if (claim != claimedTargetCells.end() and claim->second == static_cast<int>(catom->blockId))
        {
            claimedTargetCells.erase(claim);
        }
    }

    hasAssignedTarget = false;
    assignedTargetPos = Cell3DPosition(-1, -1, -1);
}

void LightWalkCatoms3DBlockCode::markAssignedTargetFilled()
{
    if (!hasAssignedTarget)
        return;

    filledTargetCells.insert(assignedTargetPos);
    auto claim = claimedTargetCells.find(assignedTargetPos);
    if (claim != claimedTargetCells.end() and claim->second == static_cast<int>(catom->blockId))
    {
        claimedTargetCells.erase(claim);
    }
    hasAssignedTarget = false;
    assignedTargetPos = Cell3DPosition(-1, -1, -1);
}

int LightWalkCatoms3DBlockCode::distanceToGoalRegion(
    const Cell3DPosition &pos) const
{
    if (targetCellsByGlobalOrder.empty())
        return 0;

    if (dpGoalBoxConfigured)
    {
        int distance = 0;
        for (int axis = 0; axis < 3; ++axis)
        {
            if (pos[axis] < dpGoalMin[axis])
                distance += dpGoalMin[axis] - pos[axis];
            else if (pos[axis] > dpGoalMax[axis])
                distance += pos[axis] - dpGoalMax[axis];
        }
        return distance;
    }

    int best = std::numeric_limits<int>::max();
    for (const auto &goalCell : targetCellsByGlobalOrder)
    {
        best = std::min(best, pos.dist_taxi(goalCell));
    }
    return best == std::numeric_limits<int>::max() ? 0 : best;
}

int LightWalkCatoms3DBlockCode::computeDpValueFromGoal(
    const Cell3DPosition &pos) const
{
    const int distance = distanceToGoalRegion(pos);
    const int goalFloor = targetCellsByGlobalOrder.empty() ? ZLINE : dpGoalMin[2];
    const int heightPenalty =
        std::max(0, static_cast<int>(pos[2]) - goalFloor) * DP_HEIGHT_COST;
    return DP_GOAL_REWARD - distance * DP_STEP_COST - heightPenalty;
}

int LightWalkCatoms3DBlockCode::dpValueForCell(const Cell3DPosition &pos) const
{
    int value = computeDpValueFromGoal(pos);
    const auto known = knownDpValues.find(pos);
    if (known != knownDpValues.end())
    {
        value = std::max(value, known->second);
    }
    return value;
}

void LightWalkCatoms3DBlockCode::seedLocalDpValues()
{
    if (!targetPlanAvailable)
        return;

    const bool firstSeed = knownDpValues.empty();
    knownDpValues[catom->position] = computeDpValueFromGoal(catom->position);
    const auto reachable = Catoms3DMotionEngine::getAllReachablePositions(catom);
    for (const auto &cell : reachable)
    {
        knownDpValues[cell] = computeDpValueFromGoal(cell);
    }
    if (firstSeed)
    {
        logDpStatus("startup", true);
    }
}

void LightWalkCatoms3DBlockCode::broadcastCurrentDpValue(
    const std::string &reason)
{
    if (!targetPlanAvailable)
        return;

    const int value = dpValueForCell(catom->position);
    knownDpValues[catom->position] = value;
    logDpStatus(reason, reason == "startup");

    const int sequence = dpSequence++;
    for (int i = 0; i < catom->getNbInterfaces(); ++i)
    {
        P2PNetworkInterface *p2p = catom->getInterface(i);
        if (!p2p or !p2p->isConnected())
            continue;

        sendMessage(new DpValueMessage(catom->blockId, sequence,
                                       catom->position, value),
                    p2p, MSG_DELAY_MC, 0);
    }
}

void LightWalkCatoms3DBlockCode::handleDpValueMessage(
    int originId, int sequence, const Cell3DPosition &cell, int dpValue)
{
    (void)originId;
    (void)sequence;

    if (!targetPlanAvailable)
        return;

    bool improvedKnownCell = false;
    const auto known = knownDpValues.find(cell);
    if (known == knownDpValues.end() or dpValue > known->second)
    {
        knownDpValues[cell] = dpValue;
        improvedKnownCell = true;
    }

    if (lattice->cellsAreAdjacent(catom->position, cell))
    {
        const int propagatedValue = dpValue - DP_STEP_COST;
        const int currentValue = dpValueForCell(catom->position);
        if (propagatedValue > currentValue)
        {
            knownDpValues[catom->position] = propagatedValue;
            logDpStatus("dp-update");
        }
    }

    if (improvedKnownCell)
    {
        logDpStatus("dp-received");
    }
}

void LightWalkCatoms3DBlockCode::logDpStatus(const std::string &reason,
                                             bool force)
{
    if (!targetPlanAvailable or !dpConsoleEnabled)
        return;

    const int value = dpValueForCell(catom->position);
    if (!force and value == lastPrintedDpValue)
        return;

    lastPrintedDpValue = value;
    cout << "Dp catom #" << catom->blockId
         << " " << reason
         << " pos=" << catom->position
         << " Dp=" << value
         << " dist=" << distanceToGoalRegion(catom->position)
         << " insideGoal=" << (isTargetCell(catom->position) ? 1 : 0)
         << " occupancy=" << countModulesInsideGoal()
         << "/" << requiredGoalOccupancyCount()
         << endl;
}

bool LightWalkCatoms3DBlockCode::shouldRunDpLocomotion() const
{
    return targetPlanAvailable
        and !relocated
        and !rotating
        and !awaitingPivotReservation
        and !awaitingReleaseAck
        and !isTargetCell(catom->position)
        and !goalCompletionReached()
        and isDpFrontierCandidate();
}

bool LightWalkCatoms3DBlockCode::isDpFrontierCandidate() const
{
    if (!targetPlanAvailable or !lattice or isTargetCell(catom->position))
        return false;

    int bestAxis = -1;
    int bestDistance = 0;
    int direction = 0;
    for (int axis = 0; axis < 3; ++axis)
    {
        int axisDirection = 0;
        int axisDistance = 0;
        if (catom->position[axis] < dpGoalMin[axis])
        {
            axisDirection = 1;
            axisDistance = dpGoalMin[axis] - catom->position[axis];
        }
        else if (catom->position[axis] > dpGoalMax[axis])
        {
            axisDirection = -1;
            axisDistance = catom->position[axis] - dpGoalMax[axis];
        }

        if (axisDistance > bestDistance)
        {
            bestAxis = axis;
            bestDistance = axisDistance;
            direction = axisDirection;
        }
    }

    if (bestAxis < 0)
        return false;

    Cell3DPosition step(0, 0, 0);
    if (bestAxis == 0)
        step = Cell3DPosition(direction, 0, 0);
    else if (bestAxis == 1)
        step = Cell3DPosition(0, direction, 0);
    else
        step = Cell3DPosition(0, 0, direction);

    const Cell3DPosition towardGoal = catom->position + step;
    return lattice->isInGrid(towardGoal) and lattice->isFree(towardGoal);
}

bool LightWalkCatoms3DBlockCode::chooseNextDpStep(Cell3DPosition &nextTarget)
{
    if (!targetPlanAvailable)
        return false;

    seedLocalDpValues();

    const auto reachable = Catoms3DMotionEngine::getAllReachablePositions(catom);
    const int currentDp = dpValueForCell(catom->position);
    const int currentDistance = distanceToGoalRegion(catom->position);
    const int minAllowedZ = std::min(ZLINE, static_cast<int>(dpGoalMin[2]));

    bool found = false;
    long bestScore = std::numeric_limits<long>::min();
    Cell3DPosition bestTarget(-1, -1, -1);

    for (const auto &candidate : reachable)
    {
        if (!lattice->isInGrid(candidate) or !lattice->isFree(candidate))
            continue;
        if (candidate[2] < minAllowedZ)
            continue;
        if (reservedTargetCells.count(candidate))
            continue;

        Catoms3DBlock *pivot = findMotionPivotForTarget(candidate);
        if (!pivot)
            continue;
        if (!configurationStaysConnectedAfterMove(candidate))
            continue;

        const int candidateDp = dpValueForCell(candidate);
        const int candidateDistance = distanceToGoalRegion(candidate);
        const bool beneficial =
            isTargetCell(candidate)
            or candidateDp > currentDp
            or candidateDistance < currentDistance;
        if (!beneficial)
            continue;

        long score = static_cast<long>(candidateDp) * 10000L;
        score -= static_cast<long>(candidateDistance) * 1000L;
        score -= std::abs(candidate[2] - dpGoalMin[2]) * 25L;
        if (isTargetCell(candidate))
        {
            score += 500000L;
            score += static_cast<long>(
                         targetFillHighXFirst ? candidate[0] : -candidate[0])
                     * 1000L;
        }
        if (candidateDistance < currentDistance)
            score += 10000L;
        score -= static_cast<long>(catom->blockId % 17);

        if (!found or score > bestScore)
        {
            found = true;
            bestScore = score;
            bestTarget = candidate;
        }
    }

    if (!found)
    {
        logDpStatus("no-beneficial-step");
        return false;
    }

    nextTarget = bestTarget;
    if (dpConsoleEnabled)
    {
        cout << "Dp move choice catom #" << catom->blockId
             << " from " << catom->position
             << " Dp=" << currentDp
             << " to " << nextTarget
             << " Dp=" << dpValueForCell(nextTarget)
             << endl;
    }
    return true;
}

bool LightWalkCatoms3DBlockCode::
    chooseNextStraightWalkStep(Cell3DPosition &nextTarget) const
{
    if (catom->position[2] == ZLINE)
    {
        nextTarget = catom->position + Cell3DPosition(0, 0, 1);
    }
    else
    {
        nextTarget = catom->position +
                     (hasReachedLineTail() ? Cell3DPosition(1, 0, -1) : Cell3DPosition(1, 0, 0));
    }
    return true;
}

bool LightWalkCatoms3DBlockCode::
    chooseNextTargetPlanStep(Cell3DPosition &nextTarget) const
{
    if (!hasAssignedTarget or isAssignedTargetReached())
        return false;

    const auto reachable = Catoms3DMotionEngine::getAllReachablePositions(catom);
    const int currentDist = distanceToAssignedTarget(catom->position);
    const int targetZ = assignedTargetPos[2];
    const int minWorkZ = std::min(TARGET_TRANSIT_Z, targetZ);
    int travelLine = lineNumber;
    if (targetPlanUsesLineMapping)
    {
        const auto mappedLine = targetLineForSourceLine.find(lineNumber);
        if (mappedLine != targetLineForSourceLine.end())
        {
            travelLine = mappedLine->second;
        }
    }

    bool found = false;
    long bestScore = std::numeric_limits<long>::max();
    Cell3DPosition bestTarget(-1, -1, -1);

    for (const auto &candidate : reachable)
    {
        if (!lattice->isInGrid(candidate) or !lattice->isFree(candidate))
            continue;
        if (candidate[2] < ZLINE)
            continue;
        if (candidate[1] != travelLine)
            continue;

        const bool isFinalTarget = candidate == assignedTargetPos;
        if (!isFinalTarget and reservedTargetCells.count(candidate))
            continue;
        if (!configurationStaysConnectedAfterMove(candidate))
            continue;

        if (assignedTargetPos[1] == lineNumber and assignedTargetPos[0] >= catom->position[0])
        {
            if (candidate[0] < catom->position[0])
                continue;
            if (candidate[0] > assignedTargetPos[0])
                continue;
            if (candidate[0] == catom->position[0] and candidate[2] <= catom->position[2] and !isFinalTarget)
            {
                continue;
            }
        }

        if (candidate[2] < catom->position[2] and !isFinalTarget)
            continue;
        if (candidate[2] > std::max(targetZ, TARGET_TRANSIT_Z))
            continue;

        const int candidatePlanarDist =
            std::abs(candidate[0] - assignedTargetPos[0]) + std::abs(candidate[1] - assignedTargetPos[1]);
        if (targetZ > TARGET_TRANSIT_Z and candidate[2] > TARGET_TRANSIT_Z and candidatePlanarDist > 0)
        {
            continue;
        }

        const int candidateDist = distanceToAssignedTarget(candidate);
        long score = static_cast<long>(candidateDist) * 1000;

        if (candidateDist > currentDist)
            score += 30000;
        if (candidate[2] < minWorkZ and !isFinalTarget)
            score += 20000;
        if (isFinalTarget)
            score -= 100000;

        if (assignedTargetPos[0] >= catom->position[0] and candidate[0] < catom->position[0])
        {
            score += 10000;
        }
        else if (assignedTargetPos[0] < catom->position[0] and candidate[0] > catom->position[0])
        {
            score += 10000;
        }

        if (assignedTargetPos[0] >= catom->position[0] and candidate[0] > assignedTargetPos[0])
        {
            score += 15000;
        }
        else if (assignedTargetPos[0] < catom->position[0] and candidate[0] < assignedTargetPos[0])
        {
            score += 15000;
        }

        score += std::abs(candidate[1] - assignedTargetPos[1]) * 100;
        score += std::abs(candidate[1] - lineNumber) * 10;

        if (!found or score < bestScore)
        {
            found = true;
            bestScore = score;
            bestTarget = candidate;
        }
    }

    if (!found)
        return false;

    nextTarget = bestTarget;
    return true;
}

void LightWalkCatoms3DBlockCode::setGreenLightAndResumeFlow()
{
    greenLightIsOn = true;
    catom->setColor(GREEN);

    if (moduleAwaitingGo)
    {
        bool nextToModule = isAdjacentToPosition(awaitingModulePos);
        P2PNetworkInterface *itf = nextToModule ? catom->getInterface(awaitingModulePos) : catom->getInterface(catom->position + Cell3DPosition(-1, 0, 0));

        greenLightIsOn = false;
        catom->setColor(RED);
        sendMessage(new GreenLightIsOnMessage(catom->position, awaitingModulePos),
                    itf, MSG_DELAY_MC, 0);
        moduleAwaitingGo = false;
    }
}

bool LightWalkCatoms3DBlockCode::isLineHead() const
{
    return catom->position[2] == ZLINE and not hasLeftNeighbor();
}

bool LightWalkCatoms3DBlockCode::isTopLineHead() const
{
    if (not isLineHead())
        return false;

    int maxLine = lineNumber;
    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (bb and bb->position[2] == ZLINE)
        {
            maxLine = std::max(maxLine, static_cast<int>(bb->position[1]));
        }
    }

    return lineNumber == maxLine;
}

int LightWalkCatoms3DBlockCode::getNextLineNumber() const
{
    int nextLine = std::numeric_limits<int>::min();

    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb or bb->position[2] != ZLINE)
            continue;

        const int candidate = bb->position[1];
        if (candidate < lineNumber and candidate > nextLine)
        {
            nextLine = candidate;
        }
    }

    return nextLine == std::numeric_limits<int>::min() ? lineNumber : nextLine;
}

long long LightWalkCatoms3DBlockCode::makeLineLaunchKey(int targetLine,
                                                        int launchId) const
{
    return (static_cast<long long>(targetLine) << 32) ^ static_cast<unsigned int>(launchId);
}

long long LightWalkCatoms3DBlockCode::makeTokenMessageKey(int mobileId,
                                                          int token,
                                                          int tag) const
{
    return (static_cast<long long>(mobileId) << 40) ^ (static_cast<long long>(token) << 8) ^ static_cast<unsigned int>(tag);
}

long long LightWalkCatoms3DBlockCode::
    makePositionMessageKey(const Cell3DPosition &a, const Cell3DPosition &b,
                           int tag) const
{
    std::size_t seed = 0;
    std::hash<int> hasher;
    const int values[] = {a[0], a[1], a[2], b[0], b[1], b[2], tag};
    for (int value : values)
    {
        seed ^= hasher(value) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    }
    return static_cast<long long>(seed);
}

void LightWalkCatoms3DBlockCode::startLineLocomotion()
{
    if (targetPlanAvailable)
    {
        if (rotating or awaitingPivotReservation or awaitingReleaseAck or relocated)
            return;
        if (goalCompletionReached())
        {
            relocated = true;
            catom->setColor(GREEN);
            logDpStatus("dp-stop", true);
            return;
        }

        lineStarted = true;
        Cell3DPosition nextTarget;
        if (!chooseNextMotionTarget(nextTarget))
        {
            clearMovementSpacingWait(catom->blockId);
            retryMotionRequest(1);
            return;
        }

        catom->setColor(YELLOW);
        requestMotionToTarget(nextTarget, 1);
        return;
    }

    if (rotating or awaitingPivotReservation or awaitingReleaseAck or relocated or not isLineHead())
        return;

    lineStarted = true;
    Cell3DPosition nextTarget;
    if (!chooseNextMotionTarget(nextTarget))
    {
        clearMovementSpacingWait(catom->blockId);
        if (targetPlanAvailable and !hasUnfilledTargetCell())
        {
            relocated = true;
            catom->setColor(GREEN);
            return;
        }
        retryMotionRequest(2);
        return;
    }

    catom->setColor(YELLOW);

    requestMotionToTarget(nextTarget, 2);
}

void LightWalkCatoms3DBlockCode::requestMotionToTarget(const Cell3DPosition &nextTarget,
                                                       int retryEvent)
{
    if (rotating or awaitingPivotReservation or awaitingReleaseAck)
        return;
    if (targetPlanAvailable and goalCompletionReached())
    {
        relocated = true;
        catom->setColor(GREEN);
        logDpStatus("request-suppressed-goal", true);
        return;
    }

    recordMotionRequest();
    targetPos = nextTarget;
    if (targetPlanAvailable
        and !configurationStaysConnectedAfterMove(targetPos))
    {
        clearMovementSpacingWait(catom->blockId);
        retryMotionRequest(retryEvent);
        return;
    }

    Catoms3DBlock *pivot = findMotionPivotForTarget(targetPos);
    if (!pivot)
    {
        clearMovementSpacingWait(catom->blockId);
        retryMotionRequest(retryEvent);
        return;
    }

    P2PNetworkInterface *itf = catom->getInterface(pivot->position);
    if (!itf or !itf->isConnected())
    {
        clearMovementSpacingWait(catom->blockId);
        retryMotionRequest(retryEvent);
        return;
    }

    activeReservationToken = nextReservationToken++;
    activeReservationRetryEvent = retryEvent;
    requestedPivotPos = pivot->position;
    pendingMotionPivotPos = pivot->position;
    awaitingPivotReservation = true;

    if (targetPlanAvailable and dpConsoleEnabled)
    {
        cout << "Dp reservation request catom #" << catom->blockId
             << " pos=" << catom->position
             << " Dp=" << dpValueForCell(catom->position)
             << " target=" << targetPos
             << " targetDp=" << dpValueForCell(targetPos)
             << " pivot=" << requestedPivotPos
             << endl;
    }

    sendMessage(new ReservationRequestMessage(catom->blockId,
                                              activeReservationToken,
                                              lineNumber,
                                              catom->position,
                                              targetPos,
                                              requestedPivotPos),
                itf, MSG_DELAY_MC, 0);
    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + (20 * MSG_DELAY_MC), catom,
        300000 + activeReservationToken));
}

void LightWalkCatoms3DBlockCode::retryMotionRequest(int retryEvent,
                                                    bool spacingBackoff)
{
    Time delay = MSG_DELAY_MC;
    if (targetPlanAvailable and retryEvent == 1)
    {
        delay = std::max<Time>(
            20 * MSG_DELAY_MC,
            static_cast<Time>(Catoms3DRotation::ANIMATION_DELAY
                              * BaseSimulator::motionDelayMultiplier));
        const Time jitterSlot = std::max<Time>(
            MSG_DELAY_MC,
            static_cast<Time>(Catoms3DRotation::ANIMATION_DELAY
                              * BaseSimulator::motionDelayMultiplier / 20.0));
        delay += static_cast<Time>(catom->blockId % 13) * jitterSlot;
    }
    if (spacingBackoff and movingCatomRadius > 0)
    {
        const int jitterSlots = std::max(2, movingCatomRadius + 2);
        delay *= static_cast<Time>(4 + movingCatomRadius);
        delay += static_cast<Time>(catom->blockId % jitterSlots) * MSG_DELAY_MC;
    }

    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + delay, catom, retryEvent));
}

bool LightWalkCatoms3DBlockCode::
    movementSpacingIsAvailable(int mobileId, int token,
                               const Cell3DPosition &mobilePos,
                               const Cell3DPosition &targetPos) const
{
    if (movingCatomRadius <= 0)
        return true;

    const long long reservationKey = makeTokenMessageKey(mobileId, token, 30);
    for (const auto &entry : movementReservations)
    {
        if (entry.first == reservationKey)
            continue;
        const auto &reservation = entry.second;
        if (movementReservationsConflict(mobilePos, targetPos,
                                         reservation.mobilePos,
                                         reservation.targetPos,
                                         movingCatomRadius))
        {
            return false;
        }
    }

    return true;
}

bool LightWalkCatoms3DBlockCode::
    hasOlderConflictingSpacingWaiter(int mobileId,
                                     const Cell3DPosition &mobilePos,
                                     const Cell3DPosition &targetPos) const
{
    if (movingCatomRadius <= 0)
        return false;

    long long currentOrder = std::numeric_limits<long long>::max();
    const auto currentWaiter = spacingWaitingRequests.find(mobileId);
    if (currentWaiter != spacingWaitingRequests.end())
    {
        currentOrder = currentWaiter->second.order;
    }

    for (const auto &entry : spacingWaitingRequests)
    {
        const auto &waiter = entry.second;
        if (waiter.mobileId == mobileId)
            continue;
        if (waiter.order >= currentOrder)
            continue;
        if (movementReservationsConflict(mobilePos, targetPos,
                                         waiter.mobilePos, waiter.targetPos,
                                         movingCatomRadius))
        {
            return true;
        }
    }

    return false;
}

bool LightWalkCatoms3DBlockCode::isWaitingForMetrics() const
{
    if (!initialized or relocated or rotating)
        return false;

    return awaitingPivotReservation or awaitingReleaseAck or moduleAwaitingGo or spacingWaitingRequests.find(catom->blockId) != spacingWaitingRequests.end();
}

void LightWalkCatoms3DBlockCode::
    reserveMovementSpacing(int mobileId, int token,
                           const Cell3DPosition &mobilePos,
                           const Cell3DPosition &targetPos)
{
    if (movingCatomRadius <= 0)
        return;

    const long long reservationKey = makeTokenMessageKey(mobileId, token, 30);
    movementReservations[reservationKey] = {mobileId, token, mobilePos, targetPos, 0};
    clearMovementSpacingWait(mobileId);
}

void LightWalkCatoms3DBlockCode::releaseMovementSpacing(int mobileId, int token)
{
    const long long reservationKey = makeTokenMessageKey(mobileId, token, 30);
    movementReservations.erase(reservationKey);
}

void LightWalkCatoms3DBlockCode::
    noteMovementSpacingDenied(int mobileId, int token,
                              const Cell3DPosition &mobilePos,
                              const Cell3DPosition &targetPos)
{
    if (movingCatomRadius <= 0)
        return;

    auto waiter = spacingWaitingRequests.find(mobileId);
    if (waiter == spacingWaitingRequests.end())
    {
        spacingWaitingRequests[mobileId] =
            {mobileId, token, mobilePos, targetPos, nextSpacingWaitingOrder++};
        return;
    }

    waiter->second.token = token;
    waiter->second.mobilePos = mobilePos;
    waiter->second.targetPos = targetPos;
}

void LightWalkCatoms3DBlockCode::clearMovementSpacingWait(int mobileId)
{
    spacingWaitingRequests.erase(mobileId);
}

void LightWalkCatoms3DBlockCode::handleReservationReply(
    int mobileId, int token, const Cell3DPosition &pivotPos,
    const Cell3DPosition &replyTargetPos, bool granted, bool spacingDenied)
{
    if (static_cast<int>(catom->blockId) != mobileId or token != activeReservationToken or not awaitingPivotReservation)
    {
        return;
    }

    awaitingPivotReservation = false;

    if (!granted or replyTargetPos != targetPos)
    {
        activeReservationToken = 0;
        requestedPivotPos = Cell3DPosition(-1, -1, -1);
        pendingMotionPivotPos = Cell3DPosition(-1, -1, -1);
        if (!spacingDenied)
        {
            clearMovementSpacingWait(catom->blockId);
        }
        retryMotionRequest(activeReservationRetryEvent, spacingDenied);
        return;
    }

    Catoms3DBlock *pivot = findMotionPivotForTarget(targetPos);
    if (!pivot or pivot->position != pivotPos)
    {
        sendReleaseToPivot(pivotPos, token, false);
        releaseMovementSpacing(catom->blockId, token);
        clearMovementSpacingWait(catom->blockId);
        activeReservationToken = 0;
        requestedPivotPos = Cell3DPosition(-1, -1, -1);
        pendingMotionPivotPos = Cell3DPosition(-1, -1, -1);
        retryMotionRequest(activeReservationRetryEvent);
        return;
    }

    reservedPivotPos = pivotPos;
    reservedMotionPivotPos = pivotPos;
    requestedPivotPos = Cell3DPosition(-1, -1, -1);
    pendingMotionPivotPos = Cell3DPosition(-1, -1, -1);
    if (targetPlanAvailable and dpConsoleEnabled)
    {
        cout << "Dp reservation granted catom #" << catom->blockId
             << " target=" << targetPos
             << " targetDp=" << dpValueForCell(targetPos)
             << " pivot=" << pivotPos
             << endl;
    }
    recordMotionStarted(catom->position, targetPos);
    rotating = true;
    scheduler->schedule(new Catoms3DRotationStartEvent(scheduler->now() + MSG_DELAY_MC,
                                                       catom, pivot, targetPos));
}

void LightWalkCatoms3DBlockCode::handleReleaseAck(int mobileId,
                                                  int token,
                                                  bool completedMotion)
{
    if (static_cast<int>(catom->blockId) != mobileId or token != activeReservationToken or not awaitingReleaseAck)
    {
        return;
    }

    awaitingReleaseAck = false;
    activeReservationToken = 0;
    releaseAckRetryCount = 0;
    releaseAckPivotPos = Cell3DPosition(-1, -1, -1);
    releaseAckCompletedMotion = false;

    if (completedMotion)
    {

        if (!flatteningPhaseActive)
        {
            acknowledgeCompletedMotion();
        }
    }

    continueAfterReleaseAck();
}

void LightWalkCatoms3DBlockCode::acknowledgeCompletedMotion()
{
    releasedPivotCount++;
    if (initialLineHead and releasedPivotCount == 1)
    {
        sendLaunchToNextLine();
    }
    if (isAssignedTargetReached() or hasSameLineSuccessorGap())
    {
        sendLaunchToCurrentLineSuccessor();
    }
}

void LightWalkCatoms3DBlockCode::continueAfterReleaseAck()
{
    if (resumeMotionAfterRelease)
    {
        resumeMotionAfterRelease = false;

        if (flatteningPhaseActive)
        {
            continueFlatteningStep();
            return;
        }

        if (targetPlanAvailable and hasAssignedTarget and isAssignedTargetReached())
        {
            markAssignedTargetFilled();
            relocated = true;
            lineStarted = true;
            catom->setColor(GREEN);
            logDpStatus("resume-assigned-target", true);
            return;
        }

        if (targetPlanAvailable and shouldSettleAtTargetCell())
        {
            relocated = true;
            lineStarted = true;
            catom->setColor(GREEN);
            logDpStatus("resume-settled-target", true);
            return;
        }

        if (targetPlanAvailable and goalCompletionReached())
        {
            relocated = true;
            lineStarted = true;
            catom->setColor(GREEN);
            logDpStatus("resume-stop", true);
            return;
        }

        if (!chooseNextMotionTarget(targetPos))
        {
            clearMovementSpacingWait(catom->blockId);
            if (targetPlanAvailable)
            {
                if (hasAssignedTarget and isAssignedTargetReached())
                {
                    markAssignedTargetFilled();
                    relocated = true;
                    lineStarted = true;
                    catom->setColor(GREEN);
                    logDpStatus("resume-assigned-complete", true);
                    return;
                }
                if (shouldSettleAtTargetCell())
                {
                    relocated = true;
                    lineStarted = true;
                    catom->setColor(GREEN);
                    logDpStatus("resume-settled-complete", true);
                    return;
                }
                if (goalCompletionReached())
                {
                    relocated = true;
                    lineStarted = true;
                    catom->setColor(GREEN);
                    logDpStatus("resume-complete", true);
                    return;
                }
            }
            else if (!hasUnfilledTargetCell())
            {
                relocated = true;
                lineStarted = true;
                catom->setColor(GREEN);
                return;
            }
            retryMotionRequest(1);
            return;
        }

        requestMotionToTarget(targetPos, 1);
        return;
    }

    if (finalizeRelocationAfterRelease)
    {
        finalizeRelocationAfterRelease = false;
        if (targetPlanAvailable)
        {
            updateFilledTargetCellsFromOccupancy();
            if (hasAssignedTarget and isAssignedTargetReached())
            {
                markAssignedTargetFilled();
                relocated = true;
            }
            else if (shouldSettleAtTargetCell())
            {
                relocated = true;
            }
            else
            {
                relocated = goalCompletionReached();
            }
            logDpStatus(relocated ? "finalized" : "finalize-not-in-goal", true);
        }
        else if (isAssignedTargetReached())
        {
            markAssignedTargetFilled();
        }
        clearMovementSpacingWait(catom->blockId);
        greenLightIsOn = true;
        moduleAwaitingGo = false;
        if (!targetPlanAvailable)
            relocated = true;
        lineStarted = true;
        catom->setColor(GREEN);
    }
}

void LightWalkCatoms3DBlockCode::sendLineLaunch(int targetLine, int launchId,
                                                int excludedId)
{
    if (targetLine == lineNumber)
    {
        seenLineLaunchMessages.insert(makeLineLaunchKey(targetLine, launchId));
    }

    for (int i = 0; i < catom->getNbInterfaces(); ++i)
    {
        P2PNetworkInterface *p2p = catom->getInterface(i);
        if (!p2p or !p2p->isConnected())
            continue;
        if (excludedId != -1 and
            static_cast<int>(p2p->getConnectedBlockId()) == excludedId)
        {
            continue;
        }

        sendMessage(new LaunchLineMessage(targetLine, launchId),
                    p2p, MSG_DELAY_MC, 0);
    }
}

void LightWalkCatoms3DBlockCode::sendLaunchToNextLine()
{
    if (nextLineLaunchSent)
        return;

    const int nextLine = getNextLineNumber();
    if (nextLine == lineNumber)
        return;

    nextLineLaunchSent = true;
    sendLineLaunch(nextLine, catom->blockId);
}

void LightWalkCatoms3DBlockCode::sendLaunchToCurrentLineSuccessor()
{
    if (sameLineSuccessorLaunchSent)
        return;

    sameLineSuccessorLaunchSent = true;
    sendLineLaunch(lineNumber, catom->blockId);
}

bool LightWalkCatoms3DBlockCode::releaseReservedPivot(bool completedMotion)
{
    const Cell3DPosition noPivot(-1, -1, -1);
    if (reservedPivotPos == noPivot and reservedMotionPivotPos == noPivot)
        return false;

    bool released = false;
    if (reservedPivotPos != noPivot)
    {
        released = sendReleaseToPivot(reservedPivotPos,
                                      activeReservationToken,
                                      completedMotion) or
                   released;
    }
    if (reservedMotionPivotPos != noPivot and reservedMotionPivotPos != reservedPivotPos)
    {
        released = sendReleaseToPivot(reservedMotionPivotPos,
                                      activeReservationToken,
                                      completedMotion) or
                   released;
    }
    if (!released)
    {
        console << "Could not release reserved pivot at "
                << reservedPivotPos << " / " << reservedMotionPivotPos
                << " from " << catom->position << "\n";
        activeReservationToken = 0;
    }
    else
    {
        releaseAckPivotPos = reservedPivotPos != noPivot ? reservedPivotPos : reservedMotionPivotPos;
        releaseAckCompletedMotion = completedMotion;
        releaseAckRetryCount = 0;
    }

    reservedPivotPos = Cell3DPosition(-1, -1, -1);
    reservedMotionPivotPos = Cell3DPosition(-1, -1, -1);
    awaitingReleaseAck = released;
    if (released)
    {
        scheduler->schedule(new InterruptionEvent<int>(
            scheduler->now() + (20 * MSG_DELAY_MC), catom,
            400000 + activeReservationToken));
    }
    return released;
}

bool LightWalkCatoms3DBlockCode::sendReleaseToPivot(const Cell3DPosition &pivotPos,
                                                    int token,
                                                    bool completedMotion)
{
    P2PNetworkInterface *directItf = catom->getInterface(pivotPos);
    if (directItf and directItf->isConnected())
    {
        sendMessage(new ReleasePivotLightMessage(catom->position,
                                                 pivotPos,
                                                 lineNumber,
                                                 catom->blockId,
                                                 token,
                                                 completedMotion),
                    directItf, MSG_DELAY_MC, 0);
        return true;
    }

    bool sent = false;
    for (int i = 0; i < catom->getNbInterfaces(); ++i)
    {
        P2PNetworkInterface *p2p = catom->getInterface(i);
        if (!p2p or !p2p->isConnected())
            continue;

        sendMessage(new ReleasePivotLightMessage(catom->position,
                                                 pivotPos,
                                                 lineNumber,
                                                 catom->blockId,
                                                 token,
                                                 completedMotion),
                    p2p, MSG_DELAY_MC, 0);
        sent = true;
    }

    return sent;
}

int LightWalkCatoms3DBlockCode::countSameLineCatomsBetweenCurrentAndHead() const
{
    int headX = std::numeric_limits<int>::max();
    auto &blocks = BaseSimulator::getWorld()->getMap();

    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb or bb->position[1] != lineNumber or bb->position[2] != ZLINE)
        {
            continue;
        }

        headX = std::min(headX, static_cast<int>(bb->position[0]));
    }

    if (headX == std::numeric_limits<int>::max() or catom->position[0] <= headX)
    {
        return 0;
    }

    int gapCount = 0;
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb or bb->position[1] != lineNumber or bb->position[2] != ZLINE)
        {
            continue;
        }

        const int blockX = bb->position[0];
        if (blockX > headX and blockX < catom->position[0])
        {
            ++gapCount;
        }
    }

    return gapCount;
}

bool LightWalkCatoms3DBlockCode::hasSameLineSuccessorGap() const
{
    return countSameLineCatomsBetweenCurrentAndHead() >= SAME_LINE_GAP_CATOMS;
}

bool LightWalkCatoms3DBlockCode::hasLeftNeighbor() const
{
    return catom->getNeighborOnCell(catom->position + Cell3DPosition(-1, 0, 0)) != NULL;
}

bool LightWalkCatoms3DBlockCode::hasReachedLineTail() const
{
    return catom->getNeighborOnCell(catom->position + Cell3DPosition(1, 0, -1)) == NULL;
}

bool LightWalkCatoms3DBlockCode::isLineTip() const
{
    return catom->getNeighborOnCell(catom->position + Cell3DPosition(1, 0, 0)) == NULL;
}

bool LightWalkCatoms3DBlockCode::isAdjacentToPosition(const Cell3DPosition &pos) const
{
    return lattice->cellsAreAdjacent(catom->position, pos);
}

bool LightWalkCatoms3DBlockCode::sendToNeighborAt(const Cell3DPosition &pos,
                                                  Message *msg)
{
    P2PNetworkInterface *itf = catom->getInterface(pos);
    if (!itf)
    {
        delete msg;
        return false;
    }

    sendMessage(msg, itf, MSG_DELAY_MC, 0);
    return true;
}

Catoms3DBlock *LightWalkCatoms3DBlockCode::
    findMotionPivotForTarget(const Cell3DPosition &targetPos) const
{
    const auto pivotLinks =
        Catoms3DMotionEngine::findPivotLinkPairsForTargetCell(catom, targetPos);

    for (const auto &pivotLink : pivotLinks)
    {
        Catoms3DBlock *pivot = pivotLink.first;
        if (pivot and pivot->position[1] == targetPos[1])
        {
            return pivot;
        }
    }

    return Catoms3DMotionEngine::findMotionPivot(catom, targetPos);
}

Catoms3DBlock *LightWalkCatoms3DBlockCode::
    findTargetLightAmongNeighbors(const Cell3DPosition &targetPos) const
{
    for (const auto &cell : lattice->getActiveNeighborCells(catom->position))
    {
        if (lattice->cellsAreAdjacent(cell, targetPos) and cell[2] == ZLINE and cell[1] == targetPos[1] and cell[0] > catom->position[0])
            return static_cast<Catoms3DBlock *>(lattice->getBlock(cell));
    }

    return NULL;
}

bool LightWalkCatoms3DBlockCode::isFlatteningMobile() const
{
    return static_cast<int>(catom->position[2]) > ZLINE;
}

bool LightWalkCatoms3DBlockCode::isFlatteningLeftHead() const
{
    if (!isFlatteningMobile()) return false;

    const int myX    = static_cast<int>(catom->position[0]);
    const int myY    = static_cast<int>(catom->position[1]);
    const int myZ    = static_cast<int>(catom->position[2]);

    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb) continue;
        if (static_cast<int>(bb->position[2]) != myZ) continue;
        if (static_cast<int>(bb->position[1]) != myY) continue;
        if (static_cast<int>(bb->position[0]) < myX)  return false;
    }
    return true;
}

long long LightWalkCatoms3DBlockCode::makeFlattenKey(int layer, int launchId) const
{
    return (static_cast<long long>(layer) << 32) ^
           static_cast<long long>(static_cast<unsigned int>(launchId));
}

void LightWalkCatoms3DBlockCode::broadcastFlattenLaunch(int targetLayer, int launchId)
{
    const long long key = makeFlattenKey(targetLayer, launchId);
    seenFlattenLaunchMessages.insert(key);

    for (int i = 0; i < catom->getNbInterfaces(); ++i)
    {
        P2PNetworkInterface *p2p = catom->getInterface(i);
        if (!p2p or !p2p->isConnected()) continue;
        sendMessage(new FlattenLayerLaunchMessage(targetLayer, launchId),
                    p2p, MSG_DELAY_MC, 0);
    }
}

void LightWalkCatoms3DBlockCode::broadcastFlattenDone(int completedLayer, int launchId)
{
    const long long key = makeFlattenKey(completedLayer, launchId);
    seenFlattenDoneMessages.insert(key);

    for (int i = 0; i < catom->getNbInterfaces(); ++i)
    {
        P2PNetworkInterface *p2p = catom->getInterface(i);
        if (!p2p or !p2p->isConnected()) continue;
        sendMessage(new FlattenLayerDoneMessage(completedLayer, launchId),
                    p2p, MSG_DELAY_MC, 0);
    }
}

void LightWalkCatoms3DBlockCode::startFlatteningLocomotion()
{
    if (flatteningLocomotionStarted) return;
    if (!isFlatteningMobile())       return;

    flatteningLocomotionStarted = true;
    catom->setColor(YELLOW);

    continueFlatteningStep();
}

void LightWalkCatoms3DBlockCode::continueFlatteningStep()
{

    if (static_cast<int>(catom->position[2]) == ZLINE)
    {
        if (!flatteningLanded)
        {
            flatteningLanded = true;
            catom->setColor(GREEN);
            lineNumber = static_cast<int>(catom->position[1]);
            checkFlatteningLayerDone();
        }
        return;
    }

    if (rotating or awaitingPivotReservation or awaitingReleaseAck) return;

    const auto reachable = Catoms3DMotionEngine::getAllReachablePositions(catom);
    const int  curZ = static_cast<int>(catom->position[2]);
    const int  curX = static_cast<int>(catom->position[0]);
    const int  curY = static_cast<int>(catom->position[1]);

    bool found = false;
    long bestScore = std::numeric_limits<long>::max();
    Cell3DPosition bestTarget(-1, -1, -1);

    for (const auto &cand : reachable)
    {
        if (!lattice->isInGrid(cand)) continue;
        if (!lattice->isFree(cand))   continue;

        const int cz = static_cast<int>(cand[2]);
        const int cx = static_cast<int>(cand[0]);
        const int cy = static_cast<int>(cand[1]);

        if (cz < ZLINE) continue;

        if (cz > curZ)  continue;

        if (cy != curY) continue;

        long score = 0;

        if (cz < curZ)
        {

            score = 1000L * (cz - ZLINE);
        }
        else
        {

            if (cx <= curX)
            {

                continue;
            }

            score = 500L + static_cast<long>(cx - curX);
        }

        if (!found or score < bestScore)
        {
            found     = true;
            bestScore = score;
            bestTarget = cand;
        }
    }

    if (!found)
    {

        retryMotionRequest(10);
        return;
    }

    requestMotionToTarget(bestTarget, 10);
}

void LightWalkCatoms3DBlockCode::checkFlatteningLayerDone()
{
    if (flatteningLayerReported) return;

    const int myOriginalLayer = static_cast<int>(initialPosition[2]);

    auto &blocks = BaseSimulator::getWorld()->getMap();
    for (const auto &entry : blocks)
    {
        auto *bb = static_cast<Catoms3DBlock *>(entry.second);
        if (!bb) continue;
        auto *bc = dynamic_cast<LightWalkCatoms3DBlockCode *>(bb->blockCode);
        if (!bc or !bc->initialized) continue;

        if (static_cast<int>(bc->initialPosition[2]) != myOriginalLayer) continue;

        if (!bc->flatteningLanded) return;
    }

    flatteningLayerReported = true;
    broadcastFlattenDone(myOriginalLayer, static_cast<int>(catom->blockId));
}
