#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

#define SERVICE_UUID "b2bbc642-ad5a-12ed-b878-0242ac120000"  
#define CHARACTERISTIC_UUID "c9af9c76-ad5a-11ed-b879-0242ac120000"

// Function prototype
String scanWiFiNetworks();

class MyCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        Serial.print("Received Value: ");
        Serial.println(value.c_str());

        if (value == "scan") {
            Serial.println("Scanning for Wi-Fi networks...");
            String result = scanWiFiNetworks();  // Get the scan results
            Serial.println(result);  // Log the result to ensure it’s correct

            // Prepare the result for sending as a char array
            char buffer[512]; // Ensure it's big enough to hold the JSON
            result.toCharArray(buffer, sizeof(buffer)); // Convert String to char array
            pCharacteristic->setValue(buffer); // Set the results back to the client
            pCharacteristic->notify(); // Notify the client about the new value
        }
    }
};

// Function to scan Wi-Fi networks and return the results in JSON format
String scanWiFiNetworks() {
    String result = "{\"wifiNetworks\":[";
    int n = WiFi.scanNetworks();  // Start scanning for networks
    for (int i = 0; i < 3; i++) {
        if (i > 0) {
            result += ",";
        }
        result += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    result += "]}";
    return result;  // Return the formatted results
}

void setup() {
    Serial.begin(115200);
    NimBLEDevice::init("MyESP");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Hello BLE"); // Initial value
    pService->start();
    
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("Waiting for a client to connect...");
}

void loop() {
    delay(1000);
}
