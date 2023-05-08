#pragma once

void IMU_init();
void IMU_dataReady();

float IMU_getYaw();
float IMU_getPitch();
float IMU_getRoll();

// extern float roll;
// extern float pitch;
// extern float yaw;