#include <ArduinoFake.h>
#include <NeoPixelBus.h>

class MockStrip : NeoPixelBus<NeoGrbwFeature, NeoEsp8266Uart1800KbpsMethod>{

};