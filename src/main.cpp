#include <Arduino.h>

// put function declarations here:
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "xctask.h"

#define SERVICE_UUID        "0000ffe0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"


void testPolyLine(const char *polyline) {
    int precision = 5; /* oder die gewünschte Präzision */
    int size = 0;
    Coordinate* decoded = decodePolyline(polyline, precision, &size);

    for (int i = 0; i < size; i++) {
        printf("Längengrad: %f, Breitengrad: %f, Höhe: %d, Radius: %d\n",
               decoded[i].lon, decoded[i].lat, decoded[i].alt, decoded[i].rad);
    }

    free(decoded);
}



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      pServer->startAdvertising(); // restart advertising
    };

    void onDisconnect(BLEServer* pServer) {
      pServer->startAdvertising(); // restart advertising
    }
};


class MyCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic* pCharacteristic) {
        Serial.println("Read request received");
        // You can modify the characteristic value here
        // pCharacteristic->setValue("New value");
    }

    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.printf("New value written (%d):\n", value.length());
            for (int i = 0; i < value.length(); i++) {
                Serial.print(value[i]);
            }
            Serial.println();
            pCharacteristic->setValue("#OK");
            pCharacteristic->notify();
        }
    }
};



void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Long name works now");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello World says Neil");
  
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  
  char test[] = "ctacA}{p{GgQowH"; // this is a test polyline
  testPolyLine(test);
}

void loop() {}