#ifndef DistClustBlockCode_H_
#define DistClustBlockCode_H_

#include "robots/catoms3D/catoms3DSimulator.h"
#include "robots/catoms3D/catoms3DBlockCode.h"
#include <set>


#define INF 2
#define NB_CLUSTERS 4
// #define NB_NODES 20

static const int REQUESTDENSITY_MSG_ID = 1000;
static const int SENDDENSITY_MSG_ID = 1001;
static const int CONNECT_MSG_ID = 1002;
static const int INITIATE_MSG_ID = 1003;
static const int TEST_MSG_ID = 1004;
static const int REPORT_MSG_ID = 1005;
static const int REJECT_MSG_ID = 1006;
static const int ACCEPT_MSG_ID = 1007;
static const int CHANGEROOT_MSG_ID = 1008;
static const int REQUESTDCUT_MSG_ID = 1009;
static const int RESPONSEDCUT_MSG_ID = 1010;
static const int CUT_MSG_ID = 1011;
static const int CUTACK_MSG_ID = 1012;
static const int REQUESTNBNODES_MSG_ID = 1013;
static const int RESPONSENBNODES_MSG_ID = 1014;

using namespace Catoms3D;

enum EdgeState {basic, branch, rejected};
enum NodeState {SLEEP, FIND, FOUND};

struct ConnectMsg {
    P2PNetworkInterface* sender;
    MessagePtr msgPtr;

    ConnectMsg(){};
    ConnectMsg(P2PNetworkInterface *s, MessagePtr m): sender(s), msgPtr(m){};        
};

struct TestMsg{
    P2PNetworkInterface* sender;
    MessagePtr msgPtr;

    TestMsg(){};
    TestMsg(P2PNetworkInterface *s, MessagePtr m): sender(s), msgPtr(m){};
};

struct ReportMsg{
    P2PNetworkInterface* sender;
    MessagePtr msgPtr;

    ReportMsg(){};
    ReportMsg(P2PNetworkInterface *s, MessagePtr m): sender(s), msgPtr(m){};
};

struct Density{
    float density;
    double unique;

    Density(){};

    bool operator < (const Density &d){
        if(density == d.density){
            return unique < d.unique;
        }else{
            return density > d.density;
        }
    }

    bool operator == (const Density &d){
        return density == d.density && unique == d.unique; 
    }

};

struct DCutMsg{
    int CHid;
    int nbNodes;
    float dcut;
    int cutAt;
    int subTreeSize;
    DCutMsg(int cid, int nbc):CHid(cid), nbNodes(nbc){};
};

struct CutMsg{
    int CHid;
    bool found;
    int nbc;
    CutMsg(){
        found = false;
    };
    CutMsg(int cid): CHid(cid){
        found = false;
    }
    CutMsg(int cid, int nc): CHid(cid), nbc(nc){
        found = false;
    };
};

struct Edge{
    //P2PNetworkInterface *interface;
    EdgeState SE;
    Density w;
    int myid;
    int nid;

    Edge(){};
    Edge(int m, int n, float d): myid(m), nid(n){
        SE = basic;
        string s1 = to_string(m);
        string s2 = to_string(n);
        string s;
        if(m <= n){
            s = s1 + s2;
        } else {
            s = s2 + s1;
        }
        w.density = d;
        w.unique = stod(s);
    };
};

struct MsgData{
    int LN;
    Density FN;
    NodeState SN;

    MsgData(){};
    MsgData(int l, Density f): LN(l), FN(f){};
    MsgData(int l, Density f, NodeState s): LN(l), FN(f), SN(s){};
};

static vector<int> clustersColors;

class DistClustBlockCode : public Catoms3DBlockCode {
private:
    Catoms3DBlock *module;
    vector<int> myNeighborhood;
    //vector<Cell3DPosition> anchors;
    //float minDist;
    //GHS
    int LN;
    Density FN;
    NodeState SN;
    Cell3DPosition leaderPosition;
    map<P2PNetworkInterface* ,Edge> edges;
    P2PNetworkInterface* best_edge;
    Density best_wt;
    P2PNetworkInterface* test_edge;
    P2PNetworkInterface* in_branch;
    int find_count;
    deque<TestMsg> waitingTestMsgs;
    deque<ConnectMsg> waitingConnectMsgs;
    deque<ReportMsg> waitingReportMsgs;

    // Clusters distribution
    set<P2PNetworkInterface*> children;
    int CH;
    int cutAt;
    float Dcut;
    P2PNetworkInterface* toMinCut;
    int nbWaitedFromChildren;
    int nbNodes;
    int subTreeSize;
    int subtree_tmp;
    bool isCH;
    int nbc; // nb of clusters

    void setMyNeighborhood();
    void setEdges();
    float JC(vector<int> receivedNeigh);
    int min(int, int);
    void cut();
    bool isLeaf();
    P2PNetworkInterface* get_min_wt();
    void setColor(int CH);

public :
    DistClustBlockCode(Catoms3DBlock *host);
    ~DistClustBlockCode() {};

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

    void wakeup();
    void test();
    void report();
    void change_root();

    /**
     * @brief Sample message handler for this instance of the blockcode
     * @param msgPtr Pointer to the message received by the module
     * @param sender Connector of the module that has received the message and that is connected to the sender */
    void handleRequestDensityMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleSendDensityMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleConnectMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleInitiateMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleTestMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleReportMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleAcceptMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleRejectMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleChangeRootMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleRequestdcutMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleResponsedcutMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleCutMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleCutAckMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleRequestNbNodesMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleResponseNbNodesMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    /// Advanced blockcode handlers below

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from it.
     * @param config : pointer to the TiXmlDocument representing the configuration file, all information related to VisibleSim's core have already been parsed
     *
     * Called from BuildingBlock constructor, only once.
     */
    void parseUserElements(TiXmlDocument *config) override {}

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
     * @brief This function is called when a module is tapped by the user. Prints a message to the console by default.
     Can be overloaded in the user blockCode
     * @param face face that has been tapped */
    void onTap(int face) override {}

    // NOT YET IMPLEMENTED ON GIT/DEV
    // bool parseUserCommandLineArgument(int& argc, char **argv[]) override;

/*****************************************************************************/
/** needed to associate code to module                                      **/
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new DistClustBlockCode((Catoms3DBlock*)host));
    };
/*****************************************************************************/
};

#endif /* DistClustBlockCode_H_ */
