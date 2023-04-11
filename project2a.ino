#include "lcdhelper.h"
#include "irhelper.h"
#include <Servo.h>
#include <Wire.h>
void InitializePWM(){
    TCCR5A = 0b00000001;
    TCCR5B = 0b00000011;
}
// Insert step 9 here
void setup(){
Serial.begin(115200);
}
