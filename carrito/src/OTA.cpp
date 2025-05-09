#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
WiFiMulti wifiMulti;
bool isConnected()
{
  return WiFi.isConnected();
}
void OTATask(void *params)
{
  WiFi.mode(WIFI_STA);
  // WiFi.setSleep(false);

  // wifiMulti.addAP("tucasa", "el de tu casa");
  wifiMulti.addAP("MiniMakers", "Sayaab01");
  wifiMulti.addAP("Luciano", "chaser25");
  wifiMulti.addAP("Sayab", "PlanetarioSaYaB");
  wifiMulti.addAP("INFINITUM7A10_2.4", "p73G4HBe8c");
  wifiMulti.addAP("luciano", "chaser 25");

  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    // ESP.restart();
  }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("chaser");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";
 
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("OTA IP address: ");
  Serial.println(WiFi.localIP());
  while (true)
  {
    ArduinoOTA.handle();
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
