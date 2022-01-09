#include <Wire.h>
#include <HCSR04.h>
#include <Servo.h>

/*-------------------------------------------------------------------------------------------

  PIN DEFINE

-------------------------------------------------------------------------------------------*/
Servo usServoObj;  // create servo object to control a servo
int usServo=9;
int usInput=A1;
int usInputoutput=A0;
UltraSonicDistanceSensor uDS(A0, A1);

#define TSLs 8 //input pin of left sensor
#define TSMs 11 //input pin of middle sensor
#define TSRs 12 //input pin of right sensor

#define Lpwm_pin  6 //pin of controlling speed
#define Rpwm_pin  5 //pin of controlling speed

int pinRF=2;            //IN1 of motor driver board
int pinRB=3;            //IN2 of motor driver board
int pinLF=4;            //IN3 of motor driver board
int pinLB=7;            //IN4 of motor driver board

/*-------------------------------------------------------------------------------------------

  VARIABLES

-------------------------------------------------------------------------------------------*/
int speedL = 80;
int speedR = 80;

int defaultSpeedL = 80;
int defaultSpeedR = 80;

int countL = 0;
int countR = 0;

int angleDefault = 70;
int angleMin = 0;
int angleMax = 170;

bool TSRv = false;
bool TSLv = false;

bool TSRold = false;
bool TSLold = false;

bool TSRchanged = false;
bool TSLchanged = false;

bool scanned = false;
int scanCount= 0;

unsigned int USv;
int start;

/*-------------------------------------------------------------------------------------------

  MAIN CODE

-------------------------------------------------------------------------------------------*/
void setSpeed()
{
  analogWrite(Lpwm_pin, speedL);
  analogWrite(Rpwm_pin, speedR);
}

void InitMotor()
{
  setSpeed();
  pinMode(pinLB,OUTPUT);
  pinMode(pinLF,OUTPUT);
  pinMode(pinRB,OUTPUT);
  pinMode(pinRF,OUTPUT);
  pinMode(Lpwm_pin,OUTPUT);
  pinMode(Rpwm_pin,OUTPUT);
  turnStop();
}

void InitTS()
{
  pinMode(TSLs,INPUT);
  pinMode(TSRs,INPUT);
}

void setup() 
{
  Serial.begin(9600);
  setSpeed();
  InitMotor();
  InitTS();
  usServoObj.attach(9);
  usServoObj.write(angleDefault);
  while(start != 49)
  {
    start = Serial.read();
  }
  scan();
}

void scan()
{
  usServoObj.write(0);
  delay(100);
  for(int j = angleMin; j <= angleMax; j += 2)
  {
    usServoObj.write(j); 
    readUDS();
    delay(250);
    Serial.print(USv);
    Serial.print(",");
  }
  Serial.print(";");
  scanCount++;
  scanned = true;
  delay(500);
  usServoObj.write(angleDefault);
  delay(500);
}

void readTS()
{
  TSLold = TSLv;
  TSRold = TSRv;
  TSLv = digitalRead(TSLs);
  TSRv = digitalRead(TSRs);
  TSRchanged = TSRold != TSRv;
  TSLchanged = TSLold != TSLv;
  if(TSLchanged && TSLv)
  {
    countL++;
    scanned = false;
  }
  if(TSRchanged && TSRv)
  {
    countR++;
    scanned = false;
  }
}

void readUDS()
{
  USv = uDS.measureDistanceCm();
}


void turnForward()
{
  digitalWrite(pinLF,HIGH);
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,LOW);
  digitalWrite(pinRB,LOW);
  speedL = defaultSpeedL;
  speedR = defaultSpeedR;
  setSpeed();
}

void turnStop()
{
  digitalWrite(pinLF,LOW);
  digitalWrite(pinRF,LOW);
  digitalWrite(pinLB,LOW);
  digitalWrite(pinRB,LOW);
  speedL = 0;
  speedR = 0;
  setSpeed();
}

void loop()
{
  readTS();
  turnForward();
  if(!scanned && scanCount < 11 && TSLv && TSRv && countR > 0 && countL > 0 && (countL % 2 == 0))
  {
    turnStop();
    scan();
  }
}
