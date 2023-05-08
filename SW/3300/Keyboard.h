#pragma once

#include "stdint.h"

enum Key_State
{
    KEY_STATE_PRESSED = 1,
    KEY_STATE_RELEASED = 0
};

extern struct Keys
{
    enum Key_State Q;
    enum Key_State W;
    enum Key_State E;
    enum Key_State R;
    enum Key_State T;
    enum Key_State Y;

    enum Key_State A;
    enum Key_State S;
    enum Key_State D;
    enum Key_State F;
    enum Key_State G;
    enum Key_State H;

    enum Key_State L;

}keys;

extern uint8_t imuMotorCtr;
extern float firstRoll;
extern float firstPitch;
extern float firstYaw;

extern float firstRollMotor;
extern float firstPitchMotor;
extern float firstYawMotor;