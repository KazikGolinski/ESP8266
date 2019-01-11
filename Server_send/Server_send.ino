#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "It hurts when IP";
const char* password = "niepowiem";
const int led = 2;
const int relayPin = 13;
String DeviceID = "12345678";
String FriendlyName = "SalaMatematyczna";
String State = "ON";
String PowerUse;
String payload;
int LastSensorState;
int SensorState;
int RelayState;
int SensorVal;
String ReqVal;
String LastReqVal;
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
  
  server.on("/rec", []() { 
    server.send(200, "text/plain", payload);
   });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}



void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", "DeaviceID:" + DeviceID + " <br> " + "FriendlyName: " + FriendlyName + " <br> " + "State:" + SensorState + " <br> " + "PowerUse:" + PowerUse + "<br>" + "RelayState :" + RelayState);// zmienic PowerUse na SensorVal
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
    String httpAdress = "http://192.168.43.207/index.php?state=" + StateString;
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
  //SensorVal = getMaxValue(); // Odkomentowac 
  LastSensorState = SensorState;// Zakomentowac
  String SensorValStr = Serial.readString();// Zakomentowac
  SensorVal = SensorValStr.toInt();// Zakomentowac
  if (SensorVal != 0){ // Zakomentowac
  PowerUse = SensorVal; // Zakomentowac
  } // Zakomentowac
  Serial.println (SensorVal);
  if (SensorVal > 40 && SensorVal != 0) { // usunac drugi warunek
    SensorState = 1;
  }
  if (SensorVal < 40 && SensorVal != 0) { // usunac drugi warunek
    SensorState = 0;
  }
}




void ReqToSwitch()
{

  String ReqVal = server.arg("state");
  if (ReqVal != LastReqVal){
    if (ReqVal == "ON")
    {
      if (SensorState == 1){
        Serial.println("already ON");
      }
      if (SensorState == 0)
      {
        if (RelayState == 1) 
        {
          digitalWrite(relayPin , LOW);
          Serial.println("state1");
          RelayState=0;
        }else
        {
          digitalWrite(relayPin, HIGH);
          Serial.println("state2");
          RelayState=1;
        }
      }
    }
  
    if (ReqVal == "OFF")
    {
      if (SensorState == 0){
        Serial.println("already OFF");
      }
      if (SensorState == 1)
      {
        if (RelayState == 1) 
        {
          digitalWrite(relayPin , LOW);
          RelayState=0;
          Serial.println("state1");
        }else
        {
          digitalWrite(relayPin, HIGH);
          Serial.println("state2");
          RelayState=1;
        }
      }
    }
  }
    LastReqVal = ReqVal;
    Serial.println("Relay state : " + RelayState);
}


int getMaxValue()
{
  int sensorValue;
  int sensorMax = 0;
  uint32_t start_time = millis();
  while ((millis() - start_time) < 500)
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
  WifiStart();
  ServerStart();
}

void loop(void) {
  server.handleClient();
  SendData();
  SensorRead();
  ReqToSwitch();
  delay(100);

}
