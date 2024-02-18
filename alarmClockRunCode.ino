#include <PulseSensorPlayground.h>
#include "Grove_Temperature_And_Humidity_Sensor.h"
#include "SevSeg.h"

#define DHTTYPE DHT11
#define DHTPIN 2

SevSeg sevseg;
PulseSensorPlayground pulseSensor;
DHT dht(DHTPIN, DHTTYPE);

bool volumeIsHighest = false;
const int buttonPin = 2;
int buttonState = 0;
const int pinAdc = A0;
int currentHour = 3;
int currentMinute = 6;
int currentSeconds = 40 + 15;
int currentTimeInms = 0;
int alarmHour = 3;
int alarmMinute = 7;
bool hasRun = false;
int tempRead;
int universalBPM = 0;

const int PulseWire = 7;
const int LED = LED_BUILTIN;
int Threshold = 550;

#ifdef ARDUINO_ARCH_AVR
#define debug Serial0
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
#define debug SerialUSB
#else
#define debug Serial
#endif

const int buzzerPin = 6;

void setup() {
  SerialUSB.begin(115200);

  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(LED);
  pulseSensor.setThreshold(Threshold);

  if (pulseSensor.begin()) {
    SerialUSB.println("We created a pulseSensor Object !");
  }

  pinMode(buzzerPin, OUTPUT);
  debug.begin(115200);
  debug.println("DHTxx test!");
  Wire.begin();
  dht.begin();

  byte numDigits = 4;
  byte digitPins[] = {22, 23, 24, 25};
  byte segmentPins[] = {26, 27, 28, 29, 30, 31, 32, 33};
  bool resistorsOnSegments = 0;
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins,
               resistorsOnSegments);
  sevseg.setBrightness(60);
}

float getTemp() {
  float temp_hum_val[2] = {0};
  if (!dht.readTempAndHumidity(temp_hum_val)) {
    debug.print("Humidity: ");
    debug.print(temp_hum_val[0]);
    debug.print(" %\t");
    debug.print("Temperature: ");
    debug.print(temp_hum_val[1]);
    debug.println(" *C");
    hasRun = true;
    return (float)temp_hum_val[1];
  } else {
    debug.println("Failed to get temperature and humidity value.");
    hasRun = true;
  }
}

long readSound() {
  long sum = 0;
  for (int i = 0; i < 32; i++) {
    sum += analogRead(pinAdc);
  }

  sum >>= 5;
  debug.println("Your sound level is: ");
  debug.println(sum);
  return sum;
}

void loop() {
  if (pulseSensor.sawStartOfBeat()) {
    int myBPM = pulseSensor.getBeatsPerMinute();
    universalBPM = myBPM;
    SerialUSB.println("♥  A HeartBeat Happened ! ");
    SerialUSB.print("BPM: ");
    SerialUSB.println(myBPM);
  }

  if (debug.available() > 0) {
    int number = debug.parseInt();
    debug.println("You entered: ");
    debug.println(number);
    int volume = readSound();
    if ((int)number == (int)tempRead && universalBPM > 60 && volume > 900) {
      digitalWrite(buzzerPin, LOW);
    } else {
      number = -200;
    }
  }
  currentTimeInms = (currentHour * 3600 * 1000) + (currentMinute * 60 * 1000) +
                    (currentSeconds * 1000);
  unsigned long currentMillis = millis() + currentTimeInms;
  int hour = (currentMillis / 1000 / 3600) % 24;
  int minute = ((currentMillis / 1000) / 60) % 60;
  int digit1 = hour / 10;
  int digit2 = hour % 10;
  int digit3 = minute / 10;
  int digit4 = minute % 10;

  char charDigit1 = '0' + digit1;
  char charDigit2 = '0' + digit2;
  char charDigit3 = '0' + digit3;
  char charDigit4 = '0' + digit4;
  char arr[] = {charDigit1, charDigit2, charDigit3, charDigit4};
  sevseg.setChars(arr);
  sevseg.refreshDisplay();

  if (hour == alarmHour && minute == alarmMinute) {
    if (!hasRun) {
      digitalWrite(buzzerPin, HIGH);
      tempRead = getTemp();
    }
  }
}
