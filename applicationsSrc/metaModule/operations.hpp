#include "localRules.hpp"

/**
 * base operation class
*/

#ifndef OPERATIONS_HPP_
#define OPERATIONS_HPP_

#include <memory>
#include "robots/catoms3D/catoms3DBlockCode.h"

class Operation {
protected:
   
    //vector<LocalMovement> *localRules;
    Direction direction;
    MMShape mmShape;
   
public:
    Operation(/* args */);
    Operation(Direction _direction, MMShape _mmshape);
    virtual ~Operation();

    std::shared_ptr<vector<LocalMovement>> localRules;
    MMOperation op;

    virtual void handleAddNeighborEvent(BaseSimulator::BlockCode*,const Cell3DPosition&) {};
    virtual void updateState(BaseSimulator::BlockCode*) {};
    virtual bool isDismantle() const {return false;};
    virtual bool isFill() const {return false;};
    virtual bool isTransfer() const  {return false;};
    virtual bool isConstruct() const {return false;};
    Direction getDirection() const {return direction;};
};

class Dismantle_Operation: public virtual Operation {
private:
    /* data */
public:
    Dismantle_Operation(Direction _direction, MMShape _mmShape);
    ~Dismantle_Operation ();

    void handleAddNeighborEvent(BaseSimulator::BlockCode*, const Cell3DPosition&) override;
    void updateState(BaseSimulator::BlockCode*) override;
    bool isDismantle() const override {return true;};
};

class Fill_Operation: public Operation {
private:
    
public:
    Fill_Operation(Direction _direction, MMShape _mmShape);
    ~Fill_Operation ();

    void handleAddNeighborEvent(BaseSimulator::BlockCode*, const Cell3DPosition&) override;
    void updateState(BaseSimulator::BlockCode*) override;
    bool isFill() const override {return true;};
};

class Transfer_Operation: public Operation {
private:
    bool isLastTransferOperation;
public:
    Transfer_Operation(Direction _direction, MMShape _mmShape, bool _isLastTransferOperation = false);
    ~Transfer_Operation();

    void handleAddNeighborEvent(BaseSimulator::BlockCode*, const Cell3DPosition&) override;
    void updateState(BaseSimulator::BlockCode*) override;
    bool isTransfer() const override {return true;};
    /**
     * @brief Special operation for bride on Coordinate message reception
    **/
    bool handleBridgeMovements(BaseSimulator::BlockCode*);
};

#endif


