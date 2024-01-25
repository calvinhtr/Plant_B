#include <SPI.h> // What is used to communicate with the WiFi chip
#include <WiFiNINA.h> // Wifi library fro Arduino MKR1000 and WiFi shield
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>

int lControl =  6; //Digital pin that LED is connected to on the MKR1000
char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS; //status of wifi

WiFiServer server(80); //declare server object and spedify port, 80 is port used for internet

void setup() {
  //Uncomment serial for debugging and to see details of WiFi connection
 // Serial.begin(9600);
 // while (!Serial) {
     // wait for serial port to connect. Needed for native USB port only
//  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
  //  Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
 //   Serial.print("Attempting to connect to SSID: ");
 //   Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
 printWifiStatus();
}


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
   // Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
       // Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("Value at A0 is ");
            client.print(analogRead(A0));
            client.print("<br>");
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          WiFiDrv::analogWrite(25, 50);
          WiFiDrv::analogWrite(26, 50);
          WiFiDrv::analogWrite(27, 0);
          // digitalWrite(lControl, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          WiFiDrv::analogWrite(25, 0);
          WiFiDrv::analogWrite(26, 0);
          WiFiDrv::analogWrite(27, 0);
          // digitalWrite(lControl, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
   // Serial.println("client disconnected");
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
