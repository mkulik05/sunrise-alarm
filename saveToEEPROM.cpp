#include <LittleFS.h>
#include <string>

const int reservedB = 512;

// START
string resFile = "<!DOCTYPE html>
<html lang='en'>
 <head>
  <style>mvgjhgf
fjgj

jhf</style>
 </head>

 <body>
  <div class='alarm-list'>

  </div>  
 </body>
 <script>var fcewqaoifeasw
fdasmofi</script>
</html>"
// END

void setup (){
  if(!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
 }
}

void deleteData() {
  LittleFS.remove("/index.html");
}

void writeData(String data) {
  File file = LittleFS.open("/index.html", "w");
  file.print(data);
  file.close();
  Serial.println("Write successful");
}

void loop() {
  deleteData()
  writeData(resFile);
  break;
}