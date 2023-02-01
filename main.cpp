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
#include <ctime>
#include <time.h>

#define ALARM_SIZE 4
#define ALARM_ENABLED(i) (i - 1) * ALARM_SIZE + 1
#define ALARM_DAYS(i) (i - 1) * ALARM_SIZE + 2
#define ALARM_HOUR(i) (i - 1) * ALARM_SIZE + 3
#define ALARM_MIN(i) (i - 1) * ALARM_SIZE + 4

#define ALARM_PIN 0

const long syncEveryMs = 1 * 3600;
const long utcOffsetInSeconds = 10800;
const char* ssid = "ssid";
const char* password = "pwd";

bool alarmWorking = false;

unsigned long previousMillis = 0; 
unsigned long previousMillis2 = 0; 
unsigned long lastSyncMillis = 0; 
unsigned int alarmInd;
const unsigned long interval = 5000;

int timeRise = 15;
int timeWorkAfter = 40 * 60000;
const unsigned long pwmInterval = timeRise * 60000 / 255;
int brightness = 0;

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// START
String htmlData = "<!DOCTYPE html>\n<meta name='viewport' \n   content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0'>\n<html lang='en'>\n <head>\n  <style>#alarms-list {\n flex-direction: column;\n}\n\n.alarm-block {\n width: 500px;\n height: 100px;\n display: flex;\n justify-content: space-between;\n align-items: center;\n}\n\n.wrapper {\n display: flex;\n justify-content: center;  \n align-items: center;\n}\n\n.alarm-time {\n font-size: 40px;\n font-family: 'Gill Sans', sans-serif;\n width: 30%;\n}\n\n\n\n.time-name-days {\n width: 85%;\n}\n\n.days {\n color: grey; \n   \n}\n\n#alarmAdder input[type='time'] {\n font-family: 'Gill Sans', sans-serif;\n font-size: 20px;\n height: 40px;\n width: 70%;\n border: 0px;\n border-bottom: 1px solid rgb(143, 142, 142);\n}\n\n#alarmAdder input[type='time']:focus {\n outline: none;\n border-bottom: 1px solid rgb(122, 111, 249);;\n}\n\n#alarmAdder {\n box-shadow: 5px 4px 2px rgb(178, 177, 177);\n border: 1px solid black;\n width: 500px;\n height: 400px;\n position: fixed;\n top: 50%;\n left: 50%;\n transform: translate(-50%, -50%);\n z-index: 100;\n display:none;\n background-color: white;\n flex-direction: column;\n justify-content: space-around;\n}\n\n#adderBtns {\n justify-content: space-around;\n width: 100%\n}\n#alarm-day-select {\n display: flex;\n justify-content: space-between;\n width: 70%\n}\n#alarm-day-select label {\n color: rgb(133, 133, 133);\n font-family: 'Gill Sans', sans-serif;\n}\n#alarm-day-select input {\n display: none;\n}\n#alarm-day-select input[type=checkbox]:checked + label {\n color: rgb(52, 75, 255);\n}\n#nameInput {\n width: 70%;\n}\n\nbutton {\n background-color: white;\n border: 0px;\n}\n\nbutton:hover {\n color: rgb(99, 96, 246);\n border: 0px;\n}\n\n#create-alarm {\n width: 50px;\n height: 50px;\n border: 1px solid rgb(118, 113, 255);\n border-radius: 50%;\n font-size: 20px;\n color: rgb(118, 113, 255);\n position: fixed;\n bottom: 8%\n}\n\n#create-alarm:hover {\nbackground-color: rgb(228, 228, 255);\n}\n\n#nameInput input {\n width: 100%;\n font-family: 'Gill Sans', sans-serif;\n border: 0px;\n border-bottom: 1px solid black\n}\n\n#adderBtns button {\n font-family: 'Gill Sans', sans-serif;\n font-size: 15px;\n}\n\n#nameInput input:focus {\n outline: none;\n font-family: 'Gill Sans', sans-serif;\n border-bottom: 1px solid rgb(8, 0, 245)\n}\ninput[type='text'] {\n font-size: 24px;\n}\n\n.toggle-switch {\n position: relative;\n display: inline-block;\n width: 60px;\n height: 34px;\n}\n.toggle-input {\n display: none;\n}\n\n.toggle-slider {\n position: absolute;\n cursor: pointer;\n top: 0;\n left: 0;\n right: 0;\n bottom: 0;\n background-color: #ccc;\n transition: .4s;\n border-radius: 34px;\n}\n\n.toggle-slider:before {\n position: absolute;\n border-radius: 50%;\n content: '';\n height: 26px;\n width: 26px;\n left: 4px;\n bottom: 4px;\n background-color: white;\n transition: .4s;\n}\n \n.toggle-input:checked + .toggle-slider {\n background-color: #2196F3;\n}\n\n.toggle-input:checked + .toggle-slider:before {\n transform: translateX(26px);\n}\n\n@media (max-width: 1000px) {\n .alarm-block {\n  width: 90%;\n  height: 120px;\n }\n #alarmAdder {\n  height: 500px;\n  width: 90%;\n }\n #create-alarm {\n  font-size: 30px;\n  height: 65px;\n  width: 65px;\n }\n .alarm-time {\n  font-size: 40px;\n }\n #alarmAdder input[type='time'] {\n  font-size: 20px;\n  height: 30px;\n  width: 85%;\n }\n #adderBtns button {\n  font-size: 20px;\n }\n input[type='text'] {\n  font-size: 25px;\n }\n\n #alarm-day-select {\n  width: 85%;\n }\n\n #alarm-day-select label {\n  font-size: 20px; \n }\n\n .days {\n  font-size: 17px;\n }\n .toggle-switch {\n  width: 55px;\n  height: 28px;\n } \n\n #nameInput {\n  width: 85%\n }\n\n #alarm-name {\n  width: 85%;\n  font-size: 25px;\n }\n \n .toggle-slider {\n  transition: .4s;\n  border-radius: 28px;\n }\n \n .toggle-slider:before {\n  height: 20px;\n  width: 20px;\n  left: 4px;\n  bottom: 4px;\n  transition: .4s;\n }\n  \n .toggle-input:checked + .toggle-slider {\n  background-color: #2196F3;\n }\n \n .toggle-input:checked + .toggle-slider:before {\n  transform: translateX(28px);\n }\n}\n\n</style>\n </head>\n\n <body>\n  <div id='alarms-list' class='wrapper'>\n\n  </div>\n  <div class='wrapper'>\n   <button id='create-alarm'>\n    +\n   </button>\n  </div>\n  <div class='wrapper'>\n   <div id='alarmAdder' class='wrapper' hidden>\n    <div id='nameInput'>\n     <input placeholder='Alarm name' type='text' id='alarm_name' name='Name' maxlength='20'><br><br>\n    </div>\n    <input type='time' id='select-time' required>\n    <div class='wrapper' id='alarm-day-select'>\n    </div>\n    <div class='wrapper' id='adderBtns'>\n     <button id='cancel'>\n      Back\n     </button>\n     <button id='add-alarm'>\n      Add alarm\n     </button>\n    </div>\n   </div> \n  </div>\n  <button id='delete-alarms' hidden>\n   Delete all alarms\n  </button>  \n </body>\n <script>let getAlarmsList = async () => {\n  let resp = await fetch('/alarms-list/')\n  let data = new Uint8Array(await resp.arrayBuffer())\n  let res = []\n  if (data.length > 0) {\n    let i, j\n\n    for (i = 0, j = data.length; i < j; i += 4) {\n        res.push([i, ...data.slice(i, i + 4)]);\n    }\n\n  }\n  return res\n  return [\n    new Uint8Array([1, 1, 127, 19, 56]),\n    new Uint8Array([0, 1, 42, 19, 45]),\n    new Uint8Array([2, 1, 2, 20, 25]),\n    new Uint8Array([2, 1, 2, 13, 25])\n  ];\n}\n\nlet changeToogleState = (id) => {\n  if (!document.querySelector(`.toggle-input.alarm-id-${id}`).checked) {\n    document.querySelector(`.alarm-time.alarm-id-${id}`).style.color = '#808294'\n    document.querySelector(`.name.alarm-id-${id}`).style.color = '#808294'\n  } else {\n    document.querySelector(`.alarm-time.alarm-id-${id}`).style.color = 'black'\n    document.querySelector(`.name.alarm-id-${id}`).style.color = 'black'\n  }\n}\n\nlet getAlarmHTML = (id) => {\n\n  let resHTML = document.createElement('div')\n  resHTML.setAttribute('class', `alarm-block alarm-id-${id}`)\n\n  let time_name_days = document.createElement('div')\n  time_name_days.setAttribute('class', `time-name-days alarm-id-${id}`);\n\n  let time_name = document.createElement('div');\n  time_name.setAttribute('class', `time-name alarm-id-${id}`);\n\n  let time = document.createElement('div');\n  time.setAttribute('class', `alarm-time alarm-id-${id}`);\n\n  let name = document.createElement('div')\n  name.setAttribute('class', `name alarm-id-${id}`)\n\n  time_name.appendChild(time);\n  time_name.appendChild(name);\n\n  let days = document.createElement('div');\n  days.setAttribute('class', `days alarm-id-${id}`);\n\n  time_name_days.appendChild(time_name)\n  time_name_days.appendChild(days)\n\n  let label = document.createElement('label')\n  label.setAttribute('class', `toggle-switch alarm-id-${id}`);\n  let span = document.createElement('span')\n  span.setAttribute('class', `toggle-slider alarm-id-${id}`);\n  let toggle = document.createElement('input')\n  toggle.addEventListener('click', () => changeToogleState(id));\n  toggle.setAttribute('class', `toggle-input alarm-id-${id}`);\n  toggle.type = 'checkbox'\n  toggle.checked = true;\n\n\n  label.appendChild(toggle)\n  label.appendChild(span)\n\n\n  resHTML.appendChild(time_name_days);\n  resHTML.appendChild(label);\n\n  return resHTML\n}\n\nlet compareTime = (a, b) => {\n  if (a[3] > b[3]) {\n    return 1\n  }\n  if (a[3] < b[3]) {\n    return -1\n  }\n  if (a[4] < b[4]) {\n    return -1\n  }\n\n  if (a[4] > b[4]) {\n    return 1\n  }\n\n  return 0;\n}\n\nlet refreshAlarms = async () => {\n  let alarms = await getAlarmsList()\n  alarms.sort(compareTime)\n  alarmsList.innerHTML = '';\n  for (let i = 0; i < alarms.length; i++) {\n    let alarm = getAlarmHTML(alarms[i][0])\n    let msg = ''\n    if (alarms[i][2] == 127) {\n      msg = 'Daily'\n    } else if (alarms[i][2] == 31) {\n      msg = 'Mon to Fri'\n    } else if (alarms[i][2] == 96) {\n      msg = 'Weeknds'\n    } else {\n      for (let b = 0; b < 7; b++) {\n        if ((2**b & alarms[i][2]) != 0) {\n          msg += wdaysNames[b] + '  '\n        }\n      }\n    }\n\n    alarm.querySelector('.days').innerText = msg\n    let h = alarms[i][3] > 9 ? alarms[i][3] : '0' + alarms[i][3]\n    let m = alarms[i][4] > 9 ? alarms[i][4] : '0' + alarms[i][4]\n    alarm.querySelector('.alarm-time').innerText = h + ':' + m\n\n    alarm.querySelector('.toggle-input').checked = true\n    alarm.setAttribute('id', `alarm${i}`);\n    alarm.setAttribute('class', `alarm-block`);\n    alarmsList.appendChild(alarm)\n  }  \n  alarmN = alarms.length;\n}\n\nlet addAlarm = async () => {\n  let res = 0;\n  \n  for (let i = 0; i < days.length; i++) {\n    let day = days[i];\n    if (day.checked) {\n      res += 2 ** i\n    }\n  }\n  if (res == 0) {\n    alert('Select at lest one day')\n  } else {\n    let value = selectTime.value \n    let time = [0, 0]\n    if (value !== '') {\n      time = value.split(':')\n      const data = new Uint8Array(3);\n      data[0] = res\n      data[1] = parseInt(time[0])\n      data[2] = parseInt(time[1])\n\n      const response = await fetch('/add-alarm/', {\n        method: 'POST',\n        headers: {\n          'Content-Type': 'application/x-binary'\n        },\n        body: data \n      });\n      res = response.text()\n      if (res != 'error') {\n        await refreshAlarms()\n        popup(false, 'none')\n      }\n\n    } else {\n      alert('Specify time')\n    }\n  }\n}\n\nlet deleteAllAlarms = async () => {\nawait fetch('/remove-alarms/', {\n    method: 'POST'\n  });\n}\n\n\nlet popup = (show, styles) => {\n  document.getElementById('alarmAdder').style.display = !show? 'none' : 'flex'\n    const elements = document.querySelectorAll('.alarm-block');\n\n  for (const element of elements) {\n      element.style.filter = styles;\n  }\n\n  document.getElementById('create-alarm').style.filter = styles;\n}\n\n\n\n\n\nlet alarmN = 0;\nalarmsList = document.getElementById('alarms-list')\ndocument.getElementById('create-alarm').onclick = () => popup(true, 'blur(5px)')\n\ndocument.getElementById('add-alarm').onclick = addAlarm\ndocument.getElementById('cancel').onclick = () => popup(false, 'none')\n\ndocument.getElementById('delete-alarms').onclick = deleteAllAlarms;\n\nlet selectTime = document.getElementById('select-time')\n\nconst wdaysNames = ['Mon', 'Tue', 'Wen', 'Thu', 'Fri', 'Sat', 'Sun']\n\nlet days = [];\n\nfor (let i = 0; i < wdaysNames.length; i++) {\n  let inp = document.createElement('input');\n  inp.setAttribute('type', `checkbox`);\n  inp.setAttribute('id', `day${i}`);\n  \n  days.push(inp)\n  \n  let label = document.createElement('label');\n  label.setAttribute('for', `day${i}`)\n  label.innerText = wdaysNames[i]\n\n  document.getElementById('alarm-day-select').appendChild(inp)\n  document.getElementById('alarm-day-select').appendChild(label)\n}\n\n\n\nrefreshAlarms()</script>\n</html>";
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
  EEPROM.write(ALARM_HOUR(alarmsN), (int) newAlarm[1]);
  EEPROM.write(ALARM_MIN(alarmsN), (int) newAlarm[2]);

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

  server.on("/add-alarm/", HTTP_POST, addNewAlarm);
  server.on("/remove-alarms/", HTTP_POST, deleteAllAlarms);

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
      alarmInd = checkTime();
      if (alarmInd != 0) {

        alarmWorking = true;
      }
    }
  }
  delay(5);
  server.handleClient();
}

