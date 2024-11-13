
// define for hard coded static IP address
// undefine to use DHCP IP address assignment
#define __STATIC_IP__
// this app does not advertise via mDNS/Bonjour
// is you use DHCP you must find the IP address
// by other means

// Load Wi-Fi library
#include <WiFi.h>

#include "esp_system.h"
#include "soc/rtc.h"

#include <Arduino.h>
#include "nvs_flash.h"
#include "nvs.h"

extern "C" {
#include "Utilities.h"
};

esp_err_t err;
nvs_handle my_handle;


#include "GlobalDefinitions.h"
// Replace with your network credentials
const char* ssid     = "your network";
const char* password = "your password";


// Set web server port number to 80
WiFiServer server(80);

#if defined(__STATIC_IP__)
// Set your Static IP address
// http://192.168.11.197
IPAddress local_IP(192, 168, 11, 197);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
#endif



// Variable to store the HTTP request
String header;
// replace antenna names
#define Antenna0Name  "G5RV" // default
#define Antenna1Name  "6 Meter Vertical"
#define Antenna2Name  "10 Meter Vertical"
#define Antenna3Name  "40 Meter Dipole"
#define Antenna4Name  "unused-4"
#define Antenna5Name  "unused-5"
#define Antenna6Name  "unused-6"
#define Antenna7Name  "unused-7"

String Antenna0State = "off"; // default
String Antenna1State = "off";
String Antenna2State = "off";
String Antenna3State = "off";
String Antenna4State = "off";
String Antenna5State = "off";
String Antenna6State = "off";
String Antenna7State = "off";

int32_t lastAntenna = 0; // value will default to 0, if not set yet in NVS
int32_t nextAntenna = 0;
int     clientSelection = 0;
bool    initialized = false;


void setup()
{
  Serial.begin(115200);
  // slow down and lower power
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  // Initialize the output variables as outputs
  pinMode(SEL_A, OUTPUT);
  pinMode(SEL_B, OUTPUT);
  pinMode(SEL_C, OUTPUT);
  pinMode(EnableOutput, OUTPUT);
  //disable output
  digitalWrite(EnableOutput, HIGH);

  // initializes NVS and sets lastAntenna
  nonVolatileStorageInit();


#if defined(__STATIC_IP__)
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to Static IP via: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
#else
  // Use DHCP,
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to DHCP assigned IP via: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
#endif

}

void loop()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client)
  { // If a new client connects,
    Serial.println("New Client.");  // print a message out in the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
      //                      FROM C L I E N T
      // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

      if (client.available())
      { // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        // header: Variable to store the HTTP request
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // message from the client when a button is pressed
            // get the current client selection and the state of the button





            // Look at the client message to get the current antenna state
            // turns the GPIOs on and off
            if (header.indexOf("GET /Antenna0State/on") >= 0)
            {
              clientSelection = 0;
              Antenna0State = "on";
              Serial.println("Antenna0 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna0State/off") >= 0)
            {
              clientSelection = 0;
              Antenna0State = "off";
              Serial.println("Antenna0 is client");
              Serial.println("state is off");
            }

            else if (header.indexOf("GET /Antenna1State/on") >= 0)
            {
              clientSelection = 1;
              Antenna1State = "on";


              Serial.println("Antenna1 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna1State/off") >= 0)
            {
              clientSelection = 1;
              Antenna1State = "off";

              Serial.println("Antenna1 is client");
              Serial.println("state is off");
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
            else if (header.indexOf("GET /Antenna2State/on") >= 0)
            {
              clientSelection = 2;
              Antenna2State = "on";


              Serial.println("Antenna2 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna2State/off") >= 0)
            {
              clientSelection = 2;
              Antenna2State = "off";

              Serial.println("Antenna2 is client");
              Serial.println("state is off");
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
            else if (header.indexOf("GET /Antenna3State/on") >= 0)
            {
              clientSelection = 3;
              Antenna3State = "on";


              Serial.println("Antenna3 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna3State/off") >= 0)
            {
              clientSelection = 3;
              Antenna3State = "off";

              Serial.println("Antenna3 is client");
              Serial.println("state is off");
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
            else if (header.indexOf("GET /Antenna4State/on") >= 0)
            {
              clientSelection = 4;
              Antenna4State = "on";


              Serial.println("Antenna4 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna4State/off") >= 0)
            {
              clientSelection = 4;
              Antenna4State = "off";

              Serial.println("Antenna4 is client");
              Serial.println("state is off");
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
            else if (header.indexOf("GET /Antenna5State/on") >= 0)
            {
              clientSelection = 5;
              Antenna5State = "on";


              Serial.println("Antenna5 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna5State/off") >= 0)
            {
              clientSelection = 5;
              Antenna5State = "off";

              Serial.println("Antenna5 is client");
              Serial.println("state is off");
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
            else if (header.indexOf("GET /Antenna6State/on") >= 0)
            {
              clientSelection = 6;
              Antenna6State = "on";


              Serial.println("Antenna6 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna6State/off") >= 0)
            {
              clientSelection = 6;
              Antenna6State = "off";

              Serial.println("Antenna6 is client");
              Serial.println("state is off");
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
            else if (header.indexOf("GET /Antenna7State/on") >= 0)
            {
              clientSelection = 7;
              Antenna7State = "on";


              Serial.println("Antenna7 is client");
              Serial.println("state is on");
            }
            else if (header.indexOf("GET /Antenna7State/off") >= 0)
            {
              clientSelection = 7;
              Antenna7State = "off";

              Serial.println("Antenna7 is client");
              Serial.println("state is off");
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

            // a client connected and sent us some state
            // but we haven't initalized, so ignore
            // and set the antenna from NVM
            if (initialized == false)
            {
              Serial.println("not initialized");
              initialized = true;

              switch (lastAntenna) 
              {

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 0 : //
                  clientSelection = 0;
                  Antenna0State = "off"; // logic got inverted somehow

                  Serial.println("Antenna0 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 1 : //
                  clientSelection = 1;
                  Antenna1State = "off";

                  Serial.println("Antenna1 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 2 : //
                  clientSelection = 2;
                  Antenna2State = "off";

                  Serial.println("Antenna2 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 3 : //
                  clientSelection = 3;
                  Antenna3State = "off";

                  Serial.println("Antenna3 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 4 : //
                  clientSelection = 4;
                  Antenna4State = "off";

                  Serial.println("Antenna4 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 5 : //
                  clientSelection = 5;
                  Antenna5State = "off";

                  Serial.println("Antenna5 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 6 : //
                  clientSelection = 6;
                  Antenna6State = "off";

                  Serial.println("Antenna6 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                case 7 : //
                  clientSelection = 7;
                  Antenna7State = "off";

                  Serial.println("Antenna7 is client");
                  Serial.println("state is on");
                  break;
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                default:
                  break;
              }

            } // end switch clientSelection


            // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
            //                       TO C L I E N T
            // ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Antenna Switch Web Server</h1>");


            //__________________________________________________________
            //__________________________________________________________
            //            if (initialized == false)
            //            {
            // During initialization we must restore the NVM selection

            // Not yet Initialized

            // switch on the client antenna selection
            //switch (clientSelection) {
            printf("lastAntenna  = %d\n", lastAntenna);
            Serial.println("composing message to client");

            // a message from client with the button status
            switch (clientSelection) 
            {
              //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
              case 0 : // client selected Antenna 0
                if (Antenna0State == "on")
                {
                  Antenna0State = "off";
                  Serial.print(Antenna0State);
                  Serial.print("\n");
                  Serial.println("Antenna0State OFF");
                  client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                  client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   

                }
                else
                {
                  Antenna0State = "on";
                  Serial.print(Antenna0State);
                  Serial.print("\n");
                  Serial.println("Antenna0State ON");
                  client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                  client.println("<p><a href=\"/Antenna0State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 0;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Antenna1State = "off";
                client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna2State = "off";
                client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna3State = "off";
                client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna4State = "off";
                client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna5State = "off";
                client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna6State = "off";
                client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna7State = "off";
                client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                client.println("<p><a href=\"/Antenna7State/off\"><button class=\"button button2\">OFF</button></a></p>");

                break; // end case clientSelection 0

              case 1:
                client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                if (Antenna1State == "on")
                {
                  Antenna1State = "off";
                  Serial.print(Antenna1State);
                  Serial.print("\n");
                  Serial.println("Antenna1State OFF");
                  client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                  client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   
                }
                else
                {
                  Antenna1State = "on";
                  Serial.print(Antenna1State);
                  Serial.print("\n");
                  Serial.println("Antenna1State ON");
                  client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                  client.println("<p><a href=\"/Antenna1State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 1;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Antenna2State = "off";
                client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna3State = "off";
                client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna4State = "off";
                client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna5State = "off";
                client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna6State = "off";
                client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna7State = "off";
                client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                client.println("<p><a href=\"/Antenna7State/off\"><button class=\"button button2\">OFF</button></a></p>");

                break;


              case 2:
                client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                if (Antenna2State == "on")
                {
                  Antenna2State = "off";
                  Serial.print(Antenna2State);
                  Serial.print("\n");
                  Serial.println("Antenna2State OFF");
                  client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                  client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   
                }
                else
                {
                  Antenna2State = "on";
                  Serial.print(Antenna2State);
                  Serial.print("\n");
                  Serial.println("Antenna2State ON");
                  client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                  client.println("<p><a href=\"/Antenna2State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 2;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Antenna3State = "off";
                client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna4State = "off";
                client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna5State = "off";
                client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna6State = "off";
                client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna7State = "off";
                client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                client.println("<p><a href=\"/Antenna7State/off\"><button class=\"button button2\">OFF</button></a></p>");

                break;


              case 3:
                client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                if (Antenna3State == "on")
                {
                  Antenna3State = "off";
                  Serial.print(Antenna3State);
                  Serial.print("\n");
                  Serial.println("Antenna3State OFF");
                  client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                  client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   
                }
                else
                {
                  Antenna3State = "on";
                  Serial.print(Antenna3State);
                  Serial.print("\n");
                  Serial.println("Antenna3State ON");
                  client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                  client.println("<p><a href=\"/Antenna3State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 3;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Antenna4State = "off";
                client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna5State = "off";
                client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna6State = "off";
                client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna7State = "off";
                client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                client.println("<p><a href=\"/Antenna7State/off\"><button class=\"button button2\">OFF</button></a></p>");

                break;


              case 4:
                client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                if (Antenna4State == "on")
                {
                  Antenna4State = "off";
                  Serial.print(Antenna4State);
                  Serial.print("\n");
                  Serial.println("Antenna4State OFF");
                  client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                  client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   
                }
                else
                {
                  Antenna4State = "on";
                  Serial.print(Antenna4State);
                  Serial.print("\n");
                  Serial.println("Antenna4State ON");
                  client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                  client.println("<p><a href=\"/Antenna4State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 4;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Antenna5State = "off";
                client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna6State = "off";
                client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna7State = "off";
                client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                client.println("<p><a href=\"/Antenna7State/off\"><button class=\"button button2\">OFF</button></a></p>");

                break;




              case 5:
                client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                if (Antenna5State == "on")
                {
                  Antenna5State = "off";
                  Serial.print(Antenna5State);
                  Serial.print("\n");
                  Serial.println("Antenna5State OFF");
                  client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                  client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   
                }
                else
                {
                  Antenna5State = "on";
                  Serial.print(Antenna5State);
                  Serial.print("\n");
                  Serial.println("Antenna5State ON");
                  client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                  client.println("<p><a href=\"/Antenna5State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 5;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Antenna6State = "off";
                client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                Antenna7State = "off";
                client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                client.println("<p><a href=\"/Antenna7State/off\"><button class=\"button button2\">OFF</button></a></p>");

                break;

              case 6:
                client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                if (Antenna6State == "on")
                {
                  Antenna6State = "off";
                  Serial.print(Antenna6State);
                  Serial.print("\n");
                  Serial.println("Antenna6State OFF");
                  client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                  client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   
                }
                else
                {
                  Antenna6State = "on";
                  Serial.print(Antenna6State);
                  Serial.print("\n");
                  Serial.println("Antenna6State ON");
                  client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                  client.println("<p><a href=\"/Antenna6State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 6;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                Antenna7State = "off";
                client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                client.println("<p><a href=\"/Antenna7State/off\"><button class=\"button button2\">OFF</button></a></p>");

                break;




              case 7:
                client.println("<p>"Antenna0Name "- State " + Antenna0State + "</p>");
                // we know 0 must be off is client is 1
                client.println("<p><a href=\"/Antenna0State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna1Name "- State " + Antenna1State + "</p>");
                client.println("<p><a href=\"/Antenna1State/off\"><button class=\"button button2\">OFF</button></a></p>");
                
                client.println("<p>"Antenna2Name "- State " + Antenna2State + "</p>");
                client.println("<p><a href=\"/Antenna2State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna3Name "- State " + Antenna3State + "</p>");
                client.println("<p><a href=\"/Antenna3State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna4Name "- State " + Antenna4State + "</p>");
                client.println("<p><a href=\"/Antenna4State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna5Name "- State " + Antenna5State + "</p>");
                client.println("<p><a href=\"/Antenna5State/off\"><button class=\"button button2\">OFF</button></a></p>");

                client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");

                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                if (Antenna7State == "on")
                {
                  Antenna7State = "off";
                  Serial.print(Antenna6State);
                  Serial.print("\n");
                  Serial.println("Antenna6State OFF");
                  client.println("<p>"Antenna6Name "- State " + Antenna6State + "</p>");
                  client.println("<p><a href=\"/Antenna6State/off\"><button class=\"button button2\">OFF</button></a></p>");
                  // disconnect the output to switch over ethernet cable
                  digitalWrite(EnableOutput, HIGH);   
                }
                else
                {
                  Antenna7State = "on";
                  Serial.print(Antenna7State);
                  Serial.print("\n");
                  Serial.println("Antenna7State ON");
                  client.println("<p>"Antenna7Name "- State " + Antenna7State + "</p>");
                  client.println("<p><a href=\"/Antenna7State/on\"><button class=\"button button\">ON</button></a></p>");
                  nextAntenna = 7;
                  syncAntennaState(); // store to NVS
                  setOutputState(nextAntenna); // set the antenna switch relays
                }
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                break;
                

              default:
                break;

            } // end switch clientSelection
            // }
            //__________________________________________________________
            //__________________________________________________________

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
