#include "metaModuleBlockCode.hpp"

/* ---------------------------- COORDINATION TREE --------------------------- */
struct GOdata {
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    short distance;

    GOdata(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition, short _distance)
        : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), distance(_distance){};
};

struct Backdata {
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;
    bool isChild;

    Backdata(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition, bool _isChild)
        : fromMMPosition(_fromMMPosition), toMMPosition(_toMMPosition), isChild(_isChild){};
};
/* -------------------------------------------------------------------------- */

struct PLS {
    Cell3DPosition srcPos;
    Cell3DPosition targetPos;

    PLS() {};
    PLS(Cell3DPosition _srcPos, Cell3DPosition _targetPos):
        srcPos(_srcPos), targetPos(_targetPos) {};
};

struct SetCoordinator {
    Cell3DPosition coordinatorPosition;
    Operation operation;

    SetCoordinator() {};
    SetCoordinator(Cell3DPosition _coordinatorPosition, Operation _operation)
        : coordinatorPosition(_coordinatorPosition), operation(_operation) {};
};

struct StartMvt {
    Cell3DPosition toMMPosition;
    Cell3DPosition MMPosition;
    bool up{false};

};

struct Coordinate {
    Operation *operation;
    Cell3DPosition position;
    Cell3DPosition coordinatorPosition;
    int it;

    Coordinate() {};
    Coordinate(Operation *_operation, Cell3DPosition _position, Cell3DPosition _coordinatorPosition, int _it)
        : operation(_operation), position(_position), coordinatorPosition(_coordinatorPosition), it(_it) {};
};

struct CoordinateBack {
    int steps;
    Cell3DPosition coordinatorPosition;

    CoordinateBack() {};
    CoordinateBack(int _steps, Cell3DPosition _coordinatorPosition)
        : steps(_steps), coordinatorPosition(_coordinatorPosition){};

};

/* --------------------------------- MAXFLOW -------------------------------- */
struct BFSdata {
    Cell3DPosition MMPosition;
    Cell3DPosition fromMMPosition;
    Cell3DPosition toSeedPosition;

    BFSdata(Cell3DPosition _MMPosition, Cell3DPosition _fromMMPosition, Cell3DPosition _toSeedPosition):
        MMPosition(_MMPosition), fromMMPosition(_fromMMPosition), toSeedPosition(_toSeedPosition){};
};

struct MaxFlowMsgData {
    Cell3DPosition fromMMPosition;
    Cell3DPosition toMMPosition;

    MaxFlowMsgData(Cell3DPosition _fromMMPosition, Cell3DPosition _toMMPosition):
        fromMMPosition(_fromMMPosition),toMMPosition(_toMMPosition) {};
};

/* -------------------------------------------------------------------------- */