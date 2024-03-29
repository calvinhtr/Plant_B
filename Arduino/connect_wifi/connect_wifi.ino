#include <WiFiNINA.h>

#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

void printData() {
  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");
}

void loop() {
  // check the network connection once every 10 seconds:
 delay(10000);
 printData();
 Serial.println("----------------------------------------");
}

// #include <SPI.h>
// #include <WiFiNINA.h>

// void setup() {
//   //Initialize serial and wait for port to open:
//   Serial.begin(9600);
//   while (!Serial) {
//     ; // wait for serial port to connect. Needed for native USB port only
//   }

//   // check for the WiFi module:
//   if (WiFi.status() == WL_NO_MODULE) {
//     Serial.println("Communication with WiFi module failed!");
//     // don't continue
//     while (true);
//   }

//   String fv = WiFi.firmwareVersion();
//   if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
//     Serial.println("Please upgrade the firmware");
//   }

//   // print your MAC address:
//   byte mac[6];
//   WiFi.macAddress(mac);
//   Serial.print("MAC: ");
//   printMacAddress(mac);
// }

// void loop() {
//   // scan for existing networks:
//   Serial.println("Scanning available networks...");
//   listNetworks();
//   delay(10000);
// }

// void listNetworks() {
//   // scan for nearby networks:
//   Serial.println("** Scan Networks **");
//   int numSsid = WiFi.scanNetworks();
//   if (numSsid == -1) {
//     Serial.println("Couldn't get a wifi connection");
//     while (true);
//   }

//   // print the list of networks seen:
//   Serial.print("number of available networks:");
//   Serial.println(numSsid);

//   // print the network number and name for each network found:
//   for (int thisNet = 0; thisNet < numSsid; thisNet++) {
//     Serial.print(thisNet);
//     Serial.print(") ");
//     Serial.print(WiFi.SSID(thisNet));
//     Serial.print("\tSignal: ");
//     Serial.print(WiFi.RSSI(thisNet));
//     Serial.print(" dBm");
//     Serial.print("\tEncryption: ");
//     printEncryptionType(WiFi.encryptionType(thisNet));
//   }
// }

// void printEncryptionType(int thisType) {
//   // read the encryption type and print out the title:
//   switch (thisType) {
//     case ENC_TYPE_WEP:
//       Serial.println("WEP");
//       break;
//     case ENC_TYPE_TKIP:
//       Serial.println("WPA");
//       break;
//     case ENC_TYPE_CCMP:
//       Serial.println("WPA2");
//       break;
//     case ENC_TYPE_NONE:
//       Serial.println("None");
//       break;
//     case ENC_TYPE_AUTO:
//       Serial.println("Auto");
//       break;
//     case ENC_TYPE_UNKNOWN:
//     default:
//       Serial.println("Unknown");
//       break;
//   }
// }

// void printMacAddress(byte mac[]) {
//   for (int i = 5; i >= 0; i--) {
//     if (mac[i] < 16) {
//       Serial.print("0");
//     }
//     Serial.print(mac[i], HEX);
//     if (i > 0) {
//       Serial.print(":");
//     }
//   }
//   Serial.println();
// }