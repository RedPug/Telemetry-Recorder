
#include <Arduino.h>
#include <SPI.h>
#include "Pins.h"
#include "Renderer.hpp"
#include "WifiHandler.hpp"
#include "TelemetryHandler.hpp"

bool is_network_active = false;

void renderTask();
void checkCommandInputs();

void setup(){
  Serial.begin(9600);

  delay(200); // Wait for serial to initialize, but not too long if it isn't connected.

  pinMode(LEFT_BTN_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BTN_PIN, INPUT_PULLUP);

  TelemetryHandler::init();

  /*
  ADC_EN is the ADC detection enable port
  If the USB port is used for power supply, it is turned on by default.
  If it is powered by battery, it needs to be set to high level
  */
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  Renderer::init();
  Renderer::initPanels();

  // Serial.println("initialized rendering...");

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

void loop(){
  static bool was_right_pressed = false;
  bool is_right_pressed = !digitalRead(RIGHT_BTN_PIN);
  if (!is_right_pressed && was_right_pressed){
    Serial.println("Going to sleep...");
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, LOW);
    delay(500);
    esp_deep_sleep_start();
  }
  was_right_pressed = is_right_pressed;


  static bool was_left_pressed = false;

  bool is_left_pressed = !digitalRead(LEFT_BTN_PIN);
  // Serial.println(is_left_pressed);
  if (!is_left_pressed && was_left_pressed){
    if (!is_network_active){
      is_network_active = true;
      WifiHandler::initServer();
    }
    else{
      is_network_active = false;
      WifiHandler::terminateServer();
    }
  }

  was_left_pressed = is_left_pressed;

  WifiHandler::checkUpdates();

  checkCommandInputs();

  TelemetryHandler::loop();

  renderTask();
}

void checkCommandInputs(){
  // Check for incoming commands from laptop
  if (WifiHandler::client.connected() && WifiHandler::client.available()){
    String cmd = WifiHandler::client.readStringUntil('\n');

    cmd.trim();
    if (cmd.equalsIgnoreCase("start")){
      TelemetryHandler::is_logging = true;
      Serial.println("Logging started");
    }
    else if (cmd.equalsIgnoreCase("stop")){
      TelemetryHandler::is_logging = false;
      Serial.println("Logging stopped");
    }else if (cmd.equalsIgnoreCase("HB")){
      WifiHandler::last_heartbeat_ms = millis();
    }
  }
}

void renderTask(){
  static int dt = 0;
  static unsigned long t0 = 0;

  unsigned long t1 = millis();
  dt += t1 - t0;
  t0 = t1;

  const int mspt = 200;

  if (dt > mspt){
    Renderer::updateDisplay();
    dt = 0;
  }
  // "gshhsjfgjkf"
}