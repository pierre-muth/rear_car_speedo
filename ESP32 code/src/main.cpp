/**
 * 
 */

#include <Arduino.h>
#include "BLEDevice.h"
#include <U8g2lib.h>

#define RXD2 16
#define TXD2 17

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// The remote service we wish to connect to.
// static BLEUUID serviceUUID("ffe0");
static BLEUUID serviceUUID(SERVICE_UUID);
// // The characteristic of the remote service we are interested in.
// static BLEUUID    charUUID("ffe1");
static BLEUUID    charUUID(CHARACTERISTIC_UUID);
// // ble aadress
// static BLEAddress bleAddr("20:cd:39:90:f7:7c");
static BLEAddress bleAddr("24:6f:28:15:ce:12");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 22, 21);
String buildINString="test";
String buildString = "";
String subString = "";
String speedString = "";
int speed = -1;
char speedValueChars [2] = {'*', '*'};

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    // if(pRemoteCharacteristic->canRead()) {
    //   std::string value = pRemoteCharacteristic->readValue();
    //   Serial.print("The characteristic value was: ");
    //   Serial.println(value.c_str());
    // }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
        Serial.println("Device found by service UUID ");
    } // Found our server
   
    if (advertisedDevice.getAddress().equals(bleAddr)){
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
      Serial.println("Device found by address ");
    }
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  u8g2.begin();

  Serial.println("Starting BLE Client...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {
  Serial.println(".");

  Serial2.println("010d");
  delay(100);

  buildString = "";
  buildINString = "";
  while (Serial2.available()) {
    char inChar = (char)Serial2.read();
    if (inChar > 0x1F) {
      buildString += inChar;
      buildINString += inChar;
    }
    delay(1);
  }

  Serial.println(buildINString);

  while (buildString.length() > 7) {
    if (buildString.startsWith("41 0D")) {
      speedString = buildString.charAt(6);
      speedString += buildString.charAt(7);
      speed = strtol(speedString.c_str(), NULL, 16);  
      itoa(speed, speedValueChars, 10);
      break;
    } else {
      speedValueChars[0] = '-';
      speedValueChars[1] = '-';
    }
    buildString = buildString.substring(1);
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_5x7_mf);
    u8g2.drawStr(0,10, buildINString.c_str());
    u8g2.drawStr(10,24, speedValueChars);
  } while ( u8g2.nextPage() );

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
    if (connected) {
        String newValue = String(millis()/1000);
        Serial.println(newValue +"> " + speedString);
        // Set the characteristic's value to be the array of bytes that is actually a string.
        pRemoteCharacteristic->writeValue(speedString.c_str(), speedString.length());
    } else if (doScan) {
        BLEDevice::getScan()->start(5, false);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
    }
  
  delay(1000); // Delay a second between loops.
} // End of loop