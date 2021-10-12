#ifndef INITMESSAGES_HPP__
#define INITMESSAGES_HPP__

#include "comm/network.h"
#include "grid/cell3DPosition.h"

class GoMessage: public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    short distance;

public:
    GoMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition, short _distance)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition),
          distance(_distance){};

    virtual ~GoMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        return new GoMessage(*this);
    }
    virtual string getName() const override { return "GoMessage{" + fromMMPosition.to_string()
            + ", " + toMMPosition.to_string() + ", " + to_string(distance) + "}"; }
};

class BackMessage: public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    bool isChild;

public:
    BackMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition, bool _isChild)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition),
          isChild(_isChild){};
          
    ~BackMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        return new BackMessage(*this);
    }
    virtual string getName() const override { return "BackMessage{" + fromMMPosition.to_string()
            + ", " + toMMPosition.to_string() + ", " + to_string(isChild) + "}"; }
};

class GoTermMessage: public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

public:
    GoTermMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition){};
          
    ~GoTermMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        return new GoTermMessage(*this);
    }
    virtual string getName() const override { return "GoTermMessage{" + fromMMPosition.to_string()
            + ", " + toMMPosition.to_string() + "}"; }
};

#endif