#include <ESP8266WiFi.h>       // wifi
#include <ESP8266WiFiMulti.h>  // wifi
#include <ESP8266HTTPClient.h> // HttpClient
#include <WiFiClientSecureBearSSL.h> // For HTTPS 
#include <TZ.h>                // time
#include <time.h>              // time 
#include <sntp.h>              // time
#include <PubSubClient.h>      // mqtt
#include "settings.h"

ESP8266WiFiMulti WiFiMulti;
WiFiClient       wifiClient;
PubSubClient     client(wifiClient);
#define SERIAL_BAUD 115200
#define FORCE_DEEPSLEEP

// time
#define MYTZ TZ_Europe_Berlin
struct tm lt; //local time
static char buf[30];      
String msg ;

std::unique_ptr<BearSSL::WiFiClientSecure>SecureClient(new BearSSL::WiFiClientSecure);

void setup() {
  Serial.begin(SERIAL_BAUD);
  //while(!Serial) {} // Wait
  delay(1000); 
  splashScreen();
  delay(1000); //reprogramming
  startWIFI();
  setupTime();

}

void loop() {
  // put your main code here, to run repeatedly:
  /* get Time */
//  static time_t lastsec {0};
  time_t now = time(&now); 
  localtime_r(&now, &lt);    
  strftime (buf, sizeof(buf), "%Y-%m-%d %T", &lt); 
  Serial.printf("%s %s\n", daysOfTheWeek[lt.tm_wday], buf);

  if (!client.connected()) {
    runMQTT();
  }

  client.loop();
  delay(3000); //wait
}


void CBReceiveMQTT(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived (");
  Serial.print(length);
  Serial.print(")[");
  Serial.print(topic);
  Serial.print("] ");
  String json;
  for (unsigned int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    json.concat((char)payload[i]);
  }
  Serial.println();

  Serial.println(json);
   
  SecureClient->setFingerprint(fingerprint);
  
  HTTPClient https;
  if (https.begin(*SecureClient, RestURL)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Authorization", BasicAuth);
  
    int httpCode = https.POST(json);
  
    // httpCode will be negative on error
    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
    } else {
      Serial.printf("[HTTPS] post... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
  
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  
}




/**
 * Establish WiFi-Connection
 * 
 * If connection times out (threshold 50 sec) 
 * device will sleep for 5 minutes and will restart afterwards.
 */
void startWIFI() {
  Serial.println("---");
  WiFi.mode(WIFI_STA);
  Serial.println("(Re)Connecting to Wifi-Network with following credentials:");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Key: ");
  Serial.println(password);
  Serial.print("Device-Name: ");
  Serial.println(espName);
  
  WiFi.hostname(espName);
  WiFiMulti.addAP(ssid, password);

  msg.concat(ssid);
  msg.concat("/");
  msg.concat(espName);
  msg.concat("/");  

  int tryCnt = 0;
  
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tryCnt++;
    if (tryCnt > 100) {
      Serial.println("");
      Serial.println("Could not connect to WiFi. Sending device to bed.");
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
  msg.concat(rssi);
  msg.concat("/");
  delay(300);
}




/**
 * Establish MQTT-Connection
 * 
 * If connection fails, device will sleep for 5 minutes and will restart afterwards.
 */
void runMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(CBReceiveMQTT);
  while (!client.connected()) {
    Serial.print("Attempting connection... ");
    Serial.println("---");
    Serial.println("Starting MQTT-Client with following credentials:");
    Serial.print("Host: ");
    Serial.println(mqtt_server);
    Serial.print("User: ");
    Serial.println(mqtt_user);
    Serial.print("Password: ");
    Serial.println(mqtt_password);
    Serial.print("ClientId: ");
    Serial.println(mqtt_clientId);
  // Attempt to connect
    if (client.connect(mqtt_clientId, mqtt_user, mqtt_password)) {
      Serial.println("Success.");
      client.loop();
    } else {
      Serial.println("Failed.");
      Serial.println("Could not connect to MQTT-Server. Sending device to bed.");
    }
  }
  client.subscribe(topic_sub,1);
}




/**
 * Dump some information on startup.
 */
void splashScreen() {
  for (int i=0; i<=2; i++) Serial.println();
  Serial.println("#######################################");
  Serial.print("# ");
  Serial.print(userAgent);
  Serial.print(" - v. ");
  Serial.println(clientVer);
  Serial.println("# -----------");
  Serial.println("# Oliver Herzog (herz08)");
  Serial.println("# Mail: esp@vogtherzog.de");
  Serial.println("# -----------");
  Serial.print("# DeviceName: ");
  Serial.println(espName);
  Serial.print("# Configured Endpoint: ");
  Serial.println(mqtt_server);
  Serial.println("#######################################");
  Serial.printf("\n\nSketchname: %s\nBuild: %s\t\tIDE: %d.%d.%d\n%s\n\n",
                (__FILE__), (__TIMESTAMP__), ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10, ESP.getFullVersion().c_str());

  for (int i=0; i<2; i++) Serial.println();
  msg.concat(userAgent);
  msg.concat("/");
}


/**
 * setupTime 
 */

void setupTime() { 
    configTime(MYTZ, "pool.ntp.org");
}
