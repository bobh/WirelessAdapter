#ifndef GlobalDefinitions_H_
#define GlobalDefinitions_H_

#include <Arduino.h>

// Global copy of slave
#define NUMSLAVES 20
#define COMMANDLENGTH 100

//_________________________________________________
typedef struct commandStruct
{
    byte macAdr[6];
    int  antenna;
    char name[30]; // not used until the remote needs
                   // to know the antenna name
                   // in addition to the number
};
//_________________________________________________
typedef struct telemetryStruct
{
    int   remoteAntenna;
    float temperature;
};
//_________________________________________________


// ************************************************
// ************************************************
// global structs shared by both core processes
// must be accessed via mutex
commandStruct   command;
telemetryStruct telemetry;

// copy of global structs for local use on core 1
// core 1 is the UI menu process
// commandStruct will be written when a new antenna is
// selected via a menu selection
// telemetryStruct will be read when battery telemetry is
// displayed (not currently implemented)
commandStruct   commandCopyCore0;
commandStruct   commandCopyCore1;
telemetryStruct telemetryCopyCore0;
telemetryStruct telemetryCopyCore1;

// ************************************************
// ************************************************

// these are globals accessed by both
// cores. No access control is used on the
// theory that these are atomic bytes.
bool displayTXmsg = false;
bool displayRXmsg = false;
bool displayERRmsg = false;

String gTemperature;






#endif /* GlobalDefinitions_H_ */
