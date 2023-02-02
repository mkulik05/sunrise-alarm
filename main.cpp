#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include <EEPROM.h>
#include <string.h>
#include <cmath>

#define ALARM_NAME_SIZE 20
#define ALARM_SIZE (6 + ALARM_NAME_SIZE)
#define ALARM_ENABLED(i) (i - 1) * ALARM_SIZE + 1
#define ALARM_DAYS(i) (i - 1) * ALARM_SIZE + 2
#define ALARM_HOUR(i) (i - 1) * ALARM_SIZE + 3
#define ALARM_MIN(i) (i - 1) * ALARM_SIZE + 4
#define ALARM_RISE(i) (i - 1) * ALARM_SIZE + 5
#define ALARM_WORK_AFTER(i) (i - 1) * ALARM_SIZE + 6
#define ALARM_NAME_START(i) (i - 1) * ALARM_SIZE + 7


#define ALARM_PIN 0

const long syncEveryMs = 1 * 3600;
const long utcOffsetInSeconds = 10800;
const char* ssid = "ssid";
const char* password = "pwd";

bool alarmWorking = false;

unsigned long previousMillis = 0; 
unsigned long previousMillis2 = 0; 
unsigned long lastSyncMillis = 0; 
unsigned int workingAlarmInd;
const unsigned long interval = 5000;

int timeRise = 15;
int timeWorkAfter = 40 * 60000;
int unsigned long pwmInterval = timeRise * 60000 / 255;
int brightness = 0;

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// START
String htmlData = "";
// END


void handleRoot() {
  Serial.println("Root html request");
  server.send(200, "text/html", htmlData);
}

void saveAlarm() {
  int alarmsN = EEPROM.read(0);
  const String newAlarm = server.arg("plain");
  const int ID = (int) newAlarm[0];

  if (ID > alarmsN) {
    if (ID - alarmsN > 1) {
      server.send(400, "text/plain", "Invalid ID");
      return;
    } else {
      alarmsN += 1;
    }
  }

  EEPROM.write(ALARM_ENABLED(ID), (int) newAlarm[1]);
  EEPROM.write(ALARM_DAYS(ID), (int) newAlarm[2]);
  EEPROM.write(ALARM_HOUR(ID), (int) newAlarm[3]);
  EEPROM.write(ALARM_MIN(ID), (int) newAlarm[4]);
  EEPROM.write(ALARM_RISE(ID), (int) newAlarm[5]);
  EEPROM.write(ALARM_WORK_AFTER(ID), (int) newAlarm[6]);

  Serial.print("-**-**-");
  Serial.print(newAlarm.length());
  Serial.print("-*---*-");
  if ((newAlarm.length() <= (ALARM_SIZE + 1)) && (newAlarm.length() > 7)) {
    for (int i = 0; i < ALARM_NAME_SIZE; i++) {
      EEPROM.write(ALARM_NAME_START(ID) + i, (int) newAlarm[7 + i]);
    }
  } 

  EEPROM.write(0, alarmsN);
  EEPROM.commit();
  server.send(200, "text/plain", "added");

}

int getDayOfWeek(time_t epochTime) {
  struct tm *timeInfo = gmtime(&epochTime);
  return timeInfo->tm_wday;
}

void deleteAllAlarms() {
  EEPROM.write(0, 0);
  EEPROM.commit();
  server.send(200, "text/plain", "deleted");
}

void toggleAlarmState() {
  const int alarmsN = EEPROM.read(0);
  String data = server.arg("plain");
  const int alarmI = (int) data[0];
  const int newState = (int) data[1];
  Serial.println(alarmI);
  if (alarmI <= alarmsN) {
    EEPROM.write(ALARM_ENABLED(alarmI), newState);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Invalid index");
  }
}

void alarmsList() {
  Serial.println("\n");
  int alarmsN = EEPROM.read(0);
  Serial.print("-");
  Serial.println(alarmsN);
  Serial.print("*");
  Serial.println(alarmsN * ALARM_SIZE);
  String res = "";
  for (int i = 1; i <= ALARM_SIZE * alarmsN; i++) {
    Serial.print(EEPROM.read(i));
    Serial.print("-");
    res += (char) EEPROM.read(i);
  }
  server.send(200, "text/plain", res);
}

void setup(void) {
  EEPROM.begin(1024);
  Serial.begin(115200);
  timeClient.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }

  server.on("/", handleRoot);

  server.on("/save-alarm/", HTTP_POST, saveAlarm);
  server.on("/remove-alarms/", HTTP_POST, deleteAllAlarms);
  server.on("/turn-alarm-on-off/", HTTP_POST, toggleAlarmState);

  server.on("/alarms-list/", HTTP_GET, alarmsList);

  server.begin();
  Serial.println("HTTP server started");;
  timeClient.update();
  lastSyncMillis = millis();

}
const int base2 = 2;
int checkTime() {
  time_t secondsGone = timeClient.getEpochTime() + (millis() - lastSyncMillis) / 1000;
  struct tm *timeInfo = gmtime(&secondsGone);
  unsigned int hour = timeInfo->tm_hour;
  unsigned int min = timeInfo->tm_min;
  unsigned int sec = timeInfo->tm_sec;
  unsigned int dayWeek = timeInfo->tm_wday;
  Serial.println('-');
  Serial.print(hour);
  Serial.print(':');
  Serial.print(min);
  Serial.print(':');
  Serial.print(sec);
  Serial.print(" - ");
  Serial.println(dayWeek);
  int alarmsN = EEPROM.read(0);
  for (int i = 1; i <= alarmsN; i ++) {
    if (EEPROM.read(ALARM_ENABLED(alarmsN)) == 1) {
      int dayOfweek = (dayWeek+ 6) % 7;
      if (EEPROM.read(ALARM_DAYS(alarmsN)) & (uint8_t) pow(base2, dayOfweek)) {
        if (EEPROM.read(ALARM_HOUR(alarmsN)) == hour) {
          if (EEPROM.read(ALARM_MIN(alarmsN)) == min) {
            return i;
          }
        }
      }
    }
  }
  return 0;
}

void loop() {
  unsigned long currentMillis = millis();
  if (alarmWorking) {
    if (brightness == 255) {
      if (currentMillis - previousMillis >= timeWorkAfter) {
        previousMillis = currentMillis;

        analogWrite(ALARM_PIN, 0);
        Serial.println("turn off");
        alarmWorking = false;
      } 
    } else {
      if (currentMillis - previousMillis >= pwmInterval) {
        previousMillis = currentMillis;
        brightness += 1;
        analogWrite(ALARM_PIN, brightness);
        Serial.println(brightness);
      }    
    }
  } else {
    if (currentMillis - previousMillis2 >= syncEveryMs) {
      previousMillis2 = currentMillis;
      timeClient.update();
      lastSyncMillis = millis();
    }
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      workingAlarmInd = checkTime();
      if (workingAlarmInd != 0) {
        timeRise = EEPROM.read(ALARM_RISE(workingAlarmInd));
        timeWorkAfter = EEPROM.read(ALARM_WORK_AFTER(workingAlarmInd)) * 60000;
        pwmInterval = timeRise * 60000 / 255;
        alarmWorking = true;
      }
    }
  }
  delay(5);
  server.handleClient();
}

