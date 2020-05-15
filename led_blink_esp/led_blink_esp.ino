// Load Wi-Fi library
#include <ESP8266WiFi.h>

#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 2;

void adjustGPIOsFromRequest(String httpRequest) {
  if (httpRequest.indexOf("GET /5/on") >= 0) {
      Serial.println("GPIO 5 on");
      output5State = "on";
      digitalWrite(output5, HIGH);
    } else if (httpRequest.indexOf("GET /5/off") >= 0) {
      Serial.println("GPIO 5 off");
      output5State = "off";
      digitalWrite(output5, LOW);
    } else if (httpRequest.indexOf("GET /4/on") >= 0) {
      Serial.println("GPIO 4 on");
      output4State = "on";
      digitalWrite(output4, HIGH);
    } else if (httpRequest.indexOf("GET /4/off") >= 0) {
      Serial.println("GPIO 4 off");
      output4State = "off";
      digitalWrite(output4, LOW);
    }
}

void renderHtmlPage(WiFiClient client, String httpRequest) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();    
    // Display the HTML web page
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    // CSS to style the on/off buttons 
    // Feel free to change the background-color and font-size attributes to fit your preferences
    client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    client.println(".button2 {background-color: #77878A;}</style></head>");
    
    // Web Page Heading
    client.println("<body><h1>ESP8266 Web Server</h1>");
    
    // Display current state, and ON/OFF buttons for GPIO 5  
    client.println("<p>GPIO 5 - State " + output5State + "</p>");
    // If the output5State is off, it displays the ON button       
    if (output5State=="off") {
      client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
    } else {
      client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
    } 
       
    // Display current state, and ON/OFF buttons for GPIO 4  
    client.println("<p>GPIO 4 - State " + output4State + "</p>");
    // If the output4State is off, it displays the ON button       
    if (output4State=="off") {
      client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
    } else {
      client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
    }
    client.println("</body></html>");
    
    // The HTTP response ends with another blank line
    client.println();
}

WiFiServer server(80);
void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  // Connect to Wi-Fi network with SSID and password
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

String requestFromClient(WiFiClient client) {
  Serial.println("New Client.");
  const long timeOutTime = 2000; // milliseconds (example: 2000ms = 2s)
  
  String httpRequest = "";
  String currentLine = "";
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime; 
  bool didTimedOut = false;
  
  while (client.connected() && !didTimedOut ) { // loop while the client's connected         
    if (client.available()) {
      char currentChar = client.read();
      Serial.write(currentChar);
      httpRequest += currentChar;
      if (currentChar == '\n') {
        if (currentLine.length() == 0) {
          break;
        } else {
          currentLine = "";
        }
      } else if (currentChar != '\r') {
        currentLine += currentChar;
      }
    }

    currentTime = millis();
    didTimedOut = currentTime - previousTime > timeOutTime;
  }

  return httpRequest;
}

void loop(){
  WiFiClient client = server.available();
  
  if (client) {
    String request = requestFromClient(client);
    Serial.print("Request = ");
    Serial.println(request);
    if (request.length() > 0) {
      adjustGPIOsFromRequest(request);
      renderHtmlPage(client, request);
    }
    
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
