#ifndef MESSAGING_H
#define MESSAGING_H

#include <CmdMessenger.h> // CmdMessenger ... v4.2 was used when making this sketch

#define MESSAGING_DELAY 50
#define MESSAGING_START_DELAY 1000

enum : byte
{
    kRequest = 0,    // Request from SPAD.neXt
    kCommand = 1,    // Command to SPAD.neXt
    kEvent = 2,      // Events from SPAD.neXt
    kDebug = 3,      // Debug strings to SPAD.neXt Logfile
    kSimCommand = 4, // Send Event to Simulation
    kData = 5,       // Send Data to Device
    kLED = 6,        // Update LEDs
    kDisplay = 7,    // Update Display
    kInput = 8,      // Update Input Value from Device
};

enum : byte
{
    dModeAP = 20,     // CMDID for exposed data to SPAD.neXt.  We will see the data later as a Local Variable in Spad.Next
    dModeFD = 21,     //
    dModeHDG = 22,    //
    dModeNAV = 23,    //
    dModeALT = 24,    //
    dModeIAS = 25,    //
    dModeVS = 26,     //
    dModeAPR = 27,    //
    dModeREV = 28,    //
    dValALT = 29,     //
    dValVS = 30,      //
    dValIAS = 31,     //
    dValHDG = 32,     //
    dValCRS = 33,     //
    dValTXPDR = 34,   //
    dValBARO = 35,    //
    dValRFREQ_A = 36, //
    dValRFREQ_S = 37, //
};

enum : byte
{
    iSelRadio = 1,
    iSelCRS = 2,
};

struct Subscription
{
    uint8_t cmd;
    const char *data;
    bool enable;
};

struct InputOutput
{
    uint8_t idx;
    const char *name;
    const char *type;
    const char *inherit;
    const char *args;
};

#define MSG_INPUTS 2
const InputOutput inputs[MSG_INPUTS] PROGMEM = {
    {iSelRadio, "S_RADIO", "ROTARY", "SPAD_ENCODER_NOACC", "POS_NAMES=NAV1#NAV2#COM1#COM2#ADF,POS_VALUES=0#1#2#3#4"},
    {iSelCRS, "S_CRS", "ROTARY", "SPAD_ENCODER_NOACC", "POS_NAMES=CRS1#CRS2#GPS,POS_VALUES=0#1#2"}
};

#define MSG_OUTPUTS 14
const InputOutput outputs[MSG_OUTPUTS] PROGMEM = {
    {dModeAP, "L_AP_MASTER", "LED", "SPAD_LED_3COL", "UI_FACE=1"},
    {dModeNAV, "L_AP_NAV", "LED", "SPAD_LED_3COL", "UI_FACE=1"},
    {dModeHDG, "L_AP_HDG", "LED", "SPAD_LED_3COL", "UI_FACE=1"},
    {dModeALT, "L_AP_ALT", "LED", "SPAD_LED_3COL", "UI_FACE=1"},
    {dModeVS, "L_AP_VS", "LED", "SPAD_LED_3COL", "UI_FACE=1"},
    {dModeAPR, "L_AP_APR", "LED", "SPAD_LED_3COL", "UI_FACE=1"},
    {dValHDG, "D_AP_HDG", "DISPLAY", "SPAD_DISPLAY", "LENGTH=3"},
    {dValCRS, "D_AP_CRS", "DISPLAY", "SPAD_DISPLAY", "LENGTH=3"},
    {dValALT, "D_AP_ALT", "DISPLAY", "SPAD_DISPLAY", "LENGTH=5"},
    {dValVS, "D_AP_VS", "DISPLAY", "SPAD_DISPLAY", "LENGTH=5"},
    {dValTXPDR, "D_XPDR", "DISPLAY", "SPAD_DISPLAY", "LENGTH=4"},
    {dValBARO, "D_BARO", "DISPLAY", "SPAD_DISPLAY", "LENGTH=5"},
    {dValRFREQ_A, "D_RADIO_ACTIVE_FREQ", "DISPLAY", "SPAD_DISPLAY", "LENGTH=7"},
    {dValRFREQ_S, "D_RADIO_STANDBY_FREQ", "DISPLAY", "SPAD_DISPLAY", "LENGTH=7"}
};

#define MSG_COURSES 3
const char *const crs_subscribe[MSG_COURSES][2] PROGMEM = {{"SIMCONNECT:NAV OBS:1", "OBS 1"}, {"SIMCONNECT:NAV OBS:2", "OBS 2"}, {"SIMCONNECT:GPS WP DESIRED TRACK,DEGREES", "GPS DTK"}};

void attachCommandCallbacks();
void updateRadioSource(uint8_t selection);
void updateCourseSource(uint8_t selection);
extern CmdMessenger messenger;
extern bool isPowerOn;
extern bool isConfig;
extern bool isDisplay;
#endif