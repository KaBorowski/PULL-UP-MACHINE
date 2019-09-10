#define RESETLINE 4
#include <Wire.h>
#include <EEPROM.h>
#include "HX711.h"
#define DOUT  A2
#define CLK  A1
#define enable 12
#define on  7

HX711 scale;

const long interval = 15;
unsigned long previousMillis = 0;
unsigned long previousMillisCali = 0;
unsigned long currentMillis;
unsigned long currentMillisCali;

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

float calibration_factor = -200.0; // this calibration factor is adjusted according to my load cell

double units;
double units1;
double unitsMAX = 0.0;
double MPAMAX = 0.0;

int mm2 = 1;
float MPA = 0.0;



char * predkosc = "";
char * powierzchnia = "";

float roznica = 0.0;
float roznica1 = 0.0;
float roznica2 = 0.0;
float error = 50.0;
int inc = 0;

//PID
double Kr = 2.0;
double Ti = 9999.0;
double Td = 0.0;
double Tp = 0.1;
double slider = 6000.0;
double u = 0.0;
const long intervalCali = 100;

double r0;
double r1;
double r2;

void setup()
{
  Serial.begin(9600);
  pinMode(on, INPUT_PULLUP);
  pinMode(up, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(down, OUTPUT);


//  pinMode(RESETLINE, OUTPUT); // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
//  digitalWrite(RESETLINE, 1); // Reset the Display via D4
//  delay(100);
//  digitalWrite(RESETLINE, 0); // unReset the Display via D4
//  delay (3500); //let the display start up after the reset (This is important)

  //  calibration_factor = EEPROM.read(calibrationMemoryAddress);
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  scale.tare();


  r0 = Kr*(1.0+Tp/(2.0*Ti)+Td/Tp);
  r1 = Kr*(Tp/(2.0*Ti)-2.0*Td/Tp-1.0);
  r2 = Kr*Td/Tp;
  Serial.println(r0);
  Serial.println(r1);
  Serial.println(r2);


  digitalWrite(enable, HIGH);
  pwm = 0;

}


void loop()
{


  units = scale.get_units(), 5;
  if (units < 0)
  {
    units = 0.00;
  }                               ////////////////          GET UNITS
  if (units > unitsMAX)
  {
    unitsMAX = units;
  }


  onState = digitalRead(on);

  // check if the switch is pressed.
  if (onState == LOW)
  {
    startCalibration();
  }
  else
  {
    stopCalibration();
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

  currentMillisCali = millis();
  if (currentMillisCali - previousMillisCali >= 0) {
    // save the last time you blinked the LED
    previousMillisCali = currentMillisCali;
    units1 = units;
    if (calibration == 1)
    {
      kalibracja();
    }
  }

}


void kalibracja()
{
  roznica2 = roznica1;
  roznica1 = roznica;
  roznica = slider - units;

  Serial.println(pwm);


  if (abs(roznica) <= error)
  {
    inc += 1;
  } else {
    inc = 0;
  }
  if (inc >= 10000) {
    stopCalibration();
  }

  u = r2 * roznica2 + r1 * roznica1 + r0 * roznica + units1;

  pwm = (int)u;

  if (pwm > 255) {
    pwm = 255;
  } else if (pwm < -255) {
    pwm = -255;
  }

  if (pwm > 0) {
    digitalWrite(down, LOW);
    analogWrite(up, pwm);
  } else if (pwm < 0) {
    digitalWrite(up, LOW);
    analogWrite(down, -pwm);
  } else {
    digitalWrite(up, LOW);
    digitalWrite(down, LOW);
  }
}

void stopCalibration(void)
{
//  button = 0;
  inc = 0;
  calibration = 0;
  pwm = 0;

}

void startCalibration(void)
{
  calibration = 1;
  button = 1;
}
