#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

#define AP_SSID "SynScan_WiFi_1234" // Access Point SSID
#define AP_PASSWORD      "password" // Access Point password
#define WiFi_SSID            "HOME" // External WiFi network SSID
#define WiFi_PASSWORD    "password" // External WiFi network password

#define udpPort 11880
#define tcpPort 11881
#define mountBaudRate 9600
#define bufferSize 8192
#define SerialBufferSize 64
#define timeOut 5

int remotePortUDP;
uint8_t serialIndex = 0;
uint8_t udpIndex = 0;
uint8_t udpBuffer[bufferSize];
char udpMessage[bufferSize];
char serialBuffer[SerialBufferSize];
byte data = 0;
bool Wificommand = false;
boolean ignore = false;

uint8_t buf1[bufferSize];
uint16_t i1=0;
uint8_t buf2[bufferSize];
uint16_t i2=0;

WiFiUDP udp;
IPAddress remoteClientIP;
WiFiServer serverTCP(tcpPort);
WiFiClient clientTCP;

void setup() {
  delay(5000);
  Serial.begin(mountBaudRate);
  WiFi.mode(WIFI_AP_STA);
  IPAddress ip(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, ip, subnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD); // configure ssid and password for softAP
  WiFi.begin(WiFi_SSID, WiFi_PASSWORD);        // Connect to external WiFi network for STA
  udp.begin(udpPort);
  serverTCP.begin();
}



void loop() {
  wifiToMountUDP();
  mountToWiFiUDP();
  // serialProxyTCP(); // ToDo: Implement TCP serial bridge to be used as VCOM
}



// Send over data from UDP client to mount via serial interface
void wifiToMountUDP() {
  int packetSize = udp.parsePacket();
  if(packetSize > 0) {                     // when data arrives via WiFi
    remoteClientIP = udp.remoteIP();
    remotePortUDP = udp.remotePort();
    udp.read(udpBuffer, bufferSize);        //read the incoming data
    for(int j = 0; j < packetSize; j++) {  // write it to the log for debugging purposes
      udpMessage[j] = udpBuffer[j];
    }
    Wificommand = udpBuffer[0] == 58 ? false : true;
    if(!Wificommand) {
      Serial.write(udpBuffer, packetSize);  // forward the recieved data straight to the serial connection to the telescope mount
      ignore = true;                        // we need to ignore the first characters that we get from the telescope mount (an echo of our command / garbage) until we get the "=" character that signals the beginning of the actual response
      delay(15);
    } else {
      udp.beginPacket(remoteClientIP, remotePortUDP);
      udp.write("+CWMODE_CUR:1");
      udp.endPacket();
      yield();
      delay(10);
      udp.beginPacket(remoteClientIP, remotePortUDP);
      udp.write("OK");
      udp.endPacket();
      yield();
    }
  }
}



// Send over serial data from mount to UDP client
void mountToWiFiUDP() {
  int SerialSize = 0;
  SerialSize = Serial.available();  // Test for Serial Data Received
  if(SerialSize > 0) { // when data arrives from the mount via the serial port
    serialIndex = 0;
    for(int i = 0; i < SerialSize; i++) {
      char data = Serial.read();
      serialBuffer[serialIndex] = data;
      serialIndex++;
    }
    if(serialBuffer[0] == 61 || serialBuffer[0] == 33) { // Now we send the message recieved from the telescope mount, as an UDP packet to the client app (via WiFi):
      udp.beginPacket(remoteClientIP, remotePortUDP);
      for(int j = 0; j < SerialSize; j++) {
        udpMessage[j] = serialBuffer[j];
      }
      udp.write(serialBuffer, serialIndex);
      udp.endPacket();
    }
    yield();
    serialIndex = 0;
  }
}



// Serial bridge on TCP port
void serialProxyTCP() {
  if(!clientTCP.connected()) { // if client not connected
    clientTCP = serverTCP.available(); // wait for it to connect
    return;
  }
  // here we have a connected client
  if(clientTCP.available()) {
    while(clientTCP.available()) {
      buf1[i1] = (uint8_t)clientTCP.read(); // read char from client (RoboRemo app)
      if(i1<bufferSize-1) i1++;
    }
    // now send to UART:
    Serial.write(buf1, i1);
    i1 = 0;
  }
  if(Serial.available()) {
    // read the data until pause:
    while(1) {
      if(Serial.available()) {
        buf2[i2] = (char)Serial.read(); // read char from UART
        if(i2<bufferSize-1) i2++;
      } else {
        delay(timeOut);
        yield();
        if(!Serial.available()) {
          break;
        }
      }
    }
    // now send to WiFi:
    clientTCP.write((char*)buf2, i2);
    i2 = 0;
  }
}
