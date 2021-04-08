// Network settings
const char* ssid = "ssid";
const char* password = "passwd";
const char* espName = "mqtt2rest";

// Endpoint settings
const char* mqtt_server = "192.168.190.54";
const char* mqtt_user = "user";
const char* mqtt_password = "user";
const char* mqtt_clientId = "mqtt2rest";

// Available Topics
const char* topic_sub      = "womke4a/WS/+/data";

const long utcOffsetInSeconds = 3600;

// Minutes to sleep between updates
int minutes2sleep = 15;

String userAgent = "herz08 - MQTT-Client 2 Rest translate";
String clientVer = "0.1";

char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

// Rest URL
String RestURL = "https://kdwmkg2lw8izsck-vh01.adb.eu-frankfurt-1.oraclecloudapps.com/ords/vh/ws/sensor";
// Fingerprint for URL created via https://www.grc.com/fingerprints.htm
const uint8_t fingerprint[20] = {0xd3, 0x1e, 0x1b, 0xa4, 0x49, 0xba, 0xa3, 0x75, 0xd6, 0x4f, 0x59, 0xc1, 0x62, 0x99, 0x5b, 0x14, 0x3f, 0xc7, 0x44, 0xee};
// Auth
String BasicAuth = "Basic ";
