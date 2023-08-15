#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include "Adafruit_MAX1704X.h"

//Global Variables

//Hotspot password
const char* ssid     = "Plantmate";
const char* password = "123456789";

//Battery fule gague
Adafruit_MAX17048 maxlipo;

//Water tank sensors
int din1 = 16;
int din2 = 17;
int din3 = 5;
int din4 = 18;
int din5 = 19;

//Waterpumps
int rly1 = 12;
String rly1State = "off";
int rly2 = 14; //TODO: CHANGE THIS
String rly2State = "off";
int rly3 = 27;
String rly3State = "off";
int rly4 = 26;
String rly4State = "off";
int rly5 = 25;
String rly5State = "off";

//Soil moist sensors
int ain1 = 39; //check if this is actually the right pin
int ain2 = 36; //check if this is actually the right pin
int ain3 = 34;
int ain4 = 35;

//Led Control
int wifiLed = 2;
int lowWaterLed = 15;

//Server
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

//Functions

//Reads analog sensors. Increase maxAdcValue to sensor value when nothing is touched.
float maxAdcValue = 2940;
float analogReadPercent(int pin){
  float read = analogRead(pin);
  float percentage = 0;
  if(read != 0){
    percentage = (read / maxAdcValue) * 100;
  }
  return percentage;
}

//getSoilMoist
float getSoilMoist(int plantnr){

  if(plantnr == 1){
    return analogReadPercent(ain1);
  }
  if(plantnr == 2){
    return analogReadPercent(ain2);
  }
  if(plantnr == 3){
    return analogReadPercent(ain3);
  }
  if(plantnr == 4){
    return analogReadPercent(ain4);
  }
  return 0;
}

//Battery
#define VBAT_PIN 33

//Will show voltage level using a voltage devider. this is highly shit TODO: use battery fule gague
float getBatteryLevel()
{
    return analogRead(VBAT_PIN) * 2;
}

void waterPlant(int rly){
    digitalWrite(rly, !digitalRead(rly));
}

int getWaterLevel(){
  if(digitalRead(din5) == HIGH){
    return 90;
  }
  if(digitalRead(din4) == HIGH){
    return 70;
  }
  if(digitalRead(din3) == HIGH){
    return 50;
  }
  if(digitalRead(din2) == HIGH){
    return 20;
  }
  if(digitalRead(din1) == HIGH){
    return 10;
  }
  return 0;
}

void setup() {
  Serial.begin(115200);

  maxlipo.begin();

  //Setup pins
  pinMode(VBAT_PIN, INPUT);

  pinMode(din5, INPUT);  
  pinMode(din4, INPUT);  
  pinMode(din3, INPUT);  
  pinMode(din2, INPUT);  
  pinMode(din1, INPUT); 

  pinMode(rly1,OUTPUT); 
  pinMode(rly2,OUTPUT);
  pinMode(rly3,OUTPUT);
  pinMode(rly4,OUTPUT);
  pinMode(rly5,OUTPUT);

  digitalWrite(rly1, LOW);
  digitalWrite(rly2, LOW); //todo change this before deployment
  digitalWrite(rly3, LOW);
  digitalWrite(rly4, LOW);
  digitalWrite(rly5, LOW);

  pinMode(wifiLed, OUTPUT);
  pinMode(lowWaterLed, OUTPUT);

  digitalWrite(wifiLed, HIGH);
  digitalWrite(lowWaterLed, LOW);

  //Start wifi hotspot

  IPAddress local_ip(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /" + String(rly1)) >= 0) {
              digitalWrite(rly1, !digitalRead(rly1));
            } 
            if (header.indexOf("GET /" + String(rly2)) >= 0) {
              digitalWrite(rly2, !digitalRead(rly2));
            } 
            if (header.indexOf("GET /" + String(rly3)) >= 0) {
              digitalWrite(rly3, !digitalRead(rly3));
            } 
            if (header.indexOf("GET /" + String(rly4)) >= 0) {
              digitalWrite(rly4, !digitalRead(rly4));
            } 
            if (header.indexOf("GET /" + String(rly5)) >= 0) {
              digitalWrite(rly5, !digitalRead(rly5));
            } 

            
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Plantmate Test Server</h1>");
            
            client.println("<h2>Activate Pumps: </h2>");
            
            if (digitalRead(rly1)) {
              client.println("<p><a href=\"/" + String(rly1) + "/on\"><button class=\"button\">W Pump1 On</button></a></p>");
            } else {
              client.println("<p><a href=\"/" + String(rly1) + "/off\"><button class=\"button button2\">W Pump1 off</button></a></p>");
            } 
            if (digitalRead(rly4)) {
              client.println("<p><a href=\"/" + String(rly4) + "/on\"><button class=\"button\">W Pump 3 On</button></a></p>");
            } else {
              client.println("<p><a href=\"/" + String(rly4) + "/off\"><button class=\"button button2\">W Pump 3 off</button></a></p>");
            } 


            if (digitalRead(rly2)) {
              client.println("<p><a href=\"/" + String(rly2) + "/on\"><button class=\"button\">Suck On</button></a></p>");
            } else {
              client.println("<p><a href=\"/" + String(rly2) + "/off\"><button class=\"button button2\">Suck off</button></a></p>");
            } 
            if (digitalRead(rly3)) {
              client.println("<p><a href=\"/" + String(rly3) + "/on\"><button class=\"button\">Pump1 On</button></a></p>");
            } else {
              client.println("<p><a href=\"/" + String(rly3) + "/off\"><button class=\"button button2\">Pump3 off</button></a></p>");
            } 

            if (digitalRead(rly5)) {
              client.println("<p><a href=\"/" + String(rly5) + "/on\"><button class=\"button\">Pump1 On</button></a></p>");
            } else {
              client.println("<p><a href=\"/" + String(rly5) + "/off\"><button class=\"button button2\">Pump5 off</button></a></p>");
            } 

            client.println("<p>Sensor 1:" + String(getSoilMoist(1)) + "</p>");
            client.println("<p>Sensor 2:" + String(getSoilMoist(2)) + "</p>");
            client.println("<p>Sensor 3:" + String(getSoilMoist(3)) + "</p>");
            client.println("<p>Sensor 4:" + String(getSoilMoist(4)) + "</p>");

            client.println("<p>Battery level: " + String(maxlipo.cellPercent()) + "% </p>");

            client.println("<p>Water level: " + String(getWaterLevel()) + "% </p>");

               
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}