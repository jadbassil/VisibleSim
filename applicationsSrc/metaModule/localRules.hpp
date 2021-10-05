#ifndef LOCALRULES1_HPP__
#define LOCALRULES1_HPP__

#include "grid/cell3DPosition.h"
#include <bitset>
#include <vector>
#include <utility>

#include <iostream>
using namespace std;

enum MovingState {IN_POSITION, MOVING, WAITING};

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
    // Z even
    // {LocalMovement(Cell3DPosition(2,-1,2), Cell3DPosition(2,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,0), WAITING)},

    // {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(0,-1,3), MOVING)},
    // {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(-1,-1,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(-1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-2,-1,1), WAITING)},

    // {LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(0,-1,3), MOVING)},
    // {LocalMovement(Cell3DPosition(0,-1,3), Cell3DPosition(-1,-1,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(0,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(0,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(0,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(0,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(0,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(-2,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-1,-1,1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,0,1), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-2,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(0, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-2, -1, 1), WAITING)},

    {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(0, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, -1, 2), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 0), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_FB_Dismantle_Left = { 
    // Z even
//    {LocalMovement(Cell3DPosition(2,1,2), Cell3DPosition(2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,0), WAITING)},

//     {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(0,0,3), MOVING)}, //5
//     {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(-1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-2,0,1), WAITING)},

//     {LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(0,0,3), MOVING)}, // 10
//     {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(-1,0,3), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)}, 

//     {LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(-1,0,3), MOVING)}, //17
//     {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},


//     {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)}, //23
//     {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,0,1), MOVING)}, //28
//     {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},


//     {LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(-1,0,2), MOVING)}, //33
//     // {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(0,0,2), MOVING)},
//     // {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,1), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},


//     {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-1,0,1), MOVING)}, //39
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-1,0,1), MOVING)}, //44
//     {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,-1,1), MOVING)}, //49
//     {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(0,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), MOVING)}, //55
//     {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

//     {LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,3), MOVING)}, //59
//     {LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
//     {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(-1,0,3), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,2), MOVING)},
{LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,1), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,0), WAITING)},

{LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(0,0,3), MOVING)},
{LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(1,1,2), MOVING)},
{LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,0,1), MOVING)},
{LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-2,0,1), WAITING)},

{LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(1,1,2), MOVING)}, //10
{LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,0,1), MOVING)},
{LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(2,1,2), Cell3DPosition(1,1,2), MOVING)},
{LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,0,1), MOVING)},
{LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(-1,-1,3), Cell3DPosition(-1,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-1,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)},
{LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,0,1), MOVING)},
{LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(-1,0,1), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(-2,1,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,1,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-1,0,1), MOVING)}, //37
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(-1,0,0), Cell3DPosition(-1,0,1), MOVING)}, //42
{LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,-1,1), MOVING)}, //47
{LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(0,0,2), MOVING)},
{LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(-1,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(-1,0,2), MOVING)}, //53
{LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,3), MOVING)},
{LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},

{LocalMovement(Cell3DPosition(-1,-1,2), Cell3DPosition(-2,-1,3), MOVING)}, //57
{LocalMovement(Cell3DPosition(-2,-1,3), Cell3DPosition(-2,0,2), MOVING)},
{LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,0,1), IN_POSITION)},


};

static vector<LocalMovement> LocalRules_FB_Dismantle_Left_Zodd = {
    {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(0, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-2, 0, 1), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, 1, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 1, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},  // 39
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 0), Cell3DPosition(-1, 0, 1), MOVING)},  // 44
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, -1, 1), MOVING)},  // 49
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 2), MOVING)},  // 55
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 3), MOVING)},  // 59
    {LocalMovement(Cell3DPosition(-2, -1, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 1), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_BF_Dismantle_Left_Zodd = {
    {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-2, -1, 1), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, -1, 2), Cell3DPosition(1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 0), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(-2, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 3), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_BF_Dismantle_Back = {
    {LocalMovement(Cell3DPosition(2, -1, 2), Cell3DPosition(1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(0, -1, 3), MOVING)},  // 5
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(0, -1, 1), MOVING)},  // 12
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(0, -1, 3), MOVING)}, //19
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(0, -1, 1), MOVING)},  // 26
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(0, 0, 3), MOVING)}, //33
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, 0, 1), MOVING)}, //40
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)}, //45

    {LocalMovement(Cell3DPosition(-1, 0, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(-1, 1, 3), MOVING)}, //46
    // {LocalMovement(Cell3DPosition(-1, 1, 3), Cell3DPosition(0, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(0, 0, 2), MOVING)}, //51
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), WAITING)},

    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 2), MOVING)}, //59
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},  // 
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), IN_POSITION)}, //64
};

static vector<LocalMovement> LocalRules_FB_Dismantle_Back = {
    // {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(0, -1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(0, -1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), WAITING)},

    // {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, -1, 3), MOVING)},  // 5
    // {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, -1, 1), MOVING)},  // 13
    // {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(0, -1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, -1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(0, 0, 3), MOVING)},  // 30
    // {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 3), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(0, 0, 1), MOVING)},  // 35
    // {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(1,3,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(0, 1, 3), MOVING)},  // 40
    // {LocalMovement(Cell3DPosition(0, 1, 3), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 44
    // {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(1,0,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    // {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(1,3,2), MOVING)},
    // {LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(1, 1, 3), MOVING)},  // 49
    // {LocalMovement(Cell3DPosition(1, 1, 3), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(2, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},  // 53
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), IN_POSITION)},
    ///
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(0, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, -1, 3), MOVING)},  // 5
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, -1, 1), MOVING)},  // 13
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(0, 0, 3), MOVING)}, 
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(0, 0, 1), MOVING)},  // 35
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 0, 2), MOVING)},  // 40
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 44
    // {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), WAITING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

   {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(1,3,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(1,3,2), IN_POSITION)},


    {LocalMovement(Cell3DPosition(2, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},  // 53
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(0, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, -1, 3), MOVING)},  // 5
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, -1, 1), MOVING)},  // 13
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(0, 0, 3), MOVING)}, 
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(0, 0, 1), MOVING)},  // 35
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 0, 2), MOVING)},  // 40
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)},
     {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 44
    // {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), WAITING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},

   {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(1,3,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(1,3,2), IN_POSITION)},


    {LocalMovement(Cell3DPosition(2, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},  // 53
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_BF_Dismantle_Up_Zodd = {

    {LocalMovement(Cell3DPosition(0,0,0), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,0), Cell3DPosition(0,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(1,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,2), Cell3DPosition(1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,3), MOVING)},
    // {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,1), Cell3DPosition(-1,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,1,2), Cell3DPosition(0,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2,-1,2), Cell3DPosition(2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(2,0,2), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,0,3), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,-1,3), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0,0,4), Cell3DPosition(0,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,3), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,4), Cell3DPosition(1,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,3), Cell3DPosition(2,1,4), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,4), Cell3DPosition(1,1,5), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_FB_Dismantle_Up_Zodd = {
    {LocalMovement(Cell3DPosition(1, 0, 0), Cell3DPosition(1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 0), Cell3DPosition(0, -1, 1), MOVING)}, //7
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, -1, 2), MOVING)}, //13
    {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(2, 0, 2), MOVING)}, //18
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(0, -1, 2), MOVING)}, //23
    {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(0, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, 1, 2), Cell3DPosition(2, 0, 2), MOVING)}, //28
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, -1, 3), MOVING)}, //33
    {LocalMovement(Cell3DPosition(0, -1, 3), Cell3DPosition(1, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(2, 0, 4), MOVING)}, //37
    {LocalMovement(Cell3DPosition(2, 0, 4), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 4), Cell3DPosition(0, -1, 5), MOVING)}, //39
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 4), Cell3DPosition(1, 0, 5), IN_POSITION)}, //41

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

static vector<LocalMovement>  LocalRules_FB_Transfer_Down_Zeven = {
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, -1, 0), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 0), Cell3DPosition(1, -2, -1), MOVING)},
     {LocalMovement(Cell3DPosition(1, -2, -1), Cell3DPosition(2, -1, -2), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, -2), Cell3DPosition(1, -2, -3), IN_POSITION)},
};

static vector<LocalMovement>  LocalRules_FB_Transfer_Down_Zodd = {
    
    {LocalMovement(Cell3DPosition(1,-1,1), Cell3DPosition(1,-1,0), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,0), Cell3DPosition(1,-1,-1), MOVING)},
    {LocalMovement(Cell3DPosition(1,-1,-1), Cell3DPosition(1,0,-1), MOVING)},
    {LocalMovement(Cell3DPosition(1,0,-1), Cell3DPosition(2,1,-2), MOVING)},
    {LocalMovement(Cell3DPosition(2,1,-2), Cell3DPosition(1,0,-3), IN_POSITION)},
};


static vector<LocalMovement>
    LocalRules_FB_Transfer_Back = {
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

        {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
        {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), WAITING)},

        {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 2
        {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
        {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), WAITING)},

        {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},  // 5
        {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},
        //{LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

        {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 7
        {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
        {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
        {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},
        //{LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

        {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), IN_POSITION)},  // 11
        //{LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,1), IN_POSITION)},

};

static vector<LocalMovement>   LocalRules_BF_Transfer_Back = {
    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(0,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,1), Cell3DPosition(0,1,2), WAITING)},

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,0,2), WAITING)}, //2

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //3
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //7
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //11
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //15
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},  

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //19
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)}, 

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //23
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //27
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

     {LocalMovement(Cell3DPosition(1,0,1), Cell3DPosition(1,1,2), MOVING)}, //27
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1,0,2), Cell3DPosition(0,-1,1), MOVING)}, //31
    {LocalMovement(Cell3DPosition(0,-1,1), Cell3DPosition(-1,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-1,-1,1), Cell3DPosition(0,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,0,2), Cell3DPosition(1,1,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), WAITING)},

    {LocalMovement(Cell3DPosition(0,1,2), Cell3DPosition(1,2,2), MOVING)}, //36
    {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(0,3,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,3,2), Cell3DPosition(-1,3,3), WAITING)},

    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)}, //39
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1,3,3), Cell3DPosition(0,3,2), MOVING)}, //41 
    {LocalMovement(Cell3DPosition(0,3,2), Cell3DPosition(0,2,2), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(0,2,1), MOVING)}, 
    {LocalMovement(Cell3DPosition(0,2,1), Cell3DPosition(1,2,1), IN_POSITION)},

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
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(0, -1, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //6
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(1, -1, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //9
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 4), Cell3DPosition(1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 5), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(-1, -2, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //15
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 4), Cell3DPosition(1, -1, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //19
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 4), Cell3DPosition(1, -2, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -2, 5), Cell3DPosition(1, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 6), Cell3DPosition(0, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 5), Cell3DPosition(-1, -1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 5), Cell3DPosition(0, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 6), Cell3DPosition(-1, -2, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //28
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, 0, 4), Cell3DPosition(2, 0, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //32
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 4), Cell3DPosition(1, -2, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -2, 5), Cell3DPosition(0, -2, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -2, 5), Cell3DPosition(0, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 6), Cell3DPosition(-1, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 6), Cell3DPosition(-1, -2, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //40
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 4), Cell3DPosition(2, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 6), Cell3DPosition(1, -1, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //45
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 4), Cell3DPosition(1, -2, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, -2, 5), Cell3DPosition(0, -2, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, -2, 5), Cell3DPosition(0, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 6), Cell3DPosition(-1, -1, 7), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 7), Cell3DPosition(0, -1, 8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(2, 0, 2), MOVING)}, //53
    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, -1, 3), Cell3DPosition(2, -1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 4), Cell3DPosition(2, -1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 6), Cell3DPosition(2, -1, 8), MOVING)},
    {LocalMovement(Cell3DPosition(2, -1, 8), Cell3DPosition(1, -1, 8), IN_POSITION)},

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

static vector<LocalMovement> LocalRules_BF_Build_Up_ZOdd_ComingFromBack = {
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 5), Cell3DPosition(0, 1, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 0, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 5), Cell3DPosition(-1, 1, 5), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 5), Cell3DPosition(2, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, 1, 6), Cell3DPosition(2, 0, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 5), Cell3DPosition(0, 2, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 6), Cell3DPosition(-1, 2, 6), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 5), Cell3DPosition(2, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, 1, 6), Cell3DPosition(1, 0, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 5), Cell3DPosition(0, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 6), Cell3DPosition(-1, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 6), Cell3DPosition(-1, 1, 7), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(1, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 5), Cell3DPosition(2, 1, 6), MOVING)},
    {LocalMovement(Cell3DPosition(2, 1, 6), Cell3DPosition(1, 1, 7), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 7), Cell3DPosition(1, 1, 8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 1, 5), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 5), Cell3DPosition(0, 2, 6), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 6), Cell3DPosition(0, 2, 8), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 8), Cell3DPosition(0, 1, 8), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_FB_Build_Back = {
    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(2, 2, 2), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //3
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(1, 1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 3), Cell3DPosition(1, 2, 2), WAITING)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},//7
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 3), Cell3DPosition(1, 2, 3), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //11
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 1), Cell3DPosition(1, 2, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //15
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 1), Cell3DPosition(0, 2, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 3, 0), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //20
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(0, 3, 0), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //25
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(1, 3, 4), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //30
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(-1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //37
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(0, 4, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 4, 2), Cell3DPosition(-1, 4, 2), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //45
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(0, 4, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 4, 2), Cell3DPosition(-1, 3, 3), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 3), MOVING)}, //53 
    // {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(0, 2, 5), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 5), Cell3DPosition(0, 3, 4), IN_POSITION)},
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(2, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), WAITING)},

    // {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //2
    // {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 1, 3), Cell3DPosition(2, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //7
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 2, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //11
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //15
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 0), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //20
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(0, 3, 0), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //25
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(-1, 3, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //30
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(-1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 1), Cell3DPosition(-1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 2), Cell3DPosition(-1, 4, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //37
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(-1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 1), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 2), Cell3DPosition(-1, 3, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //45
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1,1,2), Cell3DPosition(0,2,2), MOVING)}, //48
    {LocalMovement(Cell3DPosition(0,2,2), Cell3DPosition(1,3,2), MOVING)},
    {LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(0,2,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,3), Cell3DPosition(1,3,4), IN_POSITION)},


    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 3), MOVING)}, //52
    {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(0, 3, 4), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_BF_Build_Back = {
    // not coming from back
    // TODO: Fix
    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(-1, 2, 2), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 3
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(-1, 1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 1, 3), Cell3DPosition(0, 2, 2), WAITING)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 7
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(0, 1, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 3), Cell3DPosition(-1, 2, 3), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 11
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(0, 1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 1), Cell3DPosition(-1, 2, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 15
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(0, 1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 1), Cell3DPosition(0, 2, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(0, 3, 0), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 20
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(0, 1, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 1), Cell3DPosition(0, 2, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 3, 0), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 25
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(-1, 3, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(-1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 3, 1), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 32
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(-1, 3, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(-1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 4, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 4, 2), Cell3DPosition(2, 4, 2), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 42
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(-1, 3, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(-1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 4, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 4, 2), Cell3DPosition(1, 3, 3), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 50
    // {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(-1, 3, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(-1, 3, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 4, 2), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 4, 2), Cell3DPosition(1, 4, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 4, 3), Cell3DPosition(1, 4, 4), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 4, 4), Cell3DPosition(1, 3, 4), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 4), MOVING)},  // 59
    // {LocalMovement(Cell3DPosition(0, 2, 4), Cell3DPosition(0, 3, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(-1, 1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 3), Cell3DPosition(-1, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(-1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(0, 3, 0), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 3, 0), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(-1, 2, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(0, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 3, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(2, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(2, 4, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(2, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(1, 3, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(0, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(0, 3, 4), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(2, 3, 4), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 4), Cell3DPosition(1, 3, 4), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_BF_Build_Back_ComingFromBack = {
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), WAITING)}, //3
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(-1, 1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 1, 3), Cell3DPosition(-1, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //8
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(-1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //12
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(0, 3, 0), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //17
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(-1, 2, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //21
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 3, 0), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //26
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 3, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //31
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(2, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(2, 4, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)}, //38
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(2, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(1, 3, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //44
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)}, //50
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(2, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(1, 3, 4), IN_POSITION)},

    // {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 3, 2), MOVING)}, //58
    // {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(-1, 3, 4), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 3, 4), Cell3DPosition(-1, 2, 4), MOVING)},
    // {LocalMovement(Cell3DPosition(-1, 2, 4), Cell3DPosition(0, 3, 4), IN_POSITION)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 4), MOVING)},  // 59
    {LocalMovement(Cell3DPosition(0, 2, 4), Cell3DPosition(0, 3, 4), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_BF_Fill_Left_Zeven = {
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, 
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, 0, 2), WAITING)},
    // moving 5 modules to back
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //4
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 3), Cell3DPosition(-5, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 3), Cell3DPosition(-5, 0, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //12
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 1), Cell3DPosition(-5, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 1), Cell3DPosition(-5, 0, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //20
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 1), Cell3DPosition(-5, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-5, 0, 1), Cell3DPosition(-5, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //28
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-4, 0, 1), Cell3DPosition(-5, 0, 1), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //35
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, 0, 2), Cell3DPosition(-3, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-3, 0, 2), Cell3DPosition(-4, 0, 3), WAITING)},

    // fill the front 
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //41
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-3,-1,3), Cell3DPosition(-2,0,4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //47
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-3,-1,3), Cell3DPosition(-2,-1,4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //53
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(0, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-3,-1,1), Cell3DPosition(-4,-1,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //61
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-3,-1,1), Cell3DPosition(-3,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-3,-1,2), Cell3DPosition(-4,-1,2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-2,-1,4), Cell3DPosition(-3,-1,3), MOVING)}, //68
    {LocalMovement(Cell3DPosition(-3,-1,3), Cell3DPosition(-2,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-3,-1,1), IN_POSITION)},


    {LocalMovement(Cell3DPosition(-2,0,4), Cell3DPosition(-3,-1,3), MOVING)}, //71
    {LocalMovement(Cell3DPosition(-3,-1,3), Cell3DPosition(-2,0,2), WAITING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-2,-1,2), Cell3DPosition(-3,-1,2), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-2,0,2), Cell3DPosition(-2,-1,2), IN_POSITION)},
    // get back modules to position
    {LocalMovement(Cell3DPosition(-5,0,1), Cell3DPosition(-4,0,1), IN_POSITION)}, //76
    {LocalMovement(Cell3DPosition(-5,0,4), Cell3DPosition(-5,0,3), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-5,0,0), Cell3DPosition(-5,0,1), IN_POSITION)},
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

    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,3), MOVING)}, //52//bridge
    {LocalMovement(Cell3DPosition(-2,0,3), Cell3DPosition(-3,0,3), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,3), Cell3DPosition(-4,0,3), IN_POSITION)},


    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //55
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(-1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 1), Cell3DPosition(-1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1,0,2), Cell3DPosition(-2,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-2,0,1), Cell3DPosition(-3,0,1), MOVING)},
    {LocalMovement(Cell3DPosition(-3,0,1), Cell3DPosition(-4,0,1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)}, //62
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

    {LocalMovement(Cell3DPosition(-5,-1,0), Cell3DPosition(-5,-1,1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-5,0,0), Cell3DPosition(-6,-1,1), MOVING)},
    {LocalMovement(Cell3DPosition(-6,-1,1), Cell3DPosition(-5,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-5,-1,2), Cell3DPosition(-4,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-4,-1,2), Cell3DPosition(-3,-1,2), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-5,-1,4), Cell3DPosition(-5,-1,3), MOVING)},
    {LocalMovement(Cell3DPosition(-5,-1,3), Cell3DPosition(-5,0,2), MOVING)},
    {LocalMovement(Cell3DPosition(-5,0,2), Cell3DPosition(-5,-1,2), MOVING)},
    {LocalMovement(Cell3DPosition(-5,-1,2), Cell3DPosition(-4,-1,2), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-5,0,4), Cell3DPosition(-5,0,2), IN_POSITION)},

};


static vector<LocalMovement> LocalRules_FB_Fill_Left_Zodd = { 
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

static vector<LocalMovement> LocalRules_BF_Fill_Left_Zodd = { //Z_ODD
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

static vector<LocalMovement> LocalRules_FB_Fill_Back_Zeven = {
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), WAITING)},

    // Get 5 modules to the back
    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 2
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 3), Cell3DPosition(2, 3, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 8
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 1), Cell3DPosition(2, 3, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 14
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(2, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 19
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1,2,2), Cell3DPosition(0,2,3), MOVING)},
    {LocalMovement(Cell3DPosition(0,2,3), Cell3DPosition(-1,2,3), MOVING)},
    {LocalMovement(Cell3DPosition(-1,2,3), Cell3DPosition(-1,3,4), WAITING)},
//     {LocalMovement(Cell3DPosition(1,3,2), Cell3DPosition(1,3,3), MOVING)},
// {LocalMovement(Cell3DPosition(1,3,3), Cell3DPosition(2,4,4), MOVING)},
// {LocalMovement(Cell3DPosition(2,4,4), Cell3DPosition(1,3,5), MOVING)},
// {LocalMovement(Cell3DPosition(1,3,5), Cell3DPosition(0,3,5), MOVING)},
// {LocalMovement(Cell3DPosition(0,3,5), Cell3DPosition(-1,3,5), WAITING)},

    // {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(1, 3, 3), Cell3DPosition(0, 3, 3), MOVING)},
    // {LocalMovement(Cell3DPosition(0, 3, 3), Cell3DPosition(0, 4, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 25
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(0, 4, 2), WAITING)},

    // Front modules

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 32
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(-1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 1), Cell3DPosition(-1, 3, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 38
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(-1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 3), Cell3DPosition(-1, 2, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},  // 43
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(-1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 1), Cell3DPosition(-1, 3, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, -1, 1), Cell3DPosition(1, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 3, 3), WAITING)},

    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(-1, 3, 2), Cell3DPosition(-1, 2, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 2, 4), Cell3DPosition(-1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 3), Cell3DPosition(0, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 3), Cell3DPosition(1, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 3, 0), Cell3DPosition(-1, 2, 0), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 0), Cell3DPosition(-1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 2, 2), Cell3DPosition(0, 2, 2), IN_POSITION)},

    //{LocalMovement(Cell3DPosition(0, 4, 4), Cell3DPosition(-1, 3, 5), MOVING)},
    //{LocalMovement(Cell3DPosition(-1, 3, 5), Cell3DPosition(-1, 3, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 4), Cell3DPosition(-1, 3, 2), IN_POSITION)},

    // Get back modules in place
    {LocalMovement(Cell3DPosition(0, 4, 2), Cell3DPosition(0, 3, 1), IN_POSITION)},
    {LocalMovement(Cell3DPosition(1, 3, 3), Cell3DPosition(0, 3, 3), IN_POSITION)},
    {LocalMovement(Cell3DPosition(2, 3, 4), Cell3DPosition(1, 3, 3), IN_POSITION)},
    {LocalMovement(Cell3DPosition(2, 3, 0), Cell3DPosition(1, 3, 1), IN_POSITION)},
};

static vector<LocalMovement> LocalRules_BF_Fill_Back_Zeven = {
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //2
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)}, //8
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(-1, 3, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //12
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(-1, 3, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //18
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //23
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 3), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(2, 3, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //32
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), MOVING)}, 
    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 3), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(2, 2, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //41
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 4, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)}, //48
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(2, 3, 0), WAITING)},  // must send back

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)}, //51
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //53
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 2, 1), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(2, 3, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //62
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)}, //75
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), WAITING)},

    // set front modules in place
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), IN_POSITION)}, //76

    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(1, 2, 1), MOVING)}, //77
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, 3, 0), Cell3DPosition(1, 2, 1), IN_POSITION)}, //80

    {LocalMovement(Cell3DPosition(2, 2, 4), Cell3DPosition(1, 2, 3), MOVING)}, //81
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(2, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(2, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 2, 2), Cell3DPosition(1, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, 3, 4), Cell3DPosition(2, 3, 2), MOVING)}, //85
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(2, 2, 2), IN_POSITION)},

    // set back modules in place
    {LocalMovement(Cell3DPosition(1, 4, 2), Cell3DPosition(0, 3, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 3, 4), Cell3DPosition(-1, 3, 3), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 3, 0), Cell3DPosition(-1, 3, 1), IN_POSITION)},

};

static vector<LocalMovement> LocalRules_BF_Fill_Back_Zeven_ComingFromBack = {
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), WAITING)},  // 3

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 1, 2), MOVING)},  // 4
    {LocalMovement(Cell3DPosition(1, 1, 2), Cell3DPosition(0, 2, 2), WAITING)},

    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},  // 6
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(-1, 3, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},  // 10
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(-1, 3, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},  // 17
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},  // 23
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 3), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(2, 3, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},  // 33
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 3), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 2, 3), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(2, 2, 4), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)}, // 43
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
     {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(1, 2, 1), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(2, 3, 0), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)}, //53
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 3, 2), Cell3DPosition(2, 3, 2), WAITING)},

    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},  // 62
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 3, 1), Cell3DPosition(0, 3, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 1), Cell3DPosition(1, 4, 2), WAITING)},


    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(1, 0, 1), MOVING)},  // 71
    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(0, 0, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 2), Cell3DPosition(1, 2, 2), MOVING)},  
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 3, 2), Cell3DPosition(-1, 3, 3), WAITING)},

    // set front modules in place
    {LocalMovement(Cell3DPosition(0, 2, 2), Cell3DPosition(0, 2, 1), IN_POSITION)},  // 77

    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(1, 2, 1), MOVING)},  // 78
    {LocalMovement(Cell3DPosition(1, 2, 1), Cell3DPosition(1, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(1, 2, 2), Cell3DPosition(0, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, 3, 0), Cell3DPosition(1, 2, 1), IN_POSITION)},  // 81

    {LocalMovement(Cell3DPosition(2, 2, 4), Cell3DPosition(1, 2, 3), MOVING)},  // 82
    {LocalMovement(Cell3DPosition(1, 2, 3), Cell3DPosition(2, 3, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(2, 2, 2), MOVING)},
    {LocalMovement(Cell3DPosition(2, 2, 2), Cell3DPosition(1, 2, 2), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, 3, 4), Cell3DPosition(2, 3, 2), MOVING)},  // 86
    {LocalMovement(Cell3DPosition(2, 3, 2), Cell3DPosition(2, 2, 2), IN_POSITION)},

    // set back modules in place
    {LocalMovement(Cell3DPosition(1, 4, 2), Cell3DPosition(0, 3, 1), IN_POSITION)},  // 88

    {LocalMovement(Cell3DPosition(-1, 3, 3), Cell3DPosition(0, 3, 3), IN_POSITION)},  // 89

    {LocalMovement(Cell3DPosition(-1, 3, 4), Cell3DPosition(-1, 3, 3), IN_POSITION)},  // 90

    {LocalMovement(Cell3DPosition(-1, 3, 0), Cell3DPosition(-1, 3, 1), IN_POSITION)},  // 91
};

static vector<LocalMovement> LocalRules_BF_DismantleFilled_Left_Zeven{
    {LocalMovement(Cell3DPosition(-1, 0, 2), Cell3DPosition(-1, -1, 3), WAITING)}, //0

    {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(-1, -1, 2), MOVING)}, //1
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 1), Cell3DPosition(-1, 0, 0), WAITING)},

    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-2, -1, 1), WAITING)}, //4

    {LocalMovement(Cell3DPosition(1, -1, 2), Cell3DPosition(0, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 2), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)}, //9
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)}, //13
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    // Back modules
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(1, 1, 4), MOVING)}, //18
    {LocalMovement(Cell3DPosition(1, 1, 4), Cell3DPosition(0, 1, 4), MOVING)},
    {LocalMovement(Cell3DPosition(0, 1, 4), Cell3DPosition(-1, 0, 5), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 5), Cell3DPosition(-1, 0, 4), MOVING)},
    {LocalMovement(Cell3DPosition(-1, 0, 4), Cell3DPosition(-1, -1, 3), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 0, 2), MOVING)}, //28
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(2, 0, 2), Cell3DPosition(1, 0, 3), MOVING)}, //33
    {LocalMovement(Cell3DPosition(1, 0, 3), Cell3DPosition(0, 0, 3), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 3), Cell3DPosition(0, 0, 2), MOVING)},
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(1, 0, 1), Cell3DPosition(1, 0, 2), MOVING)}, //43
    {LocalMovement(Cell3DPosition(1, 0, 2), Cell3DPosition(0, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(0, -1, 1), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(0, 0, 1), Cell3DPosition(0, 0, 2), MOVING)}, //49
    {LocalMovement(Cell3DPosition(0, 0, 2), Cell3DPosition(-1, -1, 1), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 1), Cell3DPosition(-1, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-1, -1, 2), Cell3DPosition(-2, -1, 2), MOVING)},
    {LocalMovement(Cell3DPosition(-2, -1, 2), Cell3DPosition(-3, -1, 1), IN_POSITION)},

    {LocalMovement(Cell3DPosition(-1, 0, 0), Cell3DPosition(-2, 0, 0), WAITING)}, //54
   //{LocalMovement(Cell3DPosition(-3, -1, 1), Cell3DPosition(-3, -1, 1), IN_POSITION)}, //55
    {LocalMovement(Cell3DPosition(-2, -1, 1), Cell3DPosition(-3, -1, 1), IN_POSITION)}, //56
    {LocalMovement(Cell3DPosition(-2, 0, 0), Cell3DPosition(-3, -1, 1), IN_POSITION)}, //57

};

#endif