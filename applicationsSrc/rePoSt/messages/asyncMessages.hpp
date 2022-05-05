//
// Created by jbassil on 29/03/2022.
//

#ifndef VISIBLESIM_ASYNCMESSAGES_HPP
#define VISIBLESIM_ASYNCMESSAGES_HPP

#include "grid/cell3DPosition.h"
#include "comm/network.h"
#include "rePoStBlockCode.hpp"

class CrossedSrcMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    int nbCrossedSrc;

public:

    CrossedSrcMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition, int _nbCrossedSrc)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), nbCrossedSrc(_nbCrossedSrc) {}

    ~CrossedSrcMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new CrossedSrcMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "CrossedSrcMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               to_string(nbCrossedSrc) + "}";
    }
};

class CrossedSrcAckMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    bool isChild;

public:

    CrossedSrcAckMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition, bool _isChild)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), isChild(_isChild) {}

    ~CrossedSrcAckMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new CrossedSrcAckMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "CrossedSrcAckMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               to_string(isChild) + "}";
    }
};

class FindPathMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition initiator;
    PathDirection pathDirection;
    int nbCrossed;

public:
    FindPathMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                    const Cell3DPosition &_initiator, PathDirection _direction)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_initiator), pathDirection(_direction) {
        nbCrossed = -1;
        VS_ASSERT_MSG(pathDirection == DST_SRC, "nbCrossed not provided");
    };

    FindPathMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                    const Cell3DPosition &_initiator, PathDirection _direction, int _nbCrossed)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_initiator),
              pathDirection(_direction), nbCrossed(_nbCrossed) {

    };

    ~FindPathMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new FindPathMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "FindPathMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               initiator.to_string() + ", " + to_string(pathDirection) + ", " + to_string(nbCrossed) + "}";
    }
};

class FoundPathMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition initiator;
    bool found;
    bool isChild;

public:
    FoundPathMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                     const Cell3DPosition &_initiator, bool _found, bool _isChild)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_initiator),
              found(_found), isChild(_isChild) {};

    ~FoundPathMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new FoundPathMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "FoundPathMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               initiator.to_string() + ", " + to_string(found) + ", " + to_string(isChild) + "}";
    }
};

class ConfirmPathAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition initiator;


public:
    ConfirmPathAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                            const Cell3DPosition &_initiator)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_initiator) {};

    ~ConfirmPathAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new ConfirmPathAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "ConfirmPathAsyncMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               initiator.to_string() + "}";
    }
};

class CutMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition initiator;


public:
    CutMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                      const Cell3DPosition &_initiator)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_initiator) {};

    ~CutMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new CutMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "CutMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               initiator.to_string() + "}";
    }
};

class ConfirmEdgeAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition initiator;


public:
    ConfirmEdgeAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                          const Cell3DPosition &_initiator)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_initiator) {};

    ~ConfirmEdgeAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new ConfirmEdgeAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "ConfirmEdge{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               initiator.to_string() + "}";
    }
};

class AvailableAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition initiator;


public:
    AvailableAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                   const Cell3DPosition &_initiator)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_initiator) {};

    ~AvailableAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new AvailableAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "AvailableMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               initiator.to_string() + "}";
    }
};

class GoTermAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition initiator;

public:
    GoTermAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                       const Cell3DPosition &_source)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), initiator(_source) {};

    ~GoTermAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new GoTermAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "GoTermAsyncMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               initiator.to_string() + "}";
    }
};

class BackTermAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;
    bool idle;

public:
    BackTermAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                       const Cell3DPosition &_source, bool _idle)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_source), idle(_idle) {};

    ~BackTermAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new BackTermAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "BackTermAsyncMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + ", " + to_string(idle) + "}";
    }
};

class NotifyTermMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

public:
    NotifyTermMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition) {};

    ~NotifyTermMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new NotifyTermMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "NotifyTermMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + "}";
    }
};


#endif //VISIBLESIM_ASYNCMESSAGES_HPP
