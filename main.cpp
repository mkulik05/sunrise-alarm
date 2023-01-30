#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <string.h>


#define ALARM_SIZE 4
#define ALARM_ENABLED(i) i * ALARM_SIZE + 1
#define ALARM_DAYS(i) i * ALARM_SIZE + 2
#define ALARM_WEEK(i) i * ALARM_SIZE + 3
#define ALARM_HOUR(i) i * ALARM_SIZE + 4


const char* ssid = "ssid";
const char* password = "pwd";


ESP8266WebServer server(80);

// START
String htmlData = "<!DOCTYPE html>\n<html lang='en'>\n <head>\n  <style></style>\n </head>\n\n <body>\n  <div id='alarms-list'>\n\n  </div>\n  <div id='create-alarm'>\n   Add alarm\n  </div>\n  <div id='alarmAdder' hidden>\n   <input type='time' id='select-time' required>\n   <div>\n    <input type='checkbox' id='day0' placeholder='day'>\n    <input type='checkbox' id='day1' placeholder='day'>\n    <input type='checkbox' id='day2' placeholder='day'>\n    <input type='checkbox' id='day3' placeholder='day'>\n    <input type='checkbox' id='day4' placeholder='day'>\n    <input type='checkbox' id='day5' placeholder='day'>\n    <input type='checkbox' id='day6' placeholder='day'>\n   </div>\n   <div id='add-alarm'>\n    Add alarm\n   </div>\n  </div> \n  <div id='delete-alarms' hidden>\n   Delete all alarms\n  </div>  \n </body>\n <script>let getAlarmsList = async () => {\n  let resp = await fetch('/alarms-list/')\n  let data = new Uint8Array(await resp.arrayBuffer())\n  let res = []\n  if (data.length > 0) {\n    let i, j\n\n    for (i = 0, j = data.length; i < j; i += 4) {\n        res.push(data.slice(i, i + 4));\n    }\n\n  }\n  return res;\n}\n\nlet refreshAlarms = async (N = 0) => {\n  let alarms = await getAlarmsList()\n  for (let i = N; i < alarms.length; i++) {\n    let alarm = document.createElement('div')\n    alarm.innerHTML = alarms[i]\n    alarm.setAttribute('id', `alarm${i}`);\n    alarmsList.appendChild(alarm)\n  }  \n  alarmN = alarms.length;\n}\n\nlet addAlarm = async () => {\n  let res = 0;\n  for (let i = 0; i < days.length; i++) {\n    let day = days[i];\n    if (day.checked) {\n      res += 2 ** i\n    }\n  }\n  if (res == 0) {\n    alert('Select at lest one day')\n  } else {\n    let value = selectTime.value \n    let time = [0, 0]\n    if (value !== '') {\n      time = value.split(':')\n      const data = new Uint8Array(3);\n      data[0] = res\n      data[1] = parseInt(time[0])\n      data[2] = parseInt(time[1])\n\n      const response = await fetch('/add-alarm/', {\n        method: 'POST',\n        headers: {\n          'Content-Type': 'application/x-binary'\n        },\n        body: data \n      });\n      res = response.text()\n      if (res != 'error') {\n        await refreshAlarms(alarmN)\n        document.getElementById('alarmAdder').hidden = true\n      }\n\n    } else {\n      alert('Specify time')\n    }\n  }\n}\n\nlet deleteAllAlarms = async () => {\nawait fetch('/remove-alarms/', {\n    method: 'POST'\n  });\n}\n\n\nlet alarmN = 0;\nalarmsList = document.getElementById('alarms-list')\ndocument.getElementById('create-alarm').onclick = () => document.getElementById('alarmAdder').hidden = false;\ndocument.getElementById('add-alarm').onclick = addAlarm\ndocument.getElementById('delete-alarms').onclick = deleteAllAlarms;\n\nlet selectTime = document.getElementById('select-time')\n\nlet days = []\n\nfor (let i = 0; i < 7; i++) {\n  days.push(document.getElementById('day' + i))\n}\n\nrefreshAlarms()</script>\n</html>";
// END



void handleRoot() {
  Serial.println("Root html request");
  server.send(200, "text/html", htmlData);
}

void addNewAlarm() {
  int alarmsN = EEPROM.read(0);

  String newAlarm = server.arg("plain");
  alarmsN += 1;
  EEPROM.write(ALARM_ENABLED(alarmsN), 1);
  EEPROM.write(ALARM_DAYS(alarmsN), (int) newAlarm[0]);
  EEPROM.write(ALARM_WEEK(alarmsN), (int) newAlarm[1]);
  EEPROM.write(ALARM_HOUR(alarmsN), (int) newAlarm[2]);

  EEPROM.write(0, alarmsN);
  EEPROM.commit();
  server.send(200, "text/plain", "added");

}

void deleteAllAlarms() {
  EEPROM.write(0, 0);
  EEPROM.commit();
  server.send(200, "text/plain", "deleted");
}

void alarmsList() {
  Serial.println("\n");
  int alarmsN = EEPROM.read(0);
  Serial.println(alarmsN);
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
  server.on("/remove-alarms/", HTTP_POST, deleteAllAlarms);

  server.on("/alarms-list/", HTTP_GET, alarmsList);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}