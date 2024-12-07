#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <Wire.h>
#include <Preferences.h>

#define I2C_DEV_ADDR 0x55

#define I2C_SDA GPIO_NUM_12 // SDA Connected to GPIO 14
#define I2C_SCL GPIO_NUM_13 // SCL Connected to GPIO 15
TwoWire I2CSensors = TwoWire(0);
uint32_t i = 0;
String lastRequest = "";

RTC_DATA_ATTR int wifiRetries;
Preferences prefs;
void handleSerial(void *params);
void onRequest();
void onReceive(int len);
//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
// #define CAMERA_MODEL_WROVER_KIT // Has PSRAM
// #define CAMERA_MODEL_ESP_EYE // Has PSRAM
// #define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
// #define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
// #define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
// #define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
// #define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// #define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
//  ** Espressif Internal Boards **
// #define CAMERA_MODEL_ESP32_CAM_BOARD
// #define CAMERA_MODEL_ESP32S2_CAM_BOARD
// #define CAMERA_MODEL_ESP32S3_CAM_LCD
// #define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
// #define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
// const char* ssid = "INFINITUM7A10_2.4";
// const char* password = "p73G4HBe8c";

void startCameraServer();
void setupLedFlash(int pin);

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.setDebugOutput(true);
  Serial.println();
  pinMode(GPIO_NUM_33, OUTPUT);
  digitalWrite(GPIO_NUM_33, LOW);

  I2CSensors.onReceive(onReceive);
  I2CSensors.onRequest(onRequest);
  if (!I2CSensors.begin(I2C_DEV_ADDR, I2C_SDA, I2C_SCL, 100000))
  {
    Serial.println("No pude inicializar el I2C Slave");
    delay(2000);
    ESP.restart();
  }

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
    if (psramFound())
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
    esp_restart();
    return;
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

  prefs.begin("camara");
  wifiRetries = prefs.getInt("wifiRetries", 0);
  delay(50);
  xTaskCreate(handleSerial, "handleSerial", 2048, NULL, 1, NULL);
  if (prefs.isKey("SSID") && prefs.isKey("PASS"))
  {
    String SSID = prefs.getString("SSID");
    String PASS = prefs.getString("PASS");
    Serial.print(F("SSID: ["));
    Serial.print(SSID);
    Serial.println("]");
    Serial.print(F("PASS: ["));
    Serial.print(PASS);
    Serial.println("]");

    if (SSID.equals("MiniMakers")) Serial.println("Es MiniMakers");
    if (PASS.equals("Sayaab01")) Serial.println("Es MiniMakers password igual");
    WiFi.begin(SSID, PASS);
    WiFi.setSleep(false);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(F("."));
      if (retries++ > 50)
      {
        Serial.println((String("Demasiados reintentos, reiniciar: ") + String(wifiRetries)));
        wifiRetries++;
        prefs.putInt("wifiRetries", wifiRetries);
        if (wifiRetries > 3) {
          prefs.putString("SSID", "MiniMakers");
          prefs.putString("PASS", "Sayaab01");
          prefs.end();
          Serial.println(F("Fijando la default de MiniMakers con password Sayaab01"));
        }
        delay(1000);
        esp_restart();
      }
    }
    wifiRetries = 0;
    prefs.putInt("wifiRetries", wifiRetries);
    Serial.println(F(""));
    Serial.println(F("WiFi connected"));

    startCameraServer();

    Serial.print(F("Camera Ready! Use 'http://"));
    Serial.print(WiFi.localIP());
    Serial.println(F("' to connect"));
  }
}

void evaluaComando(String line, bool i2c = false)
{
  if (line.startsWith("PING"))
  {
    Serial.println(F("PING-PONG"));
    if (i2c)
      Serial.printf("Escribi %d bytes\n", I2CSensors.println("PONG"));
  }
  else if (line.startsWith("SSID:"))
  {
    Serial.print(F("SSID:"));
    prefs.putString("SSID", line.substring(5));
    Serial.println(prefs.getString("SSID"));
    if (i2c)
    {
      I2CSensors.print(F("SSID:"));
      I2CSensors.println(prefs.getString("SSID"));
    }
  }
  else if (line.startsWith("PASS:"))
  {
    Serial.print(F("PASS:"));
    prefs.putString("PASS", line.substring(5));
    Serial.println(prefs.getString("PASS"));
    if (i2c)
    {
      I2CSensors.print(F("PASS:"));
      I2CSensors.println(prefs.getString("PASS"));
    }
  }
  else if (line.startsWith("RESTART"))
  {
    if (i2c)
      I2CSensors.println("ACK");
    delay(5000);
    ESP.restart();
  }
  else if (line.startsWith("L:1"))
  {
    digitalWrite(GPIO_NUM_33, LOW);
    if (i2c)
      I2CSensors.println("ACK");
  }
  else if (line.startsWith("L:0"))
  {
    digitalWrite(GPIO_NUM_33, HIGH);
    if (i2c)
      I2CSensors.println("ACK");
  }
  else if (line.startsWith("IP"))
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print(F("IP:"));
      Serial.println(WiFi.localIP());
      if (i2c)
      {
        I2CSensors.print(F("IP:"));
        I2CSensors.println(WiFi.localIP());
      }
    }
    else
    {
      Serial.println(F("DISCONNECTED"));
      if (i2c)
        I2CSensors.println(F("DISCONNECTED"));
    }
  }
  else if (line.length() == 0)
  {
    Serial.println("NO HAY COMANDO PENDIENTE");
    I2CSensors.write(0);
  }
  else
  {
    Serial.print(F("COMANDO_INVALIDO: ["));
    Serial.print(line);
    Serial.println("]");
    if (i2c)
    {
      I2CSensors.print(F("COMANDO_INVALIDO: ["));
      I2CSensors.print(line);
      I2CSensors.println("]");
    }
  }
}

void handleSerial(void *params)
{
  while (true)
  {
    if (Serial.available() > 0)
    {
      String line = Serial.readStringUntil('\n');
      evaluaComando(line);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void onRequest()
{
  Serial.print("onRequest: ");
  Serial.print(lastRequest);
  Serial.println();
  evaluaComando(lastRequest, true);
  lastRequest = "";
}

void onReceive(int len)
{
  Serial.printf("onReceive[%d]: ", len);
  while (I2CSensors.available())
  {
    String line = I2CSensors.readStringUntil('\n');
    line.replace("\n", "");
    line.replace("\r", "");
    Serial.println(line);
    lastRequest = line;
  }
}

void loop()
{
  Serial.println("ACK");
  // digitalWrite(GPIO_NUM_33, LOW);
  // delay(1000);
  // digitalWrite(GPIO_NUM_33, HIGH);
  delay(10000);
}
