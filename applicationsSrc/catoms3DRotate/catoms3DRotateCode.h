#ifndef Catoms3DRotateCode_H_
#define Catoms3DRotateCode_H_
#include <queue>
#include "robots/catoms3D/catoms3DSimulator.h"
#include "robots/catoms3D/catoms3DBlockCode.h"
#include "robots/catoms3D/catoms3DMotionEngine.h"
#include "motion/teleportationEvents.h"

using namespace Catoms3D;

class Catoms3DRotateCode : public Catoms3DBlockCode {
private:
    Catoms3DBlock *module;
    FCCLattice *lattice;
    queue <Cell3DPosition> cellsList;
public :
    Catoms3DRotateCode(Catoms3DBlock *host):Catoms3DBlockCode(host) { module = host; };
    ~Catoms3DRotateCode() {};

    void startup();
    void initDistances();
    bool tryToMove();

    void onMotionEnd();
    /*****************************************************************************/
    /** needed to associate code to module                                      **/
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return(new Catoms3DRotateCode((Catoms3DBlock*)host));
    };
    /*****************************************************************************/
};

#endif /* Catoms3DRotateCode_H_ */
