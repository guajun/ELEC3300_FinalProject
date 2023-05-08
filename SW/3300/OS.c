#include "OS.h"

#include "tim.h"

#include "stdint.h"

#include "DM4310.h"
#include "HT4310.h"
#include "GO_M8010_6.h"
#include "SpaceMouse.h"
#include "RotaryEncoder.h"
#include "IMU.h"
#include "Keyboard.h"


#define DM4310_POS_STEP 0.0004
#define HT4310_POS_STEP 10
#define GO_POS_STEP 0.0001

float OS_tickFreq = 0;

static void keyboardMotor()
{
    static float k;
    uint32_t encoder = RotaryEncoder_read();

    uint32_t num = encoder >> 2;

    if((int)encoder < 0)
    {
        num = 0;
    }
    else if(num > 15)
    {
        num = 15;
    }

    k = 1 + num * 0.25;
    // k = (float)(((int)RotaryEncoder_read()) > 0 ? RotaryEncoder_read() : 0) / 

    if(keys.Q == KEY_STATE_PRESSED)
    {
        DM4310_insts[0].control.position += DM4310_POS_STEP * k;
    }
    else if(keys.A == KEY_STATE_PRESSED)
    {
        DM4310_insts[0].control.position -= DM4310_POS_STEP * k;
    }

    if(keys.W == KEY_STATE_PRESSED)
    {
        DM4310_insts[1].control.position += DM4310_POS_STEP * k;
    }
    else if(keys.S == KEY_STATE_PRESSED)
    {
        DM4310_insts[1].control.position -= DM4310_POS_STEP * k;
    }

    if(keys.E == KEY_STATE_PRESSED)
    {
        HT4310_insts[0].control.position += HT4310_POS_STEP * k;
    }
    else if(keys.D == KEY_STATE_PRESSED)
    {
        HT4310_insts[0].control.position -= HT4310_POS_STEP * k;
    }

    if(keys.R == KEY_STATE_PRESSED)
    {
        HT4310_insts[1].control.position += HT4310_POS_STEP * k;
    }
    else if(keys.F == KEY_STATE_PRESSED)
    {
        HT4310_insts[1].control.position -= HT4310_POS_STEP * k;
    }

    if(keys.T == KEY_STATE_PRESSED)
    {
        GO_M8010_6_insts[0].tarPos += GO_POS_STEP * k;
    }
    else if(keys.G == KEY_STATE_PRESSED)
    {
        GO_M8010_6_insts[0].tarPos -= GO_POS_STEP * k;
    }

    if(keys.Y == KEY_STATE_PRESSED)
    {
        GO_M8010_6_insts[1].tarPos += GO_POS_STEP * k;
    }
    else if(keys.H == KEY_STATE_PRESSED)
    {
        GO_M8010_6_insts[1].tarPos -= GO_POS_STEP * k;
    }
}

static void imuMotor()
{

    static float lastYaw = 0.0f;
    static float lastPitch = 0.0f;
    static float lastRoll = 0.0f;

    // yaw 
    // pitch 
    // roll 



}

static void thread(TIM_HandleTypeDef *htim)
{
    static uint32_t lastTick = 0;
    OS_tickFreq = (HAL_GetTick() - lastTick) / 0.001;
    lastTick = HAL_GetTick();
    
    // static enum Key_State lastLKeyState = KEY_STATE_RELEASED;

    // if(lastLKeyState != keys.L && key)

    if(imuMotorCtr)
    {
        imuMotor();
    }
    else
    {
        keyboardMotor();
    }
    

    DM4310_update();
    HT4310_update();
    GO_M8010_6_update();

}

void OS_init()
{
    HAL_TIM_RegisterCallback(&htim6, HAL_TIM_PERIOD_ELAPSED_CB_ID, thread);
    HAL_TIM_Base_Start_IT(&htim6);
}