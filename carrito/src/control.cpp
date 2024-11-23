#include <BluetoothSerial.h>
#include <Wire.h>

#include <control.h>
#include <motores.h>
#include <luces.h>
#include <Preferences.h>

Preferences prefs;
BluetoothSerial esp32BT;
void xd()
{
    digitalWrite(5, true);
    delay(100);
    digitalWrite(5, false);
    delay(100);
}
void control(void *parametros)
{
    Serial.println("Control iniciado");
    esp32BT.begin("chaser");

    while (true)
    {
        if (esp32BT.available())
        {
            /* String bt = esp32BT.readString();
             Serial.println(bt);
             if (bt.equals(String(ARRIBA)))
             {
                 setvelocidad(45);
                 setdireccion(1);
             }
             if (bt.equals(String(ARRIBA_ALTO)) || bt.equals(String(ABAJO_ALTO)))
             {
                 setdireccion(0);
             }
             if ( bt.equals(String(ABAJO)))
             {
                 setvelocidad(45);
                 setdireccion(-1);
             }
             if ( bt.equals(String(DERECHA)))
             {
                 setvelocidadRotacion(35);
                 setRotacion(1);
             }
             if (bt.equals(String(IZQUIERDA)))
             {
                 setvelocidadRotacion(35);
                 setRotacion(-1);
             }
             if ( bt.equals(String(DERECHA_ALTO)) || bt.equals(String(IZQUIERDA_ALTO)))
             {
                 setRotacion(0);
             }
             if ( bt.equals(String(TERMINA_COMANDO)))
             {
                 setdireccion(0);
                 setRotacion(0);
             }
             if (bt.equals(String("101")))
             {
                 // setBrillo(1);
                 // setStripe(255,0,0);
                 setLucesEffect(RAINBOW);
             }
             if (bt.equals(String("69")))
             {
                 // setBrillo(0);
                 // setStripe(0,255,0);
                 setLucesEffect(SOLID_GREEN);
             }*/
            int bt = esp32BT.read();
            if (bt == 'x')
            {
                Serial.printf("%d,%d\n", esp32BT.read(), esp32BT.read());
            }
            else if (bt == '.')
            {
                String SSID = esp32BT.readStringUntil('\n').substring(SSID.indexOf(':'));
                String PASS = esp32BT.readStringUntil('\n').substring(PASS.indexOf(':'));
                Serial.println(SSID);
                Serial.println(PASS); /*-/*/
                prefs.begin("carrito");
                prefs.putString("SSID", SSID);
                prefs.putString("PASS", PASS);
                prefs.end();
            }
            else if (bt == ',')
            {
                Serial.println("enviar la IP de la camara");
            }
            // xd();
        }
        taskYIELD();
    }
}

void ping()
{
    Serial.print(F("ping:"));
    Wire.beginTransmission((I2C_DEV_ADDR));
    Wire.println(F("ping"));
    uint8_t error = Wire.endTransmission(true);
    if (error != 0)
        Serial.printf("endTransmission: %u/n", error);
    while (Wire.requestFrom(I2C_DEV_ADDR, 5) > 0)
    {
        String response = Wire.readStringUntil('\n');
        if (response.startsWith("PONG"))
        {

            Serial.println(response);
            while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
                delay(1);
            break;
        }
    }
}
void enviarSSID(String SSID)
{
  Serial.print(F("SSID:"));
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.println(String("SSID:") + SSID);
  uint8_t error = Wire.endTransmission(true);
  if (error != 0)
    Serial.printf("endTransmission: %u\n", error);
  while (Wire.requestFrom(I2C_DEV_ADDR, 5) > 0)
  {
    String response = Wire.readStringUntil('\n');
    if (response.startsWith("SSID:"))
    {
 
      Serial.println(response);
      while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
        delay(1);
      break;
    }
  }
}
void enviarPASS(String PASS)
{
  Serial.print(F("PASS:"));
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.println(String("PASS:") + PASS);
  uint8_t error = Wire.endTransmission(true);
  if (error != 0)
    Serial.printf("endTransmission: %u\n", error);
  while (Wire.requestFrom(I2C_DEV_ADDR, 5) > 0)
  {
    String response = Wire.readStringUntil('\n');
    if (response.startsWith("PASS:"))
    {
 
      Serial.println(response);
      while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
        delay(1);
      break;
    }
  }
}
void enviarRESTART()
{
  Serial.print(F("RESTART"));
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.println(String("RESTART"));
  uint8_t error = Wire.endTransmission(true);
  if (error != 0)
    Serial.printf("endTransmission: %u\n", error);
  while (Wire.requestFrom(I2C_DEV_ADDR, 4) > 0)
  {
    String response = Wire.readStringUntil('\n');
    if (response.startsWith("ACK"))
    {
 
      Serial.println(response);
      while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
        delay(1);
      break;
    }
  }
}
String preguntarIP()
{
  Serial.print(F("IP:"));
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.println(String("IP"));
  uint8_t error = Wire.endTransmission(true);
  if (error != 0)
    Serial.printf("endTransmission: %u\n", error);
  while (Wire.requestFrom(I2C_DEV_ADDR, 64) > 0)
  {
    String response = Wire.readStringUntil('\n');
    if (response.startsWith("IP:") || response.startsWith("DISCONNECTED"))
    {
 
      Serial.println(response);
      while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
        delay(1);
      return response;
    }
  }
  return String();
}
void enviarFLASH(bool luz)
{
  Serial.print(F("L:"));
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.println(luz ? String("L:1") : String("L:0"));
  uint8_t error = Wire.endTransmission(true);
  if (error != 0)
    Serial.printf("endTransmission: %u\n", error);
  while (Wire.requestFrom(I2C_DEV_ADDR, 4) > 0)
  {
    String response = Wire.readStringUntil('\n');
    if (response.startsWith("ACK"))
    {
 
      Serial.println(response);
      while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
        delay(1);
      break;
    }
  }
}