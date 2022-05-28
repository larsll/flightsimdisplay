#include "Arduino.h"
#include "state.hh"
#include "display.hh"
#include "lights.hh"
#include "messaging.hh"

//// ------   Spad Coms Section ------ ///////
CmdMessenger messenger(Serial);

bool isReady = false;
bool isConfig = false;
bool isPowerOn = false;
bool isDisplay = false;
unsigned long subscribeTime = 0;
uint8_t subscribeIndex = 0;

#define DEBUG

template <class T>
void sendCmdDebugMsg(uint8_t command, uint8_t idx, T arg)
{
#ifdef DEBUG
    disp.lastCommand(command, idx, static_cast<int32_t>(arg));
    // messenger.sendCmd(kDebug, arg);
#endif
}

// ------------------  C A L L B A C K S -----------------------

// Called when a received command has no attached function
void onUnknownCommand()
{
    uint8_t cmd = messenger.commandID();

#ifdef DEBUG
    String msg = F("Unknown command: ");
    disp.printDebug(msg + cmd);
#endif

    messenger.sendCmd(kDebug, F("UNKNOWN COMMAND")); // if a command comes in that is not reckognized from sketch write to the spad log
}

// Callback function to respond to indentify request. This is part of the
// Auto connection handshake.
void onIdentifyRequest()
{
    char *szRequest = messenger.readStringArg();

    if (strcmp(szRequest, "INIT") == 0)
    { // Initial Configuration declaration

        isReady = false;
        isPowerOn = false;
        isConfig = false;

        uint8_t apiVersion = messenger.readInt32Arg();
        String spadVersion = messenger.readStringArg();
        String spadAuthToken = messenger.readStringArg();

        messenger.sendCmdStart(kRequest);
        messenger.sendCmdArg(F("SPAD"));                                   // Serial Protocol
        messenger.sendCmdArg(F("{7eb4b953-64c6-4c94-a958-1fac034a0370}")); // Device GUID
        messenger.sendCmdArg(F("SimDisplay"));                             // Device Display Name
        messenger.sendCmdArg(2);                                           // SPAD.NEXT Serial Version
        messenger.sendCmdArg(F("0.2"));                                    // FW Version
        messenger.sendCmdArg(F("AUTHOR=1683e5ce90820838a39d0d3990f4c266"));
        messenger.sendCmdArg(F("ALLOWLOCAL=2"));
        messenger.sendCmdArg(F("PID=SIMDISPLAY")); // AUTHOR ID
        messenger.sendCmdEnd();

        return;
    }

    if (strcmp(szRequest, "PING") == 0)
    { // This is a watchdog timer response
        messenger.sendCmdStart(kRequest);
        messenger.sendCmdArg(F("PONG"));
        messenger.sendCmdArg(messenger.readInt32Arg());
        messenger.sendCmdEnd();
        return;
    }
    if (strcmp(szRequest, "CONFIG") == 0)
    {
        disp.printSplash(F("Configuring"));
        isReady = false;
        isConfig = false;
        isPowerOn = false;
        isDisplay = false;
        subscribeIndex = 0;

        messenger.sendCmdStart(kRequest);
        messenger.sendCmdArg("OPTION");
        messenger.sendCmdArg("ISGENERIC=" + String(1));
        messenger.sendCmdArg("PAGESUPPORT=" + String(0));
        messenger.sendCmdArg("OUT_COOLDOWN=" + String(50));
        messenger.sendCmdArg("NO_DISPLAY_CLEAR=" + String(1));
        messenger.sendCmdArg("VPSUPPORT=" + String(1));
        messenger.sendCmdEnd();

        messenger.sendCmdStart(kRequest);
        messenger.sendCmdArg(F("INPUT"));
        messenger.sendCmdArg(F("0"));
        messenger.sendCmdArg(F("CONFIGURE_PANEL_STATUS"));
        messenger.sendCmdArg(F("SYSTEM"));
        messenger.sendCmdArg(F("SPAD_VIRTUAL_POWER"));
        messenger.sendCmdArg(F("UI_TYPE=3"));
        messenger.sendCmdArg(F("CUSTOM_TYPE=POWER"));
        messenger.sendCmdEnd();

        messenger.sendCmdStart(kCommand);
        messenger.sendCmdArg(F("ADD"));
        messenger.sendCmdArg(kMode);
        messenger.sendCmdArg(F("MODE"));
        messenger.sendCmdArg(F("U8,RW,Device Mode"));
        messenger.sendCmdArg(F("Select 1 for Subscription and 0 for Output driven display"));
        messenger.sendCmdEnd();

        // Expose Inputs
        for (int i = 0; i < MSG_INPUTS; i++)
        {
            messenger.sendCmdStart(kRequest);
            messenger.sendCmdArg(F("INPUT"));
            messenger.sendCmdArg(inputs[i].idx);
            messenger.sendCmdArg(inputs[i].name);
            messenger.sendCmdArg(inputs[i].type);
            messenger.sendCmdArg(inputs[i].inherit);
            messenger.sendCmdArg(inputs[i].args);
            messenger.sendCmdEnd();
        }

        // Expose Outputs
        for (int i = 0; i < MSG_OUTPUTS; i++)
        {
            messenger.sendCmdStart(kRequest);
            messenger.sendCmdArg(F("OUTPUT"));
            messenger.sendCmdArg(outputs[i].idx);
            messenger.sendCmdArg(outputs[i].name);
            messenger.sendCmdArg(outputs[i].type);
            messenger.sendCmdArg(outputs[i].inherit);
            messenger.sendCmdArg(outputs[i].args);
            messenger.sendCmdEnd();
        }

        // tell SPAD.neXT we are done with config
        messenger.sendCmd(kRequest, F("CONFIG"));

        isConfig = true;

        return;
    }

    if (strcmp(szRequest, "SCANSTATE") == 0)
    {
        char *str = messenger.readStringArg();

        messenger.sendCmd(kRequest, F("STATESCAN,1"));

        // Provides currently selected Radio
        messenger.sendCmdStart(kInput);
        messenger.sendCmdArg(iSelRadio);
        messenger.sendCmdArg(state.radio);
        messenger.sendCmdEnd();

        // Provides currently selected CRS
        messenger.sendCmdStart(kInput);
        messenger.sendCmdArg(iSelCRS);
        messenger.sendCmdArg(state.crs);
        messenger.sendCmdEnd();

        messenger.sendCmd(kRequest, F("STATESCAN,2"));
        return;
    }
}

void onEvent()
{
    char *szRequest = messenger.readStringArg();

    if (strcmp(szRequest, "VIRTUALPOWER") == 0)
    {
        uint8_t flag = messenger.readInt16Arg();
    }
    else if (strcmp(szRequest, "PROFILECHANGED") == 0)
    {
        char *str = messenger.readStringArg();
        return;
    }
    else if (strcmp(szRequest, "PAGE") == 0)
    {
        char *uid = messenger.readStringArg();
        uint8_t pagenum = messenger.readInt16Arg();
        char *pagename = messenger.readStringArg();
        return;
    }
    else if (strcmp(szRequest, "START") == 0)
    {
        disp.printSplash(F("Starting..."));
        isDisplay = false;
        subscribeTime = millis() + MESSAGING_START_DELAY;
        disp.setActiveRadio(state.radio);

        if (isConfig)
        {
            isPowerOn = true;
        }
    }
}

void onData()
{
    uint8_t dataIdx = messenger.readInt16Arg();
    bool modeSwitch = false;
    int32_t intVal = 0;
    float_t floatVal = 0.0;

    if (messenger.commandID() == kDisplay)
    {
        // Clear next two fields for DISPLAY input.
        messenger.readInt16Arg();
        messenger.readInt16Arg();
    }

    if (dataIdx == dModeAP)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        lights.setAutopilot(modeSwitch);
    }
    else if (dataIdx == dModeFD)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        messenger.sendCmd(kDebug, F("FD Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dModeHDG)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        lights.setHeading(modeSwitch);
    }
    else if (dataIdx == dModeNAV)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        lights.setNavigation(modeSwitch);
    }
    else if (dataIdx == dModeALT)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        lights.setAltitude(modeSwitch);
    }
    else if (dataIdx == dModeIAS)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        messenger.sendCmd(kDebug, F("IAS Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dModeVS)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        lights.setVerticalSpeed(modeSwitch);
        if (!modeSwitch)
            disp.setVerticalSpeed(0);
    }
    else if (dataIdx == dModeAPR)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        lights.setApproach(modeSwitch);
    }
    else if (dataIdx == dModeIAS)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        messenger.sendCmd(kDebug, F("IAS Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dModeREV)
    {
        modeSwitch = (bool)messenger.readInt16Arg();
        messenger.sendCmd(kDebug, F("REV Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dValALT)
    {
        intVal = messenger.readInt32Arg();
        disp.setAltitude(intVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, intVal);
    }
    else if (dataIdx == dValVS)
    {
        intVal = messenger.readInt32Arg();
        disp.setVerticalSpeed(intVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, intVal);
    }
    else if (dataIdx == dValIAS)
    {
        intVal = messenger.readInt32Arg();
        messenger.sendCmd(kDebug, F("IAS Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dValHDG)
    {
        intVal = messenger.readInt32Arg();
        disp.setHeading(intVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, intVal);
    }
    else if (dataIdx == dValCRS)
    {
        intVal = messenger.readInt32Arg();
        disp.setCourse(intVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, intVal);
    }
    else if (dataIdx == dValTXPDR)
    {
        intVal = messenger.readInt32Arg();
        disp.setTransponderCode(intVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, intVal);
    }
    else if (dataIdx == dValRFREQ_A)
    {
        floatVal = messenger.readFloatArg();
        disp.setRadioFrequencyActive(floatVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, floatVal);
    }
    else if (dataIdx == dValRFREQ_S)
    {
        floatVal = messenger.readFloatArg();
        disp.setRadioFrequencyStandby(floatVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, floatVal);
    }
    else if (dataIdx == dValBARO)
    {
        floatVal = messenger.readFloatArg();
        disp.setBarometer(floatVal);
        sendCmdDebugMsg(messenger.commandID(), dataIdx, floatVal);
    }
    else
    {
        messenger.sendCmd(kDebug, "Unknown DATA index " + String(dataIdx) + "."); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
}

void onMode()
{
    state.mode = messenger.readInt16Arg();
#ifdef DEBUG
    disp.printDebug("Mode change: " + String(state.mode));
#endif
}

void onLED()
{

    uint8_t dataIdx = messenger.readInt16Arg();
    bool enable = (bool)messenger.readInt16Arg();
    String tag = messenger.readStringArg();
    String color = messenger.readStringArg();

    if (dataIdx == dModeAP)
    {
        lights.setAutopilot(enable);
    }
    else if (dataIdx == dModeFD)
    {
        messenger.sendCmd(kDebug, F("FD Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dModeHDG)
    {
        lights.setHeading(enable);
    }
    else if (dataIdx == dModeNAV)
    {
        if (color.equals("YELLOW")) {
            LightState ls = LightState();
            ls.color = LightColor::YELLOW;
            if (enable)
                ls.style = LightStyle::BRIGHT;
            else
                ls.style = LightStyle::DIM;
            lights.setNavigation(ls);
        }
        else {
            lights.setNavigation(enable);
        }
    }
    else if (dataIdx == dModeALT)
    {
        lights.setAltitude(enable);
    }
    else if (dataIdx == dModeIAS)
    {
        messenger.sendCmd(kDebug, F("IAS Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dModeVS)
    {
        lights.setVerticalSpeed(enable);
        if (!enable)
            disp.setVerticalSpeed(0);
    }
    else if (dataIdx == dModeAPR)
    {
        lights.setApproach(enable);
    }
    else if (dataIdx == dModeIAS)
    {
        messenger.sendCmd(kDebug, F("IAS Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }
    else if (dataIdx == dModeREV)
    {
        messenger.sendCmd(kDebug, F("REV Mode not enabled")); // Writing the Spad Log that we turned the FD Annunciator ON...
    }

    sendCmdDebugMsg(messenger.commandID(), dataIdx, enable);

}

void subscribeNextData()
{

    if (subscriptions[subscribeIndex].enable)
    {
        // Expose AP Mode ... Mode..
        messenger.sendCmdStart(kCommand);     // This is a "1" or Command:1 from Spad list
        messenger.sendCmdArg(F("SUBSCRIBE")); // Subcommand..ADD - SUBSCRIBE - UNSUBSCRIBE - EMULATE
        messenger.sendCmdArg(subscriptions[subscribeIndex].cmd);
        messenger.sendCmdArg(subscriptions[subscribeIndex].data);
        messenger.sendCmdEnd();

#ifdef DEBUG
        disp.printDebug(String(subscriptions[subscribeIndex].data).substring(0, 24));
#endif
    }
    subscribeIndex++;

    if (subscribeIndex == MSG_SUBSCRIPTIONS)
        isReady = true;
}

// Define callbacks for the different SPAD command sets
void attachCommandCallbacks()
{
    // Attach callback methods
    messenger.attach(onUnknownCommand);
    messenger.attach(kRequest, onIdentifyRequest);
    messenger.attach(kEvent, onEvent);
    messenger.attach(kData, onData);
    messenger.attach(kDisplay, onData);
    messenger.attach(kMode, onMode);
    messenger.attach(kLED, onLED);
}

void updateRadioSource(uint8_t selection)
{

    // Provides currently selected Radio
    messenger.sendCmdStart(kInput);
    messenger.sendCmdArg(iSelRadio);
    messenger.sendCmdArg(selection);
    messenger.sendCmdEnd();

    if (state.mode == 1)
    {
        // Changes the subscriptions
        messenger.sendCmdStart(kCommand);
        messenger.sendCmdArg(F("UNSUBSCRIBE"));
        messenger.sendCmdArg(dValRFREQ_A);
        messenger.sendCmdEnd();

        messenger.sendCmdStart(kCommand);
        messenger.sendCmdArg(F("SUBSCRIBE"));
        messenger.sendCmdArg(dValRFREQ_A);
        messenger.sendCmdArg(nav_subscribe[selection][0]);
        messenger.sendCmdEnd();

        messenger.sendCmdStart(kCommand);       // This is a "1" or Command:1 from Spad list
        messenger.sendCmdArg(F("UNSUBSCRIBE")); // Subcommand..ADD - SUBSCRIBE - UNSUBSCRIBE - EMULATE
        messenger.sendCmdArg(dValRFREQ_S);      // CMDID value defined at the top as "26" this will be the DATA channel
        messenger.sendCmdEnd();

        messenger.sendCmdStart(kCommand);     // This is a "1" or Command:1 from Spad list
        messenger.sendCmdArg(F("SUBSCRIBE")); // Subcommand..ADD - SUBSCRIBE - UNSUBSCRIBE - EMULATE
        messenger.sendCmdArg(dValRFREQ_S);    // CMDID value defined at the top as "26" this will be the DATA channel
        messenger.sendCmdArg(nav_subscribe[selection][1]);
        messenger.sendCmdEnd();
    }
#ifdef DEBUG
    String msg = F("Radio change: ");
    disp.printDebug(msg + selection + "  ");
#endif
}

void updateCourseSource(uint8_t selection)
{
    // Provides currently selected Radio
    messenger.sendCmdStart(kInput);
    messenger.sendCmdArg(iSelCRS);
    messenger.sendCmdArg(selection);
    messenger.sendCmdEnd();

    if (state.mode == 1)
    {
        messenger.sendCmdStart(kCommand);
        messenger.sendCmdArg(F("UNSUBSCRIBE"));
        messenger.sendCmdArg(dValCRS);
        messenger.sendCmdEnd();

        messenger.sendCmdStart(kCommand);     // This is a "1" or Command:1 from Spad list
        messenger.sendCmdArg(F("SUBSCRIBE")); // Subcommand..ADD - SUBSCRIBE - UNSUBSCRIBE - EMULATE
        messenger.sendCmdArg(dValCRS);        // CMDID value defined at the top as "26" this will be the DATA channel
        messenger.sendCmdArg(crs_subscribe[selection][0]);
        messenger.sendCmdEnd();
    }
#ifdef DEBUG
    String msg = F("Course change: ");
    disp.printDebug(msg + selection);
#endif
}
