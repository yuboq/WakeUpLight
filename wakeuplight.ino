/*
 * This program is a wake-up light.
 * TODO:
 * -Get clock and date working on the LCD screen
 * -Control the backlight using PWM 7
 * -Control the contrast using PWM, pin 6
 * -Control the LEDs using PWM using pins 9,10,11
 * that should be it.
 *
 * TODO:
 * create a settings menu (press and hold rotary encoder button)
 * Auto off backlight
 * birghtness control
 *
 */

#include <LiquidCrystal.h>
#include <Time.h>

//the state variable controls the current state, where:
//state=0 is sleep state
//state=1 is awake state
//state=2 is wake-up LED brightness increase
//state=3 is wake-up LED brightness decrease
//state=4 is menu. Menu will be used when I get python wrapper working :)
int state = 1;
LiquidCrystal lcd(12, 8, 4, 5, 6, 7);
int redPin = 9; //pin for red wakeup LED
int greenPin = 10; //pin for green wakeup LED
int bluePin = 11; //pin for blue wakeup LED
int contrastPin = 3; // pin for contrast PWM through a LPF, requires 0.81V
float brightness = 0;
int alarmSet[] = {05, 01};
int currentRed = 0; //current wake-up LED RED brightness
int currentGreen = 0; // current wake-up LED Green brightness
int currentBlue = 0; // current wake-up LED Blue brightness
int LCDLED = 13;//lcd LED pin

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
  Serial.println("current tripAlarmTime is" + String(tripAlarmTime));
  Serial.println("current now is " + String(now()));
  if (alarmFlag) {
    float alarmPeriod = 1800.0;
    long endAlarm = tripAlarmTime + long(alarmPeriod);
    if (endAlarm >= now()) {
      brightness = (((alarmPeriod - (endAlarm - now())) / alarmPeriod) * 255);
      //Serial.println("the brightness now is " + String(brightness));
      analogWrite(9, brightness);
      analogWrite(10, brightness);
      analogWrite(11, brightness);
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


}
void wakeLEDDown() {

}
void awakeState() {
  printTime();
  printDate();
  analogWrite(LCDLED, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  // initialize serial communication
  Serial.begin(9600);
  setTime(22, 59, 55, 29, 7, 2015);
  lcd.clear();
  lcd.begin(16, 2);
  //set led and contrast pins to output
  pinMode (LCDLED, OUTPUT);
  pinMode (redPin, OUTPUT);
  pinMode (bluePin, OUTPUT);
  pinMode (greenPin, OUTPUT);
  ishourFormat12 = 1;
  clearLCD = 1;
  analogWrite(contrastPin, 20);
  //alarm variables
  alarmFlag = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  // correct the text so that single digits dispaly as 0+digit

  alarmArmed();//check if alarm is armed
  alarm(timeGrab); //The period (in seconds) for increasing brightness

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
}
