#include "measurement.h"
#include "screen.h"

/**********************
 *  VARIABLES
 **********************/
Adafruit_ADS1115 ads;

const int sampleInterval = 10; // Interval between samples in milliseconds
const int duration = 200;    // Total averaging duration in milliseconds
const int numSamples = duration / sampleInterval;
long sum0 = 0; // Variable to store the sum of readings
long sum1 = 0; // Variable to store the sum of readings
long sum2 = 0; // Variable to store the sum of readings
long sum3 = 0; // Variable to store the sum of readings
float inputVoltage = 0;
float inputCurrent = 0;
float inputWattage = 0;
float chargeWattage = 0;
float dischargeWattage = 0;
float chargeCurrent = 0;
float dischargeCurrent = 0;
float totalCharge_Ah = 0;
float lastCharge_Ah = 0;
float lastDischarge_Ah =0;
float netChange_Ah =0;
float batteryCapacity_Ah = 100; // Define Battery Capacity
float batteryVoltage_V = 12.8; // Define Battery Capacity
float watthr = 0;
float offset_main;
float offset_charge;
float offset_discharge;
float batteryPercentage;
float batteryTimeLeft_h;

int time_s = 1;
unsigned long timeLast = 0;

float a = 0;
float b = 0;
float c = 0;
float d = 0;


/**********************
 *  INITIALIZATION FUNCTION
 **********************/
void Measurement_Init()
{
  EEPROM.begin(512);
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!ads.begin()) 
  {
    Serial.println("Failed to initialize ADS1115!");
    while (1);
  }

  EEPROM.get(0,batteryPercentage);
  //batteryPercentage = 87;
  EEPROM.get(4,offset_main);
  EEPROM.get(8,offset_charge);
  EEPROM.get(12,offset_discharge);

  ads.setGain(GAIN_TWOTHIRDS);
  reset = false;
}

/**********************
 *  MEASUREMENT FUNCTION
 **********************/
void Measure()
{
  EEPROM.get(0,a);
  EEPROM.get(4,b);
  EEPROM.get(8,c);
  EEPROM.get(12,d);
  Serial.println(a);
  Serial.println(b);
  Serial.println(c);
  Serial.println(d);
  Serial.println("");
  // Collect samples for one second
  sum0=0;
  sum1=0;
  sum2=0;
  sum3=0;
  for (int i = 0; i < numSamples; i++) 
  {
    sum0 += ads.readADC_SingleEnded(0);
    sum1 += ads.readADC_SingleEnded(1);
    sum2 += ads.readADC_SingleEnded(2);
    sum3 += ads.readADC_SingleEnded(3);
    delay(sampleInterval); // Wait between samples
    //ButtonAction();
  }

  // Calculate the average value
  int avg0 = sum0 / numSamples;
  int avg1 = sum1 / numSamples;
  int avg2 = sum2 / numSamples;
  int avg3 = sum3 / numSamples;

  if(reset==true)
  {
    //EEPROM.put(0, 0);
    //batteryPercentage = 0;
    offset_main = ads.computeVolts(avg0);
    EEPROM.put(4, offset_main);
    offset_charge = ads.computeVolts(avg2);
    EEPROM.put(8, offset_charge);
    offset_discharge = ads.computeVolts(avg1);
    EEPROM.put(12, offset_discharge);
    EEPROM.commit();
    totalCharge_Ah = 0;
    watthr = 0;
    reset=false;
    Serial.println("Current transducers and integrators zeroed.");
  }
  
  // Convert to onput voltage using calibration equation
  inputVoltage = 13.41*ads.computeVolts(avg3)-0.0222;
  inputCurrent = 15.2064*(ads.computeVolts(avg0)-offset_main)+0.024639;
  if(inputCurrent<0.2)
  {
    inputCurrent=0;
  }
  chargeCurrent = (52.4246/2)*(ads.computeVolts(avg2)-offset_charge); //50A
  //chargeCurrent = 52.4246*(ads.computeVolts(avg2)-offset_charge)+0.020605; //100A
  if(chargeCurrent<0.2)
  {
    chargeCurrent=0;
  }
  dischargeCurrent = 0.023896*(ads.computeVolts(avg1)-offset_discharge)*1000;  //50A
  Serial.println((ads.computeVolts(avg1)-offset_discharge));
  Serial.println(dischargeCurrent);
  //dischargeCurrent = 104.8804*(ads.computeVolts(avg1)-offset_discharge)-0.1;  //200A
  if(dischargeCurrent<0.2)
  {
    dischargeCurrent=0;
  }

  inputWattage = inputVoltage*inputCurrent;
  chargeWattage = batteryVoltage_V*chargeCurrent;
  dischargeWattage = batteryVoltage_V*dischargeCurrent;

  watthr = watthr + (inputWattage * (millis()-timeLast)/(1000*60*60));
  lastCharge_Ah = (chargeCurrent * (millis()-timeLast)/(1000*60*60));
  lastDischarge_Ah = (dischargeCurrent * (millis()-timeLast)/(1000*60*60));
  netChange_Ah = lastCharge_Ah-lastDischarge_Ah;
  totalCharge_Ah = totalCharge_Ah + lastCharge_Ah;
  batteryPercentage = batteryPercentage + (netChange_Ah/batteryCapacity_Ah)*100;

  if(batteryPercentage>100)
  {
    batteryPercentage = 100;
  }
  else if(batteryPercentage<0){
    batteryPercentage = 0;
  }

  if(chargeCurrent > dischargeCurrent){
    batteryTimeLeft_h = (((100-batteryPercentage)/100) * batteryCapacity_Ah)/(chargeCurrent - dischargeCurrent);
  }
  else if(chargeCurrent < dischargeCurrent){
    batteryTimeLeft_h = ((batteryPercentage/100) * batteryCapacity_Ah)/(dischargeCurrent - chargeCurrent);
  }
  else{
    batteryTimeLeft_h = INT_MAX;
  }
  
  EEPROM.put(0, batteryPercentage);
  EEPROM.commit();
  timeLast = millis();

  //Print information in serial
  Serial.print("Input Voltage Analog: "); Serial.print(ads.computeVolts(avg3),4);
  Serial.print(" Input Current Analog: "); Serial.print(ads.computeVolts(avg0),4);
  Serial.print(" Charging Current Analog: "); Serial.print(ads.computeVolts(avg2)-offset_charge,4);
  Serial.print(" Discharging Current analog: "); Serial.println(ads.computeVolts(avg1)-offset_discharge,4);
  //Serial.print(" Total Charge (Ah): "); Serial.print(totalCharge_Ah);
  //Serial.print(" Watt hr: "); Serial.print(watthr);



}