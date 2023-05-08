#include "IMU.h"
#include "gpio.h"
#include "stdint.h"
#include "main.h"
#include "spi.h"
#include <string.h>

#define CHANNEL_COMMAND  0
#define CHANNEL_EXECUTABLE  1
#define CHANNEL_CONTROL  2
#define CHANNEL_REPORTS  3
#define CHANNEL_WAKE_REPORTS  4
#define CHANNEL_GYRO  5

//All the ways we can configure or talk to the BNO080, figure 34, page 36 reference manual
//These are used for low level communication with the sensor, on channel 2
#define SHTP_REPORT_COMMAND_RESPONSE 0xF1
#define SHTP_REPORT_COMMAND_REQUEST 0xF2
#define SHTP_REPORT_FRS_READ_RESPONSE 0xF3
#define SHTP_REPORT_FRS_READ_REQUEST 0xF4
#define SHTP_REPORT_PRODUCT_ID_RESPONSE 0xF8
#define SHTP_REPORT_PRODUCT_ID_REQUEST 0xF9
#define SHTP_REPORT_BASE_TIMESTAMP 0xFB
#define SHTP_REPORT_SET_FEATURE_COMMAND 0xFD

//All the different sensors and features we can get reports from
//These are used when enabling a given sensor
#define SENSOR_REPORTID_ACCELEROMETER 0x01
#define SENSOR_REPORTID_GYROSCOPE 0x02
#define SENSOR_REPORTID_MAGNETIC_FIELD 0x03
#define SENSOR_REPORTID_LINEAR_ACCELERATION 0x04
#define SENSOR_REPORTID_ROTATION_VECTOR 0x05
#define SENSOR_REPORTID_GRAVITY 0x06
#define SENSOR_REPORTID_GAME_ROTATION_VECTOR 0x08
#define SENSOR_REPORTID_GEOMAGNETIC_ROTATION_VECTOR 0x09
#define SENSOR_REPORTID_GYRO_INTEGRATED_ROTATION_VECTOR 0x2A
#define SENSOR_REPORTID_TAP_DETECTOR 0x10
#define SENSOR_REPORTID_STEP_COUNTER 0x11
#define SENSOR_REPORTID_STABILITY_CLASSIFIER 0x13
#define SENSOR_REPORTID_RAW_ACCELEROMETER 0x14
#define SENSOR_REPORTID_RAW_GYROSCOPE 0x15
#define SENSOR_REPORTID_RAW_MAGNETOMETER 0x16
#define SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER 0x1E
#define SENSOR_REPORTID_AR_VR_STABILIZED_ROTATION_VECTOR 0x28
#define SENSOR_REPORTID_AR_VR_STABILIZED_GAME_ROTATION_VECTOR 0x29

//Record IDs from figure 29, page 29 reference manual
//These are used to read the metadata for each sensor type
#define FRS_RECORDID_ACCELEROMETER 0xE302
#define FRS_RECORDID_GYROSCOPE_CALIBRATED 0xE306
#define FRS_RECORDID_MAGNETIC_FIELD_CALIBRATED 0xE309
#define FRS_RECORDID_ROTATION_VECTOR 0xE30B

//Command IDs from section 6.4, page 42
//These are used to calibrate, initialize, set orientation, tare etc the sensor
#define COMMAND_ERRORS 1
#define COMMAND_COUNTER 2
#define COMMAND_TARE 3
#define COMMAND_INITIALIZE 4
#define COMMAND_DCD 6
#define COMMAND_ME_CALIBRATE 7
#define COMMAND_DCD_PERIOD_SAVE 9
#define COMMAND_OSCILLATOR 10
#define COMMAND_CLEAR_DCD 11

#define CALIBRATE_ACCEL 0
#define CALIBRATE_GYRO 1
#define CALIBRATE_MAG 2
#define CALIBRATE_PLANAR_ACCEL 3
#define CALIBRATE_ACCEL_GYRO_MAG 4
#define CALIBRATE_STOP 5

#define MAX_PACKET_SIZE 128 //Packets can be up to 32k but we don't have that much RAM.
#define MAX_METADATA_SIZE 9 //This is in words. There can be many but we mostly only care about the first 9 (Qs, range, etc)

struct BNO080Error {
	uint8_t severity;
	uint8_t error_sequence_number;
	uint8_t error_source;
	uint8_t error;
	uint8_t error_module;
	uint8_t error_code;
};


uint32_t stateCntr = 0;
__attribute__((section(".D1"))) uint8_t shtpRxRawData[280] = {0};
uint8_t shtpTxRawData[280] = {};
uint8_t sequenceNumbers[6] = {0, 0, 0, 0, 0, 0};






void IMU_sendPacket(uint8_t channelNumber, uint8_t dataLen)
{
    uint8_t packetLen = dataLen + 4;

    shtpTxRawData[0] = packetLen & 0xFF;
    shtpTxRawData[1] = packetLen >> 8;
    shtpTxRawData[2] = channelNumber;
    shtpTxRawData[3] = sequenceNumbers[channelNumber]++;

    HAL_SPI_Transmit(&hspi2, shtpTxRawData, packetLen, 10);

}

void IMU_receivePacket(uint16_t dataLen)
{

    memset(shtpRxRawData, 0, dataLen);
    HAL_SPI_Receive(&hspi2, shtpRxRawData, dataLen, 10);

}

void IMU_setFeatureCommand(uint8_t reportID, uint16_t timeBetweenReports, uint32_t specificConfig)
{
	long microsBetweenReports = (long)timeBetweenReports * 1000;

	shtpTxRawData[0 + 4] = SHTP_REPORT_SET_FEATURE_COMMAND;	 //Set feature command. Reference page 55
	shtpTxRawData[1 + 4] = reportID;							   //Feature Report ID. 0x01 = Accelerometer, 0x05 = Rotation vector
	shtpTxRawData[2 + 4] = 0;								   //Feature flags
	shtpTxRawData[3 + 4] = 0;								   //Change sensitivity (LSB)
	shtpTxRawData[4 + 4] = 0;								   //Change sensitivity (MSB)
	shtpTxRawData[5 + 4] = (microsBetweenReports >> 0) & 0xFF;  //Report interval (LSB) in microseconds. 0x7A120 = 500ms
	shtpTxRawData[6 + 4] = (microsBetweenReports >> 8) & 0xFF;  //Report interval
	shtpTxRawData[7 + 4] = (microsBetweenReports >> 16) & 0xFF; //Report interval
	shtpTxRawData[8 + 4] = (microsBetweenReports >> 24) & 0xFF; //Report interval (MSB)
	shtpTxRawData[9 + 4] = 0;								   //Batch Interval (LSB)
	shtpTxRawData[10 + 4] = 0;								   //Batch Interval
	shtpTxRawData[11 + 4] = 0;								   //Batch Interval
	shtpTxRawData[12 + 4] = 0;								   //Batch Interval (MSB)
	shtpTxRawData[13 + 4] = (specificConfig >> 0) & 0xFF;	   //Sensor-specific config (LSB)
	shtpTxRawData[14 + 4] = (specificConfig >> 8) & 0xFF;	   //Sensor-specific config
	shtpTxRawData[15 + 4] = (specificConfig >> 16) & 0xFF;	  //Sensor-specific config
	shtpTxRawData[16 + 4] = (specificConfig >> 24) & 0xFF;	  //Sensor-specific config (MSB)

	//Transmit packet on channel 2, 17 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 17);
}

void IMU_enableGyroIntegratedRotationVector(uint16_t timeBetweenReports)
{
	IMU_setFeatureCommand(SENSOR_REPORTID_GYRO_INTEGRATED_ROTATION_VECTOR, timeBetweenReports, 0);
}

uint16_t rawQuatI = 0;
uint16_t rawQuatJ = 0;
uint16_t rawQuatK = 0;
uint16_t rawQuatReal = 0;

float dqw = 0.0f;
float dqx = 0.0f;
float dqy = 0.0f;
float dqz = 0.0f;

float roll = 0.0f;
float pitch = 0.0f;
float yaw = 0.0f;

float IMU_qToFloat(int16_t fixedPointValue, uint8_t qPoint)
{
	float qFloat = fixedPointValue;
	qFloat *= pow(2, qPoint * -1);
	return (qFloat);
}

void IMU_decode()
{


    rawQuatI = (uint16_t)shtpRxRawData[1 + 4] << 8 | shtpRxRawData[0 + 4];
    rawQuatJ = (uint16_t)shtpRxRawData[3 + 4] << 8 | shtpRxRawData[2 + 4];
    rawQuatK = (uint16_t)shtpRxRawData[5 + 4] << 8 | shtpRxRawData[4 + 4];
    rawQuatReal = (uint16_t)shtpRxRawData[7 + 4] << 8 | shtpRxRawData[6 + 4];

    // rawQuatI = (uint16_t)shtpRxRawData[5 + 5 + 4] << 8 | shtpRxRawData[5 + 4 + 4];
    // rawQuatJ = (uint16_t)shtpRxRawData[5 + 7 + 4] << 8 | shtpRxRawData[5 + 6 + 4];
    // rawQuatK = (uint16_t)shtpRxRawData[5 + 9 + 4] << 8 | shtpRxRawData[5 + 8 + 4];
    // rawQuatReal = (uint16_t)shtpRxRawData[5 + 11 + 4] << 8 | shtpRxRawData[5 + 10 + 4];

    dqw = IMU_qToFloat(rawQuatReal, 14);
    dqx = IMU_qToFloat(rawQuatI, 14);
    dqy = IMU_qToFloat(rawQuatJ, 14);
    dqz = IMU_qToFloat(rawQuatK, 14);

    float norm = sqrt(dqw*dqw + dqx*dqx + dqy*dqy + dqz*dqz);
	dqw = dqw/norm;
	dqx = dqx/norm;
	dqy = dqy/norm;
	dqz = dqz/norm;

    float t2 = +2.0 * (dqw * dqy - dqz * dqx);
	t2 = t2 > 1.0 ? 1.0 : t2;
	t2 = t2 < -1.0 ? -1.0 : t2;
	// pitch = asin(t2) * 180.0f / M_PI;
    pitch = asin(t2);

    float ysqr = dqy * dqy;
	// yaw (z-axis rotation)
	float t3 = +2.0 * (dqw * dqz + dqx * dqy);
	float t4 = +1.0 - 2.0 * (ysqr + dqz * dqz);
	// yaw = atan2(t3, t4) * 180.0f / M_PI;
    yaw = atan2(t3, t4);

	// roll (x-axis rotation)
	float t0 = +2.0 * (dqw * dqx + dqy * dqz);
	float t1 = +1.0 - 2.0 * (dqx * dqx + ysqr);
	// roll = atan2(t0, t1) * 180.0f / M_PI;
    roll = atan2(t0, t1);
}

void IMU_dataReady()
{
    switch (stateCntr)
    {
    case 0:
    case 1:
    case 3:
        IMU_receivePacket(280);
        break;
    case 4:
        IMU_enableGyroIntegratedRotationVector(1);
        break;
    case 2:
        shtpTxRawData[4] = SHTP_REPORT_PRODUCT_ID_REQUEST;
        shtpTxRawData[5] = 0;
        IMU_sendPacket(CHANNEL_CONTROL, 2);
        break;
    default:
        if(stateCntr > 10)
        {
            memset(shtpRxRawData, 0, 25);
            HAL_SPI_Receive_DMA(&hspi2, shtpRxRawData, 25);
            // IMU_decode();
        }
        else
        {
            IMU_receivePacket(280);
        }
        break;
    }
    stateCntr++;
}



static void rxCallback()
{
    IMU_decode();
}


void IMU_init()
{
    // Low RST
    HAL_GPIO_WritePin(IMU_RST_GPIO_Port, IMU_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(IMU_RST_GPIO_Port, IMU_RST_Pin, GPIO_PIN_SET);

    HAL_SPI_RegisterCallback(&hspi2, HAL_SPI_RX_COMPLETE_CB_ID, rxCallback);
}



float IMU_getYaw() { return yaw; }
float IMU_getPitch() { return pitch; }
float IMU_getRoll() { return roll; }