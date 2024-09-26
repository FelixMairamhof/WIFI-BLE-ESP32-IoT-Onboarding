#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

// Define service and characteristic UUIDs
#define SERVICE_UUID "b2bbc642-46da-11ed-b878-0242ac120002"  // Replace with your UUID
#define CHARACTERISTIC_UUID "c9af9c76-46de-11ed-b878-0242ac120002" // Replace with your UUID

// Callback class to handle incoming write requests
class MyCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
      Serial.println(F("Received message: "));
      Serial.println(value.c_str());

      // Check for connect command using compare
      if (value.compare(0, 8, "connect:") == 0)
      {
        // Extract credentials
        String credentials = String(value.substr(8).c_str());
        int delimiterIndex = credentials.indexOf(':'); // Find delimiter
        if (delimiterIndex != -1) {
          String wifi_ssid = credentials.substring(0, delimiterIndex);
          String wifi_password = credentials.substring(delimiterIndex + 1);

          // Connect to Wi-Fi
          WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
          Serial.print(F("Connecting to Wi-Fi: "));
          Serial.println(wifi_ssid);

          // Wait for connection
          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(F("."));
          }
          Serial.println(F("\nConnected to Wi-Fi!"));

          // Notify the client about the connection status
          String response = "Connected to " + wifi_ssid;
          pCharacteristic->setValue(response.c_str());
          pCharacteristic->notify();
        }
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Starting NimBLE work!"));

  // Initialize NimBLE
  NimBLEDevice::init("MyESP32NimBLE");

  // Create service
  NimBLEService *pService = new NimBLEService(SERVICE_UUID);

  // Create characteristic with read/write properties
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::WRITE |
      NIMBLE_PROPERTY::NOTIFY
  );

  pCharacteristic->setValue("Hello World!");
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  // Start advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  NimBLEDevice::startAdvertising();
  Serial.println(F("Characteristic defined! Now you can read it on your phone!"));
}

void loop()
{
  // Keep the loop empty or you can add periodic tasks
  delay(2000);
}
