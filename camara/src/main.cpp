#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <BluetoothSerial.h>
// #include <Preferences.h>

// Preferences prefs;
BluetoothSerial SerialBT;

#define BT_DISCOVER_TIME 10000
esp_spp_sec_t sec_mask = ESP_SPP_SEC_NONE; // or ESP_SPP_SEC_ENCRYPT|ESP_SPP_SEC_AUTHENTICATE to request pincode confirmation
esp_spp_role_t role = ESP_SPP_ROLE_SLAVE;  // or ESP_SPP_ROLE_MASTER

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

void startCameraServer();
void setupLedFlash(int pin);

void HandleBluetooth(void *params)
{

  if (!SerialBT.begin("esp32cam", true))
  {
    Serial.println("========== serialBT failed!");
    delay(1000);
    ESP.restart();
  }

  Serial.println("Starting discoverAsync...");
  BTScanResults *btDeviceList = SerialBT.getScanResults(); // maybe accessing from different threads!
  if (SerialBT.discoverAsync([](BTAdvertisedDevice *pDevice)
                             {
        // BTAdvertisedDeviceSet*set = reinterpret_cast<BTAdvertisedDeviceSet*>(pDevice);
        // btDeviceList[pDevice->getAddress()] = * set;
        Serial.printf(">>>>>>>>>>>Found a new device asynchronously: %s\n", pDevice->toString().c_str()); }))
  {
    delay(BT_DISCOVER_TIME);
    Serial.print("Stopping discoverAsync... ");
    SerialBT.discoverAsyncStop();
    Serial.println("discoverAsync stopped");
    delay(5000);
    if (btDeviceList->getCount() > 0)
    {
      BTAddress addr;
      int channel = 0;
      Serial.println(F("Found devices:"));
      for (int i = 0; i < btDeviceList->getCount(); i++)
      {
        BTAdvertisedDevice *device = btDeviceList->getDevice(i);
        Serial.printf(" ----- %s  %s %d\n", device->getAddress().toString().c_str(), device->getName().c_str(), device->getRSSI());
        std::map<int, std::string> channels = SerialBT.getChannels(device->getAddress());
        Serial.printf("scanned for services, found %d\n", channels.size());
        for (auto const &entry : channels)
        {
          Serial.printf("     channel %d (%s)\n", entry.first, entry.second.c_str());
        }
        if (channels.size() > 0)
        {
          addr = device->getAddress();
          channel = channels.begin()->first;
        }
      }
      if (addr)
      {
        Serial.printf("connecting to %s - %d\n", addr.toString().c_str(), channel);
        SerialBT.connect(addr, channel, sec_mask, role);
      }
    }
    else
    {
      Serial.println(F("Didn't find any devices"));
    }
  }
  else
  {
    Serial.println(F("Error on discoverAsync f.e. not workin after a \"connect\""));
  }
}
void setup()
{
  // ===========================
  // Enter your WiFi credentials
  // ===========================
  const char *ssid = "INFINITUM7A10_2.4";
  const char *password = "p73G4HBe8c";

  Serial.begin(115200);
  while (!Serial)
    delay(1);
  Serial.setDebugOutput(true);
  Serial.println();

  // prefs.begin("camara");
  // prefs.getString("SSID");
  // prefs.getString("PASS");

  HandleBluetooth(NULL);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    if (psramFound() && false)
    {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  }
  else
  {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(10000);
    ESP.restart();
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID)
  {
    s->set_vflip(s, 1);       // flip it back
    s->set_brightness(s, 1);  // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));

  startCameraServer();

  Serial.print(F("Camera Ready! Use 'http://"));
  Serial.print(WiFi.localIP());
  Serial.println(F("' to connect"));
}

void loop()
{
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}
