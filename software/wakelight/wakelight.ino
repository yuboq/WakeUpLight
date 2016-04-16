#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68
/*
   This program is a wake-up light.
   TODO:
   -Get clock and date working on the LCD screen
   -Control the backlight using PWM 7
   -Control the contrast using PWM, pin 6
   -Control the LEDs using PWM using pins 9,10,11
   that should be it.

   TODO:
   create a settings menu (press and hold rotary encoder button)
   Auto off backlight
   birghtness control

*/




//the state variable controls the current state, where:
//state=0 is sleep state
//state=1 is awake state
//state=2 is wake-up LED brightness increase
//state=3 is wake-up LED brightness decrease
//state=4 is menu. Menu will be used when I get python wrapper working :)
int state = 1;
//LCD PINOUTS:
//RS pin to D7
//EN pin to D8
//D4 pin to D9
//D5 pin to D10
//D6 to pin D11
//D7 pin to D12
//Contrast pin to D13
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
int redPin = 4; //pin for red wakeup LED, D4 on arduino
int greenPin = 3; //pin for green wakeup LED, D3 on arduino
int bluePin = 5; //pin for blue wakeup LED, D5 on arduino. Foglight is the same pin.
int contrastPin = 13; // pin for contrast PWM through a LPF, requires 0.81V. D13 on arduino
float brightness = 0;
int alarmSet[] = {0, 53};
int currentRed = 0; //current wake-up LED RED brightness
int currentGreen = 0; // current wake-up LED Green brightness
int currentBlue = 0; // current wake-up LED Blue brightness
int LCDLED = 6;//lcd LED pin. D6 on arduino
byte extHr, extMin, extSec, extWeekday, extDay, extMonth, extYr;

//button variables
bool buttonEvent, leftEvent, rightEvent;

long timeGrab;//used to store time alarm was tripped as reference

//setup flags: active high
bool ishourFormat12, isAlarm, clearLCD;
bool alarmFlag;
String minuteUpdate; //when the minute hand increases, change PWM of LEDs

//function calls goes here
String monthDay() {
  String date;
  date = (day() < 10) ? "0" + String (day()) : String (day());
  switch (month()) {
    case 1:
      return "Jan " + date;
      break;
    case 2:
      return "Feb" + date;
      break;
    case 3:
      return "Mar" + date;
      break;
    case 4:
      return "Apr" + date;
      break;
    case 5:
      return "May" + date;
      break;
    case 6:
      return "Jun" + date;
      break;
    case 7:
      return "Jul " + date;
      break;
    case 8:
      return "Aug" + date;
      break;
    case 9:
      return "Sep" + date;
      break;
    case 10:
      return "Oct" + date;
      break;
    case 11:
      return "Nov" + date;
      break;
    case 12:
      return "Dec" + date;
      break;
    default:
      return "MMM" + date;
  }
}

String weekDay() {
  switch (weekday()) {
    case 1:
      return "  Sun"; //the spaces are used for spacing
      break;
    case 2:
      return "  Mon";
      break;
    case 3:
      return " Tues";
      break;
    case 4:
      return "  Weds";
      break;
    case 5:
      return " Thur";
      break;
    case 6:
      return "  Fri";
      break;
    case 7:
      return "Sat";
      break;
    default:
      return "DDD";
      break;
  }
}



//turns down wakeup LEDs
void lightsDown() {
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}
//turn up wakeup LEDs
void lightsUp() {
  ;
}
//function call for alarm. Produces a brightness gradient from 0 to
//100% brightness over half an hour.
void alarmArmed() {
  //arms the alarm by grabbing the current now() and store it to memory
  if ((hour() == alarmSet[0]) && ((minute()) == alarmSet[1]) && (alarmFlag == 0)) {
    //will repeat gradually increasing brightness if interval is less than 1min
    alarmFlag = 1;
    timeGrab = now();//used to compare for the 30min of brightness increasing
    //Serial.println("current timeGrab is " + String(timeGrab));
  }
}
void alarm(long tripAlarmTime) {
  //ends alarm in 30 minutes from when timeGrab is set. dearms alarm if button is pushed
  //Serial.println("current tripAlarmTime is" + String(tripAlarmTime));
  //Serial.println("current now is " + String(now()));
  if (alarmFlag) {
    float alarmPeriod = 60.0;
    long endAlarm = tripAlarmTime + long(alarmPeriod);
    if (endAlarm >= now()) {
      brightness = (((alarmPeriod - (endAlarm - now())) / alarmPeriod) * 255);
      //Serial.println("the brightness now is " + String(brightness));
      analogWrite(redPin, (int) brightness);
      analogWrite(bluePin, (int) brightness);
      analogWrite(greenPin, (int) brightness);
    }
    else {
      alarmFlag = 0;
    }
  }
}

void printTime() {
  //prints the current time in 12hr or 24hr format
  String PMAM, minuteTime, hourTime;
  if (ishourFormat12) {
    PMAM = isPM() ? "PM" : "AM";
    minuteTime = (minute() < 10) ? "0" + String(minute()) : String(minute());
    hourTime = (hourFormat12() < 10) ? " " + String(hourFormat12()) : String(hourFormat12());
  }
  else {
    hourTime = (hour() < 10) ? "0" + String(hour()) : String(hour());
    minuteTime = (minute() < 10) ? "0" + String(minute()) : String(minute());
    PMAM = "";
  }
  lcd.setCursor(4, 0);
  lcd.print (hourTime + ":" + minuteTime + PMAM);
}

void printDate() {
  lcd.setCursor(0, 1);
  lcd.print (weekDay() + " " + monthDay());
}

void sleepState() {
  printTime();
  printDate();
  analogWrite(LCDLED, LOW);
}

void wakeLEDUp() {
  ;

}
void wakeLEDDown() {
  ;
}
void awakeState() {
  printTime();
  printDate();
  analogWrite(LCDLED, 40);
  lightsDown();
}

//bcdToDec used for reformating time received from DS3231
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

//read the external time
void readDS3231time(byte *extSec,
                    byte *extMin,
                    byte *extHr,
                    byte *extWeekday,
                    byte *extDay,
                    byte *extMonth,
                    byte *extYr)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *extSec = bcdToDec(Wire.read() & 0x7f);
  *extMin = bcdToDec(Wire.read());
  *extHr = bcdToDec(Wire.read() & 0x3f);
  *extWeekday = bcdToDec(Wire.read());
  *extDay = bcdToDec(Wire.read());
  *extMonth = bcdToDec(Wire.read());
  *extYr = bcdToDec(Wire.read());
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  // initialize serial communication
  Serial.begin(9600);

  readDS3231time(&extSec, &extMin, &extHr, &extWeekday, &extDay, &extMonth, &extYr);
  setTime(extHr, //external hour
          extMin, //ext Minute
          extSec, //external second
          extDay, //external day
          extMonth, //external Month
          (2000 + extYr));

  lcd.clear();
  lcd.begin(16, 2);
  //set led and contrast pins to output
  pinMode (LCDLED, OUTPUT);
  pinMode (redPin, OUTPUT);
  pinMode (bluePin, OUTPUT);
  pinMode (greenPin, OUTPUT);
  pinMode (contrastPin, OUTPUT);
  ishourFormat12 = 1;
  clearLCD = 1;
  analogWrite(contrastPin, 20);
  //alarm variables
  alarmFlag = 0;
  lightsDown();
}

void loop() {
  // put your main code here, to run repeatedly:
  // correct the text so that single digits dispaly as 0+digit

  //alarmArmed();//check if alarm is armed
  //alarm(timeGrab); //The period (in seconds) for increasing brightness
  analogWrite(redPin, 0);
  analogWrite(bluePin, 0);
  analogWrite(greenPin, 0);

  switch (state) {
    case 0:
      sleepState();
      break;
    case 1:
      awakeState();
      break;
    case 2:
      wakeLEDUp();
      break;
    case 3:
      wakeLEDDown();
      break;
    default:
      sleepState();
  }
  delay(500);
}
