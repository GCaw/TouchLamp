/*
board info: https://www.adafruit.com/product/2590?gclid=Cj0KCQjwqrb7BRDlARIsACwGad4omjkSnG2wzD92cPblV0ZUcKddf2i4r0uq-8b_yhhfE1wsU9NSlcMaAtiLEALw_wcB
arduino programming: https://www.arduino.cc/reference/en/
mcu: https://www.microchip.com/wwwproducts/en/atmega328p
*/
 
int ANALOG_MAX = 1023;
int ANLALOG_THRESH = 100;
unsigned long LAST_TOUCH_TOL = 3000000; //us - time between touches

int PIN_TOUCH = A5;       // read voltage to determine if touching or not
int PIN_TOUCH_CHARGE = 2; // pin to toggle charge on RC circuit for touch
int PIN_LED = 13;         // on board diagnostic LED
int PIN_PWM = 6;          // PWM output pin for driving lamp LEDs

unsigned long startTime;
unsigned long endTime;
unsigned long lastTouch = 0;
bool ledToggle = true;
int chargetime = 0;
int counter = 0;
bool light_on = false;
int incrementer = 1;

void setup() {
  analogWrite(PIN_PWM, 0);
  
  Serial.begin(9600);
  analogReference(DEFAULT); // Default for Uno is 5V
  
  pinMode(PIN_TOUCH_CHARGE, OUTPUT);
  digitalWrite(PIN_TOUCH_CHARGE, LOW);
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  
  Calibrate();
}


void loop()
{
    if (BeingTouched())
    {
      if (micros() - lastTouch > LAST_TOUCH_TOL)
      {
        if (counter > 0)
        {
          counter = 0;
          incrementer = 1;
        }
        else
        {
          counter = 7;
        }
      }
      else
      {
        // PWM write value is 0-255.
        // we want to limit max output to the LED drivers, so we max out at 16*5 = 80
        // 80/255 = 31% of max brightness
        counter += incrementer;
        if (counter > 15)
        {
          incrementer = -1;
        }
        if (counter < 1)
        {
          incrementer = 1;
        }
      }
      digitalWrite(PIN_LED, HIGH);
      analogWrite(PIN_PWM, 5*counter);
      lastTouch = micros();
      delay(200); //debounce
      Serial.print("\nCounter: ");
      Serial.print(counter);
    }
    else
    {
      digitalWrite(PIN_LED, LOW);
    }
}

// Determine whether the lamp is being touched or not
bool BeingTouched()
{
  bool touched = false;
  int val = 0;
  startTime = micros(); // overflows after 70min
  digitalWrite(PIN_TOUCH_CHARGE, HIGH);
  while (true)
  {
    val = analogRead(PIN_TOUCH);
    if (val >= ANALOG_MAX - ANLALOG_THRESH)
    {
      touched = false;
      break;
    }
    
    endTime = micros() - startTime;
    if (endTime > chargetime*2)
    {
      Serial.print("\ntime: ");
      Serial.print(endTime);
      Serial.print("\nPushed\n\n");
      touched = true;
      break;
    }
    
  }
  
  digitalWrite(PIN_TOUCH_CHARGE, LOW);
  return touched;
}

// Get a baseline for how long a charge discharge cycle should take.
// Assumes no one is touching it.
bool Calibrate()
{
  int cal_loops = 5;
  for (int i = 0; i < cal_loops; i++)
  {
    startTime = micros(); // overflows after 70min
    int val = 0;
    digitalWrite(PIN_TOUCH_CHARGE, HIGH);
    while (val < ANALOG_MAX - ANLALOG_THRESH)
    {
      val = analogRead(PIN_TOUCH);
    }
    digitalWrite(PIN_TOUCH_CHARGE, LOW);
    while (val > ANLALOG_THRESH)
    {
      val = analogRead(PIN_TOUCH);
    }
    endTime = micros();
    Serial.print("\ncharge: ");
    Serial.print(endTime-startTime);
    chargetime += endTime-startTime;
  }
  
  chargetime = chargetime / cal_loops;
  Serial.print("\ncharge: ");
  Serial.print(chargetime);
}
