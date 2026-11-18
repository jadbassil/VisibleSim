#ifndef datomMotionsCode_H_
#define datomMotionsCode_H_

#include <string_view>
#include <vector>
#include "robots/datoms/datomsSimulator.h"
#include "robots/datoms/datomsBlockCode.h"

using namespace Datoms;

class DatomMotionsCode : public DatomsBlockCode {
private:
    DatomsBlock *module = nullptr;
    bool isLeader = false;
    std::vector<Cell3DPosition> path;  // cells to visit, from the "path" XML attribute
    size_t currentPathStep = 0;

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
