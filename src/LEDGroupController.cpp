// LEDGroupController.cpp
#include <cstdint>
#include <NeoPixelBus.h>
#include <algorithm>
#include "LEDGroupController.h"



LEDGroupController::LEDGroupController(const char *name, const uint8_t *ledIndexes, uint8_t numLeds, NeoPixelBus<NeoGrbwFeature, NeoEsp8266Uart1800KbpsMethod> &strip)
  : name(name), led_indexes(ledIndexes), num_leds(numLeds), strip(strip) {

  transition_complete = true;
  transition_duration = 1000;
  breathe_duration= 1000;
  breathe_depth = 50;
}

void LEDGroupController::update() {

  if(!transition_complete && strip.CanShow()){

    if (transition_start == 0)
      transition_start = millis();

    switch (curr_animation){
    case DIRECT:
      updateDirectAnimation();
      break;
    case FADE:
      updateFadeAnimation();
      break;
    case BREATHE:
      updateBreatheAnimation();
      break;
    default:
      break;
    }

    strip.Show();
  }

}


void LEDGroupController::updateDirectAnimation(){

  for (uint8_t i = 0; i<num_leds; i++){
    strip.SetPixelColor(led_indexes[i], group_color);
  }
  transition_complete = true;

}

void LEDGroupController::updateFadeAnimation(){

  uint16_t elapsed_time = std::min((millis() - transition_start), (unsigned long)(transition_duration));
  uint8_t progress = (uint16_t(255) * elapsed_time) / transition_duration;
  RgbwColor color = RgbwColor::LinearBlend(last_group_color, group_color, progress);

  for (uint8_t i = 0; i<num_leds; i++){
    strip.SetPixelColor(led_indexes[i], color);
  }

  if (progress == 255){
    transition_complete = true;
  }
}

void LEDGroupController::updateBreatheAnimation(){
  // RgbwColor::Dim();
  
}

void LEDGroupController::startBreathe(){
  transition_complete = false;
  curr_animation = BREATHE;
}


void LEDGroupController::setGroupColor(RgbwColor color, Animations transition){
  last_group_color = group_color;
  group_color = color;
  curr_animation = transition;
  transition_complete = false;
  transition_start = 0;
}

bool LEDGroupController::transitionComplete(){
  return transition_complete;
}