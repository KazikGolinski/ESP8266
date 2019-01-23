/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "BSR - Uczniowie";
const char* password = "bednarsk@";
const int led = 2;
const int relayPin = 14;
String DeviceID = "12345678";
String FriendlyName = "SalaMatematyczna";
String State = "ON";
String PowerUse;
String payload;
int SensorState;
int RelayState = 1;
int SensorVal;
String ReqVal = "0";
String LastReqVal;
String TimeStamp;
String LastTimeStamp;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(relayPin, OUTPUT);
  ServerStart();

 
}

void loop() {
  SendAndReceiveData();
 
  delay(50);
  Serial.println("sensor val :" + SensorVal);
  Serial.println((SensorRead()) ? "ON" : "OFF");
}

void SendAndReceiveData()
{
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("new client");


  String req = client.readStringUntil('/r/n');
  Serial.println(req);
  client.flush();

  int val;
  if (req.indexOf("/ON") != -1) {
    ReqVal = "ON";
  } else if (req.indexOf("/OFF") != -1) {
    ReqVal = "OFF";
  } else {
    Serial.println("invalid request");
    client.stop();
    return;
  }
  ReqToSwitch();
  SensorRead();
  client.flush();
  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nState: ";
  s += (SensorRead()) ? "ON" : "OFF";
  s += "  ID: ";
  s += DeviceID;
  s += "  Friendly name: ";
  s += FriendlyName;
  s += "\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

}

void ServerStart()
{
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}




int SensorRead()
{
  
  int  SensorVal = getMaxValue();

  Serial.println (SensorVal);
  if (SensorVal > 70 ) {
    SensorState = 1;
    return (1);
  }
  if (SensorVal < 70) {
    SensorState = 0;
    return (0);
  }
}


void ReqToSwitch()
{

  Serial.println("request value :" + ReqVal);
  //Serial.println("Last request value :" + LastReqVal);
  //Serial.println("TimeStmap :" + TimeStamp);
  //Serial.println("last TimeStamp :" + LastTimeStamp);

  if (ReqVal == "ON") {
    if (RelayState == 0 && SensorState == 0)
    {
      digitalWrite(14, LOW);
      Serial.println("state2");
      RelayState = 1;
    }
    else if (RelayState == 1 && SensorState == 0)
    {
      digitalWrite(14, HIGH);
      Serial.println("state1");
      RelayState = 0;
    }

    else if (RelayState == 0 && SensorState == 1) Serial.println("Already ON");
    else if (RelayState == 1 && SensorState == 1) Serial.println("Already ON");
  }



  if (ReqVal == "OFF") {
    if (RelayState == 0 && SensorState == 1)
    {
      digitalWrite(14, LOW);
      Serial.println("state2");
      RelayState = 1;
    }
    else if (RelayState == 1 && SensorState == 1)
    {
      digitalWrite(14 , HIGH);
      Serial.println("state1");
      RelayState = 0;
    }
    else if (RelayState == 0 && SensorState == 0) Serial.println("Already OFF");
    else if (RelayState == 1 && SensorState == 0) Serial.println("Already OFF");
  }



  LastReqVal = ReqVal;
  LastTimeStamp = TimeStamp;
  Serial.println(" ");
  ReqVal = "0";
}







int getMaxValue()
{
  int sensorValue;
  int sensorMax = 0;
  uint32_t start_time = millis();
  while ((millis() - start_time) < 300)
  {
    sensorValue = analogRead(A0);
    if (sensorValue > sensorMax)
    {
      sensorMax = sensorValue;
    }
    
  }

  return sensorMax;
 
}
