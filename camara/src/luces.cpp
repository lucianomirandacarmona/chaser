#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <luces.h>
#include <carrito.h>


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
float brillo = 1.0;
int effect = SOLID_GREEN;

uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
void setStripe(int r, int g, int b)
{
    strip.fill(strip.Color(r, g, b));
    strip.show();
}
void luces(void *)
{
    strip.begin();
    strip.show();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
    display.setRotation(2);
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Connecting to SSID\n'adafruit':");
    display.print("connected!");
    display.println("IP: 10.0.1.23");
    display.println("Sending val #0");
    display.setCursor(0, 0);
    display.display(); // actually display all of the above
    while (true)
    {
        switch (effect)
        {
        case RAINBOW:
            rainbowCycle(3);
            break;
        case SOLID_GREEN:
        default:
            setStripe(0, 128, 0);
        }
        taskYIELD();
    }
}
void rainbowCycle(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256 * 5; j++)
    { // 5 cycles of all colors on wheel
        for (i = 0; i < strip.numPixels(); i++)
        {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

void setBrillo(float b)
{
    brillo = b;
}

void setLucesEffect(int e)
{
    effect = e;
}
