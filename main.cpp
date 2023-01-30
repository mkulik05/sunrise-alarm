#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

const char* ssid = "ssid";
const char* password = "pwd";

const int alarmSize = 5; // N bytes for each alarm

ESP8266WebServer server(80);

// START
string htmlData = "";
// END



void handleRoot() {
  Serial.println("Root html request");
  server.send(200, "text/html", postForms);
}

void addNewAlarm() {
  int alarmsN = EEPROM.read(0);

  newAlarm = server.arg("plain");
  
  for (int i = 0; i < newAlarm.length(); i++) {
    EEPROM.write(alarmSize * alarmsN + 1 + i, (int) newAlarm[i]);
  }

  EEPROM.write(0, alarmsN + 1);
  server.send(200, "text/plain", "added");

}

void alarmsList() {
  int alarmsN = EEPROM.read(0);
  string res = "";
  for (int i = 0; i <= alarmSize * alarmsN; i++) {
    res += (char) EEPROM.read(i);
  }
  server.send(200, "text/plain", res);
}

void setup(void) {
  EEPROM.begin(1024);
  htmlData = readData();
  
  WiFi.begin(ssid, password);
  Serial.println("");


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

  server.on("/add-alarm/", HTTP_POST, addNewAlarm);

  server.on("/alarms-list/", HTTP_GET, alarmsList);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}