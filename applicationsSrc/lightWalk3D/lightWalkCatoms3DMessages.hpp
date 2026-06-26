#ifndef LIGHTWALKCATOMS3D_MESSAGES_H_
#define LIGHTWALKCATOMS3D_MESSAGES_H_

#include "comm/network.h"

static const uint MSG_DELAY_MC = 5000;

class ProbePivotLightStateMessage : public HandleableMessage {
    const Cell3DPosition srcPos;
    const Cell3DPosition targetPos;
    const int line;
    const int mobileId;
public:
    ProbePivotLightStateMessage(const Cell3DPosition& _srcPos,
                                const Cell3DPosition& _targetPos,
                                int _mobileId)
        : HandleableMessage(), srcPos(_srcPos), targetPos(_targetPos),
          line(_targetPos[1]), mobileId(_mobileId) {};
    virtual ~ProbePivotLightStateMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {

        adjustClonedMessageCount();
        adjustClonedMessageCount()
            ;
        return new ProbePivotLightStateMessage(*this);
    }
    virtual string getName() const override { return "ProbePivotLightState{" + srcPos.to_string()
            + ", " + targetPos.to_string() + ", line=" + std::to_string(line)
            + ", mobile=" + std::to_string(mobileId) + "}";
    }
};


class GreenLightIsOnMessage : public HandleableMessage {
    const Cell3DPosition srcPos;
    const Cell3DPosition dstPos;
    const int line;
public:
    GreenLightIsOnMessage(const Cell3DPosition& _srcPos,
                          const Cell3DPosition& _dstPos)
        : HandleableMessage(), srcPos(_srcPos), dstPos(_dstPos),
          line(_dstPos[1]) {};
    virtual ~GreenLightIsOnMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new GreenLightIsOnMessage(*this);
    }
    virtual string getName() const override { return "GreenLightIsOn{" + srcPos.to_string()
            + ", " + dstPos.to_string() + ", line=" + std::to_string(line) + "}";
    }
};

class ReservationRequestMessage : public HandleableMessage {
    const int mobileId;
    const int token;
    const int line;
    const Cell3DPosition mobilePos;
    const Cell3DPosition targetPos;
    const Cell3DPosition pivotPos;
public:
    ReservationRequestMessage(int _mobileId,
                              int _token,
                              int _line,
                              const Cell3DPosition& _mobilePos,
                              const Cell3DPosition& _targetPos,
                              const Cell3DPosition& _pivotPos)
        : HandleableMessage(), mobileId(_mobileId), token(_token), line(_line),
          mobilePos(_mobilePos), targetPos(_targetPos), pivotPos(_pivotPos) {};
    virtual ~ReservationRequestMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new ReservationRequestMessage(*this);
    }
    virtual string getName() const override {
        return "ReservationRequest{mobile=" + std::to_string(mobileId)
            + ", token=" + std::to_string(token)
            + ", pivot=" + pivotPos.to_string() + "}";
    }
};

class ReservationReplyMessage : public HandleableMessage {
    const int mobileId;
    const int token;
    const int line;
    const Cell3DPosition pivotPos;
    const Cell3DPosition targetPos;
    const bool granted;
    const bool spacingDenied;
public:
    ReservationReplyMessage(int _mobileId,
                            int _token,
                            int _line,
                            const Cell3DPosition& _pivotPos,
                            const Cell3DPosition& _targetPos,
                            bool _granted,
                            bool _spacingDenied)
        : HandleableMessage(), mobileId(_mobileId), token(_token), line(_line),
          pivotPos(_pivotPos), targetPos(_targetPos), granted(_granted),
          spacingDenied(_spacingDenied) {};
    virtual ~ReservationReplyMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new ReservationReplyMessage(*this);
    }
    virtual string getName() const override {
        return "ReservationReply{mobile=" + std::to_string(mobileId)
            + ", token=" + std::to_string(token)
            + ", granted=" + std::to_string(granted)
            + ", spacingDenied=" + std::to_string(spacingDenied) + "}";
    }
};

class LaunchLineMessage : public HandleableMessage {
    const int targetLine;
    const int launchId;
public:
    LaunchLineMessage(int _targetLine, int _launchId)
        : HandleableMessage(), targetLine(_targetLine), launchId(_launchId) {};
    virtual ~LaunchLineMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new LaunchLineMessage(*this);
    }
    virtual string getName() const override {
        return "LaunchLine{line=" + std::to_string(targetLine)
            + ", launch=" + std::to_string(launchId) + "}";
    }
};

class ReleasePivotLightMessage : public HandleableMessage {
    const Cell3DPosition mobilePos;
    const Cell3DPosition pivotPos;
    const int line;
    const int mobileId;
    const int token;
    const bool completedMotion;
public:
    ReleasePivotLightMessage(const Cell3DPosition& _mobilePos,
                             const Cell3DPosition& _pivotPos,
                             int _line,
                             int _mobileId,
                             int _token,
                             bool _completedMotion)
        : HandleableMessage(), mobilePos(_mobilePos), pivotPos(_pivotPos),
          line(_line), mobileId(_mobileId), token(_token),
          completedMotion(_completedMotion) {};
    virtual ~ReleasePivotLightMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new ReleasePivotLightMessage(*this);
    }
    virtual string getName() const override {
        return "ReleasePivotLight{" + mobilePos.to_string()
            + ", " + pivotPos.to_string()
            + ", line=" + std::to_string(line)
            + ", mobile=" + std::to_string(mobileId)
            + ", token=" + std::to_string(token) + "}";
    }
};

class ReleasePivotAckMessage : public HandleableMessage {
    const int mobileId;
    const int token;
    const int line;
    const bool completedMotion;
public:
    ReleasePivotAckMessage(int _mobileId,
                           int _token,
                           int _line,
                           bool _completedMotion)
        : HandleableMessage(), mobileId(_mobileId), token(_token), line(_line),
          completedMotion(_completedMotion) {};
    virtual ~ReleasePivotAckMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new ReleasePivotAckMessage(*this);
    }
    virtual string getName() const override {
        return "ReleasePivotAck{mobile=" + std::to_string(mobileId)
            + ", token=" + std::to_string(token) + "}";
    }
};

class DpValueMessage : public HandleableMessage {
    const int originId;
    const int sequence;
    const Cell3DPosition cell;
    const int dpValue;
public:
    DpValueMessage(int _originId,
                   int _sequence,
                   const Cell3DPosition& _cell,
                   int _dpValue)
        : HandleableMessage(), originId(_originId), sequence(_sequence),
          cell(_cell), dpValue(_dpValue) {};
    virtual ~DpValueMessage() {};

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new DpValueMessage(*this);
    }
    virtual string getName() const override {
        return "DpValue{origin=" + std::to_string(originId)
            + ", seq=" + std::to_string(sequence)
            + ", cell=" + cell.to_string()
            + ", Dp=" + std::to_string(dpValue) + "}";
    }
};

class FlattenLayerLaunchMessage : public HandleableMessage {
public:
    const int targetLayer;
    const int launchId;

    FlattenLayerLaunchMessage(int _targetLayer, int _launchId)
        : HandleableMessage(), targetLayer(_targetLayer), launchId(_launchId) {}
    virtual ~FlattenLayerLaunchMessage() {}

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new FlattenLayerLaunchMessage(*this);
    }
    virtual string getName() const override {
        return "FlattenLayerLaunch{layer=" + std::to_string(targetLayer)
            + ", launch=" + std::to_string(launchId) + "}";
    }
};

class FlattenLayerDoneMessage : public HandleableMessage {
public:
    const int completedLayer;
    const int launchId;

    FlattenLayerDoneMessage(int _completedLayer, int _launchId)
        : HandleableMessage(), completedLayer(_completedLayer), launchId(_launchId) {}
    virtual ~FlattenLayerDoneMessage() {}

    virtual void handle(BaseSimulator::BlockCode*) override;
    virtual Message* clone() const override {
        adjustClonedMessageCount();
        return new FlattenLayerDoneMessage(*this);
    }
    virtual string getName() const override {
        return "FlattenLayerDone{layer=" + std::to_string(completedLayer)
            + ", launch=" + std::to_string(launchId) + "}";
    }
};

#endif
