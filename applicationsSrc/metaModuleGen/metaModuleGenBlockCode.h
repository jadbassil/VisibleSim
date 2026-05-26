#ifndef METAMODULE_GEN_BLOCKCODE_H_
#define METAMODULE_GEN_BLOCKCODE_H_

#include "robots/catoms3D/catoms3DBlockCode.h"

class MetaModuleGenBlockCode : public Catoms3D::Catoms3DBlockCode {
public:
    explicit MetaModuleGenBlockCode(Catoms3D::Catoms3DBlock* host)
        : Catoms3D::Catoms3DBlockCode(host) {}
    ~MetaModuleGenBlockCode() override = default;

    void startup() override;

    static BlockCode* buildNewBlockCode(BuildingBlock* host) {
        return new MetaModuleGenBlockCode(static_cast<Catoms3D::Catoms3DBlock*>(host));
    }
};

#endif // METAMODULE_GEN_BLOCKCODE_H_
