#ifndef LIGHTWALKCATOMS3DBLOCKCODE_H_
#define LIGHTWALKCATOMS3DBLOCKCODE_H_

#include <fstream>
#include <limits>
#include <queue>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "robots/catoms3D/catoms3DBlockCode.h"
#include "robots/catoms3D/catoms3DSimulator.h"
#include "robots/catoms3D/catoms3DMotionRules.h"
#include "robots/catoms3D/catoms3DRotationEvents.h"
#include "robots/catoms3D/catoms3DBlock.h"
#include "math/cell3DPosition.h"

#include "lightWalkCatoms3DMessages.hpp"

class LightWalkCatoms3DBlockCode : public Catoms3D::Catoms3DBlockCode {
private:
public:
    Scheduler *scheduler = nullptr;
    World *world = nullptr;
    Lattice *lattice = nullptr;
    Catoms3D::Catoms3DBlock *catom = nullptr;

    bool initialized = false;

    LightWalkCatoms3DBlockCode(Catoms3D::Catoms3DBlock *host);
    ~LightWalkCatoms3DBlockCode();

    void processReceivedMessage(MessagePtr msg, P2PNetworkInterface* sender);

    void startup() override;
    void processLocalEvent(EventPtr pev) override;
    void onBlockSelected() override;
    std::string onInterfaceDraw() override;


    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new LightWalkCatoms3DBlockCode((Catoms3DBlock*)host));
    }

    static const int ZLINE;
    static const int SAME_LINE_GAP_CATOMS;
    static const int TARGET_TRANSIT_Z;
    static const int METRICS_SAMPLE_EVENT;

    static bool targetPlanInitialized;
    static bool targetPlanAvailable;
    static bool targetPlanUsesLineMapping;
    static bool targetFillHighXFirst;
    static bool dpGoalBoxConfigured;
    static Cell3DPosition dpGoalMin;
    static Cell3DPosition dpGoalMax;
    static bool dpGoalBoxSetFromCommandLine;
    static double dpGoalFillPercent;
    static bool dpGoalFillPercentSetFromCommandLine;
    static bool dpConsoleEnabled;
    static std::map<int, int> targetLineForSourceLine;
    static std::map<int, std::vector<Cell3DPosition>> targetCellsByLine;
    static std::vector<Cell3DPosition> targetCellsByGlobalOrder;
    static std::map<Cell3DPosition, long long> reservedTargetCells;
    static std::map<Cell3DPosition, int> claimedTargetCells;
    static std::set<Cell3DPosition> filledTargetCells;
    struct MovementReservation {
        int mobileId;
        int token;
        Cell3DPosition mobilePos;
        Cell3DPosition targetPos;
        long long order = 0;
    };
    static int movingCatomRadius;
    static bool movingCatomRadiusSetFromCommandLine;
    static long long nextSpacingWaitingOrder;
    static std::map<long long, MovementReservation> movementReservations;
    static std::map<int, MovementReservation> spacingWaitingRequests;
    static bool metricsEnabled;
    static bool metricsCsvPathSetFromCommandLine;
    static bool metricsSamplePeriodSetFromCommandLine;
    static std::string metricsCsvPath;
    static Time metricsSamplePeriodUs;
    static bool metricsSamplingStarted;
    static bool metricsCompleted;
    static bID metricsOwnerId;
    static Time metricsStartTime;
    static long long metricsMessagesSent;
    static long long metricsMessagesReceived;
    static long long metricsMotionRequests;
    static long long metricsMotionsStarted;
    static long long metricsMotionsCompleted;
    static long long metricsMotionDistance;
    static long long metricsMaxMotionDistance;
    static long long metricsSpacingDenials;
    static long long metricsPivotBusyDenials;
    static long long metricsTargetCellDenials;
    static long long metricsReservationTimeouts;
    static long long metricsReleaseAckTimeouts;
    static long long metricsReleaseRetries;
    static long long metricsTargetPlanningWallTimeUs;
    static int metricsMaxMovingModules;
    static int metricsMaxWaitingModules;
    static bool metricsInitialGraphCaptured;
    static int metricsInitialGraphNodes;
    static int metricsInitialGraphEdges;
    static int metricsInitialConnectedComponents;
    static int metricsInitialConfigurationDiameterCells;
    static int metricsInitialBottleneckCandidates;
    static std::vector<Cell3DPosition> metricsInitialCells;
    static std::ofstream metricsCsv;

    static bool flatteningPhaseActive;
    static bool flatteningPhaseComplete;
    static int  flatteningTotalLayers;
    static int  flatteningCompletedLayers;
    static std::unordered_set<int> flatteningDoneLayers;
    static std::unordered_set<long long> seenFlattenLaunchMessages;
    static std::unordered_set<long long> seenFlattenDoneMessages;

    bool flatteningLocomotionStarted = false;
    bool flatteningLanded            = false;
    bool flatteningLayerReported     = false;

    bool greenLightIsOn = true;
    bool moduleAwaitingGo = false;
    Cell3DPosition awaitingModulePos = Cell3DPosition(-1, -1, -1);
    Cell3DPosition actuationTargetPos;
    Cell3DPosition reservedPivotPos = Cell3DPosition(-1, -1, -1);
    Cell3DPosition reservedMotionPivotPos = Cell3DPosition(-1, -1, -1);
    Cell3DPosition pendingMotionPivotPos = Cell3DPosition(-1, -1, -1);

    bool relocated;
    bool rotating;
    bool lineStarted = false;
    bool initialLineHead = false;
    bool nextLineLaunchSent = false;
    bool sameLineSuccessorLaunchSent = false;
    bool awaitingPivotReservation = false;
    bool awaitingReleaseAck = false;
    bool resumeMotionAfterRelease = false;
    bool finalizeRelocationAfterRelease = false;
    int releasedPivotCount = 0;
    int lineNumber = 0;
    int initialLineOrder = 0;
    int initialGlobalOrder = 0;
    int nextReservationToken = 1;
    int activeReservationToken = 0;
    int activeReservationRetryEvent = 0;
    int reservedForMobileId = -1;
    int reservedForToken = 0;
    int releaseAckRetryCount = 0;
    Cell3DPosition reservedTargetPos = Cell3DPosition(-1, -1, -1);
    Cell3DPosition targetPos;
    Cell3DPosition initialPosition = Cell3DPosition(-1, -1, -1);
    Cell3DPosition assignedTargetPos = Cell3DPosition(-1, -1, -1);
    bool hasAssignedTarget = false;
    Cell3DPosition requestedPivotPos = Cell3DPosition(-1, -1, -1);
    Cell3DPosition releaseAckPivotPos = Cell3DPosition(-1, -1, -1);
    bool releaseAckCompletedMotion = false;
    int dpSequence = 1;
    int lastPrintedDpValue = std::numeric_limits<int>::min();
    std::map<Cell3DPosition, int> knownDpValues;
    std::unordered_set<long long> seenLineLaunchMessages;
    std::unordered_set<long long> seenProbeMessages;
    std::unordered_set<long long> seenReleaseMessages;
    std::unordered_set<long long> seenReservationRequestMessages;
    std::unordered_set<long long> seenReservationReplyMessages;
    std::unordered_set<long long> seenReservationAckMessages;

    void parseUserElements(TiXmlDocument *config) override;
    bool parseUserCommandLineArgument(int& argc, char **argv[]) override;
    int sendMessage(HandleableMessage *msg, P2PNetworkInterface *dest,
                    Time t0, Time dt) override;
    using Catoms3D::Catoms3DBlockCode::sendMessage;
    bool isLineHead() const;
    bool isTopLineHead() const;
    static void setMovingCatomRadius(int radius);
    static void setDpGoalBox(const Cell3DPosition& minCorner,
                             const Cell3DPosition& maxCorner);
    static void setDpGoalFillPercent(double percent);
    static void setMetricsCsvPath(const std::string& path);
    static void setMetricsSamplePeriod(Time period);
    static void setMetricsEnabled(bool enabled);
    static void startMetricsSamplingIfNeeded(Catoms3DBlock *owner,
                                             Scheduler *scheduler);
    static void sampleMetrics(Scheduler *scheduler, const std::string& rowKind);
    static void finalizeMetrics();
    static void recordMotionRequest();
    static void recordMotionStarted(const Cell3DPosition& from,
                                    const Cell3DPosition& to);
    static void recordMotionCompleted();
    static void recordReservationDenied(bool targetCellAvailable,
                                        bool spacingDenied);
    static void recordMessageReceived();
    static bool metricsSimulationComplete();
    static void initializeTargetPlanIfNeeded(Lattice *lattice);
    static bool compareTargetFillOrder(const Cell3DPosition& lhs,
                                       const Cell3DPosition& rhs);
    static void updateFilledTargetCellsFromOccupancy();
    static int countInitializedModules();
    static int countModulesInsideGoal();
    static int requiredGoalOccupancyCount();
    static bool goalCompletionReached();
    int computeInitialLineOrder() const;
    int computeInitialGlobalOrder() const;
    void assignTargetFromPlan();
    bool isUsingTargetPlan() const;
    bool isAssignedTargetReached() const;
    bool chooseNextMotionTarget(Cell3DPosition& nextTarget);
    bool chooseNextTargetPlanStep(Cell3DPosition& nextTarget) const;
    bool chooseNextStraightWalkStep(Cell3DPosition& nextTarget) const;
    int distanceToAssignedTarget(const Cell3DPosition& pos) const;
    bool isTargetCell(const Cell3DPosition& pos) const;
    bool isTargetFilled(const Cell3DPosition& pos) const;
    bool shouldSettleAtTargetCell() const;
    bool configurationStaysConnectedAfterMove(const Cell3DPosition& targetPos) const;
    /// Paper's parallel-actuation mobility test (Fitch & Butler 2008, Section 4):
    /// a bounded local connecting-cycle search. Returns true if this module is not
    /// an articulation point of the robot graph -- i.e. its neighbours stay
    /// mutually connected through a short path that avoids this cell -- so it may
    /// leave without disconnecting the structure. Constant-time in dense
    /// configurations; conservative (returns false) if the search bound is hit.
    bool isMobileByConnectingCycle() const;
    bool hasUnfilledTargetCell() const;
    bool targetHasSupport(const Cell3DPosition& pos) const;
    bool claimNextTargetCell();
    void releaseAssignedTargetClaim();
    void markAssignedTargetFilled();
    bool chooseNextDpStep(Cell3DPosition& nextTarget);
    int dpValueForCell(const Cell3DPosition& pos) const;
    int computeDpValueFromGoal(const Cell3DPosition& pos) const;
    int distanceToGoalRegion(const Cell3DPosition& pos) const;
    void seedLocalDpValues();
    void broadcastCurrentDpValue(const std::string& reason);
    void handleDpValueMessage(int originId, int sequence,
                              const Cell3DPosition& cell, int dpValue);
    void logDpStatus(const std::string& reason, bool force = false);
    bool shouldRunDpLocomotion() const;
    bool isDpFrontierCandidate() const;
    int getNextLineNumber() const;
    long long makeLineLaunchKey(int targetLine, int launchId) const;
    long long makeTokenMessageKey(int mobileId, int token, int tag) const;
    long long makePositionMessageKey(const Cell3DPosition& a,
                                     const Cell3DPosition& b,
                                     int tag) const;
    void startLineLocomotion();
    void requestMotionToTarget(const Cell3DPosition& nextTarget, int retryEvent);
    void retryMotionRequest(int retryEvent, bool spacingBackoff = false);
    bool movementSpacingIsAvailable(int mobileId, int token,
                                    const Cell3DPosition& mobilePos,
                                    const Cell3DPosition& targetPos) const;
    bool hasOlderConflictingSpacingWaiter(int mobileId,
                                          const Cell3DPosition& mobilePos,
                                          const Cell3DPosition& targetPos) const;
    bool isWaitingForMetrics() const;
    void reserveMovementSpacing(int mobileId, int token,
                                const Cell3DPosition& mobilePos,
                                const Cell3DPosition& targetPos);
    void releaseMovementSpacing(int mobileId, int token);
    void noteMovementSpacingDenied(int mobileId, int token,
                                   const Cell3DPosition& mobilePos,
                                   const Cell3DPosition& targetPos);
    void clearMovementSpacingWait(int mobileId);
    void handleReservationReply(int mobileId, int token,
                                const Cell3DPosition& pivotPos,
                                const Cell3DPosition& replyTargetPos,
                                bool granted,
                                bool spacingDenied);
    void handleReleaseAck(int mobileId, int token, bool completedMotion);
    void acknowledgeCompletedMotion();
    void continueAfterReleaseAck();
    void sendLineLaunch(int targetLine, int launchId, int excludedId = -1);
    void sendLaunchToNextLine();
    void sendLaunchToCurrentLineSuccessor();
    bool releaseReservedPivot(bool completedMotion);
    bool sendReleaseToPivot(const Cell3DPosition& pivotPos,
                            int token,
                            bool completedMotion);
    int countSameLineCatomsBetweenCurrentAndHead() const;
    bool hasSameLineSuccessorGap() const;
    bool hasLeftNeighbor() const;
    bool hasReachedLineTail() const;
    bool isLineTip() const;
    bool isAdjacentToPosition(const Cell3DPosition& pos) const;
    bool sendToNeighborAt(const Cell3DPosition& pos, Message *msg);
    Catoms3DBlock* findMotionPivotForTarget(const Cell3DPosition& targetPos) const;

    Catoms3DBlock* findTargetLightAmongNeighbors(const Cell3DPosition& targetPos) const;
    void setGreenLightAndResumeFlow();

    bool isFlatteningMobile() const;
    bool isFlatteningLeftHead() const;
    void startFlatteningLocomotion();
    void continueFlatteningStep();
    void checkFlatteningLayerDone();
    void broadcastFlattenLaunch(int targetLayer, int launchId);
    void broadcastFlattenDone(int completedLayer, int launchId);
    long long makeFlattenKey(int layer, int launchId) const;

};

#endif
