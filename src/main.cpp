#include <Arduino.h>
#include <WiFiManager.h>
#include <TaskScheduler.h>

#include "esp8266ota.h"
#include "communication/mqtt.h"

uint8_t pin = LOW;
uint16_t dutyCycleValue = 0;
uint8_t testPin = 14;
uint8_t pwmPin = 15;
uint8_t encoderPin = 12;
uint8_t encoderSmallPin = D8;
int16_t delta = 4;
int16_t pwmValue = 0;
uint16_t range = 256;
uint64_t lastMills = 0;
uint64_t lastMillsLong = 0;
volatile uint64_t encoderStep = 0;
volatile uint64_t encoderStepsSmall = 0;

WiFiManager *wifiManager = nullptr;
Esp8266OTA *updater = nullptr;
Scheduler *scheduler = nullptr;

Task *updaterTask = nullptr;
Task *mqttTickTask = nullptr;

// void IRAM_ATTR pinTriggered();
void IRAM_ATTR encoderTriggered();
void IRAM_ATTR encoderSmallTriggered();

void initializeWifi();
void initializeScheduler();
void updaterTick();
void mqttTick();

void encoderTick();
void encoderSmallTick();
Task *encoderTask = nullptr;

void setup() {
    Serial.begin(115200);
    initializeWifi();
    updater = new Esp8266OTA("esp8266", "password");
    initializeScheduler();
    MQTT::connect("10.238.75.62", 1883, "mqtt", "mqtt");
    // put your setup code here, to run once:
    // pinMode(LED_BUILTIN, OUTPUT);
    pinMode(testPin, OUTPUT);
    pinMode(pwmPin, OUTPUT);
    // pinMode(0, INPUT_PULLUP);
    pinMode(encoderPin, INPUT_PULLUP);
    // pinMode(encoderSmallPin, INPUT_PULLDOWN_16);
    // pinMode(encoderSmallPin, OUTPUT);
    digitalWrite(encoderSmallPin, LOW);
    // digitalWrite(LED_BUILTIN, pin);
    digitalWrite(testPin, HIGH);
    // analogWrite(testPin, 0);
    analogWrite(pwmPin, 0);
    analogWriteFreq(1000);
    analogWriteRange(range);
    // analogWrite(testPin, 255);
    // attachInterrupt(digitalPinToInterrupt(0), pinTriggered, FALLING);
    attachInterrupt(digitalPinToInterrupt(encoderPin), encoderTriggered, FALLING);
    // attachInterrupt(digitalPinToInterrupt(encoderSmallPin), encoderSmallTriggered, FALLING);
}

void loop() {
    scheduler->execute();
    yield();
    // put your main code here, to run repeatedly:
    // digitalWrite(12, pin);
    // pin = (pin == LOW)? HIGH : LOW;
    // analogWrite(pwmPin, dutyCycleValue);
    // dutyCycleValue += delta;
    // if (dutyCycleValue >= 255) delta = -1;
    // if (dutyCycleValue <= 0) delta = 1;
    // delay(50);
    // if (millis() - lastMills >= 1000UL) {
    //     lastMills = millis();

    // }

    // if (millis() - lastMillsLong >= 1000UL) {
    //     int rpm = (encoderStep / 2) * 60;
    //     encoderStep = 0;
    //     Serial.write("rpm: ");
    //     Serial.println(rpm);
    //     lastMillsLong = millis();
    // }
}

void pinTriggered() {
    pwmValue += delta;
    if (pwmValue >= range) delta = -4;
    if (pwmValue <= 0) delta = 4;
    Serial.println(pwmValue);
    analogWrite(testPin, pwmValue);
}

void encoderTriggered() {
    encoderStep ++;
}

void encoderSmallTriggered() {
    encoderStepsSmall ++;
    // Serial.println(encoderStepsSmall);
}

void encoderTick() {
    int rpm = (encoderStep / 2) * 60;
    int rpmSmall = (encoderStepsSmall / 2) * 60;
    Serial.println(encoderStepsSmall);
    encoderStep = 0;
    encoderStepsSmall = 0;
    Serial.write("rpm1: ");
    Serial.print(rpm);
    Serial.write(", rpm2: ");
    Serial.println(rpmSmall);
}

void initializeWifi() {
    wifiManager = new WiFiManager();
    wifiManager->autoConnect("ESP8266 AP", "12345678");
    WiFi.hostname("greenhouse-controller");
}

void initializeScheduler() {
    scheduler = new Scheduler();
    updaterTask = new Task(TASK_MILLISECOND, TASK_FOREVER, updaterTick, scheduler, true, nullptr, nullptr);
    mqttTickTask = new Task(TASK_MILLISECOND, TASK_FOREVER, mqttTick, scheduler, true, nullptr, nullptr);

    encoderTask = new Task(TASK_SECOND, TASK_FOREVER, encoderTick, scheduler, true, nullptr, nullptr);
}

void updaterTick() {
    updater->handle();
}

void mqttTick() {
    MQTT::client.loop();
}