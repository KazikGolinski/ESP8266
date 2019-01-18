#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "BSR - Uczniowie";
const char* password = "bednarsk@";
const int led = 2;
const int relayPin = 14;
String DeviceID = "12345678";
String FriendlyName = "SalaMatematyczna";
String State = "ON";
String PowerUse;
String payload;
int LastSensorState;
int SensorState;
int RelayState=1;
int SensorVal;
String ReqVal = "0";
String LastReqVal;
String TimeStamp;
String LastTimeStamp;
ESP8266WebServer server(80);



void WifiStart()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

}



void ServerStart()
{
  server.on("/", handleRoot);
  
  server.on("/test", []() { 
    server.send(200, "text/plain", "działa");
   });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}



void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", "DeaviceID:" + DeviceID + " <br> " + "FriendlyName: " + FriendlyName + " <br> " + "State:" + SensorState + " <br> " + "PowerUse:" + SensorVal + "<br>" + "RelayState :" + RelayState);// zmienic PowerUse na SensorVal
  digitalWrite(led, 0);
}


void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}



void SendData()
{
  if (SensorState != LastSensorState) {
    RelayState = 1;
    String StateString = (String)SensorState;
    String httpAdress = "http://192.168.10.97/index.php?state=" + StateString;
    HTTPClient http;
    http.begin(httpAdress);
    Serial.println("Sending..." + httpAdress);
    int change = http.GET();
    Serial.println("Responce:" + change);
    http.end();
  }
}





void SensorRead()
{
  SensorVal = getMaxValue(); 
  LastSensorState = SensorState;

  Serial.println (SensorVal);
  if (SensorVal > 70 ) {
    SensorState = 1;
  }
  if (SensorVal < 70) {
    SensorState = 0;
  }
}


void ReqToSwitch()
{
  ReqVal = server.arg("state");
  TimeStamp = server.arg("TimeStamp");
  Serial.println("request value :" + ReqVal);
  //Serial.println("Last request value :" + LastReqVal);
  Serial.println("TimeStmap :" + TimeStamp);
  //Serial.println("last TimeStamp :" + LastTimeStamp);
  
  if(ReqVal == "ON" && TimeStamp != LastTimeStamp ) {
    if (RelayState == 0 && SensorState == 0)
    {
      digitalWrite(14, LOW);
      Serial.println("state2");
      RelayState=1;
    }
    else if (RelayState == 1 && SensorState == 0)
    {
      digitalWrite(14, HIGH);
      Serial.println("state1");
      RelayState=0;  
    }
    
    else if (RelayState == 0 && SensorState == 1) Serial.println("Already ON");
    else if (RelayState == 1 && SensorState == 1) Serial.println("Already ON");
}



if (ReqVal == "OFF" && TimeStamp != LastTimeStamp) {
  if (RelayState == 0 && SensorState == 1)
  {
    digitalWrite(14, LOW);
    Serial.println("state2");
    RelayState=1;
  }
  else if (RelayState == 1 && SensorState == 1)
  {
    digitalWrite(14 , HIGH);
    Serial.println("state1");
    RelayState=0;  
  }
  else if (RelayState == 0 && SensorState == 0) Serial.println("Already OFF");
  else if (RelayState == 1 && SensorState == 0) Serial.println("Already OFF");
}

LastReqVal = ReqVal;
LastTimeStamp = TimeStamp;
Serial.println(" ");

}






int getMaxValue()
{
  int sensorValue;
  int sensorMax = 0;
  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000)
  {
    sensorValue = analogRead(A0);
    if (sensorValue > sensorMax)
    {
      sensorMax = sensorValue;
    }
  }
  return sensorMax;
}



void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  pinMode(14, OUTPUT);
  WifiStart();
  ServerStart();
}

void loop(void) {
  server.handleClient();
  SensorRead();
  SendData();
  ReqToSwitch();
}
