#include "sizeClustBlockCode.hpp"
#include <algorithm>
#include "eval.hpp"
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#define MAX(a, b) ((a) > (b)) ? (a) : (b)

using namespace Catoms3D;

SizeClustBlockCode::SizeClustBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Register callbacks
    addMessageEventFunc2(REQUESTDENSITY_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleRequestDensityMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(SENDDENSITY_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleSendDensityMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CONNECT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleConnectMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(INITIATE_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleInitiateMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(TEST_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleTestMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(REPORT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleReportMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(ACCEPT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleAcceptMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(REJECT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleRejectMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CHANGEROOT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleChangeRootMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(FINDCUT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleFindCutMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(RESPCUT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleRespCutMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CUT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleCutMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(ASSIGN_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleAssignMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(ASSIGNACK_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleAssignAckMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
     addMessageEventFunc2(REPORTCUT_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleReportCutMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(FRIEND_MSG_ID,
                         std::bind(&SizeClustBlockCode::handleFriendMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    // Set the module pointer
    module = static_cast<Catoms3DBlock*>(hostBlock);
  }


void SizeClustBlockCode::parseUserElements(TiXmlDocument *config){
    TiXmlElement *element = config->FirstChildElement()->FirstChildElement();
    // cerr<<element->Attribute("sizes")<<endl;
    // vector<float> sizes;
    if(!element->Attribute("sizes")){
        clustersSizes = {0.25, 0.25, 0.25};
        return;
    }
    string sizes = (string) element->Attribute("sizes");
    int c = count(sizes.begin(), sizes.end(), ',') + 2;
    string s;
    float count = 0;
    for(int i=0; i<sizes.size()+1;i++){
        char ch = sizes[i];
        if(ch == ','){
            clustersSizes.push_back(stof(s));
            count += stof(s);
            s.clear();
            continue;
        }
        if(isdigit(ch) || ch=='.'){
            s.push_back(ch);
        }
        if(ch == '\0'){
            clustersSizes.push_back(stof(s));
            count += stof(s);
            // clustersSizes.push_back(1-count);
        }
    }
    cerr << "clusters sizes: ";
    for(auto x: clustersSizes){
        cerr << x*100 << "% ";
    }
    cerr << (1-count)*100 << "%\n";
}

void SizeClustBlockCode::startup() {
    setMyNeighborhood();
    SN = SLEEP;
    LN = 0;
    in_branch = nullptr;
    cutAt = -1;
    subTreeSize = 0;
    nbc = 0;
   C = 0;
    C = 0;
    isCH = false;
    nbNodes = BaseSimulator::getWorld()->getNbBlocks();
    setEdges();
    if(module->blockId == 1){
        float sum = 0;
        int i = 0;
        for(i=0; i<clustersSizes.size(); i++){
            sum += clustersSizes[i];
            cerr<<"Cluster "<<i<<" Size: "<<clustersSizes[i] * BaseSimulator::getWorld()->getNbBlocks()<<endl;
        }
        cerr<<"Cluster "<<i<<" Size: "<<(1-sum) * BaseSimulator::getWorld()->getNbBlocks()<<endl;
        cerr<<"------------------"<<endl;
    }
}

void SizeClustBlockCode::setColor(int c){
    for(int i=0;i<clustersColors.size(); i++){
        if(clustersColors[i] == c){
            module->setColor(i);
            if(i == 1){
                module->setColor(MAGENTA);
            }
        }
    }
}

void SizeClustBlockCode::setMyNeighborhood(){
    myNeighborhood.clear();
    myNeighborhood.push_back(module->blockId);
    for(int i=0;i<hostBlock->getNbInterfaces();i++){
        short n = hostBlock->getNeighborIDForFace(i);
        if(n != 0){
            myNeighborhood.push_back(n);
        }
    }
}

void SizeClustBlockCode::setEdges(){
    for(int i=0;i<hostBlock->getNbInterfaces();i++){
        short n = hostBlock->getNeighborIDForFace(i);
        if(n != 0){
            if(module->getInterface(i)->getConnectedBlockId() < module->blockId){
                sendMessage("RequestDensity Msg", new MessageOf<vector<int>>(REQUESTDENSITY_MSG_ID, myNeighborhood), module->getInterface(i), 100, 200);
            }
        }
    }
}

int SizeClustBlockCode::min(int a, int b){
    return (a<b) ? a : b;
}

float SizeClustBlockCode::JC(vector<int> receivedNeigh){
    int nbIn = 0;
    int nbUn = 0;
    for(vector<int>::iterator it=receivedNeigh.begin(); it!=receivedNeigh.end(); it++){
        if(find(myNeighborhood.begin(), myNeighborhood.end(), *it) != myNeighborhood.end()){
            nbIn++;
        }
    }
    nbUn = receivedNeigh.size() + myNeighborhood.size() - nbIn;
    return (float)nbIn/(float)nbUn;
}

P2PNetworkInterface* SizeClustBlockCode::get_min_wt(){
    P2PNetworkInterface* minEdge;
    Density minWt;
    minWt.density = 0;
    for(auto e: edges){
        if(e.second.w < minWt && e.second.CH ==C){
            minWt = e.second.w;
            minEdge = e.first;
        }
    }
    return minEdge;
}

float SizeClustBlockCode::getSizeDiff(int index){
    float n = nbNodes * clustersSizes[index];
    // console<<n<<"\n";
    // console<<"subtreesize: "<<subTreeSize<<"\n";
    // float min = abs(subTreeSize - n);
    // for(int i=0; i<clustersSizes.size(); i++){
    //     n = nbNodes * clustersSizes[i];
    //     float s = abs(subTreeSize - n);
    //     if(s < min) min = s;
    // }
    return floor(subTreeSize - n);
}

void SizeClustBlockCode::wakeup(){
    if(edges.size() == myNeighborhood.size() - 1 && SN == SLEEP){
        console<<"wakeup\n";
        module->setColor(RED);
        P2PNetworkInterface* m = get_min_wt();
        edges[m].SE = branch;
        LN = 0;
        SN = FOUND;
        find_count = 0;
        sendMessage("Connect Msg", new MessageOf<int>(CONNECT_MSG_ID, 0), m, 100, 200);
    }

}

void SizeClustBlockCode::test(){
    console<<"Executing test\n";
    bool existBasic = false;
    Density minWt;
    minWt.unique = 0; minWt.density = -1;
    test_edge = nullptr;
    for(auto e: edges){
        if(e.second.SE == basic){
            existBasic = true;
            console<<"e: "<<e.first->getConnectedBlockId()<<"\n";
            if(e.second.w < minWt){
                minWt = e.second.w;
                test_edge = e.first;
            }
        }
    }
    if(test_edge != nullptr)
        console<<"test_edge: "<<test_edge->getConnectedBlockId()<<"\n";
    if(existBasic){
        if(test_edge != nullptr){
            sendMessage("Test Msg", new MessageOf<MsgData>(TEST_MSG_ID, MsgData(LN, FN)), test_edge, 100, 200);
        }
    }else{
        test_edge = nullptr;
        report();
    }
}

void SizeClustBlockCode::report(){
    console<<"Executing report\n";
    console<<"Find_count: "<<find_count<<"\n";

    if(test_edge != nullptr)
        console<<"Test_edge: "<<test_edge->getConnectedBlockId()<<"\n";
    if(SN == FIND)
        module->setColor(YELLOW);
    if(find_count == 0 && test_edge == nullptr){
        console<<"test report\n";
        subTreeSize++;
        SN = FOUND;
        module->setColor(BLUE);
        if(!isRoot){
            if(desiredSize > 0 && cutOn){
                cut(true, desiredSize, 0);
            }else {
                sendMessage("Report Msg", 
                    new MessageOf<pair<Density,int>>(REPORT_MSG_ID, make_pair(best_wt, subTreeSize)),
                    in_branch, 100, 200);
            }
            
        }  else {
            cerr << module->blockId << ":  tree updated\n";
            if(isRoot && clusterIndex < clustersSizes.size()-1){
                clusterIndex++;
                desiredSize = floor(nbNodes*clustersSizes[clusterIndex]);
                
                recut = false;
                cut(recut, desiredSize, clusterIndex);
            } else {
                check();
                Eval eval(BaseSimulator::getWorld());
                eval.printConnected();
                cout << "Additional cuts: " << nbAddCut << endl;
            }
            // if(module->blockId != root){
            //     //sendMessage("CutAck Msg1", new MessageOf<int>(CUTACK_MSG_ID, subTreeSize), toRoot, 100, 200);
            // } else {
            //     getScheduler()->toggle_pause();
            //     if(nbc > 0){
            //         cut();
            //         //nbc--;
            //     } else if(nbc == 0) {
            //         //check();
            //         Eval eval(BaseSimulator::getWorld());
            //         // eval.fixColors();
            //         Eval::fixColors(BaseSimulator::getWorld());
            //         eval.printConnected();
            //         // eval.calculateDBindexC();
            //     }
            // }

        }
    }

}

void SizeClustBlockCode::change_root(){
    if(edges[best_edge].SE == branch){
        sendMessage("ChangeRoot Msg", new Message(CHANGEROOT_MSG_ID), best_edge, 100, 200);
    }
    else{
        sendMessage("Connect Msg", new MessageOf<int>(CONNECT_MSG_ID, LN), best_edge, 100, 200);
        edges[best_edge].SE = branch;
    }
}

void SizeClustBlockCode::handleRequestDensityMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
    MessageOf<vector<int>>* msg = static_cast<MessageOf<vector<int>>*>(msgPtr.get());
    vector<int> receivedNeigh = *msg->getData();
    // vector<Cell3DPosition> anchors;
    Cell3DPosition senderPos;
    float nminDist = 0, minDist = 0;
    module->getNeighborPos(module->getFaceForNeighborID(sender->getConnectedBlockId()), senderPos);
    // // anchors.push_back(Cell3DPosition(0,0,0)); anchors.push_back(Cell3DPosition(72,0,0));
    // // // anchors.push_back(Cell3DPosition(83,-54,0)); anchors.push_back(Cell3DPosition(51,93,0));
    // anchors.push_back(Cell3DPosition(22,0,0)); anchors.push_back(Cell3DPosition(0,22,0));
    // anchors.push_back(Cell3DPosition(22,22,0)); anchors.push_back(Cell3DPosition(0,0,0));
    // anchors.push_back(Cell3DPosition(22,0,22)); anchors.push_back(Cell3DPosition(0,22,22));
    // anchors.push_back(Cell3DPosition(22,22,22)); anchors.push_back(Cell3DPosition(0,0,22));
    // float minDist = module->position.dist_euclid(anchors.front());
    // for(int i=0;i<anchors.size();i++){
    //     if(module->position.dist_euclid(anchors[i]) < minDist){
    //         minDist = module->position.dist_euclid(anchors[i]);
    //     }
    // }
    // float d = JC(receivedNeigh)*(module->position.dist_euclid(leaderPosition));
    int minX, minY, minZ, maxX, maxY, maxZ;
    minX = maxX = Catoms3D::getWorld()->buildingBlocksMap.begin()->second->position.pt[0];
    minY = maxY = Catoms3D::getWorld()->buildingBlocksMap.begin()->second->position.pt[1];
    minZ = maxZ = Catoms3D::getWorld()->buildingBlocksMap.begin()->second->position.pt[2];
    for(auto i: Catoms3D::getWorld()->buildingBlocksMap){
        if(i.second->position.pt[0] < minX) minX = i.second->position.pt[0];
        if(i.second->position.pt[0] > maxX) maxX = i.second->position.pt[0];
        if(i.second->position.pt[1] < minY) minY = i.second->position.pt[1];
        if(i.second->position.pt[1] > maxY) maxY = i.second->position.pt[1];
        if(i.second->position.pt[2] < minZ) minZ = i.second->position.pt[2];
        if(i.second->position.pt[2] > maxZ) maxZ = i.second->position.pt[2];
    }
    anchors.push_back(Cell3DPosition(minX, minY, minZ));
    anchors.push_back(Cell3DPosition(minX, maxY, minZ));
    anchors.push_back(Cell3DPosition(minX, minY, maxZ));
    anchors.push_back(Cell3DPosition(maxX, minY, minZ));
    anchors.push_back(Cell3DPosition(maxX, maxY, minZ));
    anchors.push_back(Cell3DPosition(maxX, minY, maxZ));
    anchors.push_back(Cell3DPosition(minX, maxY, maxZ));
    anchors.push_back(Cell3DPosition(maxX, maxY, maxZ));

    // anchors.push_back(Cell3DPosition(12,2,0));
    // anchors.push_back(Cell3DPosition(24,2, 0));
    // anchors.push_back(Cell3DPosition(29, 7, 25));
    // anchors.push_back(Cell3DPosition(6, 7, 25));
    // anchors.push_back(Cell3DPosition(18, 4, 35));
    // float minDist = module->position.dist_euclid(anchors.front());
    // for(int i=0;i<anchors.size();i++){
    //     if(module->position.dist_euclid(anchors[i]) < minDist){
    //         minDist = module->position.dist_euclid(anchors[i]);
    //     }
    // }
    // if(minDist == 0){
    //     minDist = 1;
    // }
    minDist = module->position.dist_euclid(anchors.front());
    nminDist = senderPos.dist_euclid(anchors.front());
    for(int i=0;i<anchors.size();i++){
        if(module->position.dist_euclid(anchors[i]) < minDist){
            minDist = module->position.dist_euclid(anchors[i]);
        }
        if(senderPos.dist_euclid(anchors[i]) < nminDist){
            nminDist = senderPos.dist_euclid(anchors[i]);
        }
    }
    if(minDist == 0){
        minDist = 1;
    }
    if(nminDist == 0){
        nminDist = 1;
    }
    minDist = min(minDist, nminDist);
    //float d = (float)JC(receivedNeigh)/minDist;
    float d = 1/minDist;
    //float d = JC(receivedNeigh);
    //float d = 1/abs(minDist - nminDist);
    //float d = JC(receivedNeigh)*minDist;
    Edge e(module->blockId, sender->getConnectedBlockId(), d);
    edges[sender] = e;
    sendMessage("Send Density", new MessageOf<float>(SENDDENSITY_MSG_ID, d), sender, 100, 200);
    if(edges.size() == myNeighborhood.size()-1){
        console<<"All edges densities computed\n";
        module->setColor(CYAN);
    }
    //anchors.clear();

}

void SizeClustBlockCode::handleSendDensityMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
    MessageOf<float>* msg = static_cast<MessageOf<float>*>(msgPtr.get());
    float d = *msg->getData();
    edges[sender] = Edge(module->blockId, sender->getConnectedBlockId(), d);
    if(edges.size() == myNeighborhood.size() - 1){
        console<<"All edges densities computed\n";
        module->setColor(CYAN);
        if(module->blockId % 2 == 0)
            wakeup();
    }
}

void SizeClustBlockCode::handleConnectMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received Connect from "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<int>* msg = static_cast<MessageOf<int>*>(msgPtr.get());
    if(edges[sender].CH !=C)
        return;
    int L = *(msg)->getData();
    if(SN == SLEEP)
        wakeup();
    //subTreeSize = 0;
    if(L < LN){
        edges[sender].SE = branch;
        //getScheduler()->schedule(new InterruptionEvent(getScheduler()->now()+300, module, WAITING_CONNECT_INTERRUPTION));
        MsgData msg(LN, FN, SN);
        sendMessage("Initiate Msg", new MessageOf<MsgData>(INITIATE_MSG_ID,msg), sender, 100, 200);
        if(SN == FIND){
            find_count++;
        }
    }else if(edges[sender].SE == basic){
        console<<"schedule connect "<<sender->getConnectedBlockId()<<"\n";
        // getScheduler()->schedule(new NetworkInterfaceReceiveEvent(getScheduler()->now() + 200, sender, msgPtr));
        waitingConnectMsgs.push_back(ConnectMsg(sender, msgPtr));
    }else{
        sendMessage("Initiate Msg", new MessageOf<MsgData>( INITIATE_MSG_ID, MsgData(LN+1, edges[sender].w, FIND)), sender, 100, 200);
    }
}

void SizeClustBlockCode::handleInitiateMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received Initiate Msg from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<MsgData>* msg = static_cast<MessageOf<MsgData>*>(msgPtr.get());
    if(edges[sender].CH != C)
        return;
    MsgData rcvMsg = *msg->getData();
    LN = rcvMsg.LN;
    FN = rcvMsg.FN;
    SN = rcvMsg.SN;

    console<<"LN: "<<LN<<"\n";
    in_branch = sender;
    module->setColor(BLUE);
    best_edge = nullptr;
    best_wt.density = -1; best_wt.unique = 0;

    subTreeSize = 0;
    for(auto e: edges){
        if(e.first != sender && e.second.SE == branch && e.second.CH == C){
            sendMessage("Initiate Msg", new MessageOf<MsgData>(INITIATE_MSG_ID, rcvMsg), e.first, 100, 200);
            if(rcvMsg.SN == FIND)
                find_count = find_count + 1;
        }
    }
    if(SN == FIND){
        console<<"best_wt: "<<best_wt.density<<" | "<<best_wt.unique<<"\n";
        // console<<"Edges: "<<"\n";
        // for(auto e: edges){
        //     console<<e.first->getConnectedBlockId()<<": "<<" | "<<e.second.w<<" | "<<e.second.SE<<"\n";
        // }
        test();
    }
}

void SizeClustBlockCode::handleTestMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received Test Message from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<MsgData>* msg = static_cast<MessageOf<MsgData>*>(msgPtr.get());
    MsgData rcvMsg = *msg->getData();
    if(edges[sender].CH !=C){
        sendMessage("Reject Msg", new Message(REJECT_MSG_ID), sender, 100, 200);
        return;
    }
    if(SN == SLEEP){
        wakeup();
    }
    console<<"rcvMsg.L: "<<rcvMsg.LN<<"\n";
    console<<"LN: "<<LN<<"\n";
    if(rcvMsg.LN>LN){
        console<<"Schedule test "<<sender->getConnectedBlockId()<<"\n";
        waitingTestMsgs.push_back(TestMsg(sender, msgPtr));
    }else if(!(rcvMsg.FN == FN)){
        sendMessage("Accept Msg", new Message(ACCEPT_MSG_ID), sender, 100, 200);
    }else{
        if(edges[sender].SE == basic){
            edges[sender].SE = rejected;
        }
        if(test_edge != sender)
            sendMessage("Reject Msg", new Message(REJECT_MSG_ID), sender, 100, 200);
        else
            test();
    }
}

void SizeClustBlockCode::handleReportMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received report from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<pair<Density,int>>* msg = static_cast<MessageOf<pair<Density,int>>*>(msgPtr.get());
    Density w = (*msg->getData()).first;
   
    if(edges[sender].CH !=C)
        return;
    console<<"w: "<<w.density<<" | "<<w.unique<<"\n";
    console<<"waiting report: "<<waitingReportMsgs.size()<<"\n";
    if(sender != in_branch){
        subTreeSize += (*msg->getData()).second;
        find_count = find_count - 1;
        console<<"find_count: "<<find_count<<"\n";
        if(w < best_wt){
            best_wt = w;
            best_edge = sender;
        }
        //console<<"best_wt: "<<best_wt<<"\n";
        report();
    }else if(SN == FIND){
        console<<"Schedule report: "<<sender->getConnectedBlockId()<<"\n";
        waitingReportMsgs.push_back(ReportMsg(sender, msgPtr));
    }else if(best_wt < w){
        //console<<"best_wt: "<<best_wt<<"\n";
        change_root();
    }else if(w == best_wt && w.density == -1){
        cerr<<module->blockId<<" : DCT Built\n";
        cerr << "SubtreeSize: " << subTreeSize << "\n";
        if(module->blockId<in_branch->getConnectedBlockId() &&C == 0){
            //module->setColor(GREEN);
            subTreeSize += ((SizeClustBlockCode*)BaseSimulator::getWorld()
                ->getBlockById(in_branch->getConnectedBlockId())->blockCode)->subTreeSize;
           
            SizeClustBlockCode *R;
            float minDist = 999;
            for(int i=1; i<=BaseSimulator::getWorld()->getNbBlocks(); i++) {
                SizeClustBlockCode* N = (SizeClustBlockCode*) BaseSimulator::getWorld()->getBlockById(i)->blockCode;
                if(N->module->position.dist_euclid(anchors[0]) < minDist) {
                    minDist = N->module->position.dist_euclid(anchors[0]);
                    R = N;
                }
            }
            R->in_branch = nullptr;
            R->isRoot = true;
            R->cutOn = true; 
            nbNodes = subTreeSize;
            R->desiredSize = floor(nbNodes*clustersSizes[clusterIndex]);
            cout << desiredSize << "\n";
            R->cut(false,R->desiredSize,-1);

            // SizeClustBlockCode *R;
            
            // in_branch = nullptr;
            // isRoot = true;
            // cutOn = true; 
            // nbNodes = subTreeSize;
            // desiredSize = floor(nbNodes*clustersSizes[clusterIndex]);
            // cout << desiredSize << "\n";
            // cut(false,desiredSize,-1);
        }
    }
}

void SizeClustBlockCode::handleRejectMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received reject message from: "<<sender->getConnectedBlockId()<<"\n";
    if(edges[sender].SE == basic){
        edges[sender].SE = rejected;
    }
    test();
}

void SizeClustBlockCode::handleAcceptMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received accept message from: "<<sender->getConnectedBlockId()<<"\n";
    test_edge = nullptr;
    if(edges[sender].w < best_wt){
        best_edge = sender;
        best_wt = edges[sender].w;
    }
    report();
}

void SizeClustBlockCode::handleChangeRootMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received change-root message from: "<<sender->getConnectedBlockId()<<"\n";
    change_root();
}


bool SizeClustBlockCode::isLeaf(){
    bool isLeaf = true;
    for(auto e: edges){
        if(e.second.SE == branch && e.first != in_branch)
            isLeaf = false;
    }
    return isLeaf;
}

int SizeClustBlockCode::isEdge(int c) {
    int outEdge = 0;
  
    return outEdge;
}

int SizeClustBlockCode::isEdge1(int c) {
    int outEdge = 0;
    
    return outEdge;
}

void SizeClustBlockCode::cut(bool _recut, int _desiredSize, int _C){
    console<<"CUT " << _desiredSize << "\n";
    //Root
    if(_recut == true) nbAddCut++;
    nbWaitedAnswers = 0;
    desiredSize = _desiredSize;
    minError = nbNodes;
    maxHeight = nbNodes;
    maxNbAdj = 0;
    toBestCut = nullptr;
    recut = _recut;
    nbAdj = 0;
    myHeight = 1;
    subTreeSize = 0;
    for(auto e: edges){
        if(e.second.SE == branch && e.first != in_branch && e.second.CH == C){
            sendMessage("FindCut Msg", 
                new MessageOf<FindCutMsg>(FINDCUT_MSG_ID, FindCutMsg(_C, desiredSize, recut)),
                e.first, 100, 200);
            nbWaitedAnswers++;
        }
    }
    
}

void SizeClustBlockCode::handleFindCutMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    MessageOf<FindCutMsg> *msg = static_cast<MessageOf<FindCutMsg>*>(msgPtr.get());
    int recvAdj = (*msg->getData()).adj;
    desiredSize = msg->getData()->desiredSize;
    recut = msg->getData()->recut;
    console << "FindCut: " << desiredSize << "\n";
    minError = nbNodes;
    maxHeight = nbNodes;
    myHeight = 1;
    subTreeSize = 0;
    maxNbAdj = 0;
    toBestCut = nullptr;
    nbAdj = 0;
    cutOn = false;
    if(recvAdj == -1){
        C = 0;
        in_branch = sender;
    }else{
        for(auto e: edges){
            if(e.second.CH == recvAdj){
                nbAdj++;
            }
        }
    }   
    module->setColor(C);
    if(isLeaf()) {
        subTreeSize = 1;
        maxHeight = myHeight;
        maxNbAdj = nbAdj;
        minError = abs(subTreeSize - desiredSize);
         console <<  recvAdj << " " << maxNbAdj << " " << minError << "\n";
        RespCutMsg resp(subTreeSize, minError, maxHeight, recvAdj, maxNbAdj);
        sendMessage("RespCut msg",
            new MessageOf<RespCutMsg>(RESPCUT_MSG_ID, resp),
            in_branch, 100, 200);
    } else {
        nbWaitedAnswers = 0;
        for(auto e: edges){
            if(e.first != in_branch && e.second.SE == branch && e.second.CH == C){
                sendMessage("FindCut Msg", 
                    new MessageOf<FindCutMsg>(FINDCUT_MSG_ID, FindCutMsg(recvAdj, desiredSize, recut)),
                    e.first, 100, 200);
                nbWaitedAnswers++;
            }
        }
    }
    
}

void SizeClustBlockCode::handleRespCutMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    MessageOf<RespCutMsg> *msg = static_cast<MessageOf<RespCutMsg>*>(msgPtr.get());
    RespCutMsg resp = *msg->getData();
    nbWaitedAnswers--;
    subTreeSize += resp.subTreeSize;

    if(resp.Height > myHeight){
        myHeight = resp.Height;
    }

    if(abs(resp.error) < minError || (abs(resp.error) == minError && resp.Height > maxHeight)){
            minError = resp.error;
            maxHeight = resp.Height;
            toBestCut = sender;
    }
    if(resp.adj != -1){ 
        if(abs(resp.error) == minError && resp.nbAdj > maxNbAdj){
            maxNbAdj = resp.nbAdj;
            toBestCut = sender;
        }
    }  
    if(nbWaitedAnswers == 0) {
        myHeight += 1;
        subTreeSize += 1;
        int myError = subTreeSize - desiredSize;
        // if(Eval::write == false){
        //     Eval::write = true;
        //     ofstream fout;
        //     fout.open("./eval/MST.py", ios::out);
        //     fout << "edges = [";
        //     for(int i=1; i<=BaseSimulator::getWorld()->getNbBlocks();i++){
        //         SizeClustBlockCode *block = (SizeClustBlockCode*) BaseSimulator::getWorld()->getBlockById(i)->blockCode;
        //         // fout << "\n\t[";
        //         if(block->in_branch != nullptr)
        //             fout << "(" << block->module->blockId << "," << block->in_branch->getConnectedBlockId() << "), ";
                
        //     }
        //     fout << "]";
        // }
        
        if(!cutOn && !isRoot){
            if(abs(myError) < minError || (  abs(myError) >= minError && abs(myError)<=(minError + minError/2) && myHeight < maxHeight)){
                //if(!recut || myHeight <= 8){
                   minError = abs(myError);
                    maxHeight = myHeight;
                    toBestCut = nullptr; 
               //} 
                
            }
            if(resp.adj != -1){
                if(abs(myError) == minError && resp.nbAdj + nbAdj > maxNbAdj){
                    maxNbAdj += (resp.nbAdj + nbAdj);
                    toBestCut = nullptr;
                }
                if(maxNbAdj == 0){
                    minError = nbNodes;
                }
               
            }
           
            RespCutMsg resp1(subTreeSize, minError, maxHeight, resp.adj, maxNbAdj);
            console << "resp " << sender->getConnectedBlockId() << " " << resp.adj << " " << resp.error << "|" << resp.nbAdj << "\n";
            sendMessage("RespCut msg",
                new MessageOf<RespCutMsg>(RESPCUT_MSG_ID, resp1),
                in_branch, 100, 200);
        } else {
            if(isRoot && (!recut || desiredSize>0)){
                getScheduler()->toggle_pause();
                sendMessage("Cut Msg1", new MessageOf<pair<int,bool>>(CUT_MSG_ID, make_pair(clusterIndex+1, recut)), toBestCut, 100, 200);
            } else {
                sendMessage("Cut Msg3", new MessageOf<pair<int,bool>>(CUT_MSG_ID, make_pair(0, recut)), toBestCut, 100, 200);
            }
        }
    }
}

void SizeClustBlockCode::handleCutMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    MessageOf<pair<int,bool>> *msg = static_cast<MessageOf<pair<int,bool>>*>(msgPtr.get());
    //bool recvRecut = msg->getData()->second;
    int recvC = msg->getData()->first;
     if(!recut){
            toLastCH = toBestCut;
    }
    if(toBestCut == nullptr) {   
        C = msg->getData()->first;
        module->setColor(PINK);
       
        cutOn = true;
        nbWaitedAnswers = 0;
        if(isLeaf()){
           
            console << "C: " << C << "\n";
            for(auto &e: edges){
                //if(e.first != in_branch && (e.second.SE == rejected || e.second.SE == basic)){
                    sendMessage("Friend Msg", 
                        new MessageOf<pair<int,int>>(FRIEND_MSG_ID, make_pair(C,recut)),
                        e.first, 100, 200);
                //}
            }
            module->setColor(C);
            int myError = subTreeSize - desiredSize;
            (myError < 0)? myError = abs(myError): myError = -myError;
            sendMessage("ReporCut Msg", 
                    new MessageOf<int>(REPORTCUT_MSG_ID, myError),
                    in_branch, 100, 200);
            cutOn =false;
        }
        for(auto e:edges) {
            if(e.first != in_branch && e.second.SE == branch) {
                sendMessage("Assign Msg",
                    new MessageOf<pair<int,bool>>(ASSIGN_MSG_ID, make_pair(C, recut)), 
                    e.first, 100, 200);
                nbWaitedAnswers++;
            }
        }
        if(recut) {
                SN = SLEEP;
                LN = 0;
                for(auto &e: edges){
                    if(e.second.CH == C){
                        e.second.SE = basic;
                    } else {
                        e.second.SE = rejected;
                    }
                }
            }
    } else {
       
        sendMessage("Cut Msg4", new MessageOf<pair<int,bool>>(CUT_MSG_ID, make_pair(recvC, recut)), toBestCut, 100, 200);
    }
}


void SizeClustBlockCode::handleAssignMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    MessageOf<pair<int,bool>> *msg = static_cast<MessageOf<pair<int,bool>>*>(msgPtr.get());
    //recut = msg->getData()->second;
    C = msg->getData()->first;
    module->setColor(C);
    if(recut) {
        SN = SLEEP;
        FN.unique = 0; FN.density = 0;
        best_edge = nullptr;
        edges[in_branch].SE = basic;
        for(auto &e: edges){
            if(e.second.CH == C){
                e.second.SE = basic;
            }
        }
    }
    if(isLeaf()){
        edges[in_branch].CH = C;
        for(auto e: edges) {
            if(e.second.SE == rejected || e.second.SE == basic) {
                sendMessage("Friend Msg", 
                    new MessageOf<pair<int,int>>(FRIEND_MSG_ID, make_pair(C,recut)),
                   e.first, 100, 200);
            }
            
        }
        sendMessage("AssignAck msg", 
            new Message(ASSIGNACK_MSG_ID), 
            in_branch, 100, 200);
            // if(recut){
            //     in_branch = nullptr;
            // }
    } else {
        nbWaitedAnswers = 0;
        for(auto e:edges) {
            if(e.first != in_branch && e.second.SE == branch) {
                sendMessage("Assign Msg",
                    new MessageOf<pair<int,bool>>(ASSIGN_MSG_ID, make_pair(C, recut)), 
                    e.first, 100, 200);
                nbWaitedAnswers++;
            }
        }
    }
}

void SizeClustBlockCode::handleAssignAckMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    nbWaitedAnswers--;
    edges[sender].CH = C;
    if(nbWaitedAnswers == 0){
        for(auto e: edges) {
            if(e.second.SE == rejected || e.second.SE == basic){
                sendMessage("Friend Msg", 
                    new MessageOf<pair<int,int>>(FRIEND_MSG_ID, make_pair(C,recut)),
                    e.first, 100, 200);
            }
        }
        if(!cutOn){
            edges[in_branch].CH = C;
            sendMessage("AssignAck msg", 
                new Message(ASSIGNACK_MSG_ID), 
                in_branch, 100, 200);
        } else { //cutOn
            sendMessage("Friend Msg", 
                new MessageOf<pair<int,int>>(FRIEND_MSG_ID, make_pair(C,recut)),
                in_branch, 100, 200);
            cerr << "Desired Size: " << desiredSize << "\n";
            cerr << "MinError: " << (subTreeSize - desiredSize)<< "\n";
            cerr << "isRoot: " << isRoot << "\n";
            int myError = (subTreeSize - desiredSize);
            if(myError > 0 && !recut && !isRoot){
                //recut
                cut(true, myError, 0);
            } else {
                if(recut && !isRoot){
                    cutOn = false;
                }
                (myError < 0)? myError = abs(myError): myError = -myError;
                sendMessage("ReporCut0 Msg", 
                    new MessageOf<int>(REPORTCUT_MSG_ID, myError),
                    in_branch, 100, 200);
            }
        }
    }
}

void SizeClustBlockCode::handleReportCutMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    MessageOf<int> *msg = static_cast<MessageOf<int>*>(msgPtr.get());
    int recvError = *msg->getData();
    desiredSize = recvError;
    if(!cutOn) {
        if(sender != in_branch)
            sendMessage("ReportCut Msg", 
                    new MessageOf<int>(REPORTCUT_MSG_ID, recvError),
                    in_branch, 100, 200);
        else 
            sendMessage("ReportCut Msg", 
                    new MessageOf<int>(REPORTCUT_MSG_ID, recvError),
                    toLastCH, 100, 200);
    } else {
        // if(desiredSize != 0){
        //     recut = true;
        // } else {
        //     recut = false;
        // }
        // case 1: root
        if(isRoot) {
            if(!recut){
                toLastCH = toBestCut;
            }
            cerr << "REPORT " << recvError << endl;
            if(desiredSize > 0){
                cerr << "TEST " << clusterIndex << "\n";
                 getScheduler()->toggle_pause();
                recut = true;
                cut(recut, desiredSize, clusterIndex+1);
            } else if (desiredSize < 0){
                // send back toC
                
                sendMessage("ReportCut Msg", 
                    new MessageOf<int>(REPORTCUT_MSG_ID, abs(desiredSize)),
                    toLastCH, 100, 200);
                cerr << "SENDBACK TO C !!! \n\n";
                getScheduler()->toggle_pause();
            } else {
                 getScheduler()->toggle_pause();
                cout << "ROOT : " << clusterIndex << "\n";
                // update Tree
                find_count = 0;
                for(auto e: edges){
                    if(e.first != in_branch && e.second.SE == branch){
                        sendMessage("Initiate Msg1", new MessageOf<MsgData>(INITIATE_MSG_ID, MsgData(LN+1, FN, FIND)), e.first, 100, 200);
                        find_count++;
                    }
                }
                // if(clusterIndex < clustersSizes.size()-1){
                //     clusterIndex++;
                //     desiredSize = floor(nbNodes*clustersSizes[clusterIndex]);
                    
                //     recut = false;
                //     cut(recut, desiredSize, clusterIndex);
                // } else {
                //     check();
                // }
                
            }
            
            
        }
        // case 2:C
        else if(cutOn && !isRoot) {
            if(desiredSize > 0 && !recut) {
                cut(true, desiredSize, 0);
            } else if(desiredSize > 0 && recut){
                find_count = 0;
                FN.density = 0;
                for(auto e: edges){
                    if(e.first != in_branch && e.second.SE == branch){
                        sendMessage("Initiate Msg1", new MessageOf<MsgData>(INITIATE_MSG_ID, MsgData(LN+1, FN, FIND)), e.first, 100, 200);
                        find_count++;
                    }
                }
            }else {
                // send back to root
                sendMessage("ReportCut Msg", 
                    new MessageOf<int>(REPORTCUT_MSG_ID, abs(desiredSize)),
                    in_branch, 100, 200);
            }
        } 
    }
}


void SizeClustBlockCode::handleFriendMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    MessageOf<pair<int,bool>> *msg = static_cast<MessageOf<pair<int,bool>>*>(msgPtr.get());
    int recvCH = (*msg->getData()).first;
    bool exchanged = (*msg->getData()).second;
    console << "received friend msg from: " << sender->getConnectedBlockId() << " " << recvCH << "\n";
    if(recvCH != C){
        edges[sender].SE = rejected;
    }
    edges[sender].CH = recvCH;

    if(exchanged) {
        exchangedNeighbors.insert(sender);
        if(recvCH == C){
            edges[sender].SE = basic;
            //SN = FIND;
        }
        if(sender == in_branch && !cutOn){
            SN = SLEEP;
            //LN = 0;
            FN.unique = 0; FN.density = 0;
            //in_branch = nullptr;
            best_edge = nullptr;
            best_wt.density = -1; best_wt.unique = 0;
            // for(auto &e: edges) {
            //     if(e.second.CH == C && e.first != in_branch){
            //         e.second.SE = basic;
            //         sendMessage("Friend(-1) Msg", new MessageOf<pair<int,bool>>(FRIEND_MSG_ID, make_pair(C,true)), e.first, 100, 200);
            //     }
            // }
        }
    }
}

void SizeClustBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";

    // do stuff
    // ...
}

void SizeClustBlockCode::processLocalEvent(EventPtr pev) {
    MessagePtr message;
    stringstream info;

    // Do not remove line below
   Catoms3DBlockCode::processLocalEvent(pev);

    switch (pev->eventType) {
        case EVENT_STOP: {
            cerr<<"stop\n";
            break;
        }
        case EVENT_ADD_NEIGHBOR: {
            // Do something when a neighbor is added to an interface of the module
            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            break;
        }

        case EVENT_NI_RECEIVE: {

            if(waitingReportMsgs.size() > 0){
                deque<ReportMsg> waiting;
                for(auto w: waitingReportMsgs)
                    waiting.push_back(w);
                while(waiting.size()>0){
                    ReportMsg m = waiting.front();
                    waiting.pop_front();
                    waitingReportMsgs.pop_front();
                    handleReportMessage(m.msgPtr, m.sender);
                }
            }

            if(waitingConnectMsgs.size() > 0){
                deque<ConnectMsg> waiting;
                for(auto w: waitingConnectMsgs)
                    waiting.push_back(w);
                while(waiting.size()>0){
                    ConnectMsg m = waiting.front();
                    waiting.pop_front();
                    waitingConnectMsgs.pop_front();
                    handleConnectMessage(m.msgPtr, m.sender);
                }
            }

            if(waitingTestMsgs.size() > 0){
                deque<TestMsg> waiting;
                for(auto w: waitingTestMsgs)
                    waiting.push_back(w);
                while(waiting.size()>0){
                    TestMsg m = waiting.front();
                    waiting.pop_front();
                    waitingTestMsgs.pop_front();
                    handleTestMessage(m.msgPtr, m.sender);
                }
            }




            // if(waitingTestMsgs.size() > 0){
            //             while(!waitingTestMsgs.empty()){
            //                 TestMsg m = waitingTestMsgs.front();
            //                 waitingTestMsgs.pop_front();
            //                 handleTestMessage(m.msgPtr, m.sender);
            //             }
            //         }
            break;
        }
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void SizeClustBlockCode::onBlockSelected() {
    // Debug stuff:
   
    cerr<<"myNeighborhood: ";
    for(auto n: myNeighborhood){cerr<<n<<" ";}
    cerr<<endl;
    cerr<<"Edges: "<<endl;
    for(auto e: edges){
        if(e.first != nullptr)
            cerr<<e.first->getConnectedBlockId()<<": "<<" | "<<setprecision(6)<<e.second.w.density
            <<"," <<e.second.w.unique<<" | "<<e.second.SE<<" | " << e.second.CH <<endl;
    }
    // cerr<<"LN: "<<LN<<endl;
    // cerr<<"FN: "<<FN.density<<" | "<<FN.unique<<endl;
    cerr<<"SN: "<<SN<<endl;
    // cerr<<"best_wt: "<<best_wt.density<<" | "<<best_wt.unique<<endl;
    // if(best_edge != nullptr)
    //     cerr<<"best_edge: "<<best_edge->getConnectedBlockId()<<endl;
    if(in_branch != nullptr)
        cerr<<"in-branch: "<<in_branch->getConnectedBlockId()<<endl;
    cerr<<endl;
    cerr<<"Subtree size: "<<subTreeSize<<"\n";
    cerr << "desiredSize: " << desiredSize << "\n"; 
    //cerr<<"minDist: "<<minDist<<endl;
    cerr<<"cutOn: "<<cutOn<<endl;
    cerr <<"C: " << C << endl;
    cerr << "recut: " << recut << endl;
    cerr << "maxHeight: " << maxHeight << endl;
    cerr << "minError: " << minError << endl;
    cerr << module->blockId << endl;
    cerr << hostBlock->getNbInterfaces() << endl;
    //setColor(CH);
    //  Eval eval(BaseSimulator::getWorld());
    // // eval.calculateModularity();
    //  eval.writeToFile();
    //   system("python3 ./eval/evalGraph.py");
    // eval.fixColors();
    // eval.calculateSigmaC();
    // eval.calculateDBindexC();
    // eval.calculateDBindexCG();
}

void SizeClustBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

// bool SizeClustBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
//     /* Reading the command line */
//     if ((argc > 0) && ((*argv)[0][0] == '-')) {
//         switch((*argv)[0][1]) {

//             // Single character example: -b
//             case 'b':   {
//                 cerr << "-b option provided" << endl;
//                 return true;
//             } break;

//             // Composite argument example: --foo 13
//             case '-': {
//                 string varg = string((*argv)[0] + 2); // argv[0] without "--"
//                 if (varg == string("foo")) { //
//                     try {
//                         int fooArg = stoi((*argv)[1]);
//                         argc--;
//                         (*argv)++;
//                     } catch(std::logic_error&) {}

//                     cerr << "--foo option provided with value: " << fooArg << endl;
//                 } else return false;

//                 return true;
//             }

//             default: cerr << "Unrecognized command line argument: " << (*argv)[0] << endl;
//         }
//     }

//     return false;
// }