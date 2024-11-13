
//  Created by wm6h
//  twitter: @wm6h
//  rev: 20181031
/*
 *  *
 * Program Start...WiFi began
 MAC: B4:E6:2D:8B:89:35


 
 ESPNOW - Basic communication - Master
 Date: 26th September 2017
 Author: Arvind Ravulavaru <https://github.com/arvindr21>
 Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
 Description: This sketch consists of the code for the Master module.
 Resources: (A bit outdated)
 a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
 b. http://www.esploradores.com/practica-6-conexion-esp-now/
 
 << This Device Master >>
 
 Flow: Master
 Step 1 : ESPNow Init on Master and set it in STA mode
 Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
 Step 3 : Once found, add Slave as peer
 Step 4 : Register for send callback
 Step 5 : Start Transmitting data from Master to Slave
 
 Flow: Slave
 Step 1 : ESPNow Init on Slave
 Step 2 : Update the SSID of Slave with a prefix of `slave`
 Step 3 : Set Slave in AP mode
 Step 4 : Register for receive callback and wait for data
 Step 5 : Once data arrives, print it in the serial monitor
 
 Note: Master and Slave have been defined to easily understand the setup.
 Based on the ESPNOW API, there is no concept of Master and Slave.
 Any devices can act as master or slave.
 
 */

/*
 Antenna Switch Controller
 Two core ESP-32 
 Tested on stock ESP32 M5Stack.com modular unit
 Rev. 1 silicon

 Compatible with Arduino IDE
 __________________________________________
 GLOBAL INFO
 core 0 is the communications processor.  
        Uses the ESP-NOW protocol
        on the 2.4 GHz radio
        This process runs as a task created
        on core 1
 core 1 is the UI processor.
        Maintains the M5Stack LCD menu
        Runs Arduino setup() and loop()
 ___________________________________________       
 */

#include <Arduino.h>

#include <M5Stack.h>

#include <WiFi.h>
#include <esp_now.h>

#include "EEPROM.h"
#include <M5ez.h>

ezMenu main("Select Antenna");

int LED_BUILTIN = 5;

bool LedState = 0;

int   frameCNT = 0;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#include "GlobalDefinitions.h"
#include <ArduinoJson.h>


char antenna_name[30] = "g5rv";


int   remoteAntenna;
int   currentAntenna;
int   menuItemSelected = 0;


// the remote master (this code) is initialized from
// the slave processor at the antenna switch
// The slave processor, at power-up, will read
// its NV memory and restore the last antenna.
// This processor (master) must find the
// current state of the slave and initialize
// the menu to sync with the slave
// remoteInitialized is written by core 1
// and read by core0 so no access control is
// required
bool remoteInitialized = false;
bool menuInitialized = false;
int  initialAntenna = -1;

extern void initializeMenu(int);
extern void waitingMenu(void);

// test
long randNumber;


// This remote is an antenna switch
//uint8_t remoteMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t remoteMac[] = {0x30, 0xAE, 0xA4, 0x16, 0x1E, 0x61};

// This remote is a rig switch
uint8_t remoteMac1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define CHANNEL 3
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

#define EEPROM_SIZE 64

#include "Utilities.h"

TaskHandle_t TaskCore0;
SemaphoreHandle_t processUI;

esp_err_t result;


hw_timer_t* timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerTelemetry = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

void deletePeer(void);

void IRAM_ATTR onTimer()
{
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerTelemetry);
  isrCounter++;
  lastIsrAt = millis();
  portEXIT_CRITICAL_ISR(&timerTelemetry);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
  //Serial.print("onTimer no. ");
  //Serial.print(isrCounter);
  // "Arf! Arf!"
  // pet the watchdog.
  vTaskDelay(10);

}


// standard Arduino setup() and loop() run on Core: 1
void setup()
{
    
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(115200);

    // Create semaphore to inform us when the timer has fired
    timerSemaphore = xSemaphoreCreateBinary();
    
    processUI = xSemaphoreCreateMutex();
    
    //Setup: Executing on core 1
    Serial.print("Setup: Executing on core ");
    Serial.println(xPortGetCoreID());
    
    // CPU assignment happens when tasks are created. For example,
    // when Arduino core creates task which runs your "setup" and "loop" functions,
    // it pins this task to CPU1: https://github.com/espressif/arduino-es ... in.cpp#L24
    // this code is running on core 1
    // core 1 is the UI processor
    // start a thread on core 0 for
    // communications processing
    xTaskCreatePinnedToCore(
                            commProcessing,
                            "commProcessing",
                            1024*8,
                            NULL,
                            1,
                            &TaskCore0,
                            0);
    
    delay(500);  // needed to start-up TaskCore0


    // Use 1st timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
    // info).

    /*
    timer = timerBegin(0, 80, true);
    // Attach onTimer function to our timer.
    timerAttachInterrupt(timer, &onTimer, true);  
    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer, 1000000*1, true);
    */
      
    // Initialize the M5Stack object
    M5.begin();

    
    M5.lcd.setBrightness(195);
    Wire.begin();
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("Failed to initialize EEPROM.");
    }else
    {
        M5.lcd.setBrightness(byte(EEPROM.read(0)));
    }

    menuInitialized = false;

    // Start an alarm
    //////////////timerAlarmEnable(timer);
    
    ez.begin();
    ez.screen.clear(TFT_GREEN);
    
}

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// Core1Core1Core1Core1Core1Core1Core1Core1Core1Core1Core1Core1
//                   This Task runs on Core: 1
// Core1Core1Core1Core1Core1Core1Core1Core1Core1Core1Core1Core1
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

// ||||||||||||||||||
//  UI Processing
// ||||||||||||||||||
void loop()
{
    // processing is done from menu

    // the ez menu is blocking and
    // all processing (except for the timer) is done from there
    
    if( (remoteInitialized == true) && (menuInitialized == false))
    {
      
      if( (initialAntenna >= 0) && (initialAntenna < 8))
      {
        menuInitialized == true;
        initializeMenu(initialAntenna);
      }
     
    } 

/*
 // If Timer has fired
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
  {
    uint32_t isrCount = 0, isrTime = 0;
    // Read the interrupt count and time
    portENTER_CRITICAL(&timerTelemetry);
    isrCount = isrCounter;
    isrTime = lastIsrAt;
    portEXIT_CRITICAL(&timerTelemetry);
    // Print it
    Serial.print("onTimer no. ");
    Serial.print(isrCount);
    Serial.print(" at ");
    Serial.print(isrTime);
    Serial.println(" ms");
  }
*/ 

/* stopping the timer
// If timer is still running
    if (timer) {
      // Stop and free timer
      timerEnd(timer);
      timer = NULL;
    }
*/
    
} // end UI processing task on core 1

// /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
// 1111111111111111111111111111111111111111111111111111111111111
// /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
