/**
 * This code goes on the ESP32 at the Antenna Switch
 *  
 ESPNOW - Basic communication - Slave
 Date: 26th September 2017
 Author: Arvind Ravulavaru <https://github.com/arvindr21>
 Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
 Description: This sketch consists of the code for the Slave module.
 Resources: (A bit outdated)
 a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
 b. http://www.esploradores.com/practica-6-conexion-esp-now/
 
 << This Device Slave >>
 
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
 Any devices can act as master or salve.
 */
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#include "esp_system.h"
#include "soc/rtc.h"


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"


#include "GlobalDefinitions.h"
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO23
#define ONE_WIRE_BUS 23
// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//28 C1 4B 26 30 14 1 1
DeviceAddress sensor1 = { 0x28, 0xC1, 0x4B, 0x26, 0x30, 0x14, 0x01, 0x01 };
//DeviceAddress sensor2 = { 0x28, 0xFF, 0xB4, 0x6, 0x33, 0x17, 0x3, 0x4B };
//DeviceAddress sensor3= { 0x28, 0xFF, 0xA0, 0x11, 0x33, 0x17, 0x3, 0x96 };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct commandStruct
{
    byte macAdr[6];
    int  antenna;
    char name[30]; // not used until the remote needs 
    // to know the antenna name
    // in addition to the number
};

typedef struct telemetryStruct
{
    int   remoteAntenna;
    float temperature; // copy of global structs for local use on core 1

};

commandStruct   command;
telemetryStruct telemetry;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// As part of setup, the Master controller (the remote control)
// sets its MAC address to this.
// mac : 01:02:03:04:05:05
// It will be used to send the battery voltage telemetry
// and current antenna back to the remote contol from the switch.
// Note: this is a soft MAC address. It will revert back
// to its original address with new code is run.
// The utility sketch ESP32_WIFI can be run to display
// the true hard Mac address of the device set at the
// factory.
uint8_t masterDeviceMac[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x05}; // Remote Control
//uint8_t masterDeviceMac[] = {0xB4, 0xE6, 0x2D, 0x8B, 0x89, 0x35}; // Remote Control

// For its part, the Master controller will get the slave address by scanning and
// using the mac of the first device which self-ids as "slave"
// If more than one slave is present (say a switch for antennas and radios, 
// a more elaborate technique must be used.



extern "C"{
#include "Utilities.h"
};

#define CHANNEL 1
#define WIFI_CHANNEL CHANNEL


esp_err_t err;
nvs_handle my_handle;


esp_now_peer_info_t master;
const esp_now_peer_info_t *masterNode = &master;
const byte maxDataFrameSize = 200;
uint8_t dataToSend[maxDataFrameSize];
byte cnt = 0;
esp_err_t sendResult;

bool BUTTON_TOGGLE_STATE_LAST = HIGH;

int32_t lastAntenna = 0; // value will default to 0, if not set yet in NVS
int32_t nextAntenna = 0; 


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int saveValue = 0;

// Init ESP Now with fallback
void InitESPNow() {
    if (esp_now_init() == ESP_OK) {
        Serial.println("ESPNow Init Success");
    }
    else {
        Serial.println("ESPNow Init Failed");
        // Retry InitESPNow, add a counte and then restart?
        // InitESPNow();
        // or Simply Restart
        ESP.restart();
    }
}

// config AP SSID
// wireless access point (WAP)
void configDeviceAP() {
    char* SSID = "Slave_1";
    bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
    if (!result) {
        Serial.println("AP Config failed.");
    } else {
        Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    }
}

void setup() 
{
    // slow down and lower power
    rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
    
    Serial.begin(115200);
    Serial.println("ESPNow/Basic/Slave Example");
    
    pinMode(SEL_A, OUTPUT);
    pinMode(SEL_B, OUTPUT);
    pinMode(SEL_C, OUTPUT);
    pinMode(EnableOutput, OUTPUT);
    //disable output
    digitalWrite(EnableOutput, HIGH);
    
    nonVolatileStorageInit();
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    telemetry.temperature = 12.6;
    // this is an out of range value  
    telemetry.remoteAntenna = 8; 
    /*
     printf("set mac : 01:02:03:04:05:06\n");
     uint8_t new_mac[8] = {0x01,0x02,0x03,0x04,0x05,0x06};
     esp_base_mac_addr_set(new_mac);
     */
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
    //Set device in AP mode to begin with
    WiFi.mode(WIFI_AP);
    // configure device AP mode
    configDeviceAP();
    // This is the mac address of the Slave in AP Mode
    Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
    // Init ESPNow with a fallback logic
    InitESPNow();
    
    //Add the master node to this slave node
    memcpy( &master.peer_addr, &masterDeviceMac, 6 );
    master.channel = WIFI_CHANNEL;
    master.encrypt = 0;
    master.ifidx = ESP_IF_WIFI_AP;
    //Add the remote master node to this slave node
    if ( esp_now_add_peer(masterNode) == ESP_OK)
    {
        Serial.println("Added Master Node!");
    }
    else
    {
        Serial.println("Master Node could not be added...");
    }
    
    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info.

    sensors.begin();
    
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);
    
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Telemetry Delivery Success" : "Delivery Fail");
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) 
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Recv from: "); Serial.println(macStr);
    Serial.print("Last Packet Recv Data: "); Serial.println(*data);
    Serial.println("");
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
    uint8_t commandJSON[COMMANDLENGTH];
    char jsonChar[COMMANDLENGTH];
    
    Serial.printf("\r\nReceived\t%d Bytes\t%d", data_len, data[0]);
    StaticJsonBuffer<COMMANDLENGTH> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(data);
    
    if (!root.success()) 
    {
        Serial.println("parseObject() failed");
    } else 
    {
        
        nextAntenna = root["antenna"];
        
        Serial.println();
        Serial.printf("\r\nJSON Received\t%d", nextAntenna);
        Serial.println();
        // we don't have a use for the antenna description
        // at the switch
        
        // syncAntennaState() will set the hardware to the nextAntenna if it
        // is different from the currentAntenna.
        // Also, if the nextAntenna is out of range !(0-7), syncAntennaState()
        // will read the lastAntenna but not set the hardware. An out of range
        // value is how the master remote controller requests the current
        // state of the antenna switch.
        // If nextAntenna is out of range, return lastAntenna which was set
        // last time syncAntennaState() was called with a valid antenna number
        
        if(!((nextAntenna < 0) || (nextAntenna >= 8)))
        {
            syncAntennaState();
        }
        
        telemetry.remoteAntenna = lastAntenna;
        // TODO:
        // rev. 1 of the hardware will actually read the battery
        //telemetry.temperature = 12.0 + (lastAntenna*0.1);
        //Serial.printf("\r\ntelemetry last antenna set\t%d", lastAntenna);
        
        sensors.requestTemperatures(); // Send the command to get temperatures
        telemetry.temperature = sensors.getTempF(sensor1); 
          
        //Serial.print(" Sensor 1(*F): ");     
        //Serial.println(telemetry.temperature); 
    }
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // reply to every message with the current antenna and
    // battery voltage
    uint8_t replyJSON[COMMANDLENGTH];
    StaticJsonBuffer<COMMANDLENGTH> jsonBufferReply;
    JsonObject& rootReply = jsonBufferReply.createObject();
    
    rootReply["temperature"] = telemetry.temperature;
    rootReply["remoteAntenna"] = telemetry.remoteAntenna;
    rootReply.printTo(jsonChar, COMMANDLENGTH);
    rootReply.printTo(Serial);
    memcpy(replyJSON, jsonChar, sizeof(jsonChar));
    
    int JSONlen = 0;
    while (replyJSON[JSONlen] != '}' && JSONlen < COMMANDLENGTH - 1) JSONlen++; // find end of JSON string
    Serial.println(JSONlen);
    
    sendResult = esp_now_send(master.peer_addr, replyJSON, JSONlen + 1);
    if (sendResult == ESP_OK) 
    {
        Serial.println("Slave Success");
    } else if (sendResult == ESP_ERR_ESPNOW_NOT_INIT) 
    {
        // How did we get so far!!
        Serial.println("Slave  ESPNOW not Init.");
    } else if (sendResult == ESP_ERR_ESPNOW_ARG) 
    {
        Serial.println("Slave  Invalid Argument");
    } else if (sendResult == ESP_ERR_ESPNOW_INTERNAL) 
    {
        Serial.println("Slave  Internal Error");
    } else if (sendResult == ESP_ERR_ESPNOW_NO_MEM) 
    {
        Serial.println("Slave ESP_ERR_ESPNOW_NO_MEM");
    } else if (sendResult == ESP_ERR_ESPNOW_NOT_FOUND) 
    {
        Serial.println("Slave Peer not found.");
    }
    else if (sendResult == ESP_ERR_ESPNOW_IF) 
    {
        Serial.println("Slave Interface Error.");
    }   else 
    {
        Serial.printf("\r\nSlave Not sure what happened\t%d", sendResult);
    }
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
    //esp_deep_sleep_start();
    //const int deep_sleep_sec = 3;
    //ESP_LOGI(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
    //esp_deep_sleep(1000000LL * deep_sleep_sec);
    
}

void loop() 
{
    // Chill
    
}
