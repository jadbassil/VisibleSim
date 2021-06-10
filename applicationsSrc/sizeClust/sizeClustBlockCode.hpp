#ifndef SizeClustBlockCode_H_
#define SizeClustBlockCode_H_

// #include "robots/catoms3DWorld.h"
// #include "catoms3DBlockCode.h"
#include "robots/catoms3D/catoms3DBlockCode.h"
#include "robots/catoms3D/catoms3DSimulator.h"
#include "robots/catoms3D/catoms3DWorld.h"
#include <set>
#include <stack>


#define INF 2147483647
#define NB_CLUSTERS 5
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
static const int FINDCUT_MSG_ID = 1009;
static const int RESPCUT_MSG_ID = 1010;
static const int CUT_MSG_ID = 1011;
static const int ASSIGN_MSG_ID = 1012;
static const int ASSIGNACK_MSG_ID = 1013;
static const int REPORTCUT_MSG_ID = 1014;
static const int FRIEND_MSG_ID = 1015;

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
    int unique;

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
    Cell3DPosition CH_pos;
    int subTreeSize;
    int clusterSize; //index of size in clusterSizes vector
    float distance;
    DCutMsg(int cid, int nbc, int cs):CHid(cid), nbNodes(nbc), clusterSize(cs){};
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
    int CH{0};

    Edge(){};
    Edge(int m, int n, float d): myid(m), nid(n){
        SE = basic;
        w.density = d;
        int n1, n2;
        if(n <= m){
            n1 = n; n2 = m;
        }else {
            n1 = m; n2 = n;
        }
        vector<bool> res;
        while(n1>0){
            res.insert(res.begin(), n1%2);
            n1 /= 2;
        }
        int s = res.size();
        while(n2>0){
            res.insert(res.begin()+s,n2%2);
            n2 /= 2;
        }
        int ans = 0;
        for(int i=0; i<res.size(); i++){
            ans += res[i] << (res.size() - i - 1);
        }
        w.unique = ans;
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

struct AssignMsg {
    int CH;
    Cell3DPosition CH_pos;
    int distance_CH;

    AssignMsg() {};
    AssignMsg(int ch, Cell3DPosition ch_pos, int d): CH(ch), CH_pos(ch_pos), distance_CH(d){
    };
};

struct FindDistantAckMsg{
    int distant_id;
    float distance;
    bool exchanged;

    FindDistantAckMsg() {};
    FindDistantAckMsg(int did, float d): distant_id(did), distance(d) {};
    FindDistantAckMsg(int did, float d, bool e): distant_id(did), distance(d), exchanged(e) {};
};

struct BuildChainMsg{
    int CH;
    int sizeDiff;
    int distance_CH;
    Cell3DPosition src_Pos;

    BuildChainMsg() {};
    BuildChainMsg(int ch, int s, Cell3DPosition sp): CH(ch), sizeDiff(s), src_Pos(sp) {}; 
};

struct FindNextMsg {
    int CH;
    Cell3DPosition src_Pos;

    FindNextMsg() {};
    FindNextMsg(int ch, Cell3DPosition sp): CH(ch), src_Pos(sp) {}; 
};

struct FindNextAckMsg {
    int isEdge;
    float distanceToSrc;
    bool possible{true};

    FindNextAckMsg() {};
    FindNextAckMsg(int e, float d, bool p): isEdge(e), distanceToSrc(d), possible(p) {};
};

struct UpdateMsg {
    int CH;
    bool isChild;

    UpdateMsg() {};
    UpdateMsg(int ch, bool child): CH(ch), isChild(child) {};
};

struct ReportErrMsg {
    int CH;
    int sizeDiff;
    int subTreeSize;

    ReportErrMsg() {};
    ReportErrMsg(int CH, int sizeDiff, int subTreeSize):
        CH(CH), sizeDiff(sizeDiff), subTreeSize(subTreeSize) {};
};

struct FindCutMsg {
    int adj;
    int desiredSize;
    bool recut;

    FindCutMsg(){};
    FindCutMsg(int a, int d, bool r):
        adj(a), desiredSize(d), recut(r){}; 
};

struct RespCutMsg {
    int subTreeSize;
    int error;
    int nbAdj;
    int adj;
    int Height;

    RespCutMsg(){};
    RespCutMsg(int s, int e, int d, int a, int n):
        subTreeSize(s), error(e), Height(d), adj(a), nbAdj(n){};
};



static vector<int> clustersColors;
// static vector<float> clustersSizes = {0.1, 0.2, 0.3};
static vector<float> clustersSizes;
//static vector<float> clustersSizes = {0.5};
static map<int,int> clustersResult;
static int nbNodes;
static int nbAddCut{0};
static vector<bID> shuffledIds;

class SizeClustBlockCode : public Catoms3DBlockCode {
public:
    Catoms3DBlock *module;
    vector<int> myNeighborhood;
    vector<Cell3DPosition> anchors;
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
    P2PNetworkInterface* toBestCut{nullptr}, *toLastCH{nullptr};
    int subTreeSize;
    int C{0};
    bool isCH;
    int nbc; // nb of clusters
    int clusterIndex{0};
    int sizeDiff, minSizeDiff;
    int desiredSize{-1};
    bool receivedAssign;
    int root{-1};
    bool isRoot{false};
    bool cutOn{false};
    int nbWaitedAnswers{0};
    int maxNbAdj{0}, nbAdj{0};
    bool RecutSrc{false};
    int minError;
    int maxHeight;
    int myHeight;
    bool recut{false};
    set<P2PNetworkInterface*> exchangedNeighbors; 

    void setMyNeighborhood();
    void setEdges();
    float JC(vector<int> receivedNeigh);
    int min(int, int);
    void cut(bool,int,int);
    bool isLeaf();
    int isEdge(int c);
    int isEdge1(int c);

    void check() {
        map<int,int> ch_count;
        int nbDiconnected = 0;
        BaseSimulator::World *world = BaseSimulator::getWorld(); 
        for(int i=1;i<=world->getNbBlocks();i++) {
            SizeClustBlockCode* N = (SizeClustBlockCode*) world->getBlockById(i)->blockCode;
            bool disconnected = true;
            // if(!N->isCH)
            //     N->setColor(N->CH);
            // else
            //     N->module->setColor(BROWN);
            // if(N->edges[N->in_branch].SE == rejected){
            //     nbDiconnected++;
            //     N->module->setColor(BLACK);
            // }
            ch_count[N->C]++;
            N->module->setColor(N->C);
        }
        cout<< endl <<"ch_count: " <<endl;
        for(auto cc: ch_count) {
            cerr << cc.first <<": " << cc.second << endl;
        }
        cerr << endl;
        cerr << "nbDisconnected: " << nbDiconnected << endl;
    }
    
    P2PNetworkInterface* get_min_wt();

    SizeClustBlockCode(Catoms3DBlock *host);
    ~SizeClustBlockCode() {};

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

    void setColor(int CH);
    Catoms3DBlock *getModule() {
        return module;
    }

    /**
     * This function returns the difference between node's subtree size and 
     * the total number of nodes
    */
    float getSizeDiff(int index);
    int getCH() { return CH; };
    vector<int> getMyNeighborhood() { return myNeighborhood; }

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
    void handleFindCutMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleRespCutMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleCutMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleAssignMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleAssignAckMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleReportCutMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    void handleFriendMessage(MessagePtr msgPtr, P2PNetworkInterface *sender);
    /// Advanced blockcode handlers below

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from it.
     * @param config : pointer to the TiXmlDocument representing the configuration file, all information related to VisibleSim's core have already been parsed
     *
     * Called from BuildingBlock constructor, only once.
     */
    void parseUserElements(TiXmlDocument *config) override;

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
        return (new SizeClustBlockCode((Catoms3DBlock*)host));
    };
/*****************************************************************************/
};

#endif /* SizeClustBlockCode_H_ */