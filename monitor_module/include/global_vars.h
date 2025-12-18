#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

extern float inputVoltage; // Declare the variable as extern
extern float inputCurrent; // Declare the variable as extern
extern float inputWattage; // Declare the variable as extern

extern float chargeCurrent; // Declare the variable as extern
extern float chargeWattage; // Declare the variable as extern

extern float dischargeCurrent; // Declare the variable as extern
extern float dischargeWattage; // Declare the variable as extern

extern float watthr;
extern float totalCharge_Ah; // Declare the variable as extern
extern float batteryPercentage; // Declare the variable as extern
extern float batteryTimeLeft_h;

extern float netChange_Ah; // Declare the variable as extern

extern bool reset;

//Wireless Messages
extern int messageTime;
extern int timeSinceLastMessage;

#endif