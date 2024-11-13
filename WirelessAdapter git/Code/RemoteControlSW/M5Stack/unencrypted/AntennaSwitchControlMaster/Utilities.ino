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
#include <M5ez.h>

///////#include <WiFi.h>
#include "EEPROM.h"

#define MSG_DELAY 3000

extern int   menuItemSelected;
//extern void  sendData(void);
extern commandStruct   command;
extern telemetryStruct telemetry;

extern commandStruct commandCopyCore0;
extern commandStruct commandCopyCore1;
extern telemetryStruct telemetryCopyCore0;
extern telemetryStruct telemetryCopyCore1;

///extern M5SAM MyMenu;
extern SemaphoreHandle_t processUI;

extern bool displayTXmsg;
extern bool displayRXmsg;
extern bool displayERRmsg;
extern String gTemperature;

extern bool remoteInitialized;
extern int  initialAntenna;


char* antennaName(int n)
{
    static char* name[] = 
    {
        "G5RV",                // antenna position 0
        " 6 Meter Vertical",   // antenna position 1
        "10 Meter Vertical",   // antenna position 2
        "40 Meter Dipole",     // antenna position 3
        "unused-4",            // antenna position 4
        "unused-5",            // antenna position 5
        "unused-6",            // antenna position 6
        "unused-7",            // antenna position 7
        "temperature"          // unused position 8
    };
    return name[n];
}



void main_menuAntenna1() 
{
     Serial.print("displayERRmsg: "); Serial.println(displayERRmsg);
// if(displayERRmsg == false)
 //{   
    menuItemSelected = 1;
    command.antenna = menuItemSelected-1;
    
    
    ezMenu main("Select Antenna");
    
    //main.txtFont(&FreeMonoBold12pt7b);
    main.addItem(antennaName(0), main_menuAntenna1);
    //main.txtFont(&FreeSans12pt7b);
    
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(6), main_menuAntenna7);
    main.addItem(antennaName(7), main_menuAntenna8);
    // 
    main.addItem(antennaName(8), main_menuTemperature);
    // 
        
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
    updateGlobalVarsCore0();
    
    ezMenu submenu("Sending to Switch (1)");
    ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);

/*
  String cr = (String)char(13);
  ez.msgBox("You can show messages", "ez.msgBox shows text");
  ez.msgBox("Looking the way you want", "In any font !", "OK", true, &FreeSerifBold24pt7b, TFT_RED);
  ez.msgBox("More ez.msgBox", "Even multi-line messages where everything lines up and is kept in the middle of the screen");
  ez.msgBox("Questions, questions...", "But can it also show any buttons you want?", "No # # Yes"); 
  ez.textBox("And there's ez.textBox", "To present or compose longer word-wrapped texts, you can use the ez.textBox function." + cr + cr + "M5ez (pronounced \"M5 easy\") is a complete interface builder library ", true);
*/
    
    delay(MSG_DELAY);
    if(displayRXmsg)
    {
        ez.msgBox("Sending To Switch (1)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
    }
    else
    {
        ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000); 

        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }
    }

    
// }
 //else
 //{
  //ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
  //delay(1000); 
  //displayERRmsg = false;
  //initializeMenu(menuItemSelected);
 //}
    main.runOnce();
    //main.run();
}


void main_menuAntenna2() 
{
    
    menuItemSelected = 2;
    command.antenna = menuItemSelected-1;
    
    
    ezMenu main("Select Antenna");
    
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(0), main_menuAntenna1);
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(6), main_menuAntenna7);
    main.addItem(antennaName(7), main_menuAntenna8);

    // 
    //main.addItem(antennaName(8), main_menuTemperature);
    // 

    
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
    updateGlobalVarsCore0();
    
    ezMenu submenu("Sending to Switch (2)");
    ez.msgBox("Sending To Switch (2)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    delay(MSG_DELAY);
    if(displayRXmsg)
    {
        ez.msgBox("Sending To Switch (2)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
    }
    else
    {
        ez.msgBox("Sending To Switch (2)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000); 
        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }         
    }
    
    main.runOnce();
    //main.run();
}

void main_menuAntenna3() 
{
    
    menuItemSelected = 3;
    command.antenna = menuItemSelected-1;
    
    
    ezMenu main("Select Antenna");
    
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(0), main_menuAntenna1);
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(6), main_menuAntenna7);
    main.addItem(antennaName(7), main_menuAntenna8);
    
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
    updateGlobalVarsCore0();
    
    ezMenu submenu("Sending to Switch (3)");
    ez.msgBox("Sending To Switch (3)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    delay(MSG_DELAY);
    if(displayRXmsg)
    {
        ez.msgBox("Sending To Switch (3)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
    }
    else
    {
        ez.msgBox("Sending To Switch (3)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000); 
        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }         
         
    }
    
    main.runOnce();
    //main.run();
    
}

void main_menuAntenna4() 
{
     menuItemSelected = 4;
     command.antenna = menuItemSelected-1;
    
    
     ezMenu main("Select Antenna");
    
     main.addItem(antennaName(3), main_menuAntenna4);
     main.addItem(antennaName(0), main_menuAntenna1);
     main.addItem(antennaName(1), main_menuAntenna2);
     main.addItem(antennaName(2), main_menuAntenna3);
     main.addItem(antennaName(4), main_menuAntenna5);
     main.addItem(antennaName(5), main_menuAntenna6);
     main.addItem(antennaName(6), main_menuAntenna7);
     main.addItem(antennaName(7), main_menuAntenna8);
    
     main.upOnFirst("last|up");
     main.downOnLast("first|down");
    
     updateGlobalVarsCore0();
    
     ezMenu submenu("Sending to Switch (4)");
     ez.msgBox("Sending To Switch (4)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
     delay(MSG_DELAY);
     if(displayRXmsg)
     {
        ez.msgBox("Sending To Switch (4)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
     }
     else
     {
        ez.msgBox("Sending To Switch (4)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000);
        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }         
                  
     }

    main.runOnce();
    //main.run();    
}

void main_menuAntenna5() 
{
    menuItemSelected = 5;
    command.antenna = menuItemSelected-1;
    
    
    ezMenu main("Select Antenna");
    
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(0), main_menuAntenna1);
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(6), main_menuAntenna7);
    main.addItem(antennaName(7), main_menuAntenna8);
    
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
    updateGlobalVarsCore0();
    
    ezMenu submenu("Sending to Switch (5)");
    ez.msgBox("Sending To Switch (5)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    delay(MSG_DELAY);
    if(displayRXmsg)
    {
        ez.msgBox("Sending To Switch (5)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
    }
    else
    {
        ez.msgBox("Sending To Switch (5)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000);
        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }         
                  
    }
    
    main.runOnce();
    //main.run();
    
}

void main_menuAntenna6() 
{
    menuItemSelected = 6;
    command.antenna = menuItemSelected-1;
    
    
    ezMenu main("Select Antenna");
    
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(0), main_menuAntenna1);
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(6), main_menuAntenna7);
    main.addItem(antennaName(7), main_menuAntenna8);
    
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
    updateGlobalVarsCore0();
    
    ezMenu submenu("Sending to Switch (6)");
    ez.msgBox("Sending To Switch (6)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    delay(MSG_DELAY);
    if(displayRXmsg)
    {
        ez.msgBox("Sending To Switch (6)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
    }
    else
    {
        ez.msgBox("Sending To Switch (6)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000);
        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }         
                  
    }
    
    main.runOnce();
   // main.run();
    
}

void main_menuAntenna7() 
{
    menuItemSelected = 7;
    command.antenna = menuItemSelected-1;
    
    
    ezMenu main("Select Antenna");
    
    main.addItem(antennaName(6), main_menuAntenna7);
    main.addItem(antennaName(0), main_menuAntenna1);
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(7), main_menuAntenna8);
    
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
    updateGlobalVarsCore0();
    
    ezMenu submenu("Sending to Switch (7)");
    ez.msgBox("Sending To Switch (7)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    delay(MSG_DELAY);
    if(displayRXmsg)
    {
        ez.msgBox("Sending To Switch (7)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
    }
    else
    {
        ez.msgBox("Sending To Switch (7)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000);
        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }         
                  
    }
    
    main.runOnce();
    //main.run();
}

void main_menuAntenna8() 
{
    menuItemSelected = 8;
    command.antenna = menuItemSelected-1;
    
    
    ezMenu main("Select Antenna");
    
    main.addItem(antennaName(7), main_menuAntenna8);
    main.addItem(antennaName(0), main_menuAntenna1);
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(6), main_menuAntenna7);
    
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
    updateGlobalVarsCore0();
    
    ezMenu submenu("Sending to Switch (8)");
    ez.msgBox("Sending To Switch (8)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    delay(MSG_DELAY);
    if(displayRXmsg)
    {
        ez.msgBox("Sending To Switch (8)", "Message RECEIVED !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
        delay(1000); 
        
    }
    else
    {
        ez.msgBox("Sending To Switch (8)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
        delay(1000); 
        while(!displayRXmsg)
        {
          ez.msgBox("Sending To Switch (1)", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_GREEN);
          delay(1000);  
          ez.msgBox("Sending To Switch (1)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
          delay(1000);     
        }                          
    }
    
    main.runOnce();
    //main.run();
}

String gTemperatureOLD;

void main_menuTemperature() 
{
    menuItemSelected = 9;
    //command.antenna = menuItemSelected-1;
    
    
    //ezMenu main("temperature F");
/*    
    main.addItem(antennaName(7), main_menuAntenna8);
    main.addItem(antennaName(0), main_menuAntenna1);
    main.addItem(antennaName(1), main_menuAntenna2);
    main.addItem(antennaName(2), main_menuAntenna3);
    main.addItem(antennaName(3), main_menuAntenna4);
    main.addItem(antennaName(4), main_menuAntenna5);
    main.addItem(antennaName(5), main_menuAntenna6);
    main.addItem(antennaName(6), main_menuAntenna7);
*/    
    main.upOnFirst("last|up");
    main.downOnLast("first|down");
    
//    updateGlobalVarsCore0();
    
    //ezMenu submenu("Temp. F");
    ez.msgBox("Temperature F", gTemperature, "OK", false, &FreeSerifBold24pt7b, TFT_RED);
//    delay(MSG_DELAY);

    gTemperatureOLD = gTemperature;
        
    delay(1000); 
    //ez.buttons.wait("OK");  

    while (true) 
    {
     String btnpressed = ez.buttons.poll();
     if (btnpressed == "OK") break;
     if(gTemperatureOLD != gTemperature)
     {
      gTemperatureOLD = gTemperature;
      ez.msgBox("Temperature F", gTemperature, "OK", false, &FreeSerifBold24pt7b, TFT_RED);
     }
     delay(1000);
    }
      
    
    main.runOnce();
    //main.run();
}





// runs on timer interrupt
void waitingMenu()
{
    //ezMenu main("+");
    //ez.msgBox("Waiting for remote", "Message SENT !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    //delay(MSG_DELAY);

    Serial.print("+ ");
    main.runOnce(); // run once, it is in Arduino loop() and will be called repeatedly 
                    // until the switch responds
    //main.run();
}

void errorMenu()
{
    ezMenu submenu("comm ERROR");
    ez.msgBox("Sending To Switch (8)", "COMM ERROR !", "OK", false, &FreeSerifBold24pt7b, TFT_RED);
    delay(1000);  
}





void initializeMenu(int initialValue)
{
  switch (initialValue)
  {
    case 0 : 
     main_menuAntenna1();
     break;
    case 1 : 
     main_menuAntenna2();
     break;
    case 2 : 
     main_menuAntenna3();
     break;
    case 3 : 
     main_menuAntenna4();
     break;     
    case 4 : 
     main_menuAntenna5();
     break;          
    case 5 : 
     main_menuAntenna6();
     break; 
    case 6 : 
     main_menuAntenna7();
     break; 
    case 7 : 
     main_menuAntenna8();
     break; 
    case 8 : 
     main_menuTemperature();
     break; 
                  
     default:
      break;
  }
  
}




// menu changes will be detected and
// sent to the remote on Core1
void updateGlobalVarsCore0()
{
    // update shared vaiables
    // menu operations may update the local antenna selecton
    // give the local selection to the global selection
    if( processUI != NULL )
    {
        /* See if we can obtain the semaphore.  If the semaphore is not
         available wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( processUI, ( TickType_t ) 10 ) == pdTRUE )
        {
            /* We were able to obtain the semaphore and can now access the
             shared resource. */
            // on core 1
            // write the command for the remote
            // read the telemetry from the remote
            
            // write the global command struct with the current antenna
            // selection
            //command = commandCopyCore1;
            // the telemetry data is updated on the communication core, core 0
            // but not currently read by the UI on core 1
            // it would look like this if implemented
            telemetryCopyCore1 = telemetry;
            /* ... */
            
            /* We have finished accessing the shared resource.  Release the
             semaphore. */
            xSemaphoreGive( processUI );
        }
        else
        {
            Serial.println("1 fail");
            /* We could not obtain the semaphore and can therefore not access
             the shared resource safely. */
        }
        
        Serial.println("updating global vars");
    }
}

void updateGlobalVarsCore1()
{
    if( processUI != NULL )
    {
        /* See if we can obtain the semaphore.  If the semaphore is not
         available wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( processUI, ( TickType_t ) 10 ) == pdTRUE )
        {
            /* We were able to obtain the semaphore and can now access the
             shared resource. */
            // on core 0
            // read the command for the remote
            // write the telemetry from the remote
            
            commandCopyCore0 = command;
            telemetry = telemetryCopyCore0;
            
            //Serial.println("0");
            
            /* ... */
            
            /* We have finished accessing the shared resource.  Release the
             semaphore. */
            xSemaphoreGive( processUI );
        }
        else
        {
            Serial.println("0 FAIL");
            /* We could not obtain the semaphore and can therefore not access
             the shared resource safely. */
        }
    }
    
}
