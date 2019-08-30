#include <genieArduino.h>
Genie genie;
#define RESETLINE 4
#include <Wire.h>
#include <EEPROM.h>
#include "HX711.h"
#define DOUT  A2
#define CLK  A1
#define enable 12
#define on  7

HX711 scale(DOUT, CLK);

const long interval = 15;
unsigned long previousMillis = 0;
unsigned long currentMillis;
int fadeValue = 0;
bool fadeUp = 1;

bool button = 0;
bool calibration = 0;

///////////////////////////////////////////////////////    PINY
int pwm = 0;
int up =  11;
int down = 10;    
int onState = 0;
int ledPin = 9;    // LED connected to digital pin 9

int calibrationMemoryAddress = 0;

float calibration_factor = 20.0; // this calibration factor is adjusted according to my load cell

float units;
float units1;
float unitsMAX = 0.0;
float MPAMAX = 0.0;

int mm2 = 1;
float MPA = 0.0;

float slider = 0.0;

char * predkosc = "";
char * powierzchnia = "";

float roznica = 0.0;
float roznica1 = 0.0;
float roznica2 = 0.0;
float error = 5.0;
int inc = 0;

//PID
float Kr = 1.0;
float Ti = 999999.9;
float Td 0.0;

float r0 = Kr*(1+Tp/(2*Ti)+Td/Tp);
float r1 = Kr*(Tp/(2*Ti)-2*Td/Tp-1);
float r2 = Kr*Td/Tp;

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(on, INPUT_PULLUP);
  pinMode(up, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(down, OUTPUT);
  Serial.begin(200000);
  genie.Begin(Serial); // Use Serial0 for talking to the Genie Library, and to the 4D Systems display
  genie.AttachEventHandler(myGenieEventHandler); // Attach the user function Event Handler for processing events

  pinMode(RESETLINE, OUTPUT); // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(RESETLINE, 1); // Reset the Display via D4
  delay(100);
  digitalWrite(RESETLINE, 0); // unReset the Display via D4
  delay (3500); //let the display start up after the reset (This is important)

  calibration_factor = EEPROM.read(calibrationMemoryAddress);
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  scale.tare();
  genie.WriteStr(0, "PullUp Machine V.08 beta by Bad Caps");
}


void loop()
{

  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////       STAN BATEWRII
  // read the input on analog pin 5:
  int bateria = analogRead(A5); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = bateria * (100.0 / 1023.0); // print out the value you read:
  
  genie.DoEvents(); // This calls the library each loop to process the queued responses from the display

  units1 = units;
  units = scale.get_units(), 5;
  if (units < 0)
  {   
    units = 0.00;
  }                               ////////////////          GET UNITS
  if (units > unitsMAX)         
  {
    unitsMAX = units;
  }
 
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  MPA = units / mm2;
  MPAMAX = unitsMAX / mm2;
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0, units);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, unitsMAX);
  genie.WriteObject(GENIE_OBJ_ISMARTGAUGE, 0, MPAMAX);
  genie.WriteObject(GENIE_OBJ_GAUGE, 0, voltage);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 5, calibration_factor);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 4, units);

  onState = digitalRead(on);

  // check if the switch is pressed.
  if (onState == LOW) 
  {
    // turn motor on:
    digitalWrite(down, LOW);
    analogWrite(up, pwm);
    fadeLed();
  }
  else
  {
    analogWrite(ledPin, 0);
    digitalWrite(up, LOW);
    digitalWrite(down, HIGH);
  }

  if (button == 1)
  {
    digitalWrite(enable, HIGH);
  }
  else
  {
    digitalWrite(enable, LOW);
  }

  if(calibration == 1)
  {
    kalibracja();
  }
  
}

void fadeLed()
{
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    if (fadeValue < 255 && fadeUp == 1)
    {
      fadeValue += 5;
    }
    else
    {
      fadeUp = 0;
    }
    if (fadeValue > 0 && fadeUp == 0)
    {
      fadeValue -= 5;
    }
    else
    {
      fadeUp = 1;
    }
    
    analogWrite(ledPin, fadeValue);
  }
}

void kalibracja()
{
    roznica2 = roznica1;
    roznica1 = roznica;
    roznica = units - slider;
    

    if (abs(roznica) <= error)
    {
        inc += 1;
    }else{
      inc = 0;
    }
    if (inc >= 10000){
        stopCalibration();
    }

    pwm = r2*roznica2+r1*roznica1+r0*roznica+units1;  

    if (pwm > 255){
      pwm = 255;
    }else if (pwm < -255){
      pwm = -255;
    }

    if (pwm > 0){
      digitalWrite(down, LOW);
      analogWrite(up, pwm);
    }else if(pwm < 0){
      digitalWrite(up, LOW);
      analogWrite(down, -pwm);
    }else{
      digitalWrite(up, LOW);
      digitalWrite(down, LOW);
    }

//    if(roznica < -error) /////// SILNIK GORA
//    {
//      digitalWrite(down, LOW);
//      digitalWrite(up, HIGH);
//    }
//    else if(roznica > error) ///////// SILNIK DOL
//    {
//      digitalWrite(down, HIGH);
//      digitalWrite(up, LOW);
//    }
//    else 
//    {
//      digitalWrite(enable, LOW);
//    }   
}

void stopCalibration(void)
{
  button = 0;
  inc = 0;
  calibration = 0;
  genie.WriteObject(GENIE_OBJ_LED, 0, 0);
}

void startCalibration(void)
{
  calibration = 1;
  button = 1;
  genie.WriteObject(GENIE_OBJ_LED, 0, 1);
}

void newMeasure()
{
  MPAMAX = 0;
  unitsMAX = 0;
}

void myGenieEventHandler(void)
{
  genieFrame Event;
  genie.DequeueEvent(&Event); // Remove the next queued event from the buffer, and process it below


  //If the cmd received is from a Reported Object, which occurs if a Read Object (genie.ReadOject) is requested in the main code, reply processed here.
  if (Event.reportObject.cmd == GENIE_REPORT_EVENT)
  {


    if (Event.reportObject.object == GENIE_OBJ_4DBUTTON) ////////////////////////////   STOP
    {
      if (Event.reportObject.index == 4 || Event.reportObject.index == 6)/////////////////////////////////////
      {
        button = 0;
      }
      else if (Event.reportObject.index == 3)/////////////////////////////////////   START
      {
        button = 1;
      }
      else if (Event.reportObject.index == 14)///////////////////////////////////// KALIBRACJA START
      {
        startCalibration();
      }
      else if (Event.reportObject.index == 11)///////////////////////////////////// NOWY POMIAR
      {
        newMeasure();
      }
      else if (Event.reportObject.index == 2)///////////////////////////////////// POMIAR
      {
         genie.WriteStr(2, "100");
         genie.WriteStr(4, powierzchnia);
      }
      else if (Event.reportObject.index == 18)///////////////////////////////////// ZAPISANIE KALIBRACJI
      {
         EEPROM.update(calibrationMemoryAddress, calibration_factor);
      }
      else if (Event.reportObject.index == 17)///////////////////////////////////// KALIBRACJA STOP
      {
         stopCalibration();
      }
      else if (Event.reportObject.index == 15)///////////////////////////////////// WZROST CALIBRATION_FACTOR
      {
         calibration_factor += 1;
         scale.set_scale(calibration_factor); //Adjust to this calibration factor
      }
      else if (Event.reportObject.index == 16)///////////////////////////////////// SPADEK CALIBRATION_FACTOR
      {
         calibration_factor -= 1;
         scale.set_scale(calibration_factor); //Adjust to this calibration factor
      }

    }

   if (Event.reportObject.object == GENIE_OBJ_FORM)
   {
      if (Event.reportObject.index == 2)
      {
         genie.WriteStr(2, predkosc);
         genie.WriteStr(4, powierzchnia);
         newMeasure();
      }
      if (Event.reportObject.index == 0)
      {
         stopCalibration();
      }
   }

    if (Event.reportObject.object == GENIE_OBJ_TRACKBAR)
    {
      if (Event.reportObject.index == 0)
      {
        slider = genie.GetEventData(&Event);
      }
    }


    if (Event.reportObject.object == GENIE_OBJ_WINBUTTON)/////////////////////////// WYBOR TESTU
    {
      if (Event.reportObject.index == 0)
      {
        pwm = 60;
        mm2 = 314;
        predkosc = "0015";
        powierzchnia = "d=020";
        genie.WriteStr(3, "0015N/s d=020");
      }
      if (Event.reportObject.index == 3)
      {
        pwm = 63 ;
        mm2 = 314;
        predkosc = "0045";
        powierzchnia = "d=020";
        genie.WriteStr(3, "0045N/s d=020");
      }

      if (Event.reportObject.index == 6)
      {
        pwm = 65;
        mm2 = 1000;
        predkosc = "0050";
        powierzchnia = "d=036";
        genie.WriteStr(3, "050N/s d=036");
      }
      if (Event.reportObject.index == 1)
      {
        pwm = 90;
        mm2 = 1000;
        predkosc = "0150";
        powierzchnia = "d=036";
        genie.WriteStr(3, "0150N/s d=036");
      }
      if (Event.reportObject.index == 4)
      {
        pwm = 70;
        mm2 = 1963;
        predkosc = "0100";
        powierzchnia = "d=050";
        genie.WriteStr(3, "0100N/s d=050");
      }
      if (Event.reportObject.index == 7)
      {
        pwm = 100;
        mm2 = 1963;
        predkosc = "0200";
        powierzchnia = "d=050";
        genie.WriteStr(3, "0200N/s d=050");
      }
      if (Event.reportObject.index == 11)
      {
        pwm = 140;
        mm2 = 1963;
        predkosc = "0300";
        powierzchnia = "d=050";
        genie.WriteStr(3, "0300N/s d=050");
      }
      if (Event.reportObject.index == 8)
      {
        pwm = 80;
        mm2 = 2500;
        predkosc = "0125";
        powierzchnia = "50x50";
        genie.WriteStr(3, "0125N/s 50x50");
      }
      if (Event.reportObject.index == 2)
      {
        pwm = 120;
        mm2 = 2500;
        predkosc = "0250";
        powierzchnia = "50x50";
        genie.WriteStr(3, "0250N/s 50x50");
      }
      if (Event.reportObject.index == 5)
      {
        pwm = 180;
        mm2 = 2500;
        predkosc = "0375";
        powierzchnia = "50x50";
        genie.WriteStr(3, "0375N/s 50x50");
      }
      if (Event.reportObject.index == 10)
      {
        pwm = 120;
        mm2 = 5027;
        predkosc = "0250";
        powierzchnia = "d=080";
        genie.WriteStr(3, "0250N/s d=080");
      }
      if (Event.reportObject.index == 12)
      {
        pwm = 230;
        mm2 = 5027;
        predkosc = "0750";
        powierzchnia = "d=080";
        genie.WriteStr(3, "0750N/s d=080");
      }
      if (Event.reportObject.index == 9)
      {
        pwm = 200;
        mm2 = 7854;
        predkosc = "0400";
        powierzchnia = "d=100";
        genie.WriteStr(3, "0400N/s d=100");
      }
      if (Event.reportObject.index == 13)
      {
        pwm = 240;
        mm2 = 7854;
        predkosc = "1200";
        powierzchnia = "d=100";
        genie.WriteStr(3, "1200N/s d=100");
      }
      if (Event.reportObject.index == 14)
      {
        pwm = 200;
        mm2 = 10000;
        predkosc = "0500";
        powierzchnia = "100x100";
        genie.WriteStr(3, "0500N/s 100x100");
      }
      if (Event.reportObject.index == 15)
      {
        pwm = 255;
        mm2 = 10000;
        predkosc = "1500";
        powierzchnia = "100x100";
        genie.WriteStr(3, "1500N/s 100x100");
      }
    }
  }
}
