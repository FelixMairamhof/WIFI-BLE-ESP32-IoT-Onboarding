#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

#define SERVICE_UUID "b2bbc642-ad5a-12ed-b878-0242ac120000"  
#define CHARACTERISTIC_UUID "c9af9c76-ad5a-11ed-b879-0242ac120000"

// Function prototype
String scanWiFiNetworks();
bool connectToWifi(String ssid, String password);


class MyCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        Serial.print("Received Value: ");
        Serial.println(value.c_str());

        if (value == "SCAN") {
    Serial.println("Scanning for Wi-Fi networks...");
    String result = scanWiFiNetworks();  // Get the scan results
    Serial.println(result);  // Log the result to ensure it’s correct

    // Prepare the result for sending as a char array
    String prefixedResult = "ALLWIFIS:" + result; // Prefix the result
    char buffer[512];  // Ensure it's big enough to hold the JSON
    prefixedResult.toCharArray(buffer, sizeof(buffer));  // Convert String to char array
    pCharacteristic->setValue(buffer);  // Set the results back to the client
    pCharacteristic->notify();  // Notify the client about the new value
}

if (value.find("CONNECT:") == 0) {
    // Extract the SSID and password from the connection string
    std::string connectCommand = value.substr(8);  // Remove the "CONNECT:" part

    // Find the positions of the SSID and password delimiter ":"
    int delimiterPos = connectCommand.find(":");
    if (delimiterPos != std::string::npos) {
        String ssid = String(connectCommand.substr(0, delimiterPos).c_str());
        String password = String(connectCommand.substr(delimiterPos + 1).c_str());

        Serial.print("Connecting to Wi-Fi SSID: ");
        Serial.println(ssid);
        Serial.println("Attempting to connect to Wi-Fi...");

        bool success = connectToWifi(ssid, password); 
        String status = ""; // Try connecting to Wi-Fi
        if(success){
            status = "STATUS: Connected (PK:" + WiFi.macAddress() + ")";
        }else{
            status = "STATUS: Failed";
        }
        Serial.print("MAC: " + WiFi.macAddress());
        // Prefix with STATUS
        Serial.println(status);
        char buffer[128];  // Ensure it's big enough to hold the JSON
        status.toCharArray(buffer, sizeof(buffer));  // Convert String to char array
        pCharacteristic->setValue(buffer); 
        pCharacteristic->notify();  // Notify the client about the new value
    }
}

    }
};
bool connectToWifi(String ssid, String password) {

    WiFi.begin(ssid.c_str(), password.c_str());

    int maxRetries = 10;
    while (WiFi.status() != WL_CONNECTED && maxRetries > 0) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
        maxRetries--;
    }

    return WiFi.status() == WL_CONNECTED;
}
String scanWiFiNetworks() {
    WiFi.mode(WIFI_STA);
    String result = "{\"wifiNetworks\":[";
    int n = WiFi.scanNetworks(false, true);  // Scan for Wi-Fi networks
    int addedNetworks = 0;  // Track how many networks we've added to the result

    for (int i = 0; i < n && addedNetworks < 5; i++) {
        String ssid = WiFi.SSID(i);

        // Log each SSID for debugging
        Serial.println("WIFISSID: " + ssid);

        // Skip if the SSID is empty
        if (ssid == "") {
            continue;
        }

        // Add a comma if this is not the first network in the list
        if (addedNetworks > 0) {
            result += ",";
        }

        // Append network info to the JSON result
        result += "{\"ssid\":\"" + ssid + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
        addedNetworks++;  // Increment the count of added networks
    }

    result += "]}";
    return result;  // Return the formatted results as a JSON string
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
