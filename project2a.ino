#include "lcdhelper.h"
#include "irhelper.h"
#include <Servo.h>
#include <Wire.h>
//create 2 servo objects
Servo panServo;
Servo tiltServo;
//create oLCD and oIR instances
lcdhelper oLCD(ILI9163_4L, 3, 2, 9, 10, 7);
irhelper oIR;
//set pin numbers
const int dig18 = 18; //D0 pin of opto left motor
const int dig19 = 19; //D0 pin of opto right motor
const int dig20 = 20; //SDA pin of temp/hum sensor
const int dig21 = 21; //SCL pin of temp/hum sensor
const int dig22 = 22; //One side of push button
const int dig28 = 28; //ECHO pin of untrasonic
const int dig29 = 29; //TRIG pin of ultrasonic
const int dig31 = 31; //signal pin to PAN servo
const int dig32 = 32; //signal pin to TILT servo
const int dig45 = 45; //gate on MOSFET left motor
const int dig46 = 46; //gate on MOSFET right motor
unsigned long int last_key_processed = KEY_NONE;
//function to clear the screen
void ClearScreen()
{
    oLCD.setBackColor(VGA_PURPLE);
    oLCD.fillScr(VGA_PURPLE);
    int x = oLCD.getDisplayXSize();
    int y = oLCD.getDisplayYSize();
    oLCD.drawRoundRect(5, 5, x - 5, y - 5);
}
void BlankScreen()
{
    oLCD.fillScr(VGA_PURPLE);
}
//function to bring up the main menu
void ShowMainMenu(screen val, char optionstate, char keypressed)

{
    char text[20];
    //ClearScreen();
    sprintf(text, "SELECTION OPTION");
    oLCD.print(text, 15, 15);
    sprintf(text, "1. Radar Sweep");
    oLCD.print(text, 15, 30);
    sprintf(text, "2. Temp/Humidity");
    oLCD.print(text, 15, 45);
    sprintf(text, "3. PID Control");
    oLCD.print(text, 15, 60);
}
void option1_screen_text(int a, int d)
{
    oLCD.setColor(VGA_WHITE);
    char text[20];
    oLCD.print("  ANGLE "+(String)a+" DEGREES  ", CENTER, 15);
    oLCD.print("  DISTANCE "+(String)d+" CM  ", CENTER, 30);
}
void draw_radar()
{   
    //0,0 it top left corner
    oLCD.setColor(VGA_WHITE);
    oLCD.fillScr(VGA_PURPLE);
    int width = oLCD.getDisplayXSize();
    int height = oLCD.getDisplayYSize();
    int r = width/2-4;

    //middle line
    oLCD.drawLine(width/2,height, width/2,height-(width/2)+3);
    //150 deg from x+
    oLCD.drawLine(width/2,height, (width/2) - (r*cos((30*PI)/180)),height-(r*sin((30*PI)/180)-1));
    //120 deg from x+
    oLCD.drawLine(width/2,height, (width/2) - (r*cos((60*PI)/180)),height-(r*sin((60*PI)/180)-1));
    //60 deg from x+
    oLCD.drawLine(width/2,height, (width/2) + (r*cos((60*PI)/180)),height-(r*sin((60*PI)/180)-1));
    //30 deg from x+
    oLCD.drawLine(width/2,height, (width/2) + (r*cos((30*PI)/180)),height-(r*sin((30*PI)/180)-1));
    
    oLCD.drawCircle(width/2,height-1,r);
    oLCD.drawCircle(width/2,height,r/2);
    
}



void option2_screen(float temp, float humid)
{
    char text[20];
    sprintf(text, "TEMPERATURE (C):");
    oLCD.print(text, CENTER, 15);
    oLCD.print((String)temp, CENTER, 30);
    sprintf(text, "HUMIDITY (%%)");
    oLCD.print(text, CENTER, 45);
    oLCD.print((String)humid, CENTER, 60);
}

void InitializePWM()
{
    TCCR5A = 0b00000001;
    TCCR5B = 0b00000011;
}

// Variables used for the duration and distance
//long duration;
//int distance;

// Function for calculating the distance measured by the Ultrasonic sensor
int calculateDistance(int tPin, int ePin)
{
    digitalWrite(tPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(tPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(tPin, LOW);
    double duration = pulseIn(ePin, HIGH); 
    // Reads the echoPin, returns the sound wave travel time in microseconds
    int distance = duration * 0.034 / 2;
    if (distance>30)
    {
        distance =30;
    }
    //Serial.print("distance: ");
    //Serial.println(distance);
    //Serial.print("duration: ");
    //Serial.println(duration);
    
    return distance;
}
// function to draw sweep lines
void radarOutput(int i, int d)
{
    int width = oLCD.getDisplayXSize();
    int height = oLCD.getDisplayYSize();
    int r = width/2-4;
    float l = (float(d)/30)*r;
    if(i==30||i==60||i==90||i==120||i==150)
    {
        oLCD.setColor(VGA_WHITE);
    }
    else
    {
        oLCD.setColor(VGA_BLUE);
    }
    float endx_blue = (width/2) + (l*cos((i*PI)/180)); //sets the end point of the blue line
    float endy_blue = height-(l*sin((i*PI)/180)-1); //sets the end point of the blue line
    float endx_red = (width/2) + (r*cos((i*PI)/180));
    float endy_red = height-(r*sin((i*PI)/180)-1);
    oLCD.drawLine(width/2,height, endx_blue, endy_blue);
    //Logic to keep the radar circles w/o redrawing them
    Serial.println(int(l));
    if(int(l)!=r)
    {
        if(i==30||i==60||i==90||i==120||i==150)
        {
            oLCD.setColor(VGA_WHITE);
        }
        else
        {
            oLCD.setColor(VGA_RED);  
        }
        oLCD.drawLine(endx_blue,endy_blue, endx_red, endy_red);
    }
    
    oLCD.setColor(VGA_WHITE);
    oLCD.drawPixel((width/2) + int((r/2*cos((i*PI)/180))),height-((float(r/2)*sin((i*PI)/180)-1)));
    


}
// Function to rotate sensor and read distance to print on LCD
void option1()
{   
    int distance = calculateDistance(dig29, dig28);
    // rotates the servo motor from 15 to 165 degrees
    for (int i = 15; i <= 165; i++)
    {
        panServo.write(i);
        delay(30);
        distance = calculateDistance(dig29,dig28);
        option1_screen_text(i,distance);
        radarOutput(i,distance);
        //implement radar display here
    }
    // Repeats the previous lines from 165 to 15 degrees
    while (oIR.GetKeyPressed() != KEY_RETURN)
    {
        delay(10);
        Serial.println("waiting");
    }
    for (int i = 165; i > 15; i--)
    {
        panServo.write(i);
        delay(30);
        distance = calculateDistance(dig29,dig28);
        option1_screen_text(i,distance);
        radarOutput(i,distance);
    }
    ClearScreen();
    ShowMainMenu(SC_MAIN, ' ', ' ');
}

// Insert step 9 here
void setup()
{
    Serial.begin(115200);
    InitializePWM();
    //set servo pins
    panServo.attach(dig31);
    tiltServo.attach(dig32);
    //set starting angles at 15 and 25 degrees
    panServo.write(15);
    tiltServo.write(25);
    
    //setting output pins
    pinMode(dig29, OUTPUT);
    pinMode(dig45, OUTPUT);
    pinMode(dig46, OUTPUT);
    //setting input pins
    pinMode(dig28, INPUT);
    pinMode(dig31, OUTPUT);//<---these need to be out put IDK why instructions say input
    pinMode(dig32, OUTPUT);//<---
    //setting pulled high pins
    pinMode(dig18, INPUT_PULLUP);
    pinMode(dig19, INPUT_PULLUP);
    pinMode(dig22, INPUT_PULLUP);
    
    
    
    oLCD.LCDInitialize(LANDSCAPE);
    oIR.IRInitialize();
    ShowMainMenu(SC_MAIN, ' ', ' ');
}

void loop()
{
    last_key_processed = oIR.GetKeyPressed();
    //last_key_processed = KEY_NONE;
    if (last_key_processed == KEY_1)
    {
        Serial.println(panServo.read());
        Serial.println(tiltServo.read());
        last_key_processed = KEY_NONE;
        //option1_screen_text is designed to be in a loop and just update the numbers
        draw_radar();
        option1();
        delay(10000);
    }
    else if(last_key_processed == KEY_2)
        {
            ClearScreen();
            last_key_processed = KEY_NONE;
            for(int i = 0;i<=5;i++)
            {
                option2_screen(i,i+1);
                delay(500);
            }
            
        }
    else if (last_key_processed == KEY_3)
        {
            last_key_processed = KEY_NONE;
        }
}
