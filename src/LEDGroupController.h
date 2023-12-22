// LEDGroupController.h
#ifndef LEDGroupController_h

#define LEDGroupController_h

#include <NeoPixelBus.h>

enum Animations {
  DIRECT,
  SWEEP,
  FADE,
  BREATHE
};

class LEDGroupController {
public:
  LEDGroupController(const char *name, const uint8_t *ledIndexes, uint8_t numLeds, NeoPixelBus<NeoGrbwFeature, NeoEsp8266Uart1800KbpsMethod> &strip);

  // Call this function in the Arduino loop to update the animation
  void update();
  // bool transitionComplete();
  void setGroupColor(RgbwColor color, Animations transition);
  bool transitionComplete();
  void startBreathe();

private:
  const uint8_t *led_indexes;
  const char *name;
  uint8_t num_leds;
  NeoPixelBus<NeoGrbwFeature, NeoEsp8266Uart1800KbpsMethod> &strip;
  RgbwColor target_color;
  Animations curr_animation;
  bool transition_complete;
  RgbwColor group_color;
  RgbwColor last_group_color;
  uint16_t transition_duration;
  unsigned long transition_start;
  uint16_t breathe_duration;
  uint8_t breathe_depth;

  void updateDirectAnimation();
  void updateFadeAnimation();
  void updateBreatheAnimation();
  
};

#endif // LED_GROUP_CONTROLLER_H
