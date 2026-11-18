#ifndef datomMotionsCode_H_
#define datomMotionsCode_H_

#include "robots/datoms/datomsSimulator.h"
#include "robots/datoms/datomsBlockCode.h"

using namespace Datoms;

class DatomMotionsCode : public DatomsBlockCode {
private:
    DatomsBlock *module = nullptr;
    bool isLeader = false;

public:
    DatomMotionsCode(DatomsBlock *host);
    ~DatomMotionsCode() {};

    void startup() override;
    void parseUserBlockElements(TiXmlElement *config) override;
    void onMotionEnd() override;
    void onUserKeyPressed(unsigned char c, int x, int y) override;

    /** needed to associate code to module **/
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new DatomMotionsCode((DatomsBlock *)host));
    };
};

#endif /* datomMotionsCode_H_ */
