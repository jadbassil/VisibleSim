// #include "blinkyBlocksWorld.h"
// #include "blinkyBlocksBlockCode.h"
#include "sizeClustBlockCode.hpp"
#include <algorithm>    
#include <queue>

class Eval {
public:
    BaseSimulator::World *World;
    int nbBlocks;
    map<int, vector<SizeClustBlockCode*>> clusters;
    map<int, Vector3D> cluster_cg;
    
    map<int, float> clusters_sigmaCG; // the average distance of all points in a cluster from the centroid
    map<int, float> clusters_sigmaC;
    map<int, set<int>> cluster_neighbors;
    float sigmaAvg{0};
    float DBindexCG{0}; // smaller is better
    float DBindexC{0};
    float DBindex1{-1};
    float modularity{0};
    void setClusterNeighbors();

    Eval(BaseSimulator::World *);
    ~Eval();
    static map<int, SizeClustBlockCode*> cluster_centroid;
    static void fixColors(BaseSimulator::World *);
    static Vector3D getCG(BaseSimulator::World *world);
    static vector<vector<int>> distanceMatrix;
    void printSizes();
    void buildDistanceMatrix();
    map<int, Vector3D> calculateCGs();
    map<int, float> calculateSigmaCG();
    map<int, float> calculateSigmaC();
    float calculateSigmaAvgCG();
    float calculateDBindexCG();
    float calculateDBindexCG1(int, int);
    float calculateDBindexC();
    float calculateModularity();
    int maxDist(int C);
    SizeClustBlockCode* calculateCentroid(int);
    void printClusterNeighbors();
    float getDBIndex(){ return DBindexCG; }
    float getDBIndex1(){ return DBindex1; }
    void fixColors();
    float distanceToCG(int, Cell3DPosition);
    static float distanceToCG(Vector3D, Cell3DPosition);
    bool checkConnected(int);
    void printConnected();
    void writeToFile();
    void writeMstToFile();
    static bool write;
};

map<int, SizeClustBlockCode*> Eval::cluster_centroid;
vector<vector<int>> Eval::distanceMatrix;
bool Eval::write = false;

Eval::Eval(BaseSimulator::World *world): World(world) {
    nbBlocks = World->getNbBlocks();
    for(int i=1; i<=nbBlocks; i++) {
        SizeClustBlockCode* N = (SizeClustBlockCode*) world->getBlockById(i)->blockCode;
        clusters[N->C].push_back(N);
        // N->setColor(N->C);
        // if(N->C == N->getModule()->blockId){
        //     N->getModule()->setColor(BMOWN);
        // }
    }
    setClusterNeighbors();
    printSizes();
    calculateCGs();
    calculateSigmaCG();
    //calculateSigmaC();
    calculateSigmaAvgCG();
    calculateDBindexCG();
    //calculateDBindexC();
}

Eval::~Eval(){}

void Eval::fixColors(BaseSimulator::World *World) {
    int nbBlocks = World->getNbBlocks();
    for(int i=1; i<=nbBlocks; i++) {
        SizeClustBlockCode* N = (SizeClustBlockCode*) World->getBlockById(i)->blockCode;
        N->setColor(N->C);
        if(N->C == N->getModule()->blockId) {
            N->getModule()->setColor(BROWN);
        }
    }
}

void Eval::fixColors() {
    for(auto c: clusters) {
        for(auto m: c.second) {
            m->setColor(c.first);
            if(m->C == m->getModule()->blockId) {
                m->getModule()->setColor(BROWN);
            }
        }
        //calculateCentroid(c.first);
    }
}

void Eval::setClusterNeighbors(){
    for(auto c: clusters) {
        cluster_neighbors[c.first].clear();
        for(auto m: c.second) {
            for(auto nid: m->getMyNeighborhood()){
                SizeClustBlockCode* N = (SizeClustBlockCode*) World->getBlockById(nid)->blockCode;
                if(N->C != c.first){
                    cluster_neighbors[c.first].insert(N->C);
                }
            }
        }
    }
    printClusterNeighbors();
}

void Eval::printClusterNeighbors() {
    cout << endl << "cluster_neighbors: " << endl;
    for(auto c: cluster_neighbors) {
        cout << c.first <<": ";
        for(auto n: c.second) {
            cout << n << " ";
        }
        cout << endl;
    }
}

void Eval::printSizes() {
    cout << "clusters_count: " << endl;
    for(auto c: clusters) {
        cout << c.first << ": " << c.second.size() << endl;
    }
    cout << endl;
}

void Eval::buildDistanceMatrix(){
    if(Eval::distanceMatrix.size() != 0){
        return;
    }
    Eval::distanceMatrix.push_back(vector<int>(World->getNbBlocks()+1, -1));
    cout << "test\n";
    for(int i=1; i<=World->getNbBlocks(); i++){
        Eval::distanceMatrix.push_back(vector<int>(World->getNbBlocks()+1, -1));
        for(int j=1; j<=i; j++) {
            SizeClustBlockCode *blockI = (SizeClustBlockCode*) World->getBlockById(i)->blockCode;
            SizeClustBlockCode *blockJ = (SizeClustBlockCode*) World->getBlockById(j)->blockCode;
            if(find(blockI->myNeighborhood.begin(), blockI->myNeighborhood.end(), blockJ->module->blockId)
                != blockI->myNeighborhood.end()){
                Eval::distanceMatrix[i][j] = Eval::distanceMatrix[j][i] = 1;
            }
            if(i == j){
                Eval::distanceMatrix[i][j] = 0;
            }
        }
    }
    // fill distance matrix
    cout << "Building distances matrix;"<< endl;
    int l = 0;
    bool e = true;
    while(e){
        cout << l++ << endl;
        e = false;
        for (int i = 1; i <= World->getNbBlocks(); i++) {
            for (int j = 1; j <= i;  j++) {
                if (Eval::distanceMatrix[i][j] > 0) {
                    for (int k = 1; k <= i ; k++) {
                        if(Eval::distanceMatrix[j][k]>0)
                            if (Eval::distanceMatrix[i][k] == -1 || Eval::distanceMatrix[i][j] + Eval::distanceMatrix[j][k] < Eval::distanceMatrix[i][k]) {
                                Eval::distanceMatrix[i][k] =  Eval::distanceMatrix[k][i]= Eval::distanceMatrix[i][j] + Eval::distanceMatrix[j][k];
                               
                                e = true;
                            }
                    }
                }
            }
        }
    }
    cout << "DistanceMatrix Built\t";
}

map<int, Vector3D> Eval::calculateCGs() {
    for(auto c: clusters) {
        cluster_cg[c.first] = Vector3D(0,0,0);
        for(auto m: c.second) {
            cluster_cg[c.first].pt[0] += m->getModule()->position.pt[0];
            cluster_cg[c.first].pt[1] += m->getModule()->position.pt[1];
            cluster_cg[c.first].pt[2] += m->getModule()->position.pt[2];
        }
        cluster_cg[c.first].pt[0] /= clusters[c.first].size();
        cluster_cg[c.first].pt[1] /= clusters[c.first].size();
        cluster_cg[c.first].pt[2] /= clusters[c.first].size();
    }
    cout << "\nClusters_centroids: " << endl;
    for(auto cc: cluster_cg) {
        cout <<cc.first << ": " << cc.second << "| size: " << clusters[cc.first].size() << endl;
    }
    return cluster_cg;
}

map<int, float> Eval::calculateSigmaCG() {
    for(auto c: clusters) {
        clusters_sigmaCG[c.first] = 0;
        for(auto m: c.second) {
            float eucl_dist = sqrt( 
                    pow(cluster_cg[c.first].pt[0] - m->getModule()->position.pt[0], 2) +
                    pow(cluster_cg[c.first].pt[1] - m->getModule()->position.pt[1], 2) +
                    pow(cluster_cg[c.first].pt[2] - m->getModule()->position.pt[2], 2)    
                );
            clusters_sigmaCG[c.first] += eucl_dist;
        }
        clusters_sigmaCG[c.first] /= clusters[c.first].size();
    }
    cout << "\nclusters_sigmaCG: " << endl;
    for(auto cc: clusters_sigmaCG) {
        cout <<cc.first << ": " << cc.second << endl;
    }
    cout << endl;
    return clusters_sigmaCG;
}

map<int, float> Eval::calculateSigmaC() {
    for(auto c: clusters) {
        clusters_sigmaC[c.first] = 0;
        SizeClustBlockCode * C = calculateCentroid(c.first);
        for(auto m: c.second) {
            clusters_sigmaC[c.first] += Eval::distanceMatrix[C->module->blockId][m->module->blockId];
        }
        clusters_sigmaC[c.first] /= (float)clusters[c.first].size();
    }
    return clusters_sigmaC;
}

float Eval::calculateSigmaAvgCG() {
    for(auto cs: clusters_sigmaCG) {
        sigmaAvg += cs.second;
    }
    sigmaAvg /= clusters_sigmaCG.size();
    cout << "sigmaAvg: " << sigmaAvg << endl;
    return sigmaAvg;
}

float Eval::calculateDBindexCG() {
    float num = 0;
    for(auto ci: clusters) {
        float max = -1;
        int maxj;
        for(auto cj: clusters) {
            if(cj.first != ci.first) {
                float dij = sqrt( 
                    pow(cluster_cg[ci.first].pt[0] - cluster_cg[cj.first].pt[0], 2) +
                    pow(cluster_cg[ci.first].pt[1] - cluster_cg[cj.first].pt[1], 2) +
                    pow(cluster_cg[ci.first].pt[2] - cluster_cg[cj.first].pt[2], 2)  
                );
                float x = (clusters_sigmaCG[ci.first] + clusters_sigmaCG[cj.first]) / dij;
                if(x > max) { max = x; maxj = cj.first;} 
            }
        }
        //cout << ci.first << " maxDB with: " << maxj << "\n";
        num += max;
    }
    DBindexCG = num / clusters.size();
    cout << "DBindexCG: " << setprecision(5) << DBindexCG << endl;
    return DBindexCG;
}

int Eval::maxDist(int C) {
    int maxDist = -1;
    if(distanceMatrix.size() == 0){
        buildDistanceMatrix();
    }
    for(auto b1: clusters[C]) {
        for(auto b2: clusters[C]) {
            if(distanceMatrix[b1->module->blockId][b2->module->blockId] > maxDist) {
                maxDist = distanceMatrix[b1->module->blockId][b2->module->blockId];
            }
        }
    }
    cerr <<"maxDist:" << maxDist << endl;
    return maxDist;
}

float Eval::calculateDBindexC() {
    if(distanceMatrix.empty()){
        buildDistanceMatrix();
    }
    float num = 0;
    for(auto ci: clusters) {
        float max = -1;
        int maxj;
        for(auto cj: clusters) {
            if(cj.first != ci.first) {
                float dij = Eval::distanceMatrix[ci.first][cj.first];
                float x = (clusters_sigmaC[ci.first] + clusters_sigmaC[cj.first]) / dij;
                if(x > max) { max = x; maxj = cj.first;} 
            }
        }
        num += max;
    }
    DBindexC = num / clusters.size();
    cout << "DBindexC: " << setprecision(5) << DBindexC << endl;
    return DBindexC;
}

float Eval::calculateModularity() {
    double Q = 0;
    int m = 0;
    vector<vector<bool>> A;
    A.push_back(vector<bool>(World->getNbBlocks()+1, 0));
    for(int i=1; i<=World->getNbBlocks(); i++){
        A.push_back(vector<bool>(World->getNbBlocks()+1, 0));
    }
    cout << A.size() << " " << A[0].size() << endl;
    for(int i=1; i<= World->getNbBlocks(); i++){
        SizeClustBlockCode *block = (SizeClustBlockCode*) World->getBlockById(i)->blockCode;
        for(int j=1; j<block->myNeighborhood.size(); j++) {
            //cout << "test\n";
            A[block->module->blockId][block->myNeighborhood[j]] = 1;
            m++;
        }
    }
    m /= 2;
    for(int v=1; v<A.size(); v++){
        int kv = count(A[v].begin(), A[v].end(), 1);
        for(int w=1; w<A.size(); w++){
            int kw = count(A[w].begin(), A[w].end(), 1);
            SizeClustBlockCode *blockV = (SizeClustBlockCode*) World->getBlockById(v)->blockCode;
            SizeClustBlockCode *blockW = (SizeClustBlockCode*) World->getBlockById(w)->blockCode;
            int delta = (blockV->C == blockW->C) ? 1: -1;
            Q += ( A[v][w] - (kv*kw)/(double)(2*m) ) * (double)delta/2;
           
        }
    } 
    Q = ((double)1/(2*m))*Q;
    modularity = Q;
    cout << "Eval Modularity: " << setprecision(12) << Q << endl;
    return modularity;
}

float Eval::calculateDBindexCG1(int ci, int cj) {
    // float num = 0;
    // for(auto ci: clusters) {
    //     for(auto cj: cluster_neighbors[ci.first]) {
    //         float dij = sqrt( 
    //             pow(cluster_cg[ci.first].pt[0] - cluster_cg[cj].pt[0], 2) +
    //             pow(cluster_cg[ci.first].pt[1] - cluster_cg[cj].pt[1], 2) +
    //             pow(cluster_cg[ci.first].pt[2] - cluster_cg[cj].pt[2], 2));
    //         num += (clusters_sigmaCG[ci.first] + clusters_sigmaCG[cj]) / dij;
    //     }
    // }
    // DBindex1 = num / clusters.size();
    // cout << "DBindex1: " << setprecision(5) << DBindex1 << endl;
    float num = 0;
    num = clusters_sigmaCG[ci] + clusters_sigmaCG[cj];
    cout << ci << " " << cj << endl;
    float dij = sqrt( 
                pow(cluster_cg[ci].pt[0] - cluster_cg[cj].pt[0], 2) +
                pow(cluster_cg[ci].pt[1] - cluster_cg[cj].pt[1], 2) +
                pow(cluster_cg[ci].pt[2] - cluster_cg[cj].pt[2], 2));
    //DBindex1 = num / dij;
    DBindex1 = clusters_sigmaCG[ci];
    return DBindex1;
}

float Eval::distanceToCG(int C, Cell3DPosition pos) {
    return sqrt(
        pow(cluster_cg[C].pt[0] - (float) pos.pt[0], 2) +
        pow(cluster_cg[C].pt[1] - (float) pos.pt[1], 2) +
        pow(cluster_cg[C].pt[2] - (float) pos.pt[2], 2)
    );
}

float Eval::distanceToCG(Vector3D cent, Cell3DPosition pos) {
    return sqrt(
        pow(cent.pt[0] - (float) pos.pt[0], 2) +
        pow(cent.pt[1] - (float) pos.pt[1], 2) +
        pow(cent.pt[2] - (float) pos.pt[2], 2)
    );
}

Vector3D Eval::getCG(BaseSimulator::World *world) {
    int nbBlocks = world->getNbBlocks();
    Vector3D CG;
    for(int i=1; i<=nbBlocks; i++) {
        SizeClustBlockCode* N = (SizeClustBlockCode*) world->getBlockById(i)->blockCode;
        CG.pt[0] += N->getModule()->position.pt[0];
        CG.pt[1] += N->getModule()->position.pt[1];
        CG.pt[2] += N->getModule()->position.pt[2];
    }
    CG.pt[0] /= nbBlocks;
    CG.pt[1] /= nbBlocks;
    CG.pt[2] /= nbBlocks;
    return CG; 
}

SizeClustBlockCode* Eval::calculateCentroid(int C){
    if(Eval::distanceMatrix.empty()){
        buildDistanceMatrix();
    }
    // if(Eval::cluster_centroid.find(C) != Eval::cluster_centroid.end()){
    //     return Eval::cluster_centroid[C];
    // }
    vector<SizeClustBlockCode*> candidates = clusters[C];
    vector<int> blocks;
    int l = 0;
    int Bindex = -1, Cindex = -1, Aindex = -1;
    for(auto c: candidates) {
        blocks.push_back(c->getModule()->blockId);
    }
    // ABC sequential
    while(blocks.size() > 2) {
        Aindex = Bindex = Cindex = -1;
        // get A
        int minId = World->getNbBlocks();
        for(int i=0;i<blocks.size();i++) {
            if(blocks[i]< minId) {
                minId = blocks[i];
                Aindex = ((SizeClustBlockCode *) World->getBlockById(minId)->blockCode)->module->blockId;
            }
        }
        cout << "A: " << Aindex<< "\n";
        // get B
       
        int max = -1;
        for(int j=0;j<blocks.size();j++){
            if(Bindex == -1 || Eval::distanceMatrix[Aindex][blocks[j]] > max || ( Eval::distanceMatrix[Aindex][blocks[j]] == max && blocks[Bindex] > blocks[blocks[j]])) {
                max = Eval::distanceMatrix[Aindex][blocks[j]];
                Bindex = ((SizeClustBlockCode *) World->getBlockById(blocks[j])->blockCode)->module->blockId;;
            }
        }
        cout << "B: " << Bindex << "\n";
        // get C
        
        max = -1;
        for(int j=0;j<blocks.size();j++){
            if(Cindex == -1 || Eval::distanceMatrix[Bindex][blocks[j]] > max || (Eval::distanceMatrix[Bindex][blocks[j]] == max && blocks[Cindex] > blocks[blocks[j]])) {
                max = Eval::distanceMatrix[Bindex][blocks[j]];
                Cindex = ((SizeClustBlockCode *) World->getBlockById(blocks[j])->blockCode)->module->blockId;;;
            }
        }
        cout << "c: " << Cindex << "\n";

        int min = 999;
        for(int l=0; l<blocks.size(); l++) {
            if(abs(Eval::distanceMatrix[blocks[l]][Bindex]-Eval::distanceMatrix[blocks[l]][Cindex]) < min){
                min = abs(Eval::distanceMatrix[blocks[l]][Bindex]-Eval::distanceMatrix[blocks[l]][Cindex]);
            }
        }
        vector<int> blockstmp;
        for(int i = 0; i < blocks.size(); i++){
            if(abs(Eval::distanceMatrix[blocks[i]][Bindex] - Eval::distanceMatrix[blocks[i]][Cindex]) == min){
                blockstmp.push_back(blocks[i]);
            }
        }
        blocks.clear();
        blocks = blockstmp;
    }
    int center = -1;
    int min = 999;
    for(int l = 0; l < blocks.size(); l++) {
        if(max(Eval::distanceMatrix[blocks[l]][Bindex], Eval::distanceMatrix[blocks[l]][Cindex]) < min)
            min = max(Eval::distanceMatrix[blocks[l]][Bindex], Eval::distanceMatrix[blocks[l]][Cindex]);
    }
    for(int k = 0; k < blocks.size(); k++) {
        if(max(Eval::distanceMatrix[blocks[k]][Bindex], Eval::distanceMatrix[blocks[k]][Cindex]) == min || (center != -1 && k < center))
            center = k;
    }
    cout << "center: " << blocks[center] << endl;
    World->getBlockById(blocks[center])->setColor(BLUE);
    Eval::cluster_centroid[C] = (SizeClustBlockCode*) World->getBlockById(blocks[center])->blockCode;
    return Eval::cluster_centroid[C];
}

bool Eval::checkConnected(int C) {
    map<SizeClustBlockCode*, bool> M; //<block, visted(1|0)>
    queue<SizeClustBlockCode*> Q;
    bool isConnected = true;
    for(auto c: clusters[C]){
        if(c->cutOn){
            M[c] = true;
            Q.push(c);
        }else{
            M[c] = false;
        }
    }
    // BFS starting from C
    while(!Q.empty()) {
        SizeClustBlockCode *B = Q.front();
        Q.pop();
        for(auto e: B->edges){
            //if(e.second.C == C) {
                SizeClustBlockCode* adj = (SizeClustBlockCode*) World->getBlockById(e.first->getConnectedBlockId())->blockCode;
                if(adj->C != C){
                    continue;
                }
                if(M[adj] == false){
                    M[adj] = true; // set as visited
                    Q.push(adj);
                }
            //}
        }
    }
    for(auto m: M){
        if(m.second == false){
            isConnected = false;
            break;
        }
    }
    return isConnected;
}

void Eval::printConnected() {
    cout << "Connectivity check: \n";
    for(auto c: clusters) {
        (checkConnected(c.first)) ? cout << c.first << ": is Connected\n": cout << c.first << ": is Disconnected\n";
    }
}

void Eval::writeToFile() {
    ofstream fout;
    fout.open("./eval/Graph.py", ios::out);
    fout << "edges = [";
    for(auto c: clusters){
        for(auto block: c.second){
            fout << "\n\t[";
            for(auto e: block->edges) {
                fout << "(" << block->module->blockId << "," << e.first->getConnectedBlockId() << "), ";
            }
            fout << "],";
        }
    }
    fout << "\n]\n\n";
    fout << "clusters = [";
    for(auto c: clusters){
        fout << "\n\t[";
        for(auto block: c.second){
            fout << block->module->blockId << ", ";
        }
        fout<<"],";
    }
    fout << "\n]\n";
    fout.close(); 
    // FILE *fp;
    // Py_Initialize();
    // fp = _Py_fopen("./eval/evalGraph.py", "r");
    // PyRun_SimpleFile(fp, "./eval/evalGraph.py");
    // Py_Finalize();
}

void Eval::writeMstToFile() {
    write = true;
    ofstream fout;
    fout.open("./eval/MST.py", ios::out);
    fout << "edges = [";
    for(auto c: clusters){
        fout << "\n\t[";
        for(auto block: c.second){
                fout << "(" << block->module->blockId << "," << block->in_branch->getConnectedBlockId() << "), ";
        }
        fout << "],";
    }
}