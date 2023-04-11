#include "lcdhelper.h"
#include "irhelper.h"
#include <Servo.h>
#include <Wire.h>

void ShowMainMenu(screen val, char optionstate, char keypressed)

{

    char text[20];

    ClearScreen();

    sprintf(text, "SELECTION OPTION");

    oLCD.print(text, 15, 15);

    sprintf(text, "1. Radar Sweep");

    oLCD.print(text, 15, 30);

    sprintf(text, "2. Temp/Humidity");

    oLCD.print(text, 15, 45);

    sprintf(text, "3. PID Control");

    oLCD.print(text, 15, 60);
}

void InitializePWM(){
    TCCR5A = 0b00000001;
    TCCR5B = 0b00000011;
}
// Insert step 9 here
void setup(){
Serial.begin(115200);

//insert step 11 here
const int dig18 = 18;
const int dig19 = 19;
const int dig22 = 22;
const int dig28 = 28;
const int dig29 = 29;
const int dig31 = 31;
const int dig32 = 32;
const int dig45 = 45;
const int dig46 = 46;

//setting output pins
pinMode(dig29,OUTPUT);
pinMode(dig45,OUTPUT);
pinMode(dig46,OUTPUT);
//setting input pins
pinMode(dig28,OUTPUT);
pinMode(dig31,OUTPUT);
pinMode(dig32,OUTPUT);
//setting pulled high pins
pinMode(dig18, INPUT_PULLUP);
pinMode(dig19,INPUT_PULLUP);
pinMode(dig22,INPUT_PULLUP);
//Initializing servos
Servo servo1;
Servo servo2;
lcdhelper oLCD(ILI9163_4L, 3, 2, 9, 10, 7);
irhelper oIR;
unsigned long int last_key_processed;
oLCD.LCDInitialize(LANDSCAPE);
oIR.IRInitialize();
ShowMainMenu(SC_MAIN, ' ', ' ');
}
void loop()
{


}
