//
// Created by jbassil on 05/07/22.
//

#ifndef SHAPERECOGNITION3D_MESSAGES_HPP
#define SHAPERECOGNITION3D_MESSAGES_HPP

#include "comm/network.h"

enum Set_Type {Width, Height, Depth};

class SetMessage : public HandleableMessage {
private:
    Set_Type type;
    int value;
public:
    SetMessage(Set_Type _type, int _value): type(_type), value(_value){};

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

class NotifyMessage : public HandleableMessage {
private:
    Set_Type type;
    int value;
public:
    NotifyMessage(Set_Type _type, int _value): type(_type), value(_value){};

    ~NotifyMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new NotifyMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "NotifyMessage{" + to_string(type) + ", " + to_string(value) + "}";
    }
};

class FindWMessage : public HandleableMessage {
private:
    int value;
public:
    FindWMessage(int _value): value(_value){};

    ~FindWMessage() override = default;

    void handle(BaseSimulator::BlockCode*) override;
    [[nodiscard]] Message* clone() const override { return new FindWMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "FindWMessage{"+ to_string(value) + "}";
    }
};


#endif //SHAPERECOGNITION3D_MESSAGES_HPP
