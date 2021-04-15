#include <ArduinoBLE.h>
#include "SevSeg.h"

BLEService tempService("1101");
BLEUnsignedCharCharacteristic tempLevelChar("2101", BLERead | BLENotify | BLEWrite);
BLEUnsignedCharCharacteristic tempReadChar("2102", BLERead | BLENotify | BLEWrite);
SevSeg sevseg;

unsigned long prevUpdate = 0;
int masterTemp = 72;
const int decrementPin = A5;
const int incrementPin = A6;
int decrementState = 0;
int incrementState = 0;
int displayTemp = 0;
int displayTempVariances[10];
int displayTempPos = 0;
const int coolPin = A1;
const int heatPin = A0;
void setup()
{
  delay(3000);
  pinMode(coolPin, OUTPUT);
  pinMode(heatPin, OUTPUT);
  pinMode(decrementPin, INPUT);
  pinMode(incrementPin, INPUT);
  byte numDigits = 4;
  byte digitPins[] = {12, 11, 8, 6};
  byte segmentPins[] = {10, 7, 4, 3, 2, 9, 5, 13};
  bool resistorsOnSegments = false;
  byte hardwareConfig = COMMON_ANODE;
  bool updateWithDelays = false;
  bool leadingZeros = false;
  bool disableDecPoint = false;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
  }

  BLE.setLocalName("Labrador Thermostat");
  BLE.setAdvertisedService(tempService);
  tempService.addCharacteristic(tempLevelChar);
  tempService.addCharacteristic(tempReadChar);
  BLE.addService(tempService);
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}
  int int_temp = 0;

void loop()
{
  unsigned long currentUpdate = millis();

  BLEDevice central = BLE.central();

  //Cooling = IN1 = A1
  //Heating = IN2 = A2
  if(int_temp != masterTemp && abs(int_temp - masterTemp) > 2) {
    if(int_temp > masterTemp) {
      digitalWrite(coolPin, HIGH);
      digitalWrite(heatPin, LOW);

    }
    if(int_temp < masterTemp) {
      digitalWrite(coolPin, LOW);
      digitalWrite(heatPin, HIGH);
    }
  }
  if(int_temp == masterTemp || abs(int_temp - masterTemp) <= 2) {
      digitalWrite(coolPin, LOW);
      digitalWrite(heatPin, LOW);
  }
  if (currentUpdate - prevUpdate >= 500)
  {
    prevUpdate = currentUpdate;
    int_temp = 0;
    for (int i = 0; i < 10000; i++)
    {
      float tmpVoltage = analogRead(A3) * 3.3 / 1023.0;
      float temp = 80.336304700162070 * tmpVoltage + -16.081977309562400;
      temp = temp * 1.8 + 32;
      int_temp += (int)temp;
      sevseg.refreshDisplay();
    }
    Serial.println();
    int_temp = int_temp / 10000;
    displayTemp = (int_temp * 100) + masterTemp;
    sevseg.setNumber(displayTemp, 2);
    if (central)
    {
      Serial.print("Connected to central: ");
      digitalWrite(LED_BUILTIN, HIGH);

      tempLevelChar.writeValue(int_temp);
      byte value = masterTemp;
      tempReadChar.readValue(value);
      if (value != 0)
      {
        masterTemp = value;
        Serial.println(value);
      }
    }
    Serial.println(displayTemp);
    decrementState = digitalRead(decrementPin);
    incrementState = digitalRead(incrementPin);
    if (decrementState == HIGH)
    {
      masterTemp = masterTemp - 1;
      displayTemp = displayTemp - 1;
      sevseg.setNumber(displayTemp, 2);
    }
    if (incrementState == HIGH)
    {
      masterTemp = masterTemp + 1;
      displayTemp = displayTemp + 1;
      sevseg.setNumber(displayTemp, 2);
    }
  }
  sevseg.refreshDisplay();
}