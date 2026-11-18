#ifndef ShapeDecompositionBlockCode_H_
#define ShapeDecompositionBlockCode_H_

#include <set>
#include <bitset>
#include "inequality2D.hpp"
#include "polygon.hpp"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include "robots/blinkyBlocks/blinkyBlocksWorld.h"

static const int BORDER_MSG_ID = 1000;
static const int BRIDGE_MSG_ID = 1001;
static const int GET_BORDER_MSG_ID = 1003;
static const int TRACE_MSG_ID = 1002;
static const int TRACE_BRIDGES_MSG_ID = 1004;

using namespace BlinkyBlocks;

enum class Direction { Front, Right, Back, Left };
struct BitsetComparator {
    bool operator()(const std::bitset<8>& lhs, const std::bitset<8>& rhs) const {
        return lhs.to_ulong() < rhs.to_ulong();
    }
};

// Custom comparison function for std::vector<std::bitset<8>>
struct VectorBitsetComparator {
    bool operator()(const std::vector<std::bitset<8>>& lhs, const std::vector<std::bitset<8>>& rhs) const {
        // Compare based on the size first, then lexicographically
        if (lhs.size() != rhs.size()) {
            return lhs.size() < rhs.size();
        }
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), BitsetComparator());
    }
};

struct Corner {
    Cell3DPosition position;
    Direction prevDir;
    Direction nextDir;

    Corner(Cell3DPosition pos, Direction prev, Direction next)
        : position(pos), prevDir(prev), nextDir(next) {}

    Corner() = default;

    bool operator==(const Corner &other) const {
        return position == other.position && prevDir == other.prevDir && nextDir == other.nextDir;
    }
};

struct Border {
    short initiatorId;
    vector <Direction> directions;
    vector <Corner> corners;

    Border(short id, vector<Direction> dirs, vector<Corner> c) : initiatorId(id), directions(dirs), corners(c) {}

    Border() = default;
};

struct BorderMessageData {
    Corner prevCorner;
    Direction dir;
    Border border;
    vector<Direction> directions;
    vector <Corner> corners;
    short initiatorId;

    BorderMessageData(Corner prev, Direction d, short id, Border b, vector<Direction> dirs, vector<Corner> c)
        : prevCorner(prev), dir(d), initiatorId(id), border(b), directions(dirs), corners(c) {}

    BorderMessageData() = default;
    
};


struct BridgeMessageData {
    int distance;
    int initiatorId;
    bool selected;
    BridgeMessageData(int dist, int id,  bool sel) : distance(dist), initiatorId(id), selected(sel) {}
    BridgeMessageData() = default;

    bool operator==(const BridgeMessageData &other) const {
        return distance == other.distance && initiatorId == other.initiatorId && selected == other.selected;
    }
};

struct TraceMessageData {
    int initiatorId;
    int round;

    TraceMessageData(int id, int r) : initiatorId(id), round(r) {} 

    TraceMessageData() = default;

    bool operator==(const TraceMessageData &other) const {
        return initiatorId == other.initiatorId && round == other.round;
    }
};

enum class TraceBridgesMessageType { IINITIATOR_TO_FIRST_BORDER, BORDER_TRACE, FIRST_BORDER_TO_BRIDGE, BACK_TO_FIRST_BORDER, BACK_TO_INITIATOR };

struct TraceBridgesMessageData {
    int initiatorId;
    int borderInitiatorId;
    int toBorderId{-1};
    TraceBridgesMessageType type;
    TraceBridgesMessageData(int id, int bId, TraceBridgesMessageType t)
        : initiatorId(id), borderInitiatorId(bId), type(t) {}
    TraceBridgesMessageData(int id, int bId, TraceBridgesMessageType t, int toBId)
        : initiatorId(id), borderInitiatorId(bId), type(t), toBorderId(toBId) {}
    TraceBridgesMessageData() = default;

    bool operator==(const TraceBridgesMessageData& other) const {
        return initiatorId == other.initiatorId && borderInitiatorId == other.borderInitiatorId;
    }
};

/**
 * @brief BlockCode class for the ShapeDecomposition application
 */
class ShapeDecompositionBlockCode : public BlinkyBlocksBlockCode {
   private:
    int distance;
    BlinkyBlocksBlock *module;
    bool isCorner{false};
    bool onBorder{false};
    bool isBridge{false};
    bool traceIsActive{false};
    int traceRound{0};
    map<Direction, short> borderInitiators;
    map<short, vector<Direction>> initiatorDirections; 
    vector<Direction> traceDirs;
    map<int, pair<SCLattice::myDirection, SCLattice::myDirection>> initiator_prevNext;
    SCLattice::myDirection nextInBorder{SCLattice::myDirection::NO_DIRECTION};
    SCLattice::myDirection prevInBorder{SCLattice::myDirection::NO_DIRECTION};
    vector<Direction> prevDirections, nextDirections;

   public:
    ShapeDecompositionBlockCode(BlinkyBlocksBlock *host);
    ~ShapeDecompositionBlockCode() {};
    std::vector<Corner> sortCornersByDirection();
    static array<SCLattice::myDirection, 4> DIRECTIONS;
    static array<string, 4> DIRECTIONS_NAMES;
    static vector<Corner> corners;
    static vector<int> recordedTimes;
    static vector<int> recordedNbMessages;
    static int nbTraces;
    static int nbCorners;
    Corner myCorner;
    Corner prevCorner;
    Border border;
    bool isInitiator{false};
    static map<Cell3DPosition, int> intersectionsCount;

    map<int, P2PNetworkInterface *> bridgesIn;
    map<int, vector<P2PNetworkInterface *>> bridgeOut;
    map<int, int> nbWaitedBridges;
    map<int, int> bridgesDistance;
    set<int> mustRemove;

    // border tracing variables
    map<int, P2PNetworkInterface *> traceIn;
    map<int, P2PNetworkInterface *> traceOut;
    vector<Cell3DPosition> latestCornersForSegment;


    array<P2PNetworkInterface *, 2> bridgeEnds{nullptr, nullptr};
    vector<vector<Corner>> getBorders();
    short prevTraceInitiatorId{-1};
    bool areColinear(Cell3DPosition &p1, Cell3DPosition &p2, Cell3DPosition &p3);
    bool isOnBresenhamLine(Cell3DPosition &p1, Cell3DPosition &p2, Cell3DPosition &p3);
    bool isOnLine(Cell3DPosition &p1, Cell3DPosition &p2, Cell3DPosition &p3);
    bool isIn(Cell3DPosition &pos, vector<vector<Corner>> &borders);

    vector<int> compress(vector<vector<Corner>> &borders);
    vector<vector<Corner>> decompress(vector<int> &compressed);

    [[maybe_unused]] bool isOnBorder() const;

    [[maybe_unused]] bool isFrontmost() const;
    [[maybe_unused]] bool isBackmost() const;
    [[maybe_unused]] bool isTopmost() const;
    [[maybe_unused]] bool isBottommost() const;
    [[maybe_unused]] bool isLeftmost() const;
    [[maybe_unused]] bool isRightmost() const;

    bool isExternalCorner() const;
    bool isInternalCorner() const;
    bool isBridgeEnd() const;
    vector<Cell3DPosition> bresenhamLine(Cell3DPosition &p1, Cell3DPosition &p2);
    void printResults(vector<Direction> &receivedDirections, vector<Corner> &receivedCorners);
    /**
     * This function is called on startup of the blockCode, it can be used to perform initial
     *  configuration of the host or this instance of the program.
     * @note this can be thought of as the main function of the module
     **/
    void startup() override;

    /**
     * @brief Handler for all events received by the host block
     * @param pev pointer to the received event
     */
    void processLocalEvent(EventPtr pev) override;

    /**
     * @brief Callback function executed whenever the module finishes a motion
     */
    void onMotionEnd() override;

    /**
     * @brief Sample message handler for this instance of the blockcode
     * @param _msg Pointer to the message received by the module, requires casting
     * @param sender Connector of the module that has received the message and that is connected to
     * the sender */
    void handleBorderMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    bool rayIntersectsSegment(const Cell3DPosition& P, const Cell3DPosition& A,
                              const Cell3DPosition& B);
    void handleBridgeMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface* sender);
    void handleGetBorderMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleTraceBridgesMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    vector<int> RLEcompress(vector<Direction> &directions);
    pair<vector<int>, int> LZWcompress(vector<Direction> &directions);
    pair<vector<int>, int> LZWcompressCorners(vector<Corner>& corners);
    vector<Direction> LZWdecompress(vector<int>& compressed);

    pair<vector<int>, int>  LZWcompressBitset(vector<bitset<8>> &directions);
    // void handleBridgeMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    // void handleTraceMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);

    short getSenderDirection(P2PNetworkInterface *sender);

    P2PNetworkInterface *getInterfaceInDirection(SCLattice::myDirection dir);

    vector<Cell3DPosition> findIntersections(const vector<set<Inequality2D>> &inequalities);
    
    void initiateBorderTracing();
    /// Advanced blockcode handlers below

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used
     * to read additional user information from it.
     * @param config : pointer to the TiXmlDocument representing the configuration file, all
     * information related to VisibleSim's core have already been parsed
     *
     * Called from BuildingBlock constructor, only once.
     */
    void parseUserElements(TiXmlDocument *config) override {}

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used
     * to read additional user information from each block config. Has to be overriden in the child
     * class.
     * @param config : pointer to the TiXmlElement representing the block configuration file, all
     * information related to concerned block have already been parsed
     *
     */
    void parseUserBlockElements(TiXmlElement *config) override {}

    /**
     * User-implemented debug function that gets called when a module is selected in the GUI
     */
    void onBlockSelected() override;

    /**
     * User-implemented debug function that gets called when a VS_ASSERT is triggered
     * @note call is made from utils::assert_handler()
     */
    void onAssertTriggered() override;

    /**
     * User-implemented keyboard handler function that gets called when
     *  a key press event could not be caught by openglViewer
     * @param c key that was pressed (see openglViewer.cpp)
     * @param x location of the pointer on the x axis
     * @param y location of the pointer on the y axis
     * @note call is made from GlutContext::keyboardFunc (openglViewer.h)
     */
    void onUserKeyPressed(unsigned char c, int x, int y) override {}

    /**
     * Call by world during GL drawing phase, can be used by a user
     *  to draw custom Gl content into the simulated world
     * @note call is made from World::GlDraw
     */
    void onGlDraw() override {}

    /**
     * @brief This function is called when a module is tapped by the user. Prints a message to the
     console by default. Can be overloaded in the user blockCode
     * @param face face that has been tapped */
    void onTap(int face) override {}

    /**
     * User-implemented keyboard handler function that gets called when
     *  a key press event could not be caught by openglViewer
     * @note call is made from GlutContext::keyboardFunc (openglViewer.h)
     */
    bool parseUserCommandLineArgument(int &argc, char **argv[]) override;

    /**
     * Called by openglviewer during interface drawing phase, can be used by a user
     *  to draw a custom Gl string onto the bottom-left corner of the GUI
     * @note call is made from OpenGlViewer::drawFunc
     * @return a string (can be multi-line with `\n`) to display on the GUI
     */
    string onInterfaceDraw() override;

    /*****************************************************************************/
    /** needed to associate code to module                                      **/
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new ShapeDecompositionBlockCode((BlinkyBlocksBlock *)host));
    };
    /*****************************************************************************/
};

#endif /* ShapeDecompositionBlockCode_H_ */
