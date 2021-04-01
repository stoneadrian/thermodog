#include <ArduinoBLE.h>
BLEService tempService("1101");
BLEUnsignedCharCharacteristic tempLevelChar("2101", BLERead | BLENotify | BLEWrite);
BLEUnsignedCharCharacteristic tempReadChar("2102", BLERead | BLENotify | BLEWrite);

void setup() {
Serial.begin(9600);

pinMode(LED_BUILTIN, OUTPUT);
if (!BLE.begin()) 
{
Serial.println("starting BLE failed!");
while (1);
}

BLE.setLocalName("BatteryMonitor");
BLE.setAdvertisedService(tempService);
tempService.addCharacteristic(tempLevelChar);
tempService.addCharacteristic(tempReadChar);
BLE.addService(tempService);

BLE.advertise();
Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() 
{
BLEDevice central = BLE.central();

if (central) 
{
Serial.print("Connected to central: ");
Serial.println(central.address());
digitalWrite(LED_BUILTIN, HIGH);

while (central.connected()) {

      /*int battery = analogRead(A0);
      int batteryLevel = map(battery, 0, 1023, 0, 100);
      Serial.print("Battery Level % is now: ");
      Serial.println(batteryLevel);
      
      tempLevelChar.writeValue(batteryLevel);*/
      // read the input from the thermistor (A2) and TMP (A3):
       int int_temp = 0;

      for(int i = 0; i < 5; i++) {
      float tmpVoltage = analogRead(A3)*3.3/1023.0;
        float temp = 130.0 * tmpVoltage + -59.7;
      int_temp += (int)temp;  
      delay(200);  
      }
      int_temp = int_temp / 5;
 
  // print out the values you read from the thermistor and the TMP to the serial monitor:
  Serial.print(int_temp);
  Serial.println();
        tempLevelChar.writeValue(int_temp);
        byte value = 0;
        tempReadChar.readValue(value);
      Serial.println(value);
      delay(200);

}
}
digitalWrite(LED_BUILTIN, LOW);
Serial.print("Disconnected from central: ");
Serial.println(central.address());
}
