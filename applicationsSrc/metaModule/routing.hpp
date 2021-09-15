
#include "metaModuleBlockCode.hpp"

/* -------------------------------- BACKFRONT ------------------------------- */
vector<Cell3DPosition> BF_Route_To_Up_Zeven = {
    Cell3DPosition(0, 0, 0),  Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2),
    Cell3DPosition(-1, 0, 3), Cell3DPosition(0, 0, 4),  Cell3DPosition(0, -1, 4),
};

vector<Cell3DPosition> BF_Route_To_Up_Zodd = {
    Cell3DPosition(0, 0, 0),  Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2),
    Cell3DPosition(-1, 0, 3), Cell3DPosition(0, 0, 4),  Cell3DPosition(0, 1, 4),
};

vector<Cell3DPosition> BF_Route_To_Right = {Cell3DPosition(0, 0, 0),  Cell3DPosition(1, 0, 0),
                                            Cell3DPosition(1, -1, 1), Cell3DPosition(2, -1, 2),
                                            Cell3DPosition(3, -1, 2), Cell3DPosition(3, -1, 1),
                                            Cell3DPosition(4, 0, 0)};

vector<Cell3DPosition> BF_Route_To_Left = {
    Cell3DPosition(0, 0, 0),  Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2),
    Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 1), Cell3DPosition(-3, 0, 0),
    Cell3DPosition(-4, 0, 0),
};

vector<Cell3DPosition> BF_Route_To_Down_Zeven = {
    Cell3DPosition(0, 0, 0),   Cell3DPosition(0, -1, 0),   Cell3DPosition(-1, -1, -1),
    Cell3DPosition(-1, 0, -2), Cell3DPosition(-1, -1, -3), Cell3DPosition(0, -1, -4)};

vector<Cell3DPosition> BF_Route_To_Down_Zodd = {
    Cell3DPosition(0, 0, 0),   Cell3DPosition(0, 1, 0),   Cell3DPosition(-1, 1, -1),
    Cell3DPosition(-1, 2, -2), Cell3DPosition(-1, 1, -3), Cell3DPosition(0, 1, -4),
};

vector<Cell3DPosition> BF_Route_To_Back = {
    Cell3DPosition(0, 0, 0),  Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2),
    Cell3DPosition(-1, 2, 2), Cell3DPosition(-1, 2, 1), Cell3DPosition(0, 3, 0),
};

vector<Cell3DPosition> BF_Route_To_Front = {Cell3DPosition(0, 0, 0),  Cell3DPosition(1, 0, 0),
                                            Cell3DPosition(1, -1, 1), Cell3DPosition(2, -1, 2),
                                            Cell3DPosition(2, -2, 2), Cell3DPosition(1, -3, 1),
                                            Cell3DPosition(1, -3, 0), Cell3DPosition(0, -3, 0)};

/* -------------------------------- FRONTBACK ------------------------------- */
vector<Cell3DPosition> FB_Route_To_Up_Zeven = {Cell3DPosition(0, 0, 0),   Cell3DPosition(-1, -1, 1),
                                               Cell3DPosition(-1, -1, 2), Cell3DPosition(-1, -1, 3),
                                               Cell3DPosition(0, 0, 4),   Cell3DPosition(0, -1, 4)};

vector<Cell3DPosition> FB_Route_To_Up_Zodd = {Cell3DPosition(0, 0, 0),   Cell3DPosition(-1, -1, 1),
                                              Cell3DPosition(-1, -1, 2), Cell3DPosition(-1, -1, 3),
                                              Cell3DPosition(0, 0, 4),   Cell3DPosition(0, 1, 4)};

vector<Cell3DPosition> FB_Route_To_Right = {
    Cell3DPosition(0, 0, 0), Cell3DPosition(1, 0, 0), Cell3DPosition(1, 0, 1),
    Cell3DPosition(2, 1, 2), Cell3DPosition(3, 1, 2), Cell3DPosition(3, 0, 1),
    Cell3DPosition(4, 0, 0),
};

vector<Cell3DPosition> FB_Route_To_Left = {
    Cell3DPosition(0, 0, 0),   Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2),
    Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), Cell3DPosition(-3, 0, 0),
    Cell3DPosition(-4, 0, 0),
};

vector<Cell3DPosition> FB_Route_To_Down_Zeven = {
    Cell3DPosition(0, 0, 0),    Cell3DPosition(0, -1, 0),   Cell3DPosition(-1, -2, -1),
    Cell3DPosition(-1, -2, -2), Cell3DPosition(-1, -2, -3), Cell3DPosition(0, -1, -4)};

vector<Cell3DPosition> FB_Route_To_Down_Zodd = {
    Cell3DPosition(0, 0, 0),   Cell3DPosition(-1, 0, -1), Cell3DPosition(-1, 0, -2),
    Cell3DPosition(-1, 0, -3), Cell3DPosition(0, 1, -4),
};

vector<Cell3DPosition> FB_Route_To_Back = {
    Cell3DPosition(0, 0, 0), Cell3DPosition(1, 0, 0), Cell3DPosition(1, 0, 1),
    Cell3DPosition(2, 1, 2), Cell3DPosition(2, 2, 2), Cell3DPosition(1, 2, 1),
    Cell3DPosition(1, 3, 0), Cell3DPosition(0, 3, 0),
};

vector<Cell3DPosition> FB_Route_To_Front = {
    Cell3DPosition(0, 0, 0),   Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2),
    Cell3DPosition(-1, -2, 2), Cell3DPosition(-1, -3, 1), Cell3DPosition(0, -3, 0),
};

vector<Cell3DPosition> getRoutingVector(Direction direction, MMShape shapeState, int Z) {
    bool Zeven = Z % 2 == 0;
    if (shapeState == FRONTBACK) {
        switch (direction) {
            case Direction::UP: {
                if(Zeven)  return FB_Route_To_Up_Zeven;
                else return FB_Route_To_Up_Zodd;
            } break;
            case Direction::DOWN: {
                if(Zeven)  return FB_Route_To_Down_Zeven;
                else return FB_Route_To_Down_Zodd;
            } break;
            case Direction::LEFT: {
                return FB_Route_To_Left;
            } break;
            case Direction::RIGHT: {
                return FB_Route_To_Right;
            } break;
            case Direction::BACK: {
                return FB_Route_To_Back;
            } break;
            case Direction::FRONT: {
                return FB_Route_To_Front;
            } break;
            default:
                VS_ASSERT(false);
        }
    } else if (shapeState == BACKFRONT) {
        switch (direction) {
            case Direction::UP: {
                if(Zeven)  return BF_Route_To_Up_Zeven;
                else return BF_Route_To_Up_Zodd;
            } break;
            case Direction::DOWN: {
                if(Zeven)  return BF_Route_To_Down_Zeven;
                else return BF_Route_To_Down_Zodd;
            } break;
            case Direction::LEFT: {
                return BF_Route_To_Left;
            } break;
            case Direction::RIGHT: {
                return BF_Route_To_Right;
            } break;
            case Direction::BACK: {
                return BF_Route_To_Back;
            } break;
            case Direction::FRONT: {
                return BF_Route_To_Front;
            } break;
            default:
                VS_ASSERT(false);
        }
    }
}