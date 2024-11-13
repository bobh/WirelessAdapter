/* 
 *  
 *  https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/nvs_flash.html
 *  
*/

#include "GlobalDefinitions.h"
#include "Utilities.h"

#include "nvs_flash.h"
#include "nvs.h"

extern esp_err_t err;
extern nvs_handle my_handle;

extern lastAntenna, nextAntenna;

//initializes NVS and sets lastAntenna
void nonVolatileStorageInit(void)
{
    // Initialize NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) 
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        printf("No Free Pages\n");
        
    }
    ESP_ERROR_CHECK( err );  

    printf("Reading lastAntenna from NVS ... ");
     // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else 
    {
        printf("Done\n");

        // Read
        printf("Reading antenna counter from NVS ... ");
        err = nvs_get_i32(my_handle, "lastAntenna", &lastAntenna);
        switch (err) 
        {
            case ESP_OK:
                printf("Done\n");
                printf("lastAntenna counter = %d\n", lastAntenna);
                setOutputState(lastAntenna); // write saved position to the switch hardware
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                // set default
                lastAntenna = 0;
                digitalWrite(SEL_A, LOW);
                digitalWrite(SEL_B, LOW);
                digitalWrite(SEL_C, LOW);
                // write an initial value to non-volatile memory
                err = nvs_set_i32(my_handle, "lastAntenna", lastAntenna);
                printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
                // Close
                nvs_close(my_handle);           
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
     } 

     // connect the output to switch over ethernet cable
     digitalWrite(EnableOutput, LOW);   
}

// stores nextAntenna to NVS
void syncAntennaState(void)
{

    printf("Reading lastAntenna from NVS ... ");
     // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) 
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else 
    {
        printf("Done\n");

        // Read
        printf("Reading antenna counter from NVS ... ");
        
        err = nvs_get_i32(my_handle, "lastAntenna", &lastAntenna);
        switch (err) 
        {
            case ESP_OK:
                printf("Done\n");
                printf("lastAntenna counter = %d\n", lastAntenna);

                // Write
                printf("Updating last antenna in NVS ... ");

                if( (lastAntenna != nextAntenna) && (nextAntenna >= 0) && (nextAntenna < 8) )
                {
                  lastAntenna = nextAntenna;
         
                  setOutputState(lastAntenna); // write to the switch hardware

                  err = nvs_set_i32(my_handle, "lastAntenna", lastAntenna);
                  printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

                  // Commit written value.
                  // After setting any values, nvs_commit() must be called to ensure changes are written
                  // to flash storage. Implementations may write to storage at other times,
                  // but this is not guaranteed.
                  printf("Committing updates in NVS ... ");
                  err = nvs_commit(my_handle);
                  printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
                  }
                 else
                 {
                   printf("No update in NVS needed and/or request for current antenna ");
                  }
               
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                nextAntenna = lastAntenna;
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
       }

        // Close
        nvs_close(my_handle); 
}

void setOutputState(int32_t antenna)
{
  // disconnect the output to switch over ethernet cable
  digitalWrite(EnableOutput, HIGH);   

  //printf("Setting Output = %d\n", antenna);

  switch(antenna) 
  {
    case 0:
     digitalWrite(SEL_A, LOW);
     digitalWrite(SEL_B, LOW);
     digitalWrite(SEL_C, LOW);
     printf("setting antenna = %d\n", antenna);
     break;
    case 1:
     digitalWrite(SEL_A, HIGH);
     digitalWrite(SEL_B, LOW);
     digitalWrite(SEL_C, LOW);
     printf("setting antenna = %d\n", antenna);
     break;

    case 2:
     digitalWrite(SEL_A, LOW);
     digitalWrite(SEL_B, HIGH);
     digitalWrite(SEL_C, LOW);
     printf("setting antenna = %d\n", antenna);
     break;

    case 3:
     digitalWrite(SEL_A, HIGH);
     digitalWrite(SEL_B, HIGH);
     digitalWrite(SEL_C, LOW);
     printf("setting antenna = %d\n", antenna);
     break;
    case 4:
     digitalWrite(SEL_A, LOW);
     digitalWrite(SEL_B, LOW);
     digitalWrite(SEL_C, HIGH);
     printf("setting antenna = %d\n", antenna);
     break;
    case 5:
     digitalWrite(SEL_A, HIGH);
     digitalWrite(SEL_B, LOW);
     digitalWrite(SEL_C, HIGH);
     printf("setting antenna = %d\n", antenna);
     break;
    case 6:
     digitalWrite(SEL_A, LOW);
     digitalWrite(SEL_B, HIGH);
     digitalWrite(SEL_C, HIGH);
     printf("setting antenna = %d\n", antenna);
     break;
    case 7:
     digitalWrite(SEL_A, HIGH);
     digitalWrite(SEL_B, HIGH);
     digitalWrite(SEL_C, HIGH);
     printf("setting antenna = %d\n", antenna);
     break;
     
    default: 
      printf("Invalid Antenna Value\n");
      digitalWrite(SEL_A, LOW);
      digitalWrite(SEL_B, LOW);
      digitalWrite(SEL_C, LOW);
      break;
    
  }
  // connect the output to switch over ethernet cable
  digitalWrite(EnableOutput, LOW);   
  return;
}
