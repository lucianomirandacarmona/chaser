#include <BluetoothSerial.h>
#include <control.h>
#include <motores.h>
#include <luces.h>
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
           int bt=esp32BT.read();
             if (bt=='x')
            {
                Serial.printf("%d,%d\n",esp32BT.read(),esp32BT.read());
                
            }
            // xd();
        }
        taskYIELD();
    }
}