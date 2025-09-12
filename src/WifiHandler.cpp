#include <WiFi.h>
#include <Arduino.h>
#include <WifiHandler.h>
#include <TelemetryHandler.h>


namespace WifiHandler{
    const char* ssid = "RaceCarESP32";
    const char* password = "ilovepeter";  // Must be >=8 characters

    WiFiServer server(4210); // TCP server on port 4210
    WiFiClient client;

    uint8_t connection_status = WIFI_INACTIVE;

    void initServer(){
        WiFi.softAP(ssid, password);
        // Serial.println("Access Point Started");
        // Serial.print("IP address: ");
        // Serial.println(WiFi.softAPIP());

        server.begin();
        connection_status = WIFI_ACTIVE;

        Serial.println("Server started!");
    }

    void checkUpdates(){
        if(connection_status == WIFI_INACTIVE) return;

        int num_clients = WiFi.softAPgetStationNum();

        if(num_clients > 0){
            connection_status = WIFI_CONNECTED;
        }else{
            connection_status = WIFI_ACTIVE;
        }
    }


    void terminateServer(){
        server.end();
        client.stop();
        connection_status = WIFI_INACTIVE;
        TelemetryHandler::is_logging = false;
        Serial.println("Server shut down!");
    }
}