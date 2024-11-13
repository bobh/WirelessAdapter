/**
 *
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
 Any devices can act as master or salve.
 
 
 
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#include <M5Stack.h>


#include "GlobalDefinitions.h"
#include <ArduinoJson.h>

// TODO: these struct are the same on master and slave
// and should be put in a common file
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
    float batteryVoltage;
};
commandStruct   command;
telemetryStruct telemetry;


char antenna_0[30] = "g5rv";
char antenna_1[30] = "6 meter vertical";

float remoteBattery;
int   remoteAntenna;
int   currentAntenna;

// test
long randNumber;

int LED_BUILTIN = 5;

#define __DEBUG__


// Global copy of slave
esp_now_peer_info_t slave;

// This remote is an antenna switch
//uint8_t remoteMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t remoteMac[] = {0x30, 0xAE, 0xA4, 0x16, 0x1E, 0x61};

// This remote is a rig switch
uint8_t remoteMac1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define CHANNEL 3
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0



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

// Scan for slaves in AP mode
void ScanForSlave() {
    int8_t scanResults = WiFi.scanNetworks();
    // reset on each scan
    bool slaveFound = 0;
    memset(&slave, 0, sizeof(slave));
    
    Serial.println("");
    if (scanResults == 0) {
        Serial.println("No WiFi devices in AP Mode found");
    } else {
        Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
        for (int i = 0; i < scanResults; ++i) {
            // Print SSID and RSSI for each device found
            String SSID = WiFi.SSID(i);
            int32_t RSSI = WiFi.RSSI(i);
            String BSSIDstr = WiFi.BSSIDstr(i);
            
            if (PRINTSCANRESULTS) {
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(SSID);
                Serial.print(" (");
                Serial.print(RSSI);
                Serial.print(")");
                Serial.println("");
            }
            delay(10);
            // Check if the current device starts with `Slave`
            if (SSID.indexOf("Slave") == 0) {
                // SSID of interest
                Serial.println("Found a Slave.");
                Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
                // Get BSSID => Mac Address of the Slave
                int mac[6];
                if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
                    for (int ii = 0; ii < 6; ++ii ) {
                        slave.peer_addr[ii] = (uint8_t) mac[ii];
                    }
                }
                
                slave.channel = CHANNEL; // pick a channel
                slave.encrypt = 0; // no encryption
                
                slaveFound = 1;
                // we are planning to have only one slave in this example;
                // Hence, break after we find one, to be a bit efficient
                break;
            }
        }
    }
    
    if (slaveFound) {
        Serial.println("Slave Found, processing..");
    } else {
        Serial.println("Slave Not Found, trying again.");
    }
    
    // clean up ram
    WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
bool manageSlave() {
    if (slave.channel == CHANNEL) {
        if (DELETEBEFOREPAIR) {
            deletePeer();
        }
        
        
        const esp_now_peer_info_t *peer = &slave;
        const uint8_t *peer_addr = slave.peer_addr;
        //++++++++++++++++++++++++++++++++++++++++++
        
        // TODO: this will be used with a new DEVC board
        memcpy( &slave.peer_addr, &remoteMac, 6 );
        //slave.channel = WIFI_CHANNEL;
        //slave.encrypt = 0;
        
        //++++++++++++++++++++++++++++++++++++++++++
        
        Serial.print("Slave Status: ");
        
        // check if the peer exists
        bool exists = esp_now_is_peer_exist(peer_addr);
        if ( exists) {
            // Slave already paired.
            Serial.println("Already Paired");
            return true;
        } else {
            // Slave not paired, attempt pair
            esp_err_t addStatus = esp_now_add_peer(peer);
            if (addStatus == ESP_OK) {
                // Pair success
                Serial.println("Pair success");
                return true;
            } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
                // How did we get so far!!
                Serial.println("ESPNOW Not Init");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
                Serial.println("Invalid Argument");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
                Serial.println("Peer list full");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
                Serial.println("Out of memory");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
                Serial.println("Peer Exists");
                return true;
            } else {
                Serial.println("Not sure what happened");
                return false;
            }
        }
    } else {
        // No slave found to process
        Serial.println("No Slave found to process");
        return false;
    }
}

void deletePeer() {
    const esp_now_peer_info_t *peer = &slave;
    const uint8_t *peer_addr = slave.peer_addr;
    esp_err_t delStatus = esp_now_del_peer(peer_addr);
    Serial.print("Slave Delete Status: ");
    if (delStatus == ESP_OK) {
        // Delete success
        Serial.println("Success");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
    } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
    } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
        Serial.println("Peer not found.");
    } else {
        Serial.println("Not sure what happened");
    }
}

uint8_t data = 0;


// send data
void sendData() 
{
    digitalWrite(LED_BUILTIN, HIGH);
    
    
#if defined(__DEBUG__)
    randNumber = random(0, 8);
    command.antenna = (int)randNumber;
#endif
    
    //+++++++++++++++++++++++++++++++++++++++++++
    uint8_t commandJSON[COMMANDLENGTH];
    StaticJsonBuffer<COMMANDLENGTH> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    
    root["antenna"] = command.antenna;
    root["name"] = command.name;
    char jsonChar[COMMANDLENGTH];
    root.printTo(jsonChar, COMMANDLENGTH);
    memcpy(commandJSON, jsonChar, sizeof(jsonChar));
    //Serial.println(jsonChar);
    
    int JSONlen = 0;
    while (commandJSON[JSONlen] != '}' && JSONlen < COMMANDLENGTH - 1) JSONlen++; // find end of JSON string
    Serial.println(jsonChar);
    
    //+++++++++++++++++++++++++++++++++++++++++++
    
    
    //data++;
    const uint8_t *peer_addr = slave.peer_addr;
    
    
    //Serial.print("Sending: "); Serial.println(data);
    
    //M5.Lcd.printf("Display Test!");
    //M5.Lcd.printf("%d ", data);
    
    //+++++++++++++++++++++++++++++++++++++++++++
    
    esp_err_t result = esp_now_send(peer_addr, commandJSON, JSONlen + 1);
    // Serial.print("Send Status: ");
    if (result == ESP_OK) {
        //Serial.println("Success");
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
        Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
        Serial.println("Peer not found.");
    } else {
        Serial.println("Not sure what happened");
    }
    delay(50);
    
    //+++++++++++++++++++++++++++++++++++++++++++
    
}


// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Sent to: "); Serial.println(macStr);
    Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    if(status == ESP_NOW_SEND_SUCCESS)
    {
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
    }
    
}
//************************************************************************
// This master will receive a return message from the slave antenna switch
// every time it sends a command to set the antenna. The master
// always initiates the data transfers.
// It also gets the current battery voltage in the same message.
// To get the current antenna without setting it or to just read
// the battery voltage, send a set antenna message with the 
// antenna set to an illegal value (value >= 8)
// then sync this controller to the value returned.
// This will synchronize the remote controller to the antenna switch
// through power cycles.
// If the slave reports an illegal antenna value, the slave might not
// yet be initialized or has a problem with non-volatile storage or 
// a corrupted message.
//************************************************************************

void OnDataRecv(const uint8_t *remote, const uint8_t *data, int data_len)
{
    
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             remote[0], remote[1], remote[2], remote[3], remote[4], remote[5]);
    Serial.print("Last Packet Recv from: "); Serial.println(macStr);
    Serial.print("Last Packet Recv Data: "); Serial.println(*data);
    Serial.println("");
    
    StaticJsonBuffer<COMMANDLENGTH> jsonBufferReply;
    JsonObject& rootReply = jsonBufferReply.parseObject(data);
    if (!rootReply.success())
    {
        Serial.println("parseObject() failed");
    } else
    {
        // float VBAT = (127.0f/100.0f) * 3.30f * float(analogRead(34)) / 4096.0f;
        // Serial.print("Battery Voltage = "); Serial.print(VBAT, 2); Serial.println(" V");
        
        remoteBattery = rootReply["battery"];
        Serial.println();
        Serial.print("Received ");
        Serial.print(remoteBattery);
        Serial.println();
        //M5.Lcd.printf("%d battery", remoteBattery);
        // This is the antenna the switch is currently set to
        
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(ORANGE);
        M5.Lcd.setTextSize(3);
        
        M5.Lcd.print("Switch Test\n");

        // this was sent
        M5.Lcd.setCursor(10, 25);
        M5.Lcd.printf("%d sent", command.antenna);

        // this was received
        M5.Lcd.setTextSize(20);        
        Serial.print("Remote Antenna ");
        remoteAntenna = rootReply["remoteAntenna"];
        Serial.print(remoteAntenna);
        Serial.println();
        M5.Lcd.setCursor(145, 85);
        M5.Lcd.printf("%d ", remoteAntenna);
    }
}

void setup() {
    
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.begin(115200);
    
    // Initialize the M5Stack object
    M5.begin();
    
    // LCD display
    M5.Lcd.setTextColor(ORANGE);
    M5.Lcd.setTextSize(3);
    
    M5.Lcd.print("Switch Test\n");
    M5.Lcd.setTextSize(5);
    
    // This code sets the mac address of the master
    // The slave uses this address to send back
    // battery voltage telemetry
    // This is a soft address for this program only
    // The device will revert to the mac address set
    // at the factory. Its true
    // mac address can be found by loading the utility
    // sketch ESP32_WIFI
    
    //printf("set mac : 01:02:03:04:05:05\n");
    //uint8_t new_mac[8] = {0x01,0x02,0x03,0x04,0x05,0x05};
    //esp_base_mac_addr_set(new_mac);
    
    
    
    //Set device in STA mode to begin with
    WiFi.mode(WIFI_STA);
    Serial.println("ESPNow/Basic/Master Example");
    // This is the mac address of the Master in Station Mode
    Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
    // Init ESPNow with a fallback logic
    InitESPNow();
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    
    // +++++++++++++++++++++++++++++++++++++++
    //command.antenna = 5;
    //
#if defined(__DEBUG__)
    randomSeed(analogRead(34));
    randNumber = random(0, 8);
    command.antenna = (int)randNumber;
#endif
    
    memcpy(command.name, antenna_0, sizeof(antenna_0));
    //Serial.println(command.antenna);
    //Serial.println(command.name);
    
    
    // +++++++++++++++++++++++++++++++++++++++
    
}

void loop()
{
    // In the loop we scan for slave
    ScanForSlave();
    // If Slave is found, it would be populate in `slave` variable
    // We will check if `slave` is defined and then we proceed further
    if (slave.channel == CHANNEL) { // check if slave channel is defined
        // `slave` is defined
        // Add slave as peer if it has not been added already
        bool isPaired = manageSlave();
        if (isPaired) {
            // pair success or already paired
            // Send data to device
            sendData();
        } else {
            // slave pair failed
            Serial.println("Slave pair failed!");
        }
    }
    else {
        // No slave found to process
    }
    
    // wait for seconds to run the logic again
    delay(500);
}
