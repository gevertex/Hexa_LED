#ifndef LEDGroupManager_h

#define LEDGroupManager_h ;

#include <cstdint>
#include <NeoPixelBus.h>

#include "LEDGroupController.h"

class LEDGroupManager{
    public:
        LEDGroupManager(LEDGroupController *controllers,  uint8_t num_controllers);
        void setGroupColor(uint8_t group_index, RgbwColor color, Animations transition);
        void loop(unsigned long curr_time_ms);
        bool transitionComplete();
        void startBreathe();
    private:
        LEDGroupController *controllers;
        uint8_t num_controllers;
};

#endif