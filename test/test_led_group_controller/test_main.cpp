#include <unity.h>
#include <ArduinoFake.h>

//Use #ifdef ESP8266 to determine if we are running on native or Android so we can optionally use setup / loop functions in our tests
//Alternatively can call setup and loop functions manually, with possibly less accurate results.


void setUp(void){
    ArduinoFakeReset();
}

void tearDown(void){

}

void test_junk(void){
    TEST_ASSERT_EQUAL(13, 13);
}

int main(){
    UNITY_BEGIN();

    RUN_TEST(test_junk);
    UNITY_END();

}


void test_setup(){
    delay(2000);
    main();
}

void test_loop(){
    
}