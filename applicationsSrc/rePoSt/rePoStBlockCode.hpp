#ifndef MetaModuleBlockCode_H_
#define MetaModuleBlockCode_H_

#include "robots/catoms3D/catoms3DWorld.h"
#include "robots/catoms3D/catoms3DBlockCode.h"
#include "operations.hpp"
#include <set>

using namespace Catoms3D;

static const int IT_MODE_FINDING_PIVOT = 2000;
static const int IT_MODE_TRANSFERBACK = 2001;
static const int IT_MODE_TRANSFERBACK_REACHCOORDINATOR = 2002;
static const int IT_MODE_TERMINATION = 2003;
static const int IT_MODE_STARTWAVE = 2004;
static const int IT_MODE_NBMOVEMENTS = 2009;

static vector<Cell3DPosition> FrontBackMM = {Cell3DPosition(0, 0, 0),   Cell3DPosition(1, 0, 0),
                                             Cell3DPosition(1, 0, 1),   Cell3DPosition(2, 1, 2),
                                             Cell3DPosition(1, 0, 3),   Cell3DPosition(1, 0, 4),
                                             Cell3DPosition(0, 0, 4),   Cell3DPosition(-1, -1, 3),
                                             Cell3DPosition(-1, -1, 2), Cell3DPosition(-1, -1, 1)};

static vector<Cell3DPosition> BackFrontMM = {Cell3DPosition(0, 0, 0),  Cell3DPosition(1, 0, 0),
                                             Cell3DPosition(1, -1, 1), Cell3DPosition(2, -1, 2),
                                             Cell3DPosition(1, -1, 3), Cell3DPosition(1, 0, 4),
                                             Cell3DPosition(0, 0, 4),  Cell3DPosition(-1, 0, 3),
                                             Cell3DPosition(-1, 1, 2), Cell3DPosition(-1, 0, 1)};

static vector<Cell3DPosition> FillingPositions_FrontBack_Zodd = {
    Cell3DPosition(0, 1, 2),  Cell3DPosition(1, 1, 2), Cell3DPosition(-1, 1, 2),
    Cell3DPosition(-1, 0, 1), Cell3DPosition(0, 0, 1), Cell3DPosition(0, -1, 1),
    Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3),
    Cell3DPosition(0, -1, 3)
};

static vector<Cell3DPosition> FillingPositions_BackFront_Zodd = {
    Cell3DPosition(1, 1, 2),   Cell3DPosition(0, 1, 2),  Cell3DPosition(2, 0, 2),
    Cell3DPosition(1, 0, 1),   Cell3DPosition(0, 0, 1),  Cell3DPosition(0, -1, 1),
    Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 2), Cell3DPosition(-1, -1, 3),
    Cell3DPosition(0, -1, 3),
};

static vector<Cell3DPosition> FillingPositions_FrontBack_Zeven = {
    Cell3DPosition(0, -1, 1),  Cell3DPosition(1, -1, 1), Cell3DPosition(0, -1, 2),
    Cell3DPosition(1, -1, 2), Cell3DPosition(2, -1, 2), Cell3DPosition(0, 0, 1),
    Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), Cell3DPosition(-1, 0, 3),
    Cell3DPosition(0, 0, 3)
};

static vector<Cell3DPosition> FillingPositions_BackFront_Zeven = {
    Cell3DPosition(-1, -1, 1),   Cell3DPosition(0, -1, 1),  Cell3DPosition(0, -1, 2),
    Cell3DPosition(1, -1, 2),   Cell3DPosition(-1, 0, 2),  Cell3DPosition(0, 0, 1),
    Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3),
    Cell3DPosition(0, 0, 3),
};

enum PathState {NONE, BFS, ConfPath, Streamline};
enum FillingState {EMPTY, FULL};
enum ProcessState {PASSIVE, ACTIVE};
enum RenconfigurationStep {SRCDEST, MAXFLOW, TRANSPORT, DONE};


static bool showSrcAndDst = false;


static RenconfigurationStep reconfigurationStep;

static int NbOfDestinationsReached = 0;
static vector<Cell3DPosition> destinations;

static int timeStep = 0;
static int nbOfIterations = 0;

class RePoStBlockCode : public Catoms3DBlockCode {
private:
    
public:
    static Catoms3DBlock *GC;
    static int NbOfStreamlines;
    static vector<array<int, 4>> initialMap;
    static vector<array<int, 4>> targetMap;
/* ----------------------- COORDINATION TREE VARIABLES ---------------------- */
    Cell3DPosition parentPosition = Cell3DPosition(-1,-1,-1);
    vector<Cell3DPosition> childrenPositions;
    Cell3DPosition parentPositionDst = Cell3DPosition(-1,-1,-1);
    vector<Cell3DPosition> childrenPositionsDst;
    short distanceSrc{0}, distanceDst{0};
    short nbWaitedAnswers{0};
    bool isSource{false};
    bool isDestination{false};
    FillingState fillingState;

    void reinitialize();
/* -------------------------------------------------------------------------- */

/* -------------------------- TERMINATION DETECTION ------------------------- */
    ProcessState state{PASSIVE};
    bool cont_passive{true};
    bool b{true};
    bool res{true};
    int deficit{0};
    void start_wave();
    void return_wave(bool b);
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */

/* ------------------------ TRANSPORTATION VARIABLES ------------------------ */
    MMShape shapeState;
    Cell3DPosition MMPosition;
    Cell3DPosition seedPosition;
    Cell3DPosition initialPosition;
    Catoms3DBlock *module;
    bool isCoordinator{false};
    MovingState movingState{MovingState::IN_POSITION};
    int mvt_it{0};
    int movingSteps{0};
    Cell3DPosition coordinatorPosition;
    bool awaitingCoordinator{false};
    bool rotating{false};
    int transferCount{0}; // Used to count the number of passed modules
    bool greenLightIsOn{true};
    bool moduleAwaitingGo{false};
    bool notFindingPivot{false};
    Cell3DPosition pivotPosition;
    set<Cell3DPosition> awaitingSources;
    int prevTransferCount; //used in pls message handler on the bridge to avoid blocking when coming from right then building back
    bool initialized{false};
    Operation *operation = NULL;
    bool sendingCoordinateBack{false};
/* -------------------------------------------------------------------------- */

/* ---------------------------- MAXFLOW variables --------------------------- */
    PathState mainPathState{NONE};	            //! state of the main path: {NONE, BFS, ConfPath, Streamline}
    Cell3DPosition mainPathIn = Cell3DPosition(-1,-1,-1);		            //! Position of parent meta-module on the main tree
    vector<Cell3DPosition> mainPathOut;         //! Position’s of child meta-modules on the main tree
    vector<Cell3DPosition >mainPathsOld;         //! old Position's of child meta-modules on the main tree
    PathState aug1PathState{NONE};	            //! state of the augmenting path 1: {NONE, BFS, ConfPath}
    Cell3DPosition aug1PathIn = Cell3DPosition(-1,-1,-1);				            //! Position of parent meta-module on the augmenting tree (type 1)
    vector<Cell3DPosition>aug1PathOut;		    //! Position’s of child meta-modules on the augmenting tree (type 1)
    vector<Cell3DPosition>aug1PathsOld;		    //! old Position's of child meta-modules on the augmenting tree (type 1)
    PathState aug2PathState{NONE};	            //! state of the augmenting path 2: {NONE, BFS, ConfPath}
    Cell3DPosition aug2PathIn = Cell3DPosition(-1,-1,-1);				    //! Position of parent meta-module on the augmenting tree (type 2)
    vector<Cell3DPosition>aug2PathOut;		    //! Position’s of child meta-modules on the augmenting tree (type 2)
    vector<Cell3DPosition>aug2PathsOld;		    //! old Position's of child meta-modules on the augmenting tree (type 2)
    Cell3DPosition destinationOut;
    /**
     * @brief get all adjacent MM seeds positions
     * 
     */
    vector<Cell3DPosition> getAdjacentMMSeeds();
/* -------------------------------------------------------------------------- */

    
    void setOperation(Cell3DPosition inPosition, Cell3DPosition outPosition);

    Color initialColor;

    RePoStBlockCode(Catoms3DBlock *host);
    ~RePoStBlockCode(){
    };

    inline static Time getRoundDuration() {
        return (Catoms3DRotation::ANIMATION_DELAY * BaseSimulator::motionDelayMultiplier
                + Catoms3DRotation::COM_DELAY) + 20128;// + (getScheduler()->now() / 1000);
    }

    MMShape getShapeState() { return shapeState; };
    void setShapeState(MMShape state) { shapeState = state; };
    

    Catoms3DBlock *getModule() { return module; };
    Cell3DPosition getMMPosition() { return MMPosition; };

    /**
     * @brief test if the MetaModule is a destination for Max-Flow
     *  
     */
    bool isPotentialDestination();
    /**
     * @brief test if the Meta-Module is a source for Max-Flow
     * 
     */
    bool isPotentialSource();
    /**
     * @param  pos a meta-module position in a 3D lattice
     * @return true if the meta-module is in the targetMap
     * @return false 
     */
    bool inTargetShape(Cell3DPosition pos);
     /**
     * @param  pos a meta-module position in a 3D lattice
     * @return true if the meta-module is in the initialMap
     * @return false 
     */
    bool inInitialShape(Cell3DPosition pos);
    
    /**
     * @brief check if a Meta-Module is filled in the initial shape
     * 
     * @param MMpos Meta-module position
     * @return bool
     */
    bool isFilledInInitial(Cell3DPosition MMpos);

     /**
     * @brief check if a Meta-Module is filled in the target shape
     * 
     * @param MMpos Meta-module position
     * @return bool
     */
    bool isFilledInTarget(Cell3DPosition MMpos);
    /**
     * @return vector<Cell3DPosition> containing all adjacent MMs Positions occupied or not
     */
    vector<Cell3DPosition> getAdjacentMMPositions();


    /**
     * @brief Update the state of a module once it becomes int its new position
     * 
     */
    void updateState();
    bool isInMM(Cell3DPosition &neighborPosition);
    /**
     * @brief Used for routing. Finds the nearest active cell to targetPosition using network distance
     * @warning Computationally heavy
     * @param targetPosition 
     * @param except 
     * @return Cell3DPosition 
     */
    Cell3DPosition nearestPositionTo(Cell3DPosition &targetPosition, P2PNetworkInterface *except = nullptr);
    Cell3DPosition nextInBorder(P2PNetworkInterface* sender);
    Cell3DPosition getSeedPositionFromMMPosition(Cell3DPosition &MMPos);
    P2PNetworkInterface *interfaceTo(Cell3DPosition& dstPos, P2PNetworkInterface *sender = nullptr);
    P2PNetworkInterface *interfaceTo(Cell3DPosition& fromMM, Cell3DPosition& toMM, P2PNetworkInterface *sender = nullptr);

    void probeGreenLight();
    bool isAdjacentToPosition(const Cell3DPosition& pos) const;
    static Catoms3DBlock* customFindMotionPivot(const Catoms3DBlock* m,
                                                const Cell3DPosition& tPos,
                                                RotationLinkType faceReq = RotationLinkType::Any);
    Catoms3DBlock* findTargetLightAmongNeighbors(const Cell3DPosition& targetPos,
                                                const Cell3DPosition& srcPos,
                                                P2PNetworkInterface *sender=NULL) const;
    
    vector<Catoms3DBlock*> findNextLatchingPoints(const Cell3DPosition& targetPos,
                                                    const Cell3DPosition& pivotPos);
    /**
     * @return module's position relative to the meta-module seedPosition
    **/
    inline Cell3DPosition relativePos() {
        return module->position - seedPosition;
    }

    Direction getPreviousOpDir();
    Direction getNextOpDir();
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

    int sendHandleableMessage(HandleableMessage *msg, P2PNetworkInterface *dest, Time t0, Time dt);

    /// Advanced blockcode handlers below

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from it.
     * @param config : pointer to the TiXmlDocument representing the configuration file, all information related to VisibleSim's core have already been parsed
     *
     * Called from BuildingBlock constructor, only once.
     */
    void parseUserElements(TiXmlDocument *config) override;

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

    void switchModulesColors();

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
        return (new RePoStBlockCode((Catoms3DBlock *)host));
    };
    /*****************************************************************************/
};

template <typename T>
void operator+=(std::vector<T> &v1, const std::vector<T> &v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
}

template <typename T>
bool operator==(T value,const std::vector<T> &v) {
    return v.size()==1 && v.front()==value;
}

template <typename T>
bool operator!=(T value,const std::vector<T> &v) {
    return v.size()!=1 || v.front()!=value;
}


template <typename T>
bool isIn(const std::vector<T> &v,T value) {
    typename vector<T>::const_iterator current=v.begin();
    while (current!=v.end()) {
        if (*current==value) return true;
        ++current;
    }
    return false;
}


#endif /* MetaModuleBlockCode_H_ */
