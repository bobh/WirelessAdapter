/* 
 *  Requires received remote control messages to cycle 
 *  through antennas
 *  But ignores message content and writes its own value to
 *  the switch.
 *  Increments each message received through 0-7.
 *  
 */


#include "GlobalDefinitions.h"
#include "Utilities.h"

#include <M5Stack.h>
//#include <M5StackSAM.h>
//#include <M5ez.h>



extern int   menuItemSelected;
extern commandStruct   command;
extern telemetryStruct telemetry;

extern commandStruct   commandCopyCore0;
extern commandStruct   commandCopyCore1;
extern telemetryStruct telemetryCopyCore0;
extern telemetryStruct telemetryCopyCore1;


///extern M5SAM MyMenu;
extern SemaphoreHandle_t processUI;

extern bool displayTXmsg;
extern bool displayRXmsg;
extern bool displayERRmsg;

extern bool remoteInitialized;
extern int  initialAntenna;

extern String gTemperature;

void deletePeer(void);

// Global copy of slave
esp_now_peer_info_t slave;
bool slaveFound = 0;

// copy of global structs for local use on core 0
// core 0 is the communications process
// commandStruct will be read by core 0 to send 
// the current menu antenna selection to the remote
// commandStruct will be written when the remote is
// requested to send the current antenna selection at 
// start up and anytime to synchronize the 
// master with the remote
// telemetryStruct is written to by core 0 on each
// reply from the slave with remote battery info
extern commandStruct   commandCopyCore0;
extern commandStruct   commandCopyCore1;
extern telemetryStruct telemetryCopyCore0;
extern telemetryStruct telemetryCopyCore1;



// Init ESP Now with fallback
void InitESPNow() 
{
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESPNow Init Success");
    }
    else
    {
        Serial.println("ESPNow Init Failed");
        // Retry InitESPNow, add a counte and then restart?
        // InitESPNow();
        // or Simply Restart
        ESP.restart();
    }
}

// Scan for slaves in AP mode
void ScanForSlave() 
{
    
    int8_t scanResults = WiFi.scanNetworks();
    
    // reset on each scan
    //bool slaveFound = 0;
    slaveFound = 0;
    
    
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
    
    if (slaveFound) 
    {
        Serial.println("Slave Found, processing..");
    } else 
    {
        Serial.println("Slave Not Found, trying again.");
        displayERRmsg = true;
    }
    
    // clean up ram
    WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
bool manageSlave() 
{
    if (slave.channel == CHANNEL)
    {
        if (DELETEBEFOREPAIR)
        {
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
        } else
        {
            // Slave not paired, attempt pair
            esp_err_t addStatus = esp_now_add_peer(peer);
            if (addStatus == ESP_OK)
            {
                // Pair success
                Serial.println("Pair success");
                return true;
            } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT)
            {
                // How did we get so far!!
                Serial.println("ESPNOW Not Init");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_ARG)
            {
                Serial.println("Invalid Argument");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_FULL)
            {
                Serial.println("Peer list full");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM)
            {
                Serial.println("Out of memory");
                return false;
            } else if (addStatus == ESP_ERR_ESPNOW_EXIST)
            {
                Serial.println("Peer Exists");
                return true;
            } else
            {
                Serial.println("Not sure what happened");
                return false;
            }
        }
    } else
    {
        // No slave found to process
        Serial.println("No Slave found to process");
        return false;
    }
}

void deletePeer() 
{
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

    displayRXmsg = false;
    displayERRmsg = false;
    // copy the latest global values
    // to a local copy for Core1 use
    updateGlobalVarsCore1();
    
#if defined(__DEBUG__)
    randNumber = random(0, 8);
    command.antenna = (int)randNumber;
#endif
    
    //+++++++++++++++++++++++++++++++++++++++++++
    uint8_t commandJSON[COMMANDLENGTH];
    StaticJsonBuffer<COMMANDLENGTH> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    
    //root["antenna"] = command.antenna;
    root["antenna"] = commandCopyCore0.antenna;
    //root["name"] = command.name;
    root["name"] = commandCopyCore0.name;
    
    char jsonChar[COMMANDLENGTH];
    root.printTo(jsonChar, COMMANDLENGTH);
    memcpy(commandJSON, jsonChar, sizeof(jsonChar));
    //Serial.println(jsonChar);
    
    int JSONlen = 0;
    while (commandJSON[JSONlen] != '}' && JSONlen < COMMANDLENGTH - 1) JSONlen++; // find end of JSON string
    Serial.println(jsonChar);
    
    //+++++++++++++++++++++++++++++++++++++++++++    
    const uint8_t *peer_addr = slave.peer_addr;    
    //+++++++++++++++++++++++++++++++++++++++++++
    // send the latest selected antenna possibly
    // updated from core 1 UI processor
    esp_err_t result = esp_now_send(peer_addr, commandJSON, JSONlen + 1);
    // Serial.print("Send Status: ");
    if (result == ESP_OK)
    {
      displayERRmsg = false;
        //Serial.println("Success");
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) 
    {
        displayERRmsg = true;
        Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) 
    {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) 
    {   // *************************
        displayERRmsg = true;  
        Serial.println("Peer not found.");
        // *************************
    } else 
    {
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
        displayERRmsg = false;
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
    }
    else
    {
      displayERRmsg = true;
    }
    
}
//************************************************************************
// This master will receive a return message from the slave antenna switch
// every time it sends a command to set the antenna. The master
// always initiates the data transfers.
// It also gets the current temperature in the same message.
// To get the current antenna without setting it or to just read
// the temperature, send a set antenna message with the 
// antenna set to an illegal value (value >= 8)
// then sync this controller to the value returned.
// This will synchronize the remote controller to the antenna switch
// through power cycles.
// If the slave reports an illegal antenna value, the slave might not
// yet be initialized or has a problem with non-volatile storage or 
// a corrupted message.
//************************************************************************
/* EXAMPLE
Found 14 devices 
Found a Slave.
1: Slave_1 [30:AE:A4:16:1E:61] (-48)
Slave Found, processing..
Slave Status: Pair success
{"antenna":-1,"name":""}
Last Packet Sent to: 30:ae:a4:16:1e:61
Last Packet Send Status: Delivery Success
Last Packet Recv from: 30:ae:a4:16:1e:61
Last Packet Recv Data: 123


Received 12.40
Received initial Antenna 4
*/
float remoteTemperature;

void OnDataRecv(const uint8_t *remote, const uint8_t *data, int data_len)
{
    if(slaveFound)
    {
        
        displayRXmsg = true; // display message on UI
        
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
            
            remoteTemperature = rootReply["temperature"];
            telemetryCopyCore0.temperature = remoteTemperature;
            Serial.println();
            Serial.print("Received ");
            Serial.print(remoteTemperature);
            Serial.println();

            gTemperature =  String(remoteTemperature, 0);
            //Serial.print("converted to string " );
            //Serial.println(gTemperature);
            
            //M5.Lcd.printf("%d temperature", remoteTemperature);
            // This is the antenna the switch is currently set to
            
            if(remoteInitialized == false)
            {
                initialAntenna = rootReply["remoteAntenna"];
                Serial.print("Received initial Antenna ");
                Serial.print(initialAntenna);
                Serial.println();

                menuItemSelected = initialAntenna;
                command.antenna = menuItemSelected;
                remoteInitialized = true;
            }
        }
    }
    else
    {
      displayERRmsg = true;
    }
}


void dummy()
{
}

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// Core0Core0Core0Core0Core0Core0Core0Core0Core0Core0Core0Core0
//                    This Task runs on Core: 0
// Core0Core0Core0Core0Core0Core0Core0Core0Core0Core0Core0Core0
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// Communication to Slave processor on antenna switch

// setup()

void commProcessing( void* parameter )
{
    // this code executes on core 0
    // this is the standard Arduino setup() code
    Serial.print("Comm Processing: Executing on core ");
    Serial.println(xPortGetCoreID());
    
    command.antenna = menuItemSelected;
    // copy the global variable to this core
    commandCopyCore0.antenna = menuItemSelected;
    
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
    
    remoteInitialized = false;
    // an antenna out of range is a request
    // to the slave for the current setting
    command.antenna = -1;
    commandCopyCore0.antenna = -1;
    initialAntenna = -1;
    
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    
    // this is the standard Arduino loop() code
    for (;;) // this is required, normally put in by
    {        // Arduino IDE
        
        // "Arf! Arf!"
        // pet the watchdog.
        vTaskDelay(10);
        
        ///////////xSemaphoreTake(processUI, portMAX_DELAY);
        //Serial.print("+\n");
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++
        // frame processing takes place here:
        int i;
        
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
        else
        {
            // No slave found to process
        }
        
        // wait for seconds to run the logic again
        delay(500);
    } // end Arduino task loop() for core 0
}


// /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
// 0000000000000000000000000000000000000000000000000000000000000
// /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
