#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include <EEPROM.h>

#define ALARM_NAME_SIZE 20
#define ALARM_SIZE (6 + ALARM_NAME_SIZE)
#define ALARM_ENABLED(i) (i - 1) * ALARM_SIZE + 1
#define ALARM_DAYS(i) (i - 1) * ALARM_SIZE + 2
#define ALARM_HOUR(i) (i - 1) * ALARM_SIZE + 3
#define ALARM_MIN(i) (i - 1) * ALARM_SIZE + 4
#define ALARM_RISE(i) (i - 1) * ALARM_SIZE + 5
#define ALARM_WORK_AFTER(i) (i - 1) * ALARM_SIZE + 6
#define ALARM_NAME_START(i) (i - 1) * ALARM_SIZE + 7


#define ALARM_PIN 4

const long syncEveryMs = 1 * 3600;
const long utcOffsetInSeconds = 10800;
const char* ssid = "ssid";
const char* passwordWIFI = "pwd";

const String webPwd = "pwd";

bool alarmWorking = false;

unsigned long previousMillis = 0; 
unsigned long previousMillis2 = 0; 
unsigned long lastSyncMillis = 0; 
unsigned int workingAlarmInd;
const unsigned long interval = 3000;

uint timeRise = 15;
uint timeWorkAfter = 40 * 60000;
uint pwmInterval = timeRise * 60000 / 255;
uint brightness = 0;

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// START
String htmlData = "";
// END


void handleRoot() {
  server.send(200, "text/html", htmlData);
}

bool checkPassword(String userPwd) {
  if (userPwd.length() < 8) {
    server.send(403, "text/plain", "Invalid pwd");
    return false;
  }

  int startI = userPwd.length() - 8;
  for (uint i =  startI; i < userPwd.length(); i++) {
    if (webPwd[i - startI] != ((char) userPwd[i])) {
      server.send(403, "text/plain", "Invalid pwd");
      return false;
    }
  }

  return true;
}

void removeAlarm() {
  int alarmsN = EEPROM.read(0);
  const String id = server.arg("plain");
  if (!checkPassword(id)) return;
  int intID = (int) id[0];
  if ((intID > 0) && (alarmsN > 0)) {
    if (intID < alarmsN) {
      EEPROM.write(ALARM_ENABLED(intID), EEPROM.read(ALARM_ENABLED(alarmsN)));
      EEPROM.write(ALARM_DAYS(intID), EEPROM.read(ALARM_DAYS(alarmsN)));
      EEPROM.write(ALARM_HOUR(intID), EEPROM.read(ALARM_HOUR(alarmsN)));
      EEPROM.write(ALARM_MIN(intID), EEPROM.read(ALARM_MIN(alarmsN)));
      EEPROM.write(ALARM_RISE(intID), EEPROM.read(ALARM_RISE(alarmsN)));
      EEPROM.write(ALARM_WORK_AFTER(intID), EEPROM.read(ALARM_WORK_AFTER(alarmsN)));
    }
    if ((intID < alarmsN) || (intID == alarmsN)) {
      for (int i = 0; i < ALARM_NAME_SIZE; i++) {
        EEPROM.write(ALARM_NAME_START(intID) + i, 0);
      }
    }
    EEPROM.write(0, alarmsN - 1);
  } else {
    server.send(400, "text/plain", "Invalid ID");
    return;
  }
  server.send(200, "text/plain", "removed");
  EEPROM.commit();
}


void checkPasswordFromUser() {
  const String userPwd = server.arg("plain");
  if (userPwd.length() != webPwd.length()) {
    server.send(403, "text/plain", "Invalid pwd");
    return;
  }

  for (uint i = 0; i < webPwd.length(); i++) {
    if (webPwd[i] != ((char) userPwd[i])) {
      Serial.print(userPwd[i]);
      server.send(403, "text/plain", "Invalid pwd");
    }
  }

  server.send(200, "text/plain", "OK");
}

void saveAlarm() {
  int alarmsN = EEPROM.read(0);
  const String newAlarm = server.arg("plain");
  if (!checkPassword(newAlarm)) return;
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

  if ((newAlarm.length() <= (ALARM_SIZE + 9)) && (newAlarm.length() > 15)) {
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
  const String pwd = server.arg("plain");
  if (!checkPassword(pwd)) return;
  EEPROM.write(0, 0);
  EEPROM.commit();
  server.send(200, "text/plain", "deleted");
}

void toggleAlarmState() {
  const int alarmsN = EEPROM.read(0);
  String data = server.arg("plain");
  if (!checkPassword(data)) return;
  const int alarmI = (int) data[0];
  const int newState = (int) data[1];
  if (alarmI <= alarmsN) {
    EEPROM.write(ALARM_ENABLED(alarmI), newState);
    EEPROM.commit();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Invalid index");
  }
}

void alarmsList() {
  int alarmsN = EEPROM.read(0);
  const String pwd = server.arg("plain");
  if (!checkPassword(pwd)) return;
  String res = "";
  for (int i = 1; i <= ALARM_SIZE * alarmsN; i++) {
    res += (char) EEPROM.read(i);
  }
  server.send(200, "text/plain", res);
}

void setup(void) {
  EEPROM.begin(1024);
  Serial.begin(115200);
  timeClient.begin();
  WiFi.begin(ssid, passwordWIFI);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
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
  server.on("/remove-alarm/", HTTP_POST, removeAlarm);
  server.on("/check-password/", HTTP_POST, checkPasswordFromUser);
  server.on("/alarms-list/", HTTP_POST, alarmsList);

  server.begin();
  Serial.println("HTTP server started");;
  timeClient.update();
  lastSyncMillis = millis();

}

int checkTime() {
  time_t secondsGone = timeClient.getEpochTime() + (millis() - lastSyncMillis) / 1000;
  struct tm *timeInfo = gmtime(&secondsGone);
  uint hour = timeInfo->tm_hour;
  uint min = timeInfo->tm_min;
  uint dayWeek = timeInfo->tm_wday;
  int alarmsN = EEPROM.read(0);
  for (uint i = 1; i <= alarmsN; i ++) {
    if (EEPROM.read(ALARM_ENABLED(i)) == 1) {
      int dayOfweek = (dayWeek+ 6) % 7;
      if (EEPROM.read(ALARM_DAYS(i)) & (uint8_t) (1 << dayOfweek)) {
        if (EEPROM.read(ALARM_HOUR(i)) == hour) {
          if (EEPROM.read(ALARM_MIN(i)) == min) {
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
        alarmWorking = false;
      } 
    } else {
      if (currentMillis - previousMillis >= pwmInterval) {
        previousMillis = currentMillis;
        brightness += 1;
        analogWrite(ALARM_PIN, brightness);
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

