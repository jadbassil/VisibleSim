#ifndef MAXFLOWMESSAGES_HPP__
#define MAXFLOWMESSAGES_HPP__

#include "comm/network.h"
#include "grid/cell3DPosition.h"

/* -------------------------------------------------------------------------- */
/*                       Termination Detection Messages                       */
/* -------------------------------------------------------------------------- */
class GoTermMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

   public:
    GoTermMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition){
              // type = 1002;
          };

    ~GoTermMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new GoTermMessage(*this); }
    virtual string getName() const override {
        return "GoTermMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() +
               "}";
    }
};

class BackTermMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    bool ans;

   public:
    BackTermMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition, bool _ans)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition),
          ans(_ans){
              // type = 1003;
          };

    ~BackTermMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new BackTermMessage(*this); }
    virtual string getName() const override {
        return "BackTermMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() +
               "}";
    }
};

class AckMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

   public:
    AckMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(), fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition){};

    ~AckMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new AckMessage(*this); }
    virtual string getName() const override {
        return "AckMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + "}";
    }
};

/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                              MaxFlow Messages                              */
/* -------------------------------------------------------------------------- */

class BFSMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition sourceMM;

   public:
    BFSMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition,
               Cell3DPosition _sourceMM)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition),
          sourceMM(_sourceMM) {
        type = 1008;
    };

    ~BFSMessage(){};

    Cell3DPosition getFromMMPosition() { return fromMMPosition; };

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual bool mustSendAck() const override { return true; };
    virtual Message* clone() const override { return new BFSMessage(*this); }
    virtual string getName() const override {
        return "BFSMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               sourceMM.to_string() + "}";
    }
};

class ConfirmEdgeMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

   public:
    ConfirmEdgeMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(), fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition) {
        type = 1009;
    };

    ~ConfirmEdgeMessage(){};

    Cell3DPosition getFromMMPosition() { return fromMMPosition; };

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new ConfirmEdgeMessage(*this); }
    virtual string getName() const override {
        return "ConfirmEdgeMessage{" + fromMMPosition.to_string() + ", " +
               toMMPosition.to_string() + "}";
    }
};

class ConfirmPathMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

   public:
    ConfirmPathMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(), fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition) {
        type = 1010;
    };

    ~ConfirmPathMessage(){};

    Cell3DPosition getFromMMPosition() { return fromMMPosition; };

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new ConfirmPathMessage(*this); }
    virtual string getName() const override {
        return "ConfirmPathMessage{" + fromMMPosition.to_string() + ", " +
               toMMPosition.to_string() + "}";
    }
};

class ConfirmStreamlineMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

   public:
    ConfirmStreamlineMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(), fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition) {
        type = 1011;
    };

    ~ConfirmStreamlineMessage(){};

    Cell3DPosition getFromMMPosition() { return fromMMPosition; };

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new ConfirmStreamlineMessage(*this); }
    virtual string getName() const override {
        return "ConfirmStreamlineMessage{" + fromMMPosition.to_string() + ", " +
               toMMPosition.to_string() + "}";
    }
};

class AvailableMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

   public:
    AvailableMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(), fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition) {
        type = 1012;
    };

    ~AvailableMessage(){};

    Cell3DPosition getFromMMPosition() { return fromMMPosition; };

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new AvailableMessage(*this); }
    virtual string getName() const override {
        return "AvailableMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() +
               "}";
    }
};

class CutOffMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

   public:
    CutOffMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
        : HandleableMessage(), fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition) {
        type = 1013;
    };

    ~CutOffMessage(){};

    Cell3DPosition getFromMMPosition() { return fromMMPosition; };

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new CutOffMessage(*this); }
    virtual string getName() const override {
        return "CutOffMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() +
               "}";
    }
};

/* -------------------------------------------------------------------------- */

#endif