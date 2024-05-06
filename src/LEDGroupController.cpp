// LEDGroupController.cpp
#include <cstdint>
#include <NeoPixelBus.h>
#include <algorithm>
#include "LEDGroupController.h"



LEDGroupController::LEDGroupController(const char *name, const uint8_t *ledIndexes, uint8_t numLeds, NeoPixelBus<NeoGrbwFeature, NeoEsp8266Uart1800KbpsMethod> &strip)
  : name(name), led_indexes(ledIndexes), num_leds(numLeds), strip(strip) {

  transition_complete = true;
  transition_duration_ms = 2000;

  //0-255 value
  breathe_depth = 128;
}

uint8_t LEDGroupController::animationProgress(uint8_t scale, unsigned long curr_time_ms){
  uint16_t elapsed_time = std::min((curr_time_ms - transition_start), (unsigned long)(transition_duration_ms));
  return (uint16_t(scale) * elapsed_time) / transition_duration_ms;
}

void LEDGroupController::update(unsigned long curr_time_ms) {

  if(!transition_complete && strip.CanShow()){

    //Set this here so time starts once update runs for the first time
    //Helps avoid situations where blocked thread setting colors takes too long and misses entire animation
    if (transition_start == 0)
      transition_start = curr_time_ms;

    switch (curr_animation){
    case DIRECT:
      updateDirectAnimation();
      break;
    case FADE:
      updateFadeAnimation(curr_time_ms);
      break;
    case BREATHE:
      updateBreatheAnimation(curr_time_ms);
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

void LEDGroupController::updateFadeAnimation(unsigned long curr_time_ms){

  uint8_t progress = animationProgress(255, curr_time_ms);
  RgbwColor color = RgbwColor::LinearBlend(last_group_color, group_color, progress);

  for (uint8_t i = 0; i<num_leds; i++){
    strip.SetPixelColor(led_indexes[i], color);
  }

  if (progress == 255){
    transition_complete = true;
  }
}

bool breathe_out;
void LEDGroupController::updateBreatheAnimation(unsigned long curr_time_ms){

  if (breathe_out){
    uint8_t progress = animationProgress(breathe_depth, curr_time_ms);
    group_color = last_group_color.Dim(255-progress);

    for (uint8_t i = 0; i<num_leds; i++){
      strip.SetPixelColor(led_indexes[i], group_color);
    }

    if (progress == breathe_depth){
      // breathe_out = false;
      transition_start = 0;
    }
  }/*else{
    uint8_t progress = animationProgress(breathe_depth);
    RgbwColor color = group_color.Brighten(255-progress);

    for (uint8_t i = 0; i<num_leds; i++){
      strip.SetPixelColor(led_indexes[i], color);
    }

    if (progress == breathe_depth){
      breathe_out = true;
      transition_start = 0;
    }
  }  */
}

void LEDGroupController::startBreathe(){
  last_group_color = group_color;
  breathe_out = true;
  curr_animation = BREATHE;
  transition_start = 0;
  transition_complete = false;
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