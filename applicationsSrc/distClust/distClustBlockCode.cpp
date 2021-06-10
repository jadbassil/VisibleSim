#include "distClustBlockCode.hpp"

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

using namespace Catoms3D;

DistClustBlockCode::DistClustBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Register callbacks
    addMessageEventFunc2(REQUESTDENSITY_MSG_ID,
                         std::bind(&DistClustBlockCode::handleRequestDensityMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(SENDDENSITY_MSG_ID,
                         std::bind(&DistClustBlockCode::handleSendDensityMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CONNECT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleConnectMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(INITIATE_MSG_ID,
                         std::bind(&DistClustBlockCode::handleInitiateMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(TEST_MSG_ID,
                         std::bind(&DistClustBlockCode::handleTestMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(REPORT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleReportMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(ACCEPT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleAcceptMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(REJECT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleRejectMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CHANGEROOT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleChangeRootMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(REQUESTDCUT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleRequestdcutMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(RESPONSEDCUT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleResponsedcutMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CUT_MSG_ID,
                         std::bind(&DistClustBlockCode::handleCutMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CUTACK_MSG_ID,
                         std::bind(&DistClustBlockCode::handleCutAckMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(REQUESTNBNODES_MSG_ID,
                         std::bind(&DistClustBlockCode::handleRequestNbNodesMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(RESPONSENBNODES_MSG_ID,
                         std::bind(&DistClustBlockCode::handleResponseNbNodesMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    // Set the module pointer
    module = static_cast<Catoms3DBlock*>(hostBlock);
  }

void DistClustBlockCode::startup() {
    setMyNeighborhood();
    SN = SLEEP;
    LN = 0;
    in_branch = nullptr;
    Dcut = INF;
    cutAt = -1;
    subTreeSize = 0;
    nbc = 0;
    CH = -1;
    isCH = false;
    leaderPosition = Cell3DPosition(50,50,50);
    //nbNodes = NB_NODES;
    //module->setColor(edges.size());
    // if(module->blockId == 2){
    //     wakeup();
    // }
    // if(module->blockId % 2 == 0){
    //     wakeup();
    // }
    //wakeup();
    leaderPosition = Cell3DPosition(50,50,50);
    //nbNodes = NB_NODES;
    //module->setColor(edges.size());
    // if(module->blockId == 2){
    //     wakeup();
    // }
    // if(module->blockId % 2 == 0){
    //     wakeup();
    // }
    //wakeup();

    setEdges();
}

void DistClustBlockCode::setColor(int CH){
    for(int i=0;i<clustersColors.size(); i++){   
       if(clustersColors[i] == CH){
            module->setColor(i);
            if(i == 2)
                module->setColor(MAGENTA);
        }
    }
}

void DistClustBlockCode::setMyNeighborhood(){
    myNeighborhood.push_back(module->blockId);
    for(int i=0;i<hostBlock->getNbInterfaces();i++){
        short n = hostBlock->getNeighborIDForFace(i);
        if(n != 0){
            myNeighborhood.push_back(n);
        }
    }
}

void DistClustBlockCode::setEdges(){
    for(int i=0;i<hostBlock->getNbInterfaces();i++){
        short n = hostBlock->getNeighborIDForFace(i);
        if(n != 0){
            if(module->getInterface(i)->getConnectedBlockId() < module->blockId){
                sendMessage("RequestDensity Msg", new MessageOf<vector<int>>(REQUESTDENSITY_MSG_ID, myNeighborhood), module->getInterface(i), 100, 200);
            }
        }
    }
}

int DistClustBlockCode::min(int a, int b){
    return (a<b) ? a : b;
}

float DistClustBlockCode::JC(vector<int> receivedNeigh){
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

P2PNetworkInterface* DistClustBlockCode::get_min_wt(){
    P2PNetworkInterface* minEdge = edges.begin()->first;
    Density minWt = edges.begin()->second.w;
    for(auto e: edges){
        if(e.second.w < minWt){
            minWt = e.second.w;
            minEdge = e.first;
        }
    }
    return minEdge;
}

void DistClustBlockCode::wakeup(){
    if(edges.size() == myNeighborhood.size() - 1){
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

void DistClustBlockCode::test(){
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

void DistClustBlockCode::report(){
    console<<"Executing report\n";
    console<<"Find_count: "<<find_count<<"\n";
     
    if(test_edge != nullptr)
        console<<"Test_edge: "<<test_edge->getConnectedBlockId()<<"\n";
    if(SN == FIND)
        module->setColor(YELLOW);
    if(find_count == 0 && test_edge == nullptr){
        console<<"test report\n";
        SN = FOUND;
        module->setColor(BLUE);

        sendMessage("Report Msg", new MessageOf<Density>(REPORT_MSG_ID, best_wt), in_branch, 100, 200);
    }
   
}

void DistClustBlockCode::change_root(){
    if(edges[best_edge].SE == branch){
        sendMessage("ChangeRoot Msg", new Message(CHANGEROOT_MSG_ID), best_edge, 100, 200);
    }
    else{
        sendMessage("Connect Msg", new MessageOf<int>(CONNECT_MSG_ID, LN), best_edge, 100, 200);
        edges[best_edge].SE = branch;
    }
}

void DistClustBlockCode::handleRequestDensityMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
    MessageOf<vector<int>>* msg = static_cast<MessageOf<vector<int>>*>(msgPtr.get());
    vector<int> receivedNeigh = *msg->getData(); 
    vector<Cell3DPosition> anchors;
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
    minX = maxX = BaseSimulator::getWorld()->buildingBlocksMap.begin()->second->position.pt[0];
    minY = maxY = BaseSimulator::getWorld()->buildingBlocksMap.begin()->second->position.pt[1];
    minZ = maxZ = BaseSimulator::getWorld()->buildingBlocksMap.begin()->second->position.pt[2];
    for(auto i: BaseSimulator::getWorld()->buildingBlocksMap){
        if(i.second->position.pt[0] < minX) minX = i.second->position.pt[0];
        if(i.second->position.pt[0] > maxX) maxX = i.second->position.pt[0];
        if(i.second->position.pt[1] < minY) minY = i.second->position.pt[1];
        if(i.second->position.pt[1] > minY) minY = i.second->position.pt[1];
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
    if(module->blockId == 5){
        cout << "minDist: " << minDist << "\n";
        cout << "nminDist: " << nminDist << "\n";
    }
    if(minDist == 0){
        minDist = 1;
    }
    if(nminDist == 0){
        nminDist = 1;
    }
    minDist = min(minDist, nminDist);
   
    float d = (float)JC(receivedNeigh)/(minDist);
    //float d = (float)1/(minDist);
    //float d = JC(receivedNeigh);
    Edge e(module->blockId, sender->getConnectedBlockId(), d);
    edges[sender] = e;
    sendMessage("Send Density", new MessageOf<float>(SENDDENSITY_MSG_ID, d), sender, 100, 200);
    if(edges.size() == myNeighborhood.size()-1){
        console<<"All edges densities computed\n";
        module->setColor(CYAN);
    }
    anchors.clear();

}

void DistClustBlockCode::handleSendDensityMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
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

void DistClustBlockCode::handleConnectMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received Connect from "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<int>* msg = static_cast<MessageOf<int>*>(msgPtr.get());
    int L = *(msg)->getData();
    if(SN == SLEEP)
        wakeup();
    
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

void DistClustBlockCode::handleInitiateMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received Initiate Msg from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<MsgData>* msg = static_cast<MessageOf<MsgData>*>(msgPtr.get());
    MsgData rcvMsg = *msg->getData();
    LN = rcvMsg.LN;
    FN = rcvMsg.FN;
    SN = rcvMsg.SN;
    
    console<<"LN: "<<LN<<"\n";
    in_branch = sender;
    module->setColor(BLUE);
    best_edge = nullptr;
    best_wt.density = -1; best_wt.unique = 0;
    
    
    for(auto e: edges){
        if(e.first != sender && e.second.SE == branch){
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

void DistClustBlockCode::handleTestMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received Test Message from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<MsgData>* msg = static_cast<MessageOf<MsgData>*>(msgPtr.get());
    MsgData rcvMsg = *msg->getData();
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

void DistClustBlockCode::handleReportMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received report from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<Density>* msg = static_cast<MessageOf<Density>*>(msgPtr.get());
    Density w = *msg->getData();
    console<<"w: "<<w.density<<" | "<<w.unique<<"\n";
    console<<"waiting report: "<<waitingReportMsgs.size()<<"\n";
    if(sender != in_branch){
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
        cout<<module->blockId<<" : DCT Built\n"; 
        if(module->blockId<in_branch->getConnectedBlockId()){
            //module->setColor(GREEN);
            in_branch = nullptr;
            nbWaitedFromChildren = 0;
            for(auto e: edges){
                if(e.second.SE == branch){
                    sendMessage("RequestNbNodes msg", new Message(REQUESTNBNODES_MSG_ID), e.first, 100, 200);
                    nbWaitedFromChildren++;
                }
            }
        }
    }
}

void DistClustBlockCode::handleRejectMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received reject message from: "<<sender->getConnectedBlockId()<<"\n";
    if(edges[sender].SE == basic){
        edges[sender].SE = rejected;
    }
    test();    
}

void DistClustBlockCode::handleAcceptMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received accept message from: "<<sender->getConnectedBlockId()<<"\n";
    test_edge = nullptr;
    if(edges[sender].w < best_wt){
        best_edge = sender;
        best_wt = edges[sender].w;
    }
    report();
}

void DistClustBlockCode::handleChangeRootMessage(MessagePtr msgPtr, P2PNetworkInterface* sender) {
    console<<"Received change-root message from: "<<sender->getConnectedBlockId()<<"\n";
    change_root();
}

void DistClustBlockCode::handleRequestNbNodesMessage(MessagePtr msgPtr, P2PNetworkInterface *sender){
    if(isLeaf()){
        subTreeSize = 1;
        sendMessage("ResponseNbNodes Msg", new MessageOf<int>(RESPONSENBNODES_MSG_ID, subTreeSize), in_branch, 100, 200);
    }else{
        nbWaitedFromChildren = 0;
        for(auto e: edges){
            if(e.second.SE == branch && e.first != in_branch){
                sendMessage("RequestNbNodes msg", new Message(REQUESTNBNODES_MSG_ID), e.first, 100, 200);
                nbWaitedFromChildren++;
            }
        }
    }
}
void DistClustBlockCode::handleResponseNbNodesMessage(MessagePtr msgPtr, P2PNetworkInterface *sender){
    MessageOf<int> *msg = static_cast<MessageOf<int>*>(msgPtr.get());
    int s = *msg->getData();
    subTreeSize += s;
    nbWaitedFromChildren--;
    if(nbWaitedFromChildren == 0){
        subTreeSize++;
        if(in_branch != nullptr)
            sendMessage("ResponseNbNodes Msg", new MessageOf<int>(RESPONSENBNODES_MSG_ID, subTreeSize), in_branch, 100, 200);
        else{//root
            isCH = true;
            nbc = NB_CLUSTERS - 1;
            clustersColors.push_back(module->blockId);
            cut();
        }
    }
}

bool DistClustBlockCode::isLeaf(){
    bool isLeaf = true;
    for(auto e: edges){
        if(e.second.SE == branch && e.first != in_branch)
            isLeaf = false;
    }
    return isLeaf;
}

void DistClustBlockCode::cut(){
    console<<"CUT\n";
    nbc--;
    CH = module->blockId;
    //getScheduler()->toggle_pause();
    //module->setColor(CH%10);
    setColor(CH);
    DCutMsg msg(CH, subTreeSize);
    nbWaitedFromChildren = 0;
    Dcut = INF;
    toMinCut = nullptr;
    for(auto e: edges){
        if(e.second.SE == branch && e.first != in_branch){
            sendMessage("RequestDcut Msg", new MessageOf<DCutMsg>(REQUESTDCUT_MSG_ID, msg), e.first, 100, 200);
            nbWaitedFromChildren++;
        }
    }
}


void DistClustBlockCode::handleRequestdcutMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
    console<<"Received RequestDcut Msg from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<DCutMsg>* msg = static_cast<MessageOf<DCutMsg>*>(msgPtr.get());
    DCutMsg dmsg = *msg->getData();
    if(isCH){
        return;
    }
    CH = dmsg.CHid;
    Dcut = INF;
    toMinCut = nullptr;
    //module->setColor(dmsg.CHid%10);
    setColor(CH);
    if(isLeaf()){
        dmsg.dcut = edges[in_branch].w.density/min(subTreeSize, abs(dmsg.nbNodes-subTreeSize));
        dmsg.cutAt = module->blockId;
        dmsg.subTreeSize = subTreeSize;
        console<<"Leaf dcut: "<<dmsg.dcut<<"\n";
        //send response to parent
        sendMessage("ResponseDcut msg", new MessageOf<DCutMsg>(RESPONSEDCUT_MSG_ID, dmsg), in_branch, 100, 200);
    }else{
        nbWaitedFromChildren = 0;
        // for(auto c: children){
        //     sendMessage("RequestDcut Msg", new MessageOf<DCutMsg>(REQUESTDCUT_MSG_ID, dmsg), c, 100, 200);
        //     nbWaitedFromChildren++;
        // }
        for(auto e: edges){
            if(e.second.SE == branch && e.first != in_branch){
                sendMessage("RequestDcut Msg", new MessageOf<DCutMsg>(REQUESTDCUT_MSG_ID, dmsg), e.first, 100, 200);
                nbWaitedFromChildren++;
            }
        }
    }
}

void DistClustBlockCode::handleResponsedcutMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
    console<<"Received ResponseDcut Msg from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<DCutMsg>* msg = static_cast<MessageOf<DCutMsg>*>(msgPtr.get());
    DCutMsg dmsg = *msg->getData();
    
    nbWaitedFromChildren--;
    // if(!isCH)
    //     subTreeSize += dmsg.subTreeSize;
    if(dmsg.dcut < Dcut){
        Dcut = dmsg.dcut;
        cutAt = dmsg.cutAt;
        toMinCut = sender;   
        subtree_tmp = dmsg.subTreeSize;  
    }
    if(nbWaitedFromChildren == 0){
        if(!isCH){
            float myDcut = edges[in_branch].w.density/min(subTreeSize, abs(dmsg.nbNodes-subTreeSize));
            console<<"Min: "<<min(subTreeSize, abs(dmsg.nbNodes-subTreeSize));
            console<<"Dcut: "<<Dcut<<"\n";
            console<<"myDcut: "<<myDcut<<"\n";
            if(myDcut <= Dcut){
                dmsg.dcut = myDcut;
                dmsg.cutAt = module->blockId;
                dmsg.subTreeSize = subTreeSize;
            }else{
                dmsg.dcut = Dcut;
                dmsg.cutAt = cutAt;
                dmsg.subTreeSize = subtree_tmp;
            }
            //dmsg.subTreeSize = subTreeSize;
            console<<"cutAt: "<<dmsg.cutAt<<" dcut: "<<dmsg.dcut<<"\n";
            sendMessage("ResponseDcut Msg", new MessageOf<DCutMsg>(RESPONSEDCUT_MSG_ID, dmsg), in_branch, 100, 200);
        }else{//isCH
            //cutAt = dmsg.cutAt;
            CutMsg cmsg(cutAt);
            if(dmsg.subTreeSize > (subTreeSize - dmsg.subTreeSize) ){
                cmsg.nbc = ( ceil( ((float)nbc)/2 ));
                //CutMsg cmsg(cutAt, ( ceil( ((float)nbc)/2 )));
                nbc = floor( ((float)nbc)/2 );
            }else{
                cmsg.nbc = ( floor( ((float)nbc)/2 ));
                nbc = ceil( ((float)nbc)/2 ); 
            }
            
            console<<"nbc: "<<nbc<<"\n";
            console<<"cutAt: "<<dmsg.cutAt<<" dcut: "<<dmsg.dcut<<"\n";
            console<<"dmsg.subtreesize: "<<dmsg.subTreeSize<<"\n";
            if(edges.find(toMinCut) != edges.end()){
                if(edges[toMinCut].SE == branch && edges[toMinCut].nid == cutAt){
                    edges[toMinCut].SE = rejected;
                }
            }
            //int c = toMinCut->getConnectedBlockId();
            // if(cutAt == c){
            //     children.erase(find(children.begin(), children.end(), toMinCut));
            // }

            // for(auto e: edges){
            //     if(e.second.SE == branch && e.first != in_branch){
            //         sendMessage("Cut Msg", new MessageOf<CutMsg>(CUT_MSG_ID, cmsg), e.first, 100, 200);
            //     }
            // } 
            console<<"cutAt: "<<cutAt<<"\n";
            if(toMinCut != nullptr){
                sendMessage("Cut Msg", new MessageOf<CutMsg>(CUT_MSG_ID, cmsg), toMinCut, 100, 200);
            }  
        } 
    }
}

void DistClustBlockCode::handleCutMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
    console<<"Received cut msg from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<CutMsg> *msg = static_cast<MessageOf<CutMsg>*>(msgPtr.get());
    CutMsg cmsg = *msg->getData();
    console<<"cmsg.CHid: "<<cmsg.CHid<<"\n";
    if(module->blockId == cmsg.CHid){
        cmsg.found = true;
        isCH = true;
        clustersColors.push_back(module->blockId);
        sendMessage("CutAck Msg", new MessageOf<int>(CUTACK_MSG_ID, subTreeSize), in_branch, 100, 200);
        nbc = cmsg.nbc;
        if(nbc > 0){
            cut();
            //nbc--;
        }else{
            cout<<"CH: "<<module->blockId<<" size: "<<subTreeSize<<endl;
        }
    }
    if(cmsg.found){
        CH = cmsg.CHid;
        //module->setColor(CH%10);
        setColor(CH);
        if(nbc == 0){
            for(auto e: edges){
                if(e.second.SE == branch && e.first != in_branch){
                    sendMessage("Cut Msg", new MessageOf<CutMsg>(CUT_MSG_ID, cmsg), e.first, 100, 200);
                }
            }
        }
        // for(auto c: children){
        //     sendMessage("Cut Msg", new MessageOf<CutMsg>(CUT_MSG_ID, cmsg), c, 100, 200);
        // }
    }else{
        int c = toMinCut->getConnectedBlockId();
        // if(cutAt == c){
        //     children.erase(find(children.begin(), children.end(), toMinCut));
        // }
        if(edges.find(toMinCut) != edges.end()){
                if(edges[toMinCut].SE == branch && edges[toMinCut].nid == cutAt){
                    edges[toMinCut].SE = rejected;
                }
            }
        if(toMinCut != nullptr){
            sendMessage("Cut Msg", new MessageOf<CutMsg>(CUT_MSG_ID, cmsg), toMinCut, 100, 200);
        } 
    }
    // for(auto& e: edges){
    //     if(e.second.SE == branch && e.first != in_branch){
    //         sendMessage("Cut Msg", new MessageOf<CutMsg>(CUT_MSG_ID, cmsg), e.first, 100, 200);
    //         if(e.first->getConnectedBlockId() == cmsg.CHid){
    //             e.second.SE = rejected;
    //             console<<"test: "<<e.first->getConnectedBlockId()<<" "<<cmsg.CHid;
    //         }
    //     }
    // }
}

void DistClustBlockCode::handleCutAckMessage(MessagePtr msgPtr, P2PNetworkInterface* sender){
    console<<"Received cutAck msg from: "<<sender->getConnectedBlockId()<<"\n";
    MessageOf<int> *msg = static_cast<MessageOf<int>*>(msgPtr.get());
    int n = *msg->getData();
    subTreeSize -= n;
    console<<nbc<<"\n";
    if(!isCH){
        sendMessage("CutAck msg", new MessageOf<int>(CUTACK_MSG_ID, n), in_branch, 100, 200);
    }else{// isCH
        console<<"nbc: "<<nbc<<"\n";
        cout<<"CH: "<<CH<<", size: "<<subTreeSize<<endl;
        if(nbc>0){
            cut();
            //nbc--;
        }
    }
}

void DistClustBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";

    // do stuff
    // ...
}

void DistClustBlockCode::processLocalEvent(EventPtr pev) {
    MessagePtr message;
    stringstream info;

    // Do not remove line below
   Catoms3DBlockCode::processLocalEvent(pev);

    switch (pev->eventType) {

        case EVENT_ADD_NEIGHBOR: {
            // Do something when a neighbor is added to an interface of the module
            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            break;
        }

        case EVENT_NI_RECEIVE: {
            console<<"handling waiting\n";
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

void DistClustBlockCode::onBlockSelected() {
    // Debug stuff:
    cout<<"myNeighborhood: ";
    for(auto n: myNeighborhood){cout<<n<<" ";}
    cout<<endl;
    cout<<"Edges: "<<endl;
    for(auto e: edges){
        if(e.first != nullptr)
            cout<<e.first->getConnectedBlockId()<<": "<<" | "<<e.second.w.density<<"," <<e.second.w.unique<<" | "<<e.second.SE<<endl;
    }
    cout<<"LN: "<<LN<<endl;
    cout<<"FN: "<<FN.density<<" | "<<FN.unique<<endl;
    cout<<"SN: "<<SN<<endl;
    cout<<"best_wt: "<<best_wt.density<<" | "<<best_wt.unique<<endl;
    if(best_edge != nullptr)
        cout<<"best_edge: "<<best_edge->getConnectedBlockId()<<endl;
    if(in_branch != nullptr)
        cout<<"in-branch: "<<in_branch->getConnectedBlockId()<<endl;
    cout<<endl;
    cout<<"Subtree size: "<<subTreeSize<<"\n";
    //cout<<"minDist: "<<minDist<<endl;
    cout<<"CH: "<<CH<<endl;
    cout<<"isCH: "<<isCH<<endl;
}

void DistClustBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

// bool DistClustBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
//     /* Reading the command line */
//     if ((argc > 0) && ((*argv)[0][0] == '-')) {
//         switch((*argv)[0][1]) {

//             // Single character example: -b
//             case 'b':   {
//                 cout << "-b option provided" << endl;
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

//                     cout << "--foo option provided with value: " << fooArg << endl;
//                 } else return false;

//                 return true;
//             }

//             default: cerr << "Unrecognized command line argument: " << (*argv)[0] << endl;
//         }
//     }

//     return false;
// }
