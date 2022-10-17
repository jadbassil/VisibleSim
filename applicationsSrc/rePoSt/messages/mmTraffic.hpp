//
// Created by jbassil on 17/10/22.
//

#ifndef VISIBLESIM_MMTRAFFIC_HPP
#define VISIBLESIM_MMTRAFFIC_HPP

#include "comm/network.h"
#include "grid/cell3DPosition.h"
#include "operations.hpp"


class MMPLSMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

public:
    MMPLSMessage(Cell3DPosition _srcPos, Cell3DPosition _destPos)
            : HandleableMessage(), fromMMPosition(_srcPos), toMMPosition(_destPos){};

    ~MMPLSMessage() override= default;

    void handle(BaseSimulator::BlockCode *) override;
    [[nodiscard]] Message *clone() const override { return new MMPLSMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "MMPLSMessage{" + fromMMPosition.to_string() + ", " + toMMPosition.to_string() + "}";
    }
};

class MMGLOMessage : public HandleableMessage {
private:
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

public:
    MMGLOMessage(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition)
            : HandleableMessage(), fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition){};

    ~MMGLOMessage() override = default;;

    void handle(BaseSimulator::BlockCode *) override;
    [[nodiscard]] Message *clone() const override { return new MMGLOMessage(*this); }
    [[nodiscard]] string getName() const override {
        return "MMGLOMessage{" + fromMMPosition.to_string() + "}";
    }
};


#endif //VISIBLESIM_MMTRAFFIC_HPP
