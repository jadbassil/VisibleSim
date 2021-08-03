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
    FB_Dismantle_Left,
    FB_Fill_Left,
    FB_Build_Up,
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
    //Z even
    {LocalMovement(Cell3DPosition(2,-1,2), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,0), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(-1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-2,-1,1), WAITING)},

    {LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(0,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(-1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},


    {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},


    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},


    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_FB_Dismantle_Left = { 
    // Z even
   {LocalMovement(Cell3DPosition(2,1,2), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,0), WAITING)},

    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(0,0,3), MOVING)}, //5
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-2,0,1), WAITING)},

    {LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(0,0,3), MOVING)}, // 10
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)}, 

    {LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(-1,0,3), MOVING)}, //17
    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},


    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)}, //23
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,0,1), MOVING)}, //28
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},


    {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(-1,0,2), MOVING)}, //33
    // {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(0,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},


    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-1,0,1), MOVING)}, //39
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-1,0,1), MOVING)}, //44
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,-1,1), MOVING)}, //49
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), MOVING)}, //55
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,3), MOVING)}, //59
    {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

};


// static vector<LocalMovement> LocalRules_FB_Fill_Left = {
//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,0), WAITING)},

//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,4), WAITING)},

//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},//12
//     {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), WAITING)},

//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,3), IN_POSITION)}, //17

//     {LocalMovement(Cell3DPosition(-1,0,4), Cell3DPosition(-1,0,3), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-1,0,1), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)}, //20
//     {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(0,-1,3), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,1), MOVING)},
//     {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(0,-1,1), IN_POSITION)},

//     // {LocalMovement(Cell3DPosition(2,0,1), Cell3DPosition(2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,1), IN_POSITION)},

//     // {LocalMovement(Cell3DPosition(2,0,3), Cell3DPosition(-2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), IN_POSITION)}, //25


// };

static vector<LocalMovement> LocalRules_FB_Transfer_Left = {
    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)}, //0
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,0), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)}, //4
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-2,0,1), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)}, //8
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,0), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,1,0), Cell3DPosition(-2,1,0), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,1,0), Cell3DPosition(-3,0,1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},
    

    {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-2,0,0), WAITING)}, //14

    {LocalMovement(Cell3DPosition(-3,0,1), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2,0,0), Cell3DPosition(-3,0,1), IN_POSITION)},

};
 
static vector<LocalMovement> LocalRules_BF_Transfer_Left = {
    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,0), WAITING)},


    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-2,-1,1), WAITING)},

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-2,0,0), WAITING)},


    {LocalMovement(Cell3DPosition(-3,-1,1), Cell3DPosition(-3,-1,1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-3,-1,1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-2,0,0), Cell3DPosition(-3,-1,1), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_FB_Transfer_Up_Zeven = {
    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-2,5), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_BF_Transfer_Up_Zeven = {
    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(1,-1,5), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_FB_Transfer_Up_Zodd = {
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 4), Cell3DPosition(1, 0, 5), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_BF_Transfer_Up_Zodd = {
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(2, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, 1, 4), Cell3DPosition(1, 1, 5), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_FB_Transfer_Back = {
    // {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,1,2), WAITING)},

    // {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), WAITING)},

    // {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(0,1,3), MOVING)},
    // {LocalMovement(Cell3DPosition(0,1,3), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(1,3,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,1,3), MOVING)},
    // {LocalMovement(Cell3DPosition(0,1,3), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(1,3,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(1,3,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,1,2), WAITING)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)}, //2
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), WAITING)},

    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)}, //5
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(1,3,2), IN_POSITION)},
    //{LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)}, //7
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(1,3,2), IN_POSITION)},
    //{LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(1,3,2), IN_POSITION)}, //11
    //{LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

};


static vector<LocalMovement> LocalRules_BF_Transfer_Back = {
    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,1,2), WAITING)},

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,0,2), WAITING)}, //2

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)}, //7
    // {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(0,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(1,1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), WAITING)},

    // {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)}, //12
    // {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(0,3,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,3,2), Cell3DPosition(-1,3,3), WAITING)},

    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)}, //15
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(-1,3,3), Cell3DPosition(0,3,2), MOVING)}, //17
    // {LocalMovement(Cell3DPosition(0,3,2), Cell3DPosition(0,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //2
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //10
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //18
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //26
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //34
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //42
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //50
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //58
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)}, //64
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), WAITING)}, //66

    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

};
static vector<LocalMovement> LocalRules_BF_Transfer_Back_ComingFromBack = {
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), WAITING)}, //3

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //4
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //8
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},  // 14
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), WAITING)},

    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},  // 17
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 2), MOVING)},  // 19
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_FB_Build_Up = {
    // Z even coming from left
//     // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
//     // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(-1, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(-1, 0, 4), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, -1, 5), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 5), Cell3DPosition(0, -1, 4), IN_POSITION)},
//     {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},
//     {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 4), MOVING)},
//     {LocalMovement(Cell3DPosition(2, 0, 4), Cell3DPosition(1, 0, 5), MOVING)},
//     {LocalMovement(Cell3DPosition(1, 0, 5), Cell3DPosition(0, 0, 5), MOVING)},
//     {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
//     {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(0, -1, 4), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},  // 6
//     {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},  // 10
//     {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(2, -1, 4), MOVING)},
//     {LocalMovement(Cell3DPosition(2, -1, 4), Cell3DPosition(2, -1, 5), MOVING)},
//     {LocalMovement(Cell3DPosition(2, -1, 5), Cell3DPosition(2, 0, 6), IN_POSITION)},

//     // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 16
//     // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(-1, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(-1, 0, 4), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, -1, 5), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 5), Cell3DPosition(-1, -2, 5), IN_POSITION)},
//     {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
//     {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(1,-1,4), MOVING)},
//     {LocalMovement(Cell3DPosition(1,-1,4), Cell3DPosition(0,-2,5), MOVING)},
//     {LocalMovement(Cell3DPosition(0,-2,5), Cell3DPosition(-1,-2,5), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},  // 22
//     {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -2, 5), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -2, 5), Cell3DPosition(1, -1, 6), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 6), Cell3DPosition(1, 0, 6), MOVING)},
//     {LocalMovement(Cell3DPosition(1, 0, 6), Cell3DPosition(1, -1, 7), IN_POSITION)},

//     // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 29
//     // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(-1, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(-1, 0, 4), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, -1, 5), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 5), Cell3DPosition(-1, -1, 6), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 6), Cell3DPosition(-1, -2, 6), IN_POSITION)},
//    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
//     {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(1,-1,4), MOVING)},
//     {LocalMovement(Cell3DPosition(1,-1,4), Cell3DPosition(0,-2,5), MOVING)},
//     {LocalMovement(Cell3DPosition(0,-2,5), Cell3DPosition(0,-2,6), MOVING)},
//     {LocalMovement(Cell3DPosition(0,-2,6), Cell3DPosition(-1,-2,6), IN_POSITION)},


//     // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 36
//     // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(-1, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(-1, 0, 4), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, -1, 5), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 5), Cell3DPosition(0, -1, 6), MOVING)},
//     // {LocalMovement(Cell3DPosition(0, -1, 6), Cell3DPosition(0, -2, 6), MOVING)},
//     // {LocalMovement(Cell3DPosition(0, -2, 6), Cell3DPosition(-1, -2, 7), IN_POSITION)},

//        {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
//     {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(1,-1,4), MOVING)},
//     {LocalMovement(Cell3DPosition(1,-1,4), Cell3DPosition(0,-2,5), MOVING)},
//     {LocalMovement(Cell3DPosition(0,-2,5), Cell3DPosition(0,-1,6), MOVING)},
//     {LocalMovement(Cell3DPosition(0,-1,6), Cell3DPosition(-1,-1,6), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,-1,6), Cell3DPosition(-1,-2,7), IN_POSITION)},


//     {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},  // 44
//     {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -2, 5), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -2, 5), Cell3DPosition(1, -1, 6), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 6), Cell3DPosition(0, -1, 7), MOVING)},
//     {LocalMovement(Cell3DPosition(0, -1, 7), Cell3DPosition(1, -1, 8), IN_POSITION)},

//     // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 51
//     // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(-1, 0, 3), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(-1, 0, 4), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, -1, 5), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 5), Cell3DPosition(-1, -1, 6), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 6), Cell3DPosition(-1, -1, 7), MOVING)},
//     // {LocalMovement(Cell3DPosition(-1, -1, 7), Cell3DPosition(0, -1, 8), IN_POSITION)},
//     {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},
//     {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(0, -2, 5), MOVING)},
//     {LocalMovement(Cell3DPosition(0, -2, 5), Cell3DPosition(0, -1, 6), MOVING)},
//     {LocalMovement(Cell3DPosition(0, -1, 6), Cell3DPosition(-1, -1, 7), MOVING)},
//     {LocalMovement(Cell3DPosition(-1, -1, 7), Cell3DPosition(0, -1, 8), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},
//     {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
//     {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), IN_POSITION)},
{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(1,-1,5), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,5), Cell3DPosition(0,-1,5), MOVING)},
{LocalMovement(Cell3DPosition(0,-1,5), Cell3DPosition(0,-1,4), IN_POSITION)},


{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(1,-1,4), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,0,4), MOVING)},
{LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(1,-1,5), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,5), Cell3DPosition(0,-1,5), MOVING)},
{LocalMovement(Cell3DPosition(0,-1,5), Cell3DPosition(-1,-2,5), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,0,4), MOVING)},
{LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(1,-1,5), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-2,5), MOVING)},
{LocalMovement(Cell3DPosition(1,-2,5), Cell3DPosition(1,-1,6), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,6), Cell3DPosition(0,-1,5), MOVING)},
{LocalMovement(Cell3DPosition(0,-1,5), Cell3DPosition(-1,-1,5), MOVING)},
{LocalMovement(Cell3DPosition(-1,-1,5), Cell3DPosition(0,-1,6), MOVING)},
{LocalMovement(Cell3DPosition(0,-1,6), Cell3DPosition(-1,-2,6), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,0,4), MOVING)},
{LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(2,0,6), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-2,5), MOVING)},
{LocalMovement(Cell3DPosition(1,-2,5), Cell3DPosition(0,-2,5), MOVING)},
{LocalMovement(Cell3DPosition(0,-2,5), Cell3DPosition(0,-1,6), MOVING)},
{LocalMovement(Cell3DPosition(0,-1,6), Cell3DPosition(-1,-1,6), MOVING)},
{LocalMovement(Cell3DPosition(-1,-1,6), Cell3DPosition(-1,-2,7), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(2,-1,6), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,6), Cell3DPosition(1,-1,7), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(1,-2,5), MOVING)},
{LocalMovement(Cell3DPosition(1,-2,5), Cell3DPosition(0,-2,5), MOVING)},
{LocalMovement(Cell3DPosition(0,-2,5), Cell3DPosition(0,-1,6), MOVING)},
{LocalMovement(Cell3DPosition(0,-1,6), Cell3DPosition(-1,-1,7), MOVING)},
{LocalMovement(Cell3DPosition(-1,-1,7), Cell3DPosition(0,-1,8), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,-1,4), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,4), Cell3DPosition(2,-1,6), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,6), Cell3DPosition(2,-1,8), MOVING)},
{LocalMovement(Cell3DPosition(2,-1,8), Cell3DPosition(1,-1,8), IN_POSITION)},


};

static vector<LocalMovement> LocalRules_BF_Build_Up = {
    // Z even coming from left
    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(0, -1, 4), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    // {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(0, -1, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(0, -1, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(2, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, -1, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, -2, 5), IN_POSITION)},
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(-1, -1, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(2, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 6), Cell3DPosition(2, -2, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(-1, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 5), Cell3DPosition(-1, 0, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 6), Cell3DPosition(2, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 6), Cell3DPosition(1, -2, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(0, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 6), Cell3DPosition(-1, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 6), Cell3DPosition(-1, -1, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 6), Cell3DPosition(1, -1, 7), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 7), Cell3DPosition(1, -1, 8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(-1, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 5), Cell3DPosition(0, 0, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 6), Cell3DPosition(-1, 0, 7), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 7), Cell3DPosition(0, 0, 8), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 8), Cell3DPosition(0, -1, 8), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_FB_Build_Up_ZOdd = {
    // Z even coming from left
    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(1,1,4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,4), Cell3DPosition(-1,0,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(1,0,5), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,5), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,4), Cell3DPosition(-1,1,5), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,5), Cell3DPosition(-1,1,6), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,6), Cell3DPosition(-1,0,6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(1,0,5), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,5), Cell3DPosition(2,1,6), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,6), Cell3DPosition(2,2,6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,4), Cell3DPosition(-1,1,5), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,5), Cell3DPosition(-1,1,6), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,6), Cell3DPosition(-1,0,7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(1,0,5), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,5), Cell3DPosition(2,1,6), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,6), Cell3DPosition(1,1,7), IN_POSITION)},
    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,4), Cell3DPosition(-1,1,5), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,5), Cell3DPosition(0,1,6), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,6), Cell3DPosition(-1,1,7), MOVING)},
    {LocalMovement(Cell3DPosition(-1,1,7), Cell3DPosition(0,1,8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(2,0,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,4), Cell3DPosition(1,0,5), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,5), Cell3DPosition(1,1,6), MOVING)},
    {LocalMovement(Cell3DPosition(1,1,6), Cell3DPosition(1,0,7), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,7), Cell3DPosition(1,1,8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,1,4), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_BF_Build_Up_ZOdd = {
    // Z even coming from left
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, 1, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(-1, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 4), Cell3DPosition(-1, 1, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 5), Cell3DPosition(2, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, 1, 6), Cell3DPosition(2, 0, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(-1, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 4), Cell3DPosition(-2, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 5), Cell3DPosition(-1, 2, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 5), Cell3DPosition(1, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 6), Cell3DPosition(1, 0, 6), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 6), Cell3DPosition(1, 0, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(-1, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 5), Cell3DPosition(0, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 6), Cell3DPosition(-1, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 6), Cell3DPosition(-1, 1, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 5), Cell3DPosition(1, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 6), Cell3DPosition(1, 1, 7), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 7), Cell3DPosition(1, 1, 8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(-1, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 4), Cell3DPosition(-1, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 5), Cell3DPosition(-1, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 6), Cell3DPosition(-1, 0, 7), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 7), Cell3DPosition(0, 1, 8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_FB_Fill_Left_Zeven = { //Z_Odd
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), WAITING)},

    // Get 5 modules to the back
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 4
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 1), Cell3DPosition(-5, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 1), Cell3DPosition(-5, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 13
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 3), Cell3DPosition(-5, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 3), Cell3DPosition(-5, 0, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 22
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 1), Cell3DPosition(-5, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 1), Cell3DPosition(-5, -1, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 31
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 3), Cell3DPosition(-5, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 3), Cell3DPosition(-5, -1, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 40
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 1), IN_POSITION)},
    

    // Fill Front
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 47
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)}, //bridge
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-3,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,3), Cell3DPosition(-4,0,3), IN_POSITION)},


    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-3,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,1), Cell3DPosition(-4,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-3,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-3,0,3), IN_POSITION)},

    // get back modules to position

    {LocalMovement(Cell3DPosition(-5, -1, 0), Cell3DPosition(-5, -1, 1), IN_POSITION)},  // 77
    {LocalMovement(Cell3DPosition(-5, -1, 4), Cell3DPosition(-5, -1, 3), MOVING)},       // 78
    {LocalMovement(Cell3DPosition(-5, -1, 3), Cell3DPosition(-4, -1, 3), IN_POSITION)},  // 79
    {LocalMovement(Cell3DPosition(-5, 0, 4), Cell3DPosition(-5, -1, 3), IN_POSITION)},   // 80
    {LocalMovement(Cell3DPosition(-5, 0, 0), Cell3DPosition(-5, -1, 0), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 0), Cell3DPosition(-5, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 2), Cell3DPosition(-5, 0, 2), IN_POSITION)},

};


static vector<LocalMovement> LocalRules_FB_Fill_Left = { //Z_Odd
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), WAITING)},

    // Get 5 modules to the back
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 4
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 1), Cell3DPosition(-5, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 1), Cell3DPosition(-5, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 13
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 3), Cell3DPosition(-5, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 3), Cell3DPosition(-5, 0, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 22
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 1), Cell3DPosition(-5, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 1), Cell3DPosition(-5, -1, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 31
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-4, -1, 3), Cell3DPosition(-5, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 3), Cell3DPosition(-5, -1, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 40
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, -1, 1), IN_POSITION)},

    // Fill Front
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 47
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 2), WAITING)},

    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 1, 2), MOVING)},  // 52
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 1), Cell3DPosition(-4, 0, 1), IN_POSITION)},  // 60

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 3), Cell3DPosition(-3, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 1, 2), Cell3DPosition(-4, 1, 2), IN_POSITION)},  // 68

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 3), Cell3DPosition(-3, 1, 2),
                   IN_POSITION)},  // 75  // send back

    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},  // 76

    // get back modules to position

    {LocalMovement(Cell3DPosition(-5, -1, 0), Cell3DPosition(-5, -1, 1), IN_POSITION)},  // 77
    {LocalMovement(Cell3DPosition(-5, -1, 4), Cell3DPosition(-5, -1, 3), MOVING)},       // 78
    {LocalMovement(Cell3DPosition(-5, -1, 3), Cell3DPosition(-4, -1, 3), IN_POSITION)},  // 79
    {LocalMovement(Cell3DPosition(-5, 0, 4), Cell3DPosition(-5, -1, 3), IN_POSITION)},   // 80
    {LocalMovement(Cell3DPosition(-5, 0, 0), Cell3DPosition(-5, -1, 0), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 0), Cell3DPosition(-5, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-5, -1, 2), Cell3DPosition(-5, 0, 2), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_BF_Fill_Left = { //Z_ODD
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, 
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, 0, 2), WAITING)},
    // moving 5 modules to back
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //3
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 3), Cell3DPosition(-5, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 3), Cell3DPosition(-5, 0, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //11
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 1), Cell3DPosition(-5, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 1), Cell3DPosition(-5, 0, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //19
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 1), Cell3DPosition(-5, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 1), Cell3DPosition(-5, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //27
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 1), Cell3DPosition(-5, 0, 1), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //34
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 3), WAITING)},

    // fill the front
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //40
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-3, -1, 3), Cell3DPosition(-4, -1, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //46
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-3, -1, 1), Cell3DPosition(-4, -1, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //52
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(0, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //59
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), WAITING)},

    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 3), Cell3DPosition(-2, 0, 4), WAITING)},
    // {LocalMovement(Cell3DPosition(-3, -1, 2), Cell3DPosition(-4, -1, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},
    
     {LocalMovement(Cell3DPosition(-2, 0, 4), Cell3DPosition(-2, -1, 3), MOVING)},
     {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), IN_POSITION)},

    // get back modules to position
    {LocalMovement(Cell3DPosition(-5, 0, 1), Cell3DPosition(-4, 0, 2), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-5, 0, 0), Cell3DPosition(-5, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 1), Cell3DPosition(-4, 0, 1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-5, 0, 2), Cell3DPosition(-5, 0, 1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-4, 0, 3), Cell3DPosition(-3, 1, 2), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-5, 0, 4), Cell3DPosition(-5, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 3), Cell3DPosition(-4, 1, 2), IN_POSITION)},
};

[[mabe_unused]]
static vector<LocalMovement>* setLocalRules(MMOperation op) {
    vector<LocalMovement>* localRules;
    switch (op) {
    case BF_Dismantle_Left: {
        localRules =  &LocalRules_BF_Dismantle_Left;
        break;
    }
    case FB_Dismantle_Left: {
        localRules = &LocalRules_FB_Dismantle_Left;
        break;
    }
    case FB_Fill_Left: {
        localRules =  &LocalRules_BF_Fill_Left;
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
    case FB_Build_Up: {
        localRules = &LocalRules_FB_Build_Up;
        break;
    } 
    default:
        break;
    }
    return localRules;
}


#endif