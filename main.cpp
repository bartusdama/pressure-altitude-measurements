#include "LPS25HB.h"
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <HTTPClient.h>


// Dane autoryzacyjne Blynk
#define BLYNK_TEMPLATE_ID "template"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
char auth[] = "authcode";
#include <BlynkSimpleEsp32.h>

const String ENDPOINT_STRING = "endpoint_string";
const String API_KEY = "api_key";
String jsonStringText;


// Dane autoryzacyjne Firebase
#define FIREBASE_HOST "url"  // Adres URL Firebase
#define FIREBASE_AUTH "webapi"  // Klucz Web API Firebase

FirebaseData firebaseData;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;

BlynkTimer timer;

void checkConnection() {
    if(Blynk.connected()) {
        Blynk.virtualWrite(V0, 255);
    } else {
        Blynk.virtualWrite(V0, 0);
    }
}


void setup() {
    Serial.begin(115200);
    LPS25HB_init();
    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Połączono z WiFi");

    firebaseConfig.host = FIREBASE_HOST; // Adres serwera Firebase
    firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH; // Token autoryzacyjny Firebase

    Firebase.begin(&firebaseConfig, &firebaseAuth);
    Firebase.reconnectWiFi(true);

    // Ustawienie timera na wywołanie funkcji checkConnection co 10 sekund
    timer.setInterval(10000L, checkConnection); 
}

void loop() {
    Blynk.run();
    timer.run();

    // Przykładowe odczyty
    float callibPressure = downloadPressure();
    float pressure = ReadPressure();
    float temperature = ReadTemp();
    float altitude = HeightCalc(pressure, temperature, callibPressure);
    Serial.println(pressure);
    Serial.println(temperature);
    Serial.println(altitude);

    // Wysyłanie danych do Blynk
    Blynk.virtualWrite(V1, pressure);
    Blynk.virtualWrite(V2, temperature);
    Blynk.virtualWrite(V3, altitude);

    if (Firebase.ready()) {

        if(!Firebase.pushFloat(firebaseData, "/pomiary/user1/pressure", pressure)){
            Serial.println("Failed to push pressure data");
            Serial.println(firebaseData.errorReason()); // Wydrukuj powód błędu
        }
        if(!Firebase.pushFloat(firebaseData, "/pomiary/user1/temperature", temperature)){
            Serial.println("Failed to push temperature data");
            Serial.println(firebaseData.errorReason()); // Wydrukuj powód błędu
        }
        if(!Firebase.pushFloat(firebaseData, "/pomiary/user1/altitude", altitude)){
            Serial.println("Failed to push altitude data");
            Serial.println(firebaseData.errorReason()); // Wydrukuj powód błędu
        }
    }
    delay(100000);
}

