

const int buttonPin = 21;
// variables will change:
int buttonState = 0; // variable for reading the pushbutton status
bool buttonPressed = false;
unsigned long buttonPressIgnoreTillTime = 200;
const unsigned long buttonPressThrottleTimeMs = 200;
int LED_BUILTIN = 2;

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT); 
}

void loop()
{
  bool isOn = digitalRead(21);

  if (buttonPressed != isOn & !isOn)
  {
    unsigned long currentTime = millis();
    if (currentTime > buttonPressIgnoreTillTime)
    {
      buttonPressIgnoreTillTime = currentTime + buttonPressThrottleTimeMs;
      Serial.println("bt,1");
    }
  }
  buttonPressed = isOn;
}