#include <Arduino.h>
#include "screen.h"
#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "wireless.h"

int time_s=0;

const int relay1pin =19;
const int relay2pin =21;
const int relay3pin =3;
const int relay4pin =1;

const int redBtnPin =16;
const int yellowBtnPin =17;
const int greenBtnPin =13;
const int blueBtnPin =26;

const int LEDPin =2;

// TRUE -> On
// FALSE -> Off
bool outlet1_status = 1;
bool outlet2_status = 1;
bool outlet3_status = 1;
bool outlet4_status = 1;

// TRUE -> On
// FALSE -> Off
bool outlet1_request = 1;
bool outlet2_request = 1;
bool outlet3_request = 1;
bool outlet4_request = 1;

// 0 -> Constant Off
// 1 -> Constant On
// 2 -> Auto Mode
int relay1_mode = 2;
int relay2_mode = 2;
int relay3_mode = 2;
int relay4_mode = 2;

// 0 -> Critical (<25%)
// 1 -> Alert 2 (25 - 50%)
// 2 -> Alert 1 (50 - 75%)
// 3 -> Normal Operation (75-100%)
int batteryState = 3;
//Note: High to Low Priority (outlet1, outlet2, outlet3, outlet4)

//Hysterisis Threshold
int threshold = 3;


// time of day state
// 0 -> daytime
// 1 -> transition
// 2 -> nightime
int timeState = 0;


void setup() {
  Serial.begin(115200);
  LCD_Init();
  Lvgl_Init();
  Serial.println("Setup Done 0 ");
  Wireless_Init();
  Serial.println("Setup Done 1 ");
  startScreen();
  Serial.println("Setup Done 2 ");

  pinMode(relay1pin, OUTPUT);
  pinMode(relay2pin, OUTPUT);
  pinMode(relay3pin, OUTPUT);
  pinMode(relay4pin, OUTPUT);

  pinMode(redBtnPin, INPUT_PULLDOWN);
  pinMode(yellowBtnPin, INPUT_PULLDOWN);
  pinMode(greenBtnPin, INPUT_PULLDOWN);
  pinMode(blueBtnPin, INPUT_PULLDOWN);
}

int handle_button_press(uint8_t pin, int current_status) {
    static uint8_t last_states[40] = {0}; // stores last state for debounce per pin

    uint8_t current_state = digitalRead(pin);
    
    if (current_state == HIGH && last_states[pin] == LOW) {
        // Button just pressed: cycle to next mode
        current_status = (current_status + 1) % 3;
    }

    // Update last state
    last_states[pin] = current_state;

    return current_status;
}

void handle_outlet_request(bool request, bool status, uint8_t relayPin) {
    if (request != status) {
        digitalWrite(relayPin, request ? LOW : HIGH);
    }

}

bool get_outlet_request_from_mode(int mode, bool current_request) {
    if (mode == 0) return false;
    if (mode == 1) return true;
    return current_request;  // Mode 2 = auto = don't override
}

void battery_state_update() {

    Serial.print(batteryState);

    int currentState = batteryState;

    if (batteryPercentage >= 75) {
        if (currentState == 3) {
            batteryState = 3;
        }
        else {
            if (batteryPercentage > (75 + threshold)) {
                batteryState = 3;
            }
        }
    }

    else if ((batteryPercentage >= 50) && (batteryPercentage < 75)) {
        if (currentState == 3 && (batteryPercentage < (75 - threshold))) {
            batteryState = 2;
            return;
        }
        if (currentState == 2) {
            return;
        }
        if (currentState == 1) {
            if (batteryPercentage > (50 + threshold)) {
                batteryState = 2;
                return;
            }
        }
    }

    else if ((batteryPercentage >= 25) && (batteryPercentage < 50)) {
        if (currentState == 2) {
            batteryState = 1;
            return;
        }
        if (currentState == 1) {
            return;
        }
        if (currentState == 0) {
            if (batteryPercentage > (25 + threshold)) {
                batteryState = 1;
                return;
            }
        }
    }

    else if (batteryPercentage < 25) {
        batteryState = 0;
    }
}

void outletAutoControl(){

//Daytime Logic Control    
if (timeState == 0) {

    if (netCharge_Ah >= 0) {

        if (batteryState == 0) {
            int shutoffCount = 0;
        
            if(relay4_mode==2){
                outlet4_request=false;
                shutoffCount++;
            }
            if(relay3_mode==2){
                outlet3_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay2_mode==2){
                outlet2_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay1_mode==2){
                outlet1_request=false;
            }         
        } 
        if (batteryState == 1) {
            int shutoffCount = 0;
        
            if(relay4_mode==2){
                outlet4_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay3_mode==2){
                outlet3_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay2_mode==2){
                outlet2_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay1_mode==2){
                outlet1_request=false;
            } 
        }
        if (batteryState == 2) {
            if(relay4_mode==2){
                outlet4_request=true;
            }
            if(relay3_mode==2){
                outlet3_request=true;
            }
            if(relay2_mode==2){
                outlet2_request=true;
            }
            if(relay1_mode==2){
                outlet1_request=true;
            }            
        } 
        if (batteryState == 3) {
            if(relay4_mode==2){
                outlet4_request=true;
            }
            if(relay3_mode==2){
                outlet3_request=true;
            }
            if(relay2_mode==2){
                outlet2_request=true;
            }
            if(relay1_mode==2){
                outlet1_request=true;
            }            
        } 
    } else { //If netCharge to the battery is NEGATIVE
        if (batteryState == 0) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }            
        } 
        if (batteryState == 1) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=false;
                shutoffCount++;
            }
            if(relay3_mode==2){
                outlet3_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay2_mode==2){
                outlet2_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay1_mode==2){
                outlet1_request=false;
            } 
        }
        if (batteryState == 2) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay3_mode==2){
                outlet3_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay2_mode==2){
                outlet2_request=true;
            }
            if(shutoffCount < 1 && relay1_mode==2){
                outlet1_request=true;
            }            
        } 
        if (batteryState == 3) {
            if(relay4_mode==2){
                outlet4_request=true;
            }
            if(relay3_mode==2){
                outlet3_request=true;
            }
            if(relay2_mode==2){
                outlet2_request=true;
            }
            if(relay1_mode==2){
                outlet1_request=true;
            }            
        } 
    }
}

//Transition Period Logic Control    
if (timeState == 1) {

    if (netCharge_Ah >= 0) {

        if (batteryState == 0) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }         
        } 
        if (batteryState == 1) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }         
        } 
        if (batteryState == 2) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
                shutoffCount++;
            }
            if(relay3_mode==2){
                outlet3_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay2_mode==2){
                outlet2_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay1_mode==2){
                outlet1_request=true;
                shutoffCount++;
            }            
        } 
        if (batteryState == 3) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
            }
            if(relay3_mode==2){
                outlet3_request=true;
            }
            if(relay2_mode==2){
                outlet2_request=true;
            }
            if(relay1_mode==2){
                outlet1_request=true;
            }            
        } 
    } else { //If netCharge to the battery is NEGATIVE
        if (batteryState == 0) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }            
        } 
        if (batteryState == 1) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }            
        } 
        if (batteryState == 2) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
                shutoffCount++;
            }
            if(relay3_mode==2){
                outlet3_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay2_mode==2){
                outlet2_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay1_mode==2){
                outlet1_request=true;
                shutoffCount++;
            }            
        } 
        if (batteryState == 3) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay3_mode==2){
                outlet3_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay2_mode==2){
                outlet2_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay1_mode==2){
                outlet1_request=true;
                shutoffCount++;
            }            
        } 
    }
}


//Nighttime Logic Control    
if (timeState == 2) {

    if (netCharge_Ah >= 0) {

        if (batteryState == 0) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }         
        } 
        if (batteryState == 1) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=false;
                shutoffCount++;
            }
            if(relay3_mode==2){
                outlet3_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay2_mode==2){
                outlet2_request=false;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay1_mode==2){
                outlet1_request=false;
            }         
        } 
        if (batteryState == 2) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay3_mode==2){
                outlet3_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay2_mode==2){
                outlet2_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay1_mode==2){
                outlet1_request=true;
                shutoffCount++;
            }            
        } 
        if (batteryState == 3) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
            }
            if(relay3_mode==2){
                outlet3_request=true;
            }
            if(relay2_mode==2){
                outlet2_request=true;
            }
            if(relay1_mode==2){
                outlet1_request=true;
            }            
        } 
    } else { //If netCharge to the battery is NEGATIVE
        if (batteryState == 0) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }            
        } 
        if (batteryState == 1) {
            if(relay4_mode==2){
                outlet4_request=false;
            }
            if(relay3_mode==2){
                outlet3_request=false;
            }
            if(relay2_mode==2){
                outlet2_request=false;
            }
            if(relay1_mode==2){
                outlet1_request=false;
            }            
        } 
        if (batteryState == 2) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
                shutoffCount++;
            }
            if(relay3_mode==2){
                outlet3_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay2_mode==2){
                outlet2_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 2 && relay1_mode==2){
                outlet1_request=true;
                shutoffCount++;
            }            
        } 
        if (batteryState == 3) {
            int shutoffCount = 0;
            if(relay4_mode==2){
                outlet4_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay3_mode==2){
                outlet3_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay2_mode==2){
                outlet2_request=true;
                shutoffCount++;
            }
            if(shutoffCount < 1 && relay1_mode==2){
                outlet1_request=true;
                shutoffCount++;
            }            
        } 
    }
}


//end of auto control method
}

void loop() {

    Serial.println(timeState);
    Serial.println(batteryState);
    
  //Battery State Update
  battery_state_update();
  

  //Relay Status Update
  outlet1_status = !digitalRead(relay1pin);
  outlet2_status = !digitalRead(relay2pin);
  outlet3_status = !digitalRead(relay3pin);
  outlet4_status = !digitalRead(relay4pin);

  //Relay Mode Update
  relay1_mode = handle_button_press(redBtnPin, relay1_mode);
  relay2_mode = handle_button_press(greenBtnPin, relay2_mode);
  relay3_mode = handle_button_press(yellowBtnPin, relay3_mode);
  relay4_mode = handle_button_press(blueBtnPin, relay4_mode);

  //Outlet Status Change Request
  handle_outlet_request(outlet1_request, outlet1_status, relay1pin);
  handle_outlet_request(outlet2_request, outlet2_status, relay2pin);
  handle_outlet_request(outlet3_request, outlet3_status, relay3pin);
  handle_outlet_request(outlet4_request, outlet4_status, relay4pin);

  outlet1_request = get_outlet_request_from_mode(relay1_mode, outlet1_request);
  outlet2_request = get_outlet_request_from_mode(relay2_mode, outlet2_request);
  outlet3_request = get_outlet_request_from_mode(relay3_mode, outlet3_request);
  outlet4_request = get_outlet_request_from_mode(relay4_mode, outlet4_request);

  outletAutoControl();

  Timer_Loop();

  delay(10);
}