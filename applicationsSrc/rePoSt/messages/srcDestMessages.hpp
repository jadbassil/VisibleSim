#ifndef SRCDESTMESSAGES_HPP__
#define SRCDESTMESSAGES_HPP__

#include "comm/network.h"
#include "grid/cell3DPosition.h"

class RePoStBlockCode;

class GoMessage : public HandleableMessage {
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
    virtual Message* clone() const override { return new GoMessage(*this); }
    virtual string getName() const override {
        return "GoMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               to_string(distance) + "}";
    }
};

class BackMessage : public HandleableMessage {
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
    virtual Message* clone() const override { return new BackMessage(*this); }
    virtual string getName() const override {
        return "BackMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() +
               ", " + to_string(isChild) + "}";
    }
};

class GoDstMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    short distance;

   public:
    GoDstMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition, short _distance)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition),
          distance(_distance){};

    virtual ~GoDstMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new GoDstMessage(*this); }
    virtual string getName() const override {
        return "GoDstMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               to_string(distance) + "}";
    }
};

class BackDstMessage : public HandleableMessage {
   private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    bool isChild;

   public:
    BackDstMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition, bool _isChild)
        : HandleableMessage(),
          fromMMPosition(_fromMMPosition),
          toMMPosition(_toMMPosition),
          isChild(_isChild){};

    ~BackDstMessage(){};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override { return new BackDstMessage(*this); }
    virtual string getName() const override {
        return "BackDstMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() +
               ", " + to_string(isChild) + "}";
    }
};

#endif