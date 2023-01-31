//
// Created by jbassil on 05/07/22.
//

#ifndef SHAPERECOGNITION3D_MESSAGES_HPP
#define SHAPERECOGNITION3D_MESSAGES_HPP

#include "comm/network.h"

class SetMessage : public HandleableMessage {
private:
    int value;
public:
    SetMessage(int _value): value(_value){};

    ~SetMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new SetMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "SetMessage{" + to_string(type) + ", " + to_string(value) + "}";
    }
};

class CheckDMessage : public HandleableMessage {
private:

public:
    CheckDMessage() = default;

    ~CheckDMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new CheckDMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "CheckDMessage{}";
    }
};

class NotifyDMessage : public HandleableMessage {
private:
    int value;
public:
    NotifyDMessage(int _value): value(_value){};

    ~NotifyDMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new NotifyDMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "NotifyDMessage{" + to_string(value) + "}";
    }
};

class FindWMessage : public HandleableMessage {
private:
    int direction;
    int value;
    int senderId;
public:
    FindWMessage(int _direction, int _value, int _senderId): direction(_direction), value(_value), senderId(_senderId){};

    ~FindWMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new FindWMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "FindWMessage{"+ to_string(direction) + + ", " + to_string(value) + + ", " + to_string(senderId) + " }";
    }
};

class CheckWMessage : public HandleableMessage {
private:

public:
    CheckWMessage() = default;

    ~CheckWMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new CheckWMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "CheckWMessage{}";
    }
};

class NotifyWMessage : public HandleableMessage {
private:

    int value;
public:
    NotifyWMessage(int _value): value(_value){};

    ~NotifyWMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new NotifyWMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "NotifyWMessage{" + to_string(type) + ", " + to_string(value) + "}";
    }
};

class FindHMessage : public HandleableMessage {
private:
    int value;
public:
    FindHMessage(int _value): value(_value){};

    ~FindHMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new FindHMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "FindHMessage{"+ to_string(value) + "}";
    }
};


#endif //SHAPERECOGNITION3D_MESSAGES_HPP
