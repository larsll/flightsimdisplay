#ifndef LIGHTS_H
#define LIGHTS_H

#include "Arduino.h"
#include "Adafruit_NeoPixel.h"

#define PIXEL_PIN 14
#define PIXEL_AP 0
#define PIXEL_HDG 1
#define PIXEL_NAV 2
#define PIXEL_ALT 3
#define PIXEL_VS 4
#define PIXEL_APR 5
#define PIXEL_NUM 6

#define LIGHT_BRIGHT 64
#define LIGHT_DIM 4
#define LIGHT_BACKLIGHT 16
#define LIGHT_OFF 0

#define REFRESH_RATE 200

enum LightStyle : uint8_t {
   OFF = LIGHT_OFF,
   DIM = LIGHT_DIM,
   BRIGHT = LIGHT_BRIGHT
};

enum LightColor : uint8_t {
    GREEN,
    YELLOW,
    WHITE
};

struct LightState {
    LightStyle style;
    LightColor color;

    LightState() : style(LightStyle::DIM), color(LightColor::WHITE) {}
    LightState(LightStyle style, LightColor color) : style(style), color(color) {}
    
};

class LightController {
    
    private:
        Adafruit_NeoPixel strip;
        LightState pixelState[6];
        void colorWipe(uint32_t c, uint8_t wait);
        void updatePinToStyle(uint8_t pixel, LightStyle style);
        void updatePinToStyle(uint8_t pixel, LightState state);
        uint32_t convertStateToColor(LightState state);
        unsigned long lastRefresh;
        bool update_due = false;

    public:
        LightController();
        void initLights();
        void update();
        void setAutopilot(LightStyle style);
        void setAutopilot(LightState state);
        void setAutopilot(bool on);
        void setHeading(LightStyle style);
        void setHeading(LightState state);
        void setHeading(bool on);
        void setNavigation(LightState state);
        void setNavigation(LightStyle style);
        void setNavigation(bool on);                
        void setAltitude(LightStyle style);
        void setAltitude(LightState state);
        void setAltitude(bool on);
        void setVerticalSpeed(LightStyle style);
        void setVerticalSpeed(LightState state);
        void setVerticalSpeed(bool on);
        void setApproach(LightStyle style);
        void setApproach(LightState state);
        void setApproach(bool on);        
};

extern LightController lights;

#endif