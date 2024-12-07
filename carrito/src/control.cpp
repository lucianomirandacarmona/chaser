#include <BluetoothSerial.h>
#include <Wire.h>

#ifdef __has_include
  #if __has_include("config_local.h")
    #include "config_local.h"
  #else
    #warning "El archivo config_local.h no se ha encontrado. Usando el nombre predeterminado para el dispositivo Bluetooth."
  #endif
#else
  #include "config_local1.h"  // En versiones de compiladores más antiguas que no soportan __has_include
#endif
#ifndef BT_DEVICE_NAME
  #define BT_DEVICE_NAME "carrito-bt-sayab"
  #warning "El archivo config_local.h no se ha encontrado. Usando el nombre predeterminado para el dispositivo Bluetooth."
#endif
#include <control.h>
#include <motores.h>
#include <luces.h>
#include <Preferences.h>

Preferences prefs;
BluetoothSerial esp32BT;
bool ping();
bool enviarSSID(String SSID);
bool enviarPASS(String PASS);
bool enviarRESTART();
String preguntarIP();
bool enviarFLASH(bool luz);

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
    esp32BT.begin(BT_DEVICE_NAME);

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
                Serial.println("Recibiendo datos WiFi");
                String SSID = esp32BT.readStringUntil('\n');
                SSID = SSID.substring(SSID.indexOf(':') + 1);
                String PASS = esp32BT.readStringUntil('\n');
                PASS = PASS.substring(PASS.indexOf(':') + 1);
                Serial.println("Recibi SSID: " + SSID);
                Serial.println("Recibi PASS: " + PASS);
                Serial.println("Almacenando datos WiFi en preferencias");
                prefs.begin("carrito");
                prefs.putString("SSID", SSID);
                prefs.putString("PASS", PASS);
                prefs.end();
                Serial.println("Enviando datos WiFi a la camara");
                if (!ping())
                {
                    Serial.println("No esta conectada la camara");
                }
                else
                {
                    if (!enviarSSID(SSID))
                        Serial.println("No pude mandar SSID");
                    if (!enviarPASS(PASS))
                        Serial.println("No pude enviar PASS");
                    Serial.println("Reiniciando camara");
                    if (!enviarRESTART())
                        Serial.println("No pude enviar RESTART");
                }
            }
            else if (bt == ',')
            {
                Serial.println("Pidiendo la IP de la camara");
                String ipCamara = preguntarIP();
                ipCamara = ipCamara.substring(ipCamara.indexOf(':') + 1);
                Serial.println("IP de la camara: " + ipCamara);
                esp32BT.println(ipCamara);
            }
            // xd();
        }
        taskYIELD();
    }
}

bool ping()
{
    Serial.print(F("PING:"));
    Wire.beginTransmission((I2C_DEV_ADDR));
    Wire.println(F("PING"));
    uint8_t error = Wire.endTransmission(true);
    if (error != 0)
    {
        Serial.printf("endTransmission: %u/n", error);
        return false;
    }
    ulong tini = millis();
    while (Wire.requestFrom(I2C_DEV_ADDR, 5) > 0 && millis() - tini <= 5e3)
    {
        String response = Wire.readStringUntil('\n');
        if (response.startsWith("PONG"))
        {

            Serial.println(response);
            while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
                delay(1);
            return true;
        }
    }
    return false;
}
bool enviarSSID(String SSID)
{
    Serial.print(String("Enviando SSID:") + SSID + " -> ");
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.println(String("SSID:") + SSID);
    uint8_t error = Wire.endTransmission(true);
    if (error != 0)
    {
        Serial.printf("endTransmission: %u/n", error);
        return false;
    }
    ulong tini = millis();
    while (Wire.requestFrom(I2C_DEV_ADDR, 5) > 0 && millis() - tini <= 5e3)
    {
        String response = Wire.readStringUntil('\n');
        if (response.startsWith("SSID:"))
        {

            Serial.println("Respuesta: " + response);
            while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
                delay(1);
            return true;
        }
    }
    return false;
}
bool enviarPASS(String PASS)
{
    Serial.print(String("Enviando PASS:") + PASS + " -> ");
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.println(String("PASS:") + PASS);
    uint8_t error = Wire.endTransmission(true);
    if (error != 0)
    {
        Serial.printf("endTransmission: %u/n", error);
        return false;
    }
    ulong tini = millis();
    while (Wire.requestFrom(I2C_DEV_ADDR, 5) > 0 && millis() - tini <= 5e3)
    {
        String response = Wire.readStringUntil('\n');
        if (response.startsWith("PASS:"))
        {

            Serial.println("Respuesta: " + response);
            while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
                delay(1);
            return true;
        }
    }
    return false;
}
bool enviarRESTART()
{
    Serial.print(F("RESTART"));
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.println(String("RESTART"));
    uint8_t error = Wire.endTransmission(true);
    if (error != 0)
    {
        Serial.printf("endTransmission: %u/n", error);
        return false;
    }
    ulong tini = millis();
    while (Wire.requestFrom(I2C_DEV_ADDR, 4) > 0 && millis() - tini <= 5e3)
    {
        String response = Wire.readStringUntil('\n');
        if (response.startsWith("ACK"))
        {

            Serial.println(response);
            // while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
            //     delay(1);
            return true;
        }
    }
    return false;
}
String preguntarIP()
{
    Serial.print(F("IP:"));
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.println(String("IP"));
    uint8_t error = Wire.endTransmission(true);
    if (error != 0)
    {
        Serial.printf("endTransmission: %u/n", error);
        return String();
    }
    ulong tini = millis();
    while (Wire.requestFrom(I2C_DEV_ADDR, 64) > 0 && millis() - tini <= 5e3)
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
bool enviarFLASH(bool luz)
{
    Serial.print(F("L:"));
    Wire.beginTransmission(I2C_DEV_ADDR);
    Wire.println(luz ? String("L:1") : String("L:0"));
    uint8_t error = Wire.endTransmission(true);
    if (error != 0)
    {
        Serial.printf("endTransmission: %u/n", error);
        return false;
    }
    ulong tini = millis();
    while (Wire.requestFrom(I2C_DEV_ADDR, 4) > 0 && millis() - tini <= 5e3)
    {
        String response = Wire.readStringUntil('\n');
        if (response.startsWith("ACK"))
        {

            Serial.println(response);
            while (Wire.requestFrom(I2C_DEV_ADDR, 1) > 0 && Wire.read() != 0)
                delay(1);
            return true;
        }
    }
    return false;
}