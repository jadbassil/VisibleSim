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
    Direction prevOpDirection;
    MMShape mmShape;
    Cell3DPosition nextSeed;
    bool Zeven;
    
   
public:
    Operation(/* args */);
    Operation(Direction _direction, MMShape _mmshape, Direction _prevOpDirection, int Z = 0);
    virtual ~Operation();
     void setMvtItToNextModule(BaseSimulator::BlockCode*);
    std::shared_ptr<vector<LocalMovement>> localRules;
    Cell3DPosition getNextSeed(BaseSimulator::BlockCode*);
    Direction getDirection() const {return direction;};
    Direction getPrevOpDirection() const {return prevOpDirection;};
    MMShape getMMShape() const {return mmShape;};
    bool mustHandleBridgeOnAdd(const Cell3DPosition&);
    /**
     * @brief Check onMotionEnd if a module must send coordinateBack msg 
     * to inform the coordinator to pass to the next waiting module
     * 
     * @return bool
     */
    virtual bool mustSendCoordinateBack(BaseSimulator::BlockCode*) {return false;};

    /**
     * @brief Operation dependent logic to be executed when a module is attached on a coordinator
     *  
     * @param pos the position of the attached module
     */
    virtual void handleAddNeighborEvent(BaseSimulator::BlockCode*,const Cell3DPosition& pos) {};
    virtual void updateState(BaseSimulator::BlockCode*) {} ;
    virtual bool isDismantle() const {return false;};
    virtual bool isFill() const {return false;};
    virtual bool isTransfer() const  {return false;};
    virtual bool isConstruct() const {return false;};
    virtual bool isBuild() const {return false;};
    bool isZeven() const {return Zeven;};
};

class Dismantle_Operation : public Operation {
   private:
    /* data */
   public:
    bool filled;

    Dismantle_Operation(Direction _direction, MMShape _mmShape, Direction _prevOpDirection,
                        int Z = 0, bool _filled = false);
    ~Dismantle_Operation();

    void handleAddNeighborEvent(BaseSimulator::BlockCode*, const Cell3DPosition&) override;
    void updateState(BaseSimulator::BlockCode*) override;
    bool mustSendCoordinateBack(BaseSimulator::BlockCode*) override;
    bool isDismantle() const override { return true; };
    bool isFill() const override { return filled ? true : false; };
};

class Fill_Operation: public Operation {
private:
    bool comingFromBack;
public:
    Fill_Operation(Direction _direction, MMShape _mmShape, Direction _prevOpDirection, bool _comingFromBack = false, int Z = 0);
    ~Fill_Operation ();

    void handleAddNeighborEvent(BaseSimulator::BlockCode*, const Cell3DPosition&) override;
    void updateState(BaseSimulator::BlockCode*) override;
    bool mustSendCoordinateBack(BaseSimulator::BlockCode*) override;
    bool isFill() const override {return true;};
};

class Transfer_Operation: public Operation {
private:
    bool comingFromBack;
    Direction nextOpDir;
public:
    Transfer_Operation(Direction _direction, MMShape _mmShape, Direction _prevOpDirection, Direction _nextOpDir, bool comingFromBack = false, int Z = 0);
    ~Transfer_Operation();

    void handleAddNeighborEvent(BaseSimulator::BlockCode*, const Cell3DPosition&) override;
    void updateState(BaseSimulator::BlockCode*) override;
    bool mustSendCoordinateBack(BaseSimulator::BlockCode*) override;
    bool isTransfer() const override {return true;};
    bool isComingFromBack() const {return comingFromBack;};
    Direction getNextOpDir() const {return nextOpDir;};
    /**
     * @brief Special operation for bride on Coordinate message reception
    **/
    bool handleBridgeMovements(BaseSimulator::BlockCode*);
};

class Build_Operation: public Operation {
private:
    bool comingFromBack;
public:
    Build_Operation(Direction _direction, MMShape _mmShape, Direction _prevOpDirection, bool comingFromBack = false, int Z = 0);
    ~Build_Operation ();

    void handleAddNeighborEvent(BaseSimulator::BlockCode*, const Cell3DPosition&) override;
    void updateState(BaseSimulator::BlockCode*) override;
    bool mustSendCoordinateBack(BaseSimulator::BlockCode*) override;
    bool isBuild() const override {return true;};
    bool isComingFromBack() const {return comingFromBack;};
};

#endif


