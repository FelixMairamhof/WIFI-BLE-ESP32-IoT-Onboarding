#include <Arduino.h>
#include <NimBLEDevice.h>

#define SERVICE_UUID "b2bbc642-ad5a-12ed-b878-0242ac120000"  
#define CHARACTERISTIC_UUID "c9af9c76-ad5a-11ed-b879-0242ac120000"

class MyCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        Serial.print("Received Value: ");
        Serial.println(value.c_str());  // Print the received value
    }
};

void setup() {
    Serial.begin(115200);
    NimBLEDevice::init("MyESP");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );

    pCharacteristic->setCallbacks(new MyCallbacks());  // Set the callbacks to handle writes
    pCharacteristic->setValue("Hello BLE");
    pService->start();
    
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("Waiting for a client to connect...");
}

void loop() {
    delay(1000);
}
