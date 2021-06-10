#ifndef LOCALRULES1_HPP__
#define LOCALRULES1_HPP__

#include "grid/cell3DPosition.h"
#include <bitset>
#include <vector>
#include <utility>

#include <iostream>
using namespace std;

enum MovingState {IN_POSITION, MOVING, WAITING};
enum Movement {
    NO_MOVEMENT,
    BFtoFB_RtoL_EtoE,
    FBtoBF_LtoR_FtoE,
    FBtoBF_UtoD_EtoF,
    FBtoFB_DtoU_FtoE
};

enum MMOperation {
    FB_Transfer_Left,
    BF_Transfer_Left,
    BF_Dismantle_Left,
    FB_Fill_Left,
    NO_OPERATION
};

enum MMShape {
    FRONTBACK,
    BACKFRONT
};
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    FRONT,
    BACK
};

static int countSteps = 0;

class LocalMovement {
private:
   
public:
    Cell3DPosition currentPosition;
    Cell3DPosition nextPosition;
    MovingState state;
    LocalMovement() {};
    LocalMovement(Cell3DPosition _currentPosition, Cell3DPosition _nextPosition, MovingState _state)
        : currentPosition(_currentPosition), nextPosition(_nextPosition), state(_state) {};
    ~LocalMovement() {};

    friend ostream& operator<<(ostream& f,const LocalMovement &lmvt) {
        f << lmvt.currentPosition << "->" << lmvt.nextPosition << "| " << lmvt.state;
        return f;
    };
};


static vector<LocalMovement> LocalRules_BF_Dismantle_Left = {
    // (-2,0,2) position of right neighbor coordinator
    {LocalMovement(Cell3DPosition(2,-1,2), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(-1,-1,3), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-1,0,2), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-2,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), IN_POSITION)},


    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_FB_Fill_Left = {
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,0), WAITING)},

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,4), WAITING)},

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},//12
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), WAITING)},

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,3), IN_POSITION)}, //17

    {LocalMovement(Cell3DPosition(-1,0,4), Cell3DPosition(-1,0,3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-1,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)}, //20
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(0,-1,3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(0,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(2,0,1), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), IN_POSITION)}, //25

    // {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_FB_Transfer_Left = {
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), WAITING)}, //3


    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), WAITING)}, //6

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3 ), Cell3DPosition(-2,0,2), IN_POSITION)}, //13


    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), WAITING)},

    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-3,0,3), WAITING)},

    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-2,0,2), IN_POSITION)}, 
    {LocalMovement(Cell3DPosition(-3,0,3), Cell3DPosition(-2,0,2), IN_POSITION)},

};
 
static vector<LocalMovement> LocalRules_BF_Transfer_Left = {
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), WAITING)}, //3


    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), WAITING)}, 


    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,1), MOVING)}, //7
    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), IN_POSITION)}, //13

    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), WAITING)},

    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-3,-1,3), WAITING)}, //18

    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-2,0,2), IN_POSITION)}, //19
    {LocalMovement(Cell3DPosition(-3,-1,3), Cell3DPosition(-2,-1,2), IN_POSITION)},


};

static vector<LocalMovement> LocalRules_BFtoFB_RtoL_EtoE = {
    {LocalMovement(Cell3DPosition(2,-1,2), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(-1,-1,3), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-1,0,2), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,2), Cell3DPosition(-4,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-4,0,1), Cell3DPosition(-5,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-5,0,1), Cell3DPosition(-5,0,0), WAITING)},

    {LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,2), Cell3DPosition(-4,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-4,0,3), Cell3DPosition(-5,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-5,0,3), Cell3DPosition(-5,0,4), WAITING)},

    {LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,2), Cell3DPosition(-4,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-4,0,1), Cell3DPosition(-5,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-5,0,1), Cell3DPosition(-5,0,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,2), Cell3DPosition(-4,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-4,0,3), Cell3DPosition(-5,0,3), WAITING)},

    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,2), Cell3DPosition(-4,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-5,0,3), Cell3DPosition(-4,0,3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-5,0,4), Cell3DPosition(-5,0,3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-5,0,0), Cell3DPosition(-5,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-3,-1,3), Cell3DPosition(-4,-1,3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-3,-1,1), Cell3DPosition(-4,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_FBtoFB_DtoU_FtoE = {
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(1,-1,4), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,4), Cell3DPosition(0,-1,4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(1,-1,4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,4), Cell3DPosition(-1,-1,5), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,5), Cell3DPosition(-1,-2,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,4), Cell3DPosition(-1,-1,5), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,5), Cell3DPosition(0,-1,5), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,5), Cell3DPosition(1,-1,6), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,6), Cell3DPosition(2,-1,6), MOVING)},
    {LocalMovement(Cell3DPosition(2,-1,6), Cell3DPosition(2,0,6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
    //{LocalMovement(Cell3DPosition(1,-1,2), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-2,5), MOVING)},
    {LocalMovement(Cell3DPosition(1,-2,5), Cell3DPosition(0,-2,5), MOVING)},
    {LocalMovement(Cell3DPosition(0,-2,5), Cell3DPosition(0,-2,6), MOVING)},
    {LocalMovement(Cell3DPosition(0,-2,6), Cell3DPosition(-1,-2,6), IN_POSITION)},
   

    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(0,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,2), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-2,5), MOVING)},
    {LocalMovement(Cell3DPosition(1,-2,5), Cell3DPosition(0,-2,5), MOVING)},
    {LocalMovement(Cell3DPosition(0,-2,5), Cell3DPosition(0,-2,6), MOVING)},

    {LocalMovement(Cell3DPosition(0,-2,6), Cell3DPosition(-1,-2,7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,4), Cell3DPosition(-1,-1,5), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,5), Cell3DPosition(0,-1,5), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,5), Cell3DPosition(1,0,6), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,6), Cell3DPosition(1,-1,7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,4), Cell3DPosition(-1,-1,5), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,5), Cell3DPosition(-1,-1,6), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,6), Cell3DPosition(-1,-1,7), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,7), Cell3DPosition(0,-1,8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-2,5), MOVING)},
    {LocalMovement(Cell3DPosition(1,-2,5), Cell3DPosition(1,-1,6), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,6), Cell3DPosition(1,-2,7), MOVING)},
    {LocalMovement(Cell3DPosition(1,-2,7), Cell3DPosition(1,-1,8), IN_POSITION)},
};

static vector<LocalMovement>* setLocalRules(MMOperation op) {
    vector<LocalMovement>* localRules;
    switch (op) {
    case BF_Dismantle_Left: {
        localRules =  &LocalRules_BF_Dismantle_Left;
        break;
    }
    case FB_Fill_Left: {
        localRules =  &LocalRules_FB_Fill_Left;
        break;
    }
    case FB_Transfer_Left: {
        localRules =  &LocalRules_FB_Transfer_Left;
        break;
    }
    case BF_Transfer_Left: {
        localRules =  &LocalRules_BF_Transfer_Left;
        break;
    }
    default:
        break;
    }
    return localRules;
}


static vector<LocalMovement>* setLocalRules(Movement movement) {
    vector<LocalMovement>* localRules;
     switch (movement) {
    case BFtoFB_RtoL_EtoE: {
        localRules =  &LocalRules_BFtoFB_RtoL_EtoE;
        break;
    }
    case FBtoBF_LtoR_FtoE: {

        localRules = new vector<LocalMovement>();
        MovingState s;
        for(int i=LocalRules_BFtoFB_RtoL_EtoE.size()-1; i>=0; i--) {
            LocalMovement lmvt = LocalRules_BFtoFB_RtoL_EtoE[i];
            Cell3DPosition tmp = lmvt.currentPosition.offsetX(4);
            lmvt.currentPosition = lmvt.nextPosition.offsetX(4);
            lmvt.nextPosition = tmp;
            if(lmvt.state != MOVING) {
                s = lmvt.state;
            }
            if(i == 0) {
                lmvt.state = IN_POSITION;
            } else if(LocalRules_BFtoFB_RtoL_EtoE[i-1].nextPosition == LocalRules_BFtoFB_RtoL_EtoE[i].currentPosition) {
                lmvt.state = MOVING;
            } else {
                bool found = false;
                for(int j=i-1;j > 0; j-- ) {
                    if(LocalRules_BFtoFB_RtoL_EtoE[j].nextPosition == LocalRules_BFtoFB_RtoL_EtoE[i].currentPosition) {
                        found = true;
                        lmvt.state = LocalRules_BFtoFB_RtoL_EtoE[j].state;
                        break;
                    }
                }
                if(!found) {
                    lmvt.state = IN_POSITION;
                }
            }
            (*localRules).push_back(lmvt);
        }
        break;
    }
    case FBtoFB_DtoU_FtoE: {
        localRules = &LocalRules_FBtoFB_DtoU_FtoE;
        break;
    }
    case FBtoBF_UtoD_EtoF: {
        localRules = new vector<LocalMovement>();
        MovingState s;
        for(int i=LocalRules_FBtoFB_DtoU_FtoE.size()-1; i >= 0; i--) {
            LocalMovement lmvt = LocalRules_FBtoFB_DtoU_FtoE[i];
            Cell3DPosition tmp = lmvt.currentPosition.offsetZ(-4).offsetY(1);
            lmvt.currentPosition = lmvt.nextPosition.offsetZ(-4).offsetY(1);
            lmvt.nextPosition = tmp;
            if(lmvt.state != MOVING) {
                s = lmvt.state;
            }
            if(i == 0) {
                lmvt.state = IN_POSITION;
            }else if(LocalRules_FBtoFB_DtoU_FtoE[i-1].nextPosition == LocalRules_FBtoFB_DtoU_FtoE[i].currentPosition) {
                lmvt.state = MOVING;
            } else {
                lmvt.state = IN_POSITION;
            }
            (*localRules).push_back(lmvt);
        }
        break;
    }
    default:
        break;
    }
    return localRules;
}

#endif