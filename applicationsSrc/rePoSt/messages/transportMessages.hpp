#ifndef TRANSPORTMESSAGES_HPP__
#define TRANSPORTMESSAGES_HPP__

#include "comm/network.h"
#include "grid/cell3DPosition.h"
#include "operations.hpp"

class CoordinateMessage : public HandleableMessage {
   private:
    Operation *operation;
    Cell3DPosition position;
    Cell3DPosition coordinatorPosition;
    int it;

   public:
    CoordinateMessage(Operation *_operation, Cell3DPosition _position,
                      Cell3DPosition _coordinatorPosition, int _it)
        : HandleableMessage(),
          operation(_operation),
          position(_position),
          coordinatorPosition(_coordinatorPosition),
          it(_it){
              // type = 1002;
          };

    ~CoordinateMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;
    Message *clone() const override { return new CoordinateMessage(*this); }
    string getName() const override {
        return "CoordinateMessage{" + position.to_string() + ", " +
               coordinatorPosition.to_string() + "}";
    }
};

class CoordinateBackMessage : public HandleableMessage {
   private:
    int steps;
    Cell3DPosition coordinatorPosition;

   public:
    CoordinateBackMessage(int _steps, Cell3DPosition _coordinatorPosition)
        : HandleableMessage(), steps(_steps), coordinatorPosition(_coordinatorPosition){};

    ~CoordinateBackMessage() override = default;;

    void handle(BaseSimulator::BlockCode *) override;
    Message *clone() const override { return new CoordinateBackMessage(*this); }
    string getName() const override {
        return "CoordinateBackMessage{" + to_string(steps) + ", " +
               coordinatorPosition.to_string() + "}";
    }
};

class PLSMessage : public HandleableMessage {
   private:
    Cell3DPosition srcPos;
    Cell3DPosition destPos;

   public:
    PLSMessage(Cell3DPosition _srcPos, Cell3DPosition _destPos)
        : HandleableMessage(), srcPos(_srcPos), destPos(_destPos){};

    ~PLSMessage() override= default;

    void handle(BaseSimulator::BlockCode *) override;
    Message *clone() const override { return new PLSMessage(*this); }
    string getName() const override {
        return "PLSMessage{" + srcPos.to_string() + ", " + destPos.to_string() + "}";
    }
};

class GLOMessage : public HandleableMessage {
   private:
    Cell3DPosition srcPos;

   public:
    GLOMessage(Cell3DPosition _srcPos)
        : HandleableMessage(), srcPos(_srcPos){};

    ~GLOMessage() override = default;;

    void handle(BaseSimulator::BlockCode *) override;
    Message *clone() const override { return new GLOMessage(*this); }
    string getName() const override {
        return "GLOMessage{" + srcPos.to_string() + "}";
    }
};

class FTRMessage : public HandleableMessage {
   private:
   public:
    FTRMessage() : HandleableMessage(){};

    ~FTRMessage(){};

    void handle(BaseSimulator::BlockCode *) override;
    Message *clone() const override { return new FTRMessage(*this); }
    string getName() const override { return "FTRMessage{}"; }
};
#endif