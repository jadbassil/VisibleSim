//
// Created by jbassil on 29/03/2022.
//

#ifndef VISIBLESIM_ASYNCMESSAGES_HPP
#define VISIBLESIM_ASYNCMESSAGES_HPP

#include "grid/cell3DPosition.h"
#include "comm/network.h"

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

class FindSrcMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;

public:
    FindSrcMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                   const Cell3DPosition &_destination)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_destination) {};

    ~FindSrcMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new FindSrcMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "FindSrcMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + "}";
    }
};

class FoundSrcMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;
    bool found;
    bool isChild;

public:
    FoundSrcMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                    const Cell3DPosition &_destination, bool _found, bool _isChild)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_destination),
              found(_found), isChild(_isChild) {};

    ~FoundSrcMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new FoundSrcMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "FoundSrcMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + ", " + to_string(found) + ", " + to_string(isChild) + "}";
    }
};

class ConfirmSrcMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;


public:
    ConfirmSrcMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                      const Cell3DPosition &_destination)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_destination) {};

    ~ConfirmSrcMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new ConfirmSrcMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "ConfirmSrcMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + "}";
    }
};

class CutMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;


public:
    CutMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                      const Cell3DPosition &_destination)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_destination) {};

    ~CutMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new CutMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "CutMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + "}";
    }
};

class ConfirmEdgeAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;


public:
    ConfirmEdgeAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                          const Cell3DPosition &_destination)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_destination) {};

    ~ConfirmEdgeAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new ConfirmEdgeAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "ConfirmEdge{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + "}";
    }
};

class AvailableAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;


public:
    AvailableAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                   const Cell3DPosition &_destination)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_destination) {};

    ~AvailableAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new AvailableAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "AvailableMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + "}";
    }
};

class GoTermAsyncMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition destination;

public:
    GoTermAsyncMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                       const Cell3DPosition &_source)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), destination(_source) {};

    ~GoTermAsyncMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new GoTermAsyncMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "GoTermAsyncMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               destination.to_string() + "}";
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

class FindDstMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    Cell3DPosition source;
    int nbDstToCross;

public:
    FindDstMessage(const Cell3DPosition &_fromMMPosition, const Cell3DPosition &_toMMPosition,
                   const Cell3DPosition &_source, int _nbDstToCross)
            : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), source(_source), nbDstToCross(_nbDstToCross) {};

    ~FindDstMessage() override = default;

    void handle(BaseSimulator::BlockCode *) override;

    [[nodiscard]] Message *clone() const override { return new FindDstMessage(*this); }

    [[nodiscard]] string getName() const override {
        return "FindDstMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + ", " +
               source.to_string() + ", " + to_string(nbDstToCross) + "}";
    }
};


#endif //VISIBLESIM_ASYNCMESSAGES_HPP
