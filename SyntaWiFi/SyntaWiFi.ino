#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define mountBaudRate 9600

#define bufferSize 8192
#define SerialBufferSize 64

#define WiFi_Access_Point_Name "SynScan_WiFi_1234"   // Name of the WiFi access point this device will create for your tablet/phone to connect to.
#define WiFi_Password "fuk4dementia"
#define udpPort 11880 // UDP udpPort expected by SynScan
WiFiUDP udp;
IPAddress remoteIp;
int UDPremoteudpPort;

uint8_t udpBuffer[bufferSize];
uint8_t udpIndex = 0;
char udpMessage[bufferSize];
char serialBuffer[SerialBufferSize];
uint8_t serialIndex = 0;

byte data = 0;
bool Wificommand = false;
boolean ignore = false;  // Because the mount connection seems to share the wire for RX and TX, commands sent to the mount are recieved back, as an "echo", and must be ignored.

void setup() {
  delay(5000);
  Serial.begin(mountBaudRate);
  WiFi.mode(WIFI_AP);
  IPAddress ip(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, ip, subnet);
  WiFi.softAP(WiFi_Access_Point_Name, WiFi_Password); // configure ssid and password for softAP
  udp.begin(udpPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  int SerialSize = 0;
  if (packetSize > 0) {                    // when data arrives via WiFi
    // take note of address and port to send our response to:
    remoteIp = udp.remoteIP();
    UDPremoteudpPort = udp.remotePort();
    udp.read(udpBuffer, bufferSize); //read the incoming data
    for (int j = 0; j < packetSize; j++) { // write it to the log for debugging purposes
      udpMessage[j] = udpBuffer[j];
    }
    if (udpBuffer[0] == 58) {
      Wificommand = false;
    } else {
      Wificommand = true;
    }
    if (Wificommand == false) {
      Serial.write(udpBuffer, packetSize);  // forward the recieved data straight to the serial connection to the telescope mount
      ignore = true;    // we need to ignore the first characters that we get from the telescope mount (an echo of our command / garbage) until we get the "=" character that signals the beginning of the actual response
      delay(15);
    } else {
      udp.beginPacket(remoteIp, UDPremoteudpPort);
      udp.write("+CWMODE_CUR:1");
      udp.endPacket();
      yield();
      delay(10);
      udp.beginPacket(remoteIp, UDPremoteudpPort);
      udp.write("OK");
      udp.endPacket();
      yield();
    }
  }
  SerialSize = Serial.available();  // Test for Serial Data Received
  if (SerialSize > 0) { // when data arrives from the mount via the serial port
    serialIndex = 0;
    for (int i = 0; i < SerialSize; i++) {
      char data = Serial.read();
      serialBuffer[serialIndex] = data;
      serialIndex++;
    }
    byte firstChar = serialBuffer[0];
    if (firstChar == 61 || firstChar == 33) { // Now we send the message recieved from the telescope mount, as an UDP packet to the client app (via WiFi):
      udp.beginPacket(remoteIp, UDPremoteudpPort);
      for (int j = 0; j < SerialSize; j++) {
        udpMessage[j] = serialBuffer[j];
      }
      udp.write(serialBuffer, serialIndex);
      udp.endPacket();
      yield();
      serialIndex = 0;
    } else {
      yield();
      serialIndex = 0;
    }
  }
}
