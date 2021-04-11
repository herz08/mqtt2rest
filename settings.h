// Network settings
String userAgent            = "herz08 - MQTT-Client 2 Rest translate";
const char* ssid            = "ssid";
const char* password        = "passwd";
const char* espName         = "mqtt2rest";

// Endpoint settings
const char* mqtt_server     = "192.168.190.54";
const char* mqtt_user       = "user";
const char* mqtt_password   = "user";
const char* mqtt_clientId   = espName;

// Available Topics
const char* topic_sub1      = "womke4a/WS/+/data";
const char* topic_sub2      = "womke4a/mqtt2rest/Upgrade/#";
const char* topic_upgrade   = "womke4a/mqtt2rest/Upgrade";
const char* topic_upg_short = "womke4a/mqtt2rest/Upgrade/short";
const char* topic_status    = "womke4a/mqtt2rest/status";

// Time settings
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

// URLs
String UpgradeUrlShort      = "http://192.168.******/fw/";
String RestURL              = "https://kdwmkg2lw8izsck-vh01.adb.eu-frankfurt-1.oraclecloudapps.com/ords/vh/ws/sensor";
// Fingerprint for URL created via https://www.grc.com/fingerprints.htm
static const char fingerprint[] PROGMEM = "D3:1E:1B:A4:49:BA:A3:75:D6:4F:59:C1:62:99:5B:14:3F:C7:44:EE";

// Auth
String BasicAuth = "Basic *******";
