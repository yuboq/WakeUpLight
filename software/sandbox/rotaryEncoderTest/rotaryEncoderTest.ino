int analogPin0 = 0;     //pin for CW rotation of rotary encoder
int analogPin1 = 1;     //pin for CCW rotation of rotary encoder

int readAnalogPin0 = 0; // variable to store the analogpin0 value
int readAnalogPin1 = 0; // variable to store the analogpin1 value
int val = 0;            //current value to output
void setup()
{
  Serial.begin(9600);          //  setup serial
}

void loop()

{  
  readAnalogPin0 = analogRead(analogPin0);  // read the input pin
  readAnalogPin1 = analogRead(analogPin1);  //read the input pin1
  if (readAnalogPin0 <1000)
  {
    val += 1;
  }
  else if (readAnalogPin1 <1000)
  {
    val -=1;
  }
  else
  {
    //do nothing
  }
  Serial.println (val); 

}
