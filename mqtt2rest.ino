/*
  mqtt2rest.ino - send defined MQTT Messages to a Rest Endpoint
  Created by Oliver Herzog, April 11th 2021.
  Released under the LGPL v2.1.
  It is the author's intention that this work may be used freely for private
  and commercial use so long as any changes/improvements are freely shared with
  the community under the same terms.
*/


#include <ESP8266WiFi.h>       // wifi
#include <ESP8266WiFiMulti.h>  // wifi
#include <ESP8266HTTPClient.h> // HttpClient
#include <ESP8266httpUpdate.h> // Upgrade 
#include <WiFiClientSecureBearSSL.h> // For HTTPS
#include <PolledTimeout.h>     // time 
#include <TZ.h>                // time
#include <time.h>              // time 
#include <PubSubClient.h>      // mqtt
#include "settings.h"

ESP8266WiFiMulti WiFiMulti;
WiFiClient       wifiClient;
PubSubClient     PSClient(wifiClient);

#define SERIAL_BAUD 115200
#define MYTZ TZ_Europe_Berlin

const char* Version = "1.1.1";
struct tm lt; //local time
static char buf[30];
static esp8266::polledTimeout::periodicMs showTimeNow(60000);      
String msg ;

/**
 * setup
 */
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000); 
  splashScreen();
  delay(1000); 
  startWIFI();
  setupTime();
  // Check for Updates first
  if (!PSClient.connected()) {
    connectMQTT();
  }


}
/**
 * loop
 */
void loop() {

  if (!PSClient.connected()) {
    connectMQTT();
  }

  if (showTimeNow) {
    sendStatus();
  }
  
  yield();
  PSClient.loop();
  delay(3000); //wait
}
/**
 * sendStatus
 */
void sendStatus () {
  time_t now = time(&now); 
  localtime_r(&now, &lt);    
  strftime (buf, sizeof(buf), "%Y-%m-%d %T", &lt); 
  Serial.printf("%s %s\n", daysOfTheWeek[lt.tm_wday], buf);

  String dt = String(daysOfTheWeek[lt.tm_wday]).c_str();
         dt.concat(' ');
         dt.concat(buf); 
  String dat = "{ ";
         dat.concat("\"clientId\": \""); 
         dat.concat( mqtt_clientId );
         dat.concat("\", \"timestampData\":\"");  
         dat.concat(String(dt).c_str());
         dat.concat("\", \"msg\":\"");  
         dat.concat(String(msg).c_str());         
         dat.concat("\" }"); 
    
  Serial.println("send Data");
  PSClient.publish(topic_status, String(dat).c_str(), true);
  msg = ""; // clear message
}


/**
 * CBReceiveMQTT
 */
void CBReceiveMQTT(char* topic, byte* payload, unsigned int length) {
  String payloadString;
  
  Serial.print("Message arrived (");
  Serial.print(length);
  Serial.print(")[");
  Serial.print(topic);
  Serial.print("] ");

  for (unsigned int i=0;i<length;i++) {  // toString
    payloadString.concat((char)payload[i]);
  }
    if (strcmp(topic,topic_upgrade)==0) {
        Serial.println("Upgrade Requested");
        handleUpgrade(payloadString);
        return;
    }    
    if (strcmp(topic,topic_upg_short)==0) {
        Serial.println("Upgrade Requested");
        String url  = UpgradeUrlShort;
               url += payloadString; 
        handleUpgrade(url);
        return;
    }
    if (postRest(payloadString))
        return;
        
  Serial.println("Nothing to do!"); 

  
}

/**
 * Post to Rest Endpoint
 * 
 */

bool postRest (String Payload){
  bool result = false;
  BearSSL::WiFiClientSecure *SecureClient = new BearSSL::WiFiClientSecure();
  SecureClient->setFingerprint(fingerprint);
  
  HTTPClient https;
  if (https.begin(*SecureClient, RestURL)) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Authorization", BasicAuth);
  
    int httpCode = https.POST(Payload);
  
    // httpCode will be negative on error
    if (httpCode > 0) {
      Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
      result = true;
    } else {
      Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
  
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return result;
  
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
  Serial.print("Device-Name: ");
  Serial.println(espName);
  
  WiFi.hostname(espName);
  WiFiMulti.addAP(ssid, password);

  msg.concat(" SSID: "); 
  msg.concat(ssid); 
  msg.concat(": espName: ");
  msg.concat(espName);
  msg.concat(":");
    
  int tryCnt = 0;
  
  while (WiFiMulti.run() != WL_CONNECTED) {
    yield();
    delay(500);
    Serial.print(".");
    tryCnt++;
    if (tryCnt > 100) {
      Serial.println("Could not connect to WiFi.");
      return;
    }
  }

  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  long rssi = WiFi.RSSI();
  msg.concat(" RSSI: ");
  msg.concat(rssi);
  msg.concat(": LocalIP: "); 
  msg.concat(WiFi.localIP().toString());
  msg.concat(":"); 
  
  delay(300);
}

/**
 * Establish MQTT-Connection
 * 
 * If connection fails, device will sleep for 5 minutes and will restart afterwards.
 */
void connectMQTT() {
  PSClient.setServer(mqtt_server, 1883);
  PSClient.setCallback(CBReceiveMQTT);
  while (!PSClient.connected()) {
  // Attempt to connect
    if (PSClient.connect(mqtt_clientId, mqtt_user, mqtt_password)) {
      Serial.println("MQTT Success.");
      PSClient.loop();
    } else {
      Serial.println("MQTT Failed.");
      Serial.println("Could not connect to MQTT-Server. Sending device to bed.");
    }
  }
  PSClient.subscribe(topic_sub1,1);
  PSClient.subscribe(topic_sub2,1);
}

/**
 * handleUpgrade 
 */
bool handleUpgrade (String UpgradeURL){

    Serial.println(UpgradeURL);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient,UpgradeURL,Version);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            msg.concat(ESPhttpUpdate.getLastErrorString());
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            msg.concat( "HTTP_UPDATE_NO_UPDATES" );
            return false;

        case HTTP_UPDATE_OK:
            msg.concat( "HTTP_UPDATE_OK" );
            return true;
    }
   return false;
}


/**
 * Dump some information on startup.
 */
void splashScreen() {
  for (int i=0; i<=2; i++) Serial.println();
  Serial.println("#################################################################################");
  Serial.print("# ");
  Serial.print(userAgent);
  Serial.print(" - v. ");
  Serial.println(Version);
  Serial.println("# -----------");
  Serial.println("# Oliver Herzog (herz08)");
  Serial.println("# Mail: esp@vogtherzog.de");
  Serial.println("# -----------");
  Serial.print("# DeviceName: ");
  Serial.println(espName);
  Serial.print("# Configured Endpoint: ");
  Serial.println(mqtt_server);
  Serial.println("#################################################################################");
  Serial.printf("\n\nSketchname: %s\nBuild: %s\t\tIDE: %d.%d.%d\n%s\n\n",
                (__FILE__), (__TIMESTAMP__), ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10, ESP.getFullVersion().c_str());

  for (int i=0; i<2; i++) Serial.println();
  msg.concat(userAgent);
  msg.concat(":");
  msg.concat(Version);
  msg.concat(":");  
}


/**
 * setupTime 
 */

void setupTime() { 
    configTime(MYTZ, "pool.ntp.org");
}

