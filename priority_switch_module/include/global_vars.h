#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H


extern bool outlet1_status;
extern bool outlet2_status;
extern bool outlet3_status;
extern bool outlet4_status;

extern int relay1_mode;
extern int relay2_mode;
extern int relay3_mode;
extern int relay4_mode;


//Wireless Messages
extern int messageTime;
extern int timeSinceLastMessage;

extern float inputVoltage;
extern float inputCurrent;
extern float inputWattage;

extern float chargeWattage;
extern float dischargeWattage;

extern float batteryPercentage;
extern float netCharge_Ah;
extern float batteryTimeLeft_h;

extern int batteryState;

#endif