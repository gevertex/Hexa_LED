#include "LEDGroupManager.h"
// Why does this work with below commented out?!?!
// #include "LEDGroupController.h"

LEDGroupManager::LEDGroupManager(LEDGroupController *controllers, uint8_t num_controllers) 
: controllers(controllers), num_controllers(num_controllers) {
}

void LEDGroupManager::loop(unsigned long curr_time_ms){
    //Call update function of all controllers
    for (uint8_t i=0; i<num_controllers; i++){
        controllers[i].update(curr_time_ms);
        //TODO: figure out how to get rid of this, there is some kind of race condition with the NeoPixel Driver, or an issue with the hardware setup (power spikes degrading voltage or data signal, etc)
        //Without this delay, some LEDs won't be set correctly.
        //Suspect lack of voltage stabilizing capacitor on 5vv line and resistor on data line are to blame. Need to put on scope
        delay(1);   
    }
}

void LEDGroupManager::setGroupColor(uint8_t group_index, RgbwColor color, Animations transition){
    controllers[group_index].setGroupColor(color, transition);
}

bool LEDGroupManager::transitionComplete(){
    for (uint8_t i=0; i<num_controllers; i++){
        if (!controllers[i].transitionComplete())
            return false;
    }

    return true;
}

void LEDGroupManager::startBreathe(){
    for (uint8_t i=0; i<num_controllers; i++){
        controllers[i].startBreathe();
    }
}