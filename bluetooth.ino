#include <ArduinoBLE.h>
#include "SevSeg.h"

BLEService tempService("1101");
BLEUnsignedCharCharacteristic tempLevelChar("2101", BLERead | BLENotify | BLEWrite);
BLEUnsignedCharCharacteristic tempReadChar("2102", BLERead | BLENotify | BLEWrite);
SevSeg sevseg;

unsigned long prevUpdate = 0;
int masterTemp = 72;

void setup()
{
  const int decrementPin = A7;
  const int incrementPin = A6;
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

  BLE.setLocalName("TemperatureMonitor");
  BLE.setAdvertisedService(tempService);
  tempService.addCharacteristic(tempLevelChar);
  tempService.addCharacteristic(tempReadChar);
  BLE.addService(tempService);

  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

int decrementState = 0;
int incrementState = 0;

void loop()
{
  unsigned long currentUpdate = millis();

  BLEDevice central = BLE.central();
  int int_temp = 0;
  if (currentUpdate - prevUpdate >= 1000)
  {
    prevUpdate = currentUpdate;
    for (int i = 0; i < 5; i++)
    {
      float tmpVoltage = analogRead(A3) * 3.3 / 1023.0;
      float temp = 80.336304700162070 * tmpVoltage + -16.081977309562400;
      temp = temp * 1.8 + 32;
      int_temp += (int)temp;
    }
    Serial.println();
    int_temp = int_temp / 5;
    int_temp = int_temp * 100;
    int_temp = int_temp + masterTemp;
    sevseg.setNumber(int_temp, 2);
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
    Serial.println(int_temp);
  }
  decrementState = digitalRead(decrementPin);
  incrementState = digitalRead(incrementPin);
  if (decrementState == HIGH)
  {
    masterTemp = masterTemp - 1;
  }
  if (incrementState == HIGH)
  {
    masterTemp = masterTemp + 1;
  }
  sevseg.refreshDisplay();
}