/*
 * Rui Santos 
 * Complete Project Details https://randomnerdtutorials.com
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <esp_now.h>
#include <WiFi.h>


// Based on the OneWire library example

OneWire ds(23);  //data wire connected to GPIO23
#define CHANNEL 1

DallasTemperature tempSensor(&ds);

// config AP SSID
void configDeviceAP() 
{
  char* SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}


void setup(void) 
{
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();

  // Pass our oneWire reference to Dallas Temperature. 
  tempSensor.begin();
  
 
}

void loop(void) 
{
  byte i;
  byte addr[8];


  if (!ds.search(addr)) 
  {
    Serial.println(" No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250*8);
    return;
  }
  Serial.print(" ROM DS18B20   =");
  for (i = 0; i < 8; i++) 
  {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  Serial.println();
  // This is the mac address of the Slave in AP Mode
  Serial.print(" ESP32 AP MAC: = "); Serial.println(WiFi.softAPmacAddress()); 

  tempSensor.requestTemperaturesByIndex(0);
  delay(500);
  Serial.print(" Temperature:  = ");
  Serial.print(tempSensor.getTempFByIndex(0));
  Serial.println(" F");

}
