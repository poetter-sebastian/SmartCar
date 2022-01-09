#include <LiquidCrystal_I2C.h> 
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

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2  lines
/*-------------------------------------------------------------------------------------------

  STATES

-------------------------------------------------------------------------------------------*/
#define WAIT 0
#define DRIVE 1
#define IN_LEFT 2
#define IN_RIGHT 3
/*-------------------------------------------------------------------------------------------

  VARIABLES

-------------------------------------------------------------------------------------------*/
int state = DRIVE;
int stateOld = DRIVE;

const int maxSpeed = 180;
const int minSpeed = 60;

int speedL = 80;
int speedR = 80;

int countL = 0;
int countR = 0;

unsigned long timeStart;
unsigned long timeDiff;
float minThreshold = 30;
float maxThreshold = 150;

bool TSRv = false;
bool TSMv = false;
bool TSLv = false;

bool TSRold = false;
bool TSMold = false;
bool TSLold = false;


unsigned int USv;

void InitLCD()
{
  lcd.init();                     
  lcd.backlight(); 
  lcd.clear();
  lcd.print("Test line 0");   
  lcd.setCursor(0, 1);
  lcd.print("Test line 1");
}

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
}

void InitTS()
{
  pinMode(TSLs,INPUT);
  pinMode(TSMs,INPUT);
  pinMode(TSRs,INPUT);
}

void setup() 
{
  Serial.begin(9600);
  InitLCD();
  setSpeed();
  InitMotor();
  InitTS();
  delay(1500);
  lcd.clear();
  usServoObj.attach(9);
  usServoObj.write(70); //default angle
}

void turnForward()
{
  digitalWrite(pinLF,HIGH);
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,LOW);
  digitalWrite(pinRB,LOW);
}

void turnStop()
{
  digitalWrite(pinLF,LOW);
  digitalWrite(pinRF,LOW);
  digitalWrite(pinLB,LOW);
  digitalWrite(pinRB,LOW);
}

void readUDS()
{
  USv = uDS.measureDistanceCm();
}

void readTS()
{
  TSLold = TSLv;
  TSMold = TSMv;
  TSRold = TSRv;
  TSLv = digitalRead(TSLs);
  TSMv = digitalRead(TSMs);
  TSRv = digitalRead(TSRs);
}

void displayValues()
{
  Serial.print("Ultra sonic sensor : ");
  Serial.println(uDS.measureDistanceCm());
  Serial.print("Tracking sensor left: ");
  Serial.println(TSLv);
  Serial.print("Tracking sensor middle: ");
  Serial.println(TSMv);
  Serial.print("Tracking sensor right: ");
  Serial.println(TSRv);
}

void displayOnLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(speedL);
  lcd.print(" ");
  lcd.print(state);
  lcd.print(" ");
  lcd.print(speedR);
  lcd.setCursor(0, 1);
  lcd.print("lt: ");
  lcd.print(TSLv); 
  lcd.print(TSMv);
  lcd.print(TSRv);
}

void setSpeedL(int speed)
{
  if(speed < minSpeed)
  {
    speedL = minSpeed;
  }
  else if(speed > maxSpeed)
  {
    speedL = maxSpeed;
  }
  else
  {
    speedL = speed;
  }
  setSpeed();
}

void setSpeedR(int speed)
{
  if(speedR+speed < minSpeed)
  {
    speedR = minSpeed;
  }
  else if(speed > maxSpeed)
  {
    speedR = maxSpeed;
  }
  else
  {
    speedR = speed;
  }
  setSpeed();
}

void usScan()
{ 
  readUDS();
  delay(3000);
}

void handleLeft()
{
  if(TSRv)
  {
    timeDiff = millis()-timeStart;
    if(timeDiff < minThreshold)
    {
      Serial.print("under threshold:");
      Serial.println(timeDiff);
      setSpeedR(80);
      timeDiff = 0;
      timeStart = 0;
      state = DRIVE;
    }
    else
    {
      Serial.print("between min/max threshold:");
      Serial.println(timeDiff);
      Serial.print("timeDiff/maxThreshold");
      Serial.println(timeDiff/maxThreshold);
      Serial.print("80+55*(timeDiff/maxThreshold))");
      Serial.println(80+55*(double)(timeDiff/maxThreshold));
      setSpeedR((int)(80+55*(double)(timeDiff/maxThreshold)));
      timeDiff = 0;
      timeStart = 0;
      state = DRIVE;
    }
  }
  else if(millis()-timeStart > maxThreshold)
  {
    Serial.println("below min/max threshold");
    setSpeedR(maxSpeed);
    timeDiff = 0;
    timeStart = 0;
    state = DRIVE;
  }
}

void handleRight()
{
  if(TSLv)
  {
    timeDiff = millis()-timeStart;
    
    if(timeDiff < minThreshold)
    {
      Serial.print("under threshold:");
      Serial.println(timeDiff);
      setSpeedL(80);
      timeDiff = 0;
      timeStart = 0;
      state = DRIVE;
    }
    else
    {
      Serial.print("between min/max threshold:");
      Serial.println(timeDiff);
      Serial.print("timeDiff/maxThreshold");
      Serial.println(timeDiff/maxThreshold);
      Serial.print("100+55*(timeDiff/maxThreshold))");
      Serial.println(100+55*(double)(timeDiff/maxThreshold));
      setSpeedL((int)(110+60*(double)(timeDiff/maxThreshold)));
      timeDiff = 0;
      timeStart = 0;
      state = DRIVE;
    }
  }
  else if(millis()-timeStart > maxThreshold)
  {
    Serial.println("below min/max threshold");
    setSpeedL(maxSpeed);
    timeDiff = 0;
    timeStart = 0;
    state = DRIVE;
  }
}

void stateM()
{
  stateOld = state;
  readTS();
  switch (state) 
  {
    case DRIVE:
      setSpeed();
      turnForward();
      if(TSLv == TSRv && TSLv == TSMv)
      {
        state = DRIVE;
      }
      else
      {
        if(TSLv != TSLold || TSLv != TSMv)
        {
          state = IN_LEFT;
          setSpeedL(80);
          timeStart = millis();
        }
        else if(TSRv != TSRold  || TSRv != TSMv)
        {
          state = IN_RIGHT;
          setSpeedR(80);
          timeStart = millis();
        }
      }
      break;
    case IN_LEFT:
      handleLeft();
      break;
    case IN_RIGHT:
      handleRight();
      break;
  }
}

void loop() 
{
  //displayValues();
  displayOnLCD();
  stateM();
}
