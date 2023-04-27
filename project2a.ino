#include "lcdhelper.h"
#include "irhelper.h"
#include <Servo.h>
#include <Wire.h>
//for a .3 interval: kp=.39,tu=.468, Ki=1 

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
const double Ki= .307;
const double Kp= 0.2556;
const double Ku= 0.8667;
const double Tu= 0.468;
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
//
// Routine to Plot data on LCD
//
void PlotBody(double x=0, double y=0, double xmin=0, double xmax=0, double ymin=0, double ymax=0, bool reset=true, bool resetstatic=false)
{
    static double lx;
    static double ly;

    if (resetstatic == true){
        lx=x;
        ly=y;
    }
    else
    {
        // Adjust upper and lower bounds if they are equal
        if (abs(xmax-xmin) < 0.001){
            xmin = 0.5*xmin;
            xmax = 1.5*xmin;
        }
        // Adjust upper and lower bounds if they are equal
        if (abs(ymax-ymin) < 0.001){
            ymin = 0.5*ymin;
            ymax = 1.5*ymin;
        }
        // Get screem coordinates.
        int xLCD = oLCD.getDisplayXSize();
        int yLCD = oLCD.getDisplayYSize();
        double xLCDrange = xLCD - 20;
        double yLCDrange = yLCD - 30;
        if (reset == true){
            oLCD.setColor(VGA_PURPLE);
            oLCD.fillRect(16, 16, xLCD - 6, yLCD - 16);
        }
        oLCD.setColor(VGA_WHITE);
        oLCD.setBrightness(0);
        
        // Calculate the x and y range of data.
        double xrange = xmax - xmin;
        double yrange = ymax - ymin;
        //Plot line between point and prior point
        oLCD.drawLine(((lx-xmin)/xrange)*(xLCDrange)+15,((ymax-ly)/yrange)*(yLCDrange)+15,
                        ((x-xmin)/xrange)*(xLCDrange)+15,((ymax-y)/yrange)*(yLCDrange)+15);
        // Plot point at current location
        oLCD.drawPixel(((x-xmin)/xrange)*(xLCDrange)+15,((ymax-y)/yrange)*(yLCDrange)+15);
        lx = x;
        ly = y;  
    }
}
//
// Routine to Plot data on LCD
//
void PlotHeader()
{
    oLCD.InitLCD(LANDSCAPE);
    oLCD.clrScr();
    oLCD.setBackColor(VGA_PURPLE);
    oLCD.fillScr(VGA_PURPLE);
    oLCD.setFont(SmallFont);
    oLCD.print(F("MOTOR CONTROL"),CENTER,2);
    oLCD.print(F("time (s)"),CENTER,115);
    oLCD.print(F("speed (rpm)"),0,115, 270);
    // Get screem coordinates.
    int xLCD = oLCD.getDisplayXSize();
    int yLCD = oLCD.getDisplayYSize();
    double xLCDrange = xLCD - 20;
    double yLCDrange = yLCD - 30;
    //  Draw rectangular border to screen.
    oLCD.drawRoundRect(15, 15, xLCD - 5, yLCD - 15);
    oLCD.setColor(VGA_WHITE);
    oLCD.setBrightness(0);
    PlotBody(0, 0, 0, 1, 0, 1, false, true);
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

bool ReadAndDisplayData()
{
    //Start communication with sensor and then end it to
    // wake up sensor so it will ready to provide data
    Wire.beginTransmission(0x5C);
    Wire.endTransmission();

    //Start communication, request 4 bytes from 0x03 starting at 0x00
    Wire.beginTransmission(0x5C);
    Wire.write(0x03);
    Wire.write(0x00);
    Wire.write(4);

    //End the transmission and delay 1500 microsecond
    Wire.endTransmission(true);
    delay(1500);

    //Request 8 bytes of data from sensor over I2C bus
    Wire.requestFrom(0x5C, 8);

    //Creates bytes array, populates using Wire.read
    byte b[8];
    for (int i = 0; i < 8; i++)
    {
        b[i] = Wire.read();
    }
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
void pi_control(int setpt, int interval)
{
    PlotHeader();
    double xmin = 0;
    double xmax = 20;
    int axis_scale = 1;
    int output =0;
    int new_output = 0;
    int count =1;
    double conversion = 1000000*60;
    unsigned long int start_time = 0;
    double et = 0;
    double rightRPM = 0;
    double leftRPM = 0;
    double integral = 0;
    double deltat = 0;
    double maxRPM = 0;
    while(analogRead(dig22) != LOW)
    {   
        start_time = millis();
        if(count == 1)
        {
            analogWrite(dig46, 90);
            analogWrite(dig45,90);
            delay(100);
            analogWrite(dig45,40);
            delay(500);
            count = count+1;
            Serial.println("still here");
        }
        else
        {
            delay(30);
            double sumR = 0;
            double sumL = 0;
            for(int i = 0; i<5;i++)
            {
                sumR = sumR + pulseIn(dig19,LOW)*2;
                sumL = sumL + pulseIn(dig18,LOW)*2;
            }
            double rightPeriod = (sumR)/5;
            double leftPeriod = (sumL)/5;
            rightRPM = (1/(rightPeriod*20))*conversion;
            leftRPM = (1/(leftPeriod*20))*conversion;
            Serial.print("Rp=");
            Serial.print(rightPeriod);
            Serial.print("Lp=");
            Serial.println(leftPeriod);
            Serial.print("RS=");
            Serial.print(rightRPM);
            Serial.print("LS=");
            Serial.println(leftRPM);
            count = count +1;
            
        }
        while(millis()-start_time < 300)
        {
            delay(1);
        }
        Serial.print("deltat=");
        deltat = (double(millis()-start_time)/1000);
        Serial.println(deltat);
        //et = (rightRPM-leftRPM)-setpt;
        et = setpt-(leftRPM-rightRPM);
        Serial.print("error=");
        Serial.println(et);
        Serial.print("integral=");
        integral = et*deltat;
        Serial.println(integral);
        new_output = (Kp*et)+(Ki*integral);
        //maxRPM = (rightRPM/9)*10;
        //output = output/maxRPM;
        Serial.print("Proportional Term=");
        Serial.print(Kp*et);
        Serial.print(", Integral Term=");
        Serial.print(Ki*integral);
        Serial.print(", Controller Output=");
        output = output + new_output;
        Serial.println(output);
        if(output<40)
        {
            output = 40;
        }
        else if(output>100)
        {
            output = 100;
        }
        analogWrite(dig45,int(output));
        Serial.print("output to motor=");
        Serial.println(output);
        axis_scale = ((millis() - start_time)/20000);
        PlotBody(double(deltat*count), leftRPM, axis_scale*20, (axis_scale+1)*20, 0, 400, true, false);
    }
}

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
    //joins the I2C bus
    Wire.begin();
    
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
            pi_control(0,0.2);//arg1 float: set point in rpm difference between mototrs
            //arg 2 float: integraion constant

        }
}
