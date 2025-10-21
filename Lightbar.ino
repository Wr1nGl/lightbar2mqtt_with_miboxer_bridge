#include <WiFi.h>

#include "constants.h"
#include "config.h"
#include "radio.h"
#include "lightbar.h"
#include "mqtt.h"

WiFiClient wifiClient;
Radio radio(RADIO_PIN_CE, RADIO_PIN_CSN);
MQTT mqtt(&wifiClient, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, MQTT_ROOT_TOPIC, HOME_ASSISTANT_DISCOVERY, HOME_ASSISTANT_DISCOVERY_PREFIX);
boolean WIFI_connection_failed = false;
void setupWifi()
{
  Serial.print("[WiFi] Connecting to network \"");
  Serial.print(WIFI_SSID);
  Serial.print("\"...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname(mqtt.getClientId().c_str());

  uint retries = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    retries++;
    if (NUMBER_OF_WIFI_RETRIES == -1 && retries >= NUMBER_OF_WIFI_RETRIES){
      ESP.restart();
    }
    else if (retries >= NUMBER_OF_WIFI_RETRIES){
      WIFI_connection_failed = true;
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED){
    Serial.println();
    Serial.println("[WiFi] Not connected! If you want to retry for connection pres RST button.");
  }
  else{
    Serial.println();
    Serial.println("[WiFi] connected!");

    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("##################################################################");
  Serial.println("# lightbar2mqtt_with_miboxer_bridge             (Version " + constants::VERSION + ")  #");
  Serial.println("# https://github.com/Wr1nGl/lightbar2mqtt_with_miboxer_bridge    #");
  Serial.println("##################################################################");

  radio.setup();

  if (NUMBER_OF_WIFI_RETRIES > 0)
    setupWifi();

  //no need to connect to MQTT if we dont have wifi
  if (NUMBER_OF_MQTT_RETRIES != 0 && WiFi.isConnected())
    mqtt.setup();
  
  for (int i = 0; i < sizeof(REMOTES) / sizeof(SerialWithName); i++)
  {
    Remote *remote = new Remote(&radio, REMOTES[i].serial, REMOTES[i].name, REMOTES[i].miboxer_groups_len, REMOTES[i].miboxer_groups);
    if (NUMBER_OF_MQTT_RETRIES != 0 && WiFi.isConnected() && !mqtt.get_MQTT_connection_failed())
      mqtt.addRemote(remote);
  }

  for (int i = 0; i < sizeof(LIGHTBARS) / sizeof(SerialWithName); i++)
  {
    Lightbar *lightbar = new Lightbar(&radio, LIGHTBARS[i].serial, LIGHTBARS[i].name);
    if (NUMBER_OF_MQTT_RETRIES != 0 && WiFi.isConnected() && !mqtt.get_MQTT_connection_failed())
      mqtt.addLightbar(lightbar);
  }
}

void loop()
{
  //if wifi havent failed and is enabled
  if(NUMBER_OF_WIFI_RETRIES != 0 && !WIFI_connection_failed){
    //if we lost connection reconnect
    if (!WiFi.isConnected())
    {
      Serial.println("[WiFi] connection lost!");
      setupWifi();
    }
    //if we have wifi and if mqtt havent failed
    if (NUMBER_OF_MQTT_RETRIES != 0 && WiFi.isConnected() && !mqtt.get_MQTT_connection_failed())
      mqtt.loop();
  }
  
  radio.loop();
}