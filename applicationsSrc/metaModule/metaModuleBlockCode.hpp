#ifndef MetaModuleBlockCode_H_
#define MetaModuleBlockCode_H_

#include "robots/catoms3D/catoms3DWorld.h"
#include "robots/catoms3D/catoms3DBlockCode.h"
#include "operations.hpp"

static const int SETCOORDINATOR_MSG_ID = 1000;
static const int COORDINATE_MSG_ID = 1003;
static const int COORDINATEBACK_MSG_ID = 1004;

static const int IT_MODE_FINDING_PIVOT = 2000;

static const int PLS_MSG_ID = 1005;
static const int GLO_MSG_ID = 1006;
static const int FTR_MSG_ID = 1007;

using namespace Catoms3D;



// enum MovingState{IN_POSITION, OPENING, MOVING, WAITING};

static vector<Cell3DPosition> FrontBackMM = {
    Cell3DPosition(0, 0, 0), Cell3DPosition(1, 0, 0), Cell3DPosition(1, 0, 1), Cell3DPosition(2, 1, 2), Cell3DPosition(1, 0, 3),
     Cell3DPosition(1, 0, 4), Cell3DPosition(0, 0, 4), Cell3DPosition(-1, -1, 3), Cell3DPosition(-1, -1, 2), Cell3DPosition(-1, -1, 1)};

static vector<Cell3DPosition> BackFrontMM = {
    Cell3DPosition(0, 0, 0), Cell3DPosition(1, 0, 0), Cell3DPosition(1, -1, 1), Cell3DPosition(2, -1, 2), Cell3DPosition(1, -1, 3),
     Cell3DPosition(1, 0, 4), Cell3DPosition(0, 0, 4), Cell3DPosition(-1, 0, 3), Cell3DPosition(-1, 1, 2), Cell3DPosition(-1, 0, 1)};

static vector<Cell3DPosition> FillingPositions_FrontBack = {
    Cell3DPosition(-1, 0, 3), Cell3DPosition(0, 0, 3), Cell3DPosition(-1, 0, 2), Cell3DPosition(-1, 0, 1), Cell3DPosition(0, 0, 1),
    Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), Cell3DPosition(0, -1, 3)};

static vector<Cell3DPosition> FillingPositions_BackFront = {
    Cell3DPosition(1, 0, 3),
    Cell3DPosition(0, 0, 3),
    Cell3DPosition(2, 0, 2),
    Cell3DPosition(1, 0, 1),
    Cell3DPosition(0, 0, 1),
    Cell3DPosition(0, -1, 1),
    Cell3DPosition(-1, -1, 1),
    Cell3DPosition(-1, 0, 2),
    Cell3DPosition(-1, -1, 3),
    Cell3DPosition(0, -1, 3),
};

static vector<Cell3DPosition> OpenedPositions = {
    Cell3DPosition(-1, 0, 0), Cell3DPosition(-1, 0, 4), Cell3DPosition(2, 0, 0), Cell3DPosition(2, 0, 4)};

static const int initialMap[][4] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {-1, 0, 0, 0},
    {-2,0,0,0}
    // {0,-1,0,1}, {0,-1,1,1},
    //{0, 0, 1, 0},
    // {1, 0, 1, 0},
    //{0, 1, 0, 1}, {0,1,1,1}
    // {1, 1, 0, 1},
    // //  {0,-1,0,1}, {1,1,-1,1},
    //{0, 0, -1, 1},
    // {0, 1, 1, 1},
};

// static int initialMap[][4] = {
//     {0,0,0,0}, {1,0,0,0}, {-1,0,0,0},
//     {0,-1,0,0}, {0,1,0,0}, {0,0,-1,0}, {0,0,1,0},
//     {-1,-1,-1,0}, {-1,0,-1,0}, {-1,1,-1,0},
//     {0,-1,-1,0}, {0,1,-1,0},
//     {-1,-1,1,0}, {-1,0,1,0}, {-1,1,1,0},
//     // {-1,-1,2,0}, {-1,0,2,0}, {-1,1,2,0},
//     {-1,-1,0,0}, {-1,1,0,0},
//     {1,-1,-1,0},  {1,0,-1,0},  {1,1,-1,0},
//     {1,-1,0,0}, {1,1,0,0},
//     {0,-1,1,0}, {0,1,1,0},
//     {1,-1,1,0}, {1,0,1,0}, {1,1,1,0}
// };

static Catoms3DBlock *seed;

class MetaModuleBlockCode : public Catoms3DBlockCode
{
private:

    
public:
    MMShape shapeState;
    Cell3DPosition MMPosition;
    Cell3DPosition seedPosition;
    Cell3DPosition initialPosition;
    Catoms3DBlock *module;
    bool isCoordinator{false};
    bool opening{false};
    Movement currentMovement{NO_MOVEMENT};
    MMOperation  nextOperation{NO_OPERATION};
    MovingState movingState{MovingState::IN_POSITION};
    int mvt_it{0};
    vector<LocalMovement> *localRules{nullptr};
    int movingSteps{0};
    Cell3DPosition coordinatorPosition;
    Cell3DPosition targetSeed;
    bool waiting{false};
    bool rotating{false};
    int transferCount{0};
    bool greenLightIsOn{true};
    bool moduleAwaitingGo{false};
    bool notFindingPivot{false};
    Cell3DPosition pivotPosition;
    Cell3DPosition awaitingModulePos;
    P2PNetworkInterface* awaitingModuleProbeItf{NULL};
    bool initialized{false};
    int *fillingPos[3];

    Operation *operation = new Operation();

    MetaModuleBlockCode(Catoms3DBlock *host);
    ~MetaModuleBlockCode(){};

    inline static Time getRoundDuration() {
        return (Catoms3DRotation::ANIMATION_DELAY * BaseSimulator::motionDelayMultiplier
                + Catoms3DRotation::COM_DELAY) + 20128;// + (getScheduler()->now() / 1000);
    }

    MMShape getShapeState() { return shapeState; };
    void setShapeState(MMShape state) { shapeState = state; };
    

    Catoms3DBlock *getModule() { return module; };
    Cell3DPosition getMMPosition() { return MMPosition; };

    void updateState();
    bool isInMM(Cell3DPosition &neighborPosition);
    Cell3DPosition nearestPositionTo(Cell3DPosition &targetPosition, P2PNetworkInterface *except = nullptr);
    //void setLocalRules(Movement movement);
    void setCoordinator(MMOperation op) ;
    Cell3DPosition nextInBorder(P2PNetworkInterface* sender);

    void probeGreenLight();
    bool isAdjacentToPosition(const Cell3DPosition& pos) const;
    static Catoms3DBlock* customFindMotionPivot(const Catoms3DBlock* m,
                                                const Cell3DPosition& tPos,
                                                RotationLinkType faceReq = RotationLinkType::Any);
    Catoms3DBlock* findTargetLightAmongNeighbors(const Cell3DPosition& targetPos,
                                                const Cell3DPosition& srcPos,
                                                P2PNetworkInterface *sender=NULL) const;
    /**
     * @return module's position relative to the meta-module seedPosition
    **/
    inline Cell3DPosition relativePos() {
        return module->position - seedPosition;
    }
    bool setGreenLight(bool onoff);

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
     * @param sender Connector of the module that has received the message and that is connected to the sender */
    void handleSetCoordinatorMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleCoordinateMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleCoordinateBackMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);

    void handlePLSMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleGLOMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleFTRMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    /// Advanced blockcode handlers below

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from it.
     * @param config : pointer to the TiXmlDocument representing the configuration file, all information related to VisibleSim's core have already been parsed
     *
     * Called from BuildingBlock constructor, only once.
     */
    void parseUserElements(TiXmlDocument *config) override {};

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from each block config. Has to be overriden in the child class.
     * @param config : pointer to the TiXmlElement representing the block configuration file, all information related to concerned block have already been parsed
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
    void onUserKeyPressed(unsigned char c, int x, int y) override;

    /**
     * Call by world during GL drawing phase, can be used by a user
     *  to draw custom Gl content into the simulated world
     * @note call is made from World::GlDraw
     */
    void onGlDraw() override {}

    /**
     * @brief This function is called when a module is tapped by the user. Prints a message to the console by default.
     Can be overloaded in the user blockCode
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
    static BlockCode *buildNewBlockCode(BuildingBlock *host)
    {
        return (new MetaModuleBlockCode((Catoms3DBlock *)host));
    };
    /*****************************************************************************/
};

#endif /* MetaModuleBlockCode_H_ */
