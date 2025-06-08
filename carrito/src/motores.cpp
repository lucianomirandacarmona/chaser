#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_PWMServoDriver.h>

#include <Servo.h>
#include <motores.h>
// MACROS are defined here
Servo miservo = Servo();
// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);

// Depending on your servo make, the pulse width min and max may vary, you
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN 150  // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 600  // This is the 'maximum' pulse length count (out of 4096)
#define USMIN 600     // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 2400    // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

int rotacion = 0;
int direccion = 0;
int velocidad = 0;
int velocidadRotacion = 0;
bool cambioRuedas = false, cambioBrazo = false;
RTC_DATA_ATTR int motoresBrazo[] = {90, 45, 180, 180, 0, 0}; // 90
void setRotacion(int r)
{
    if (r != rotacion)
    {
        rotacion = r;
        cambioRuedas = true;
    }
}
void setvelocidad(int r)
{
    if (r != rotacion)
    {
        velocidad = r;
        cambioRuedas = true;
    }
}
void setdireccion(int r)
{
    if (r != rotacion)
    {
        direccion = r;
        cambioRuedas = true;
    }
}
void setvelocidadRotacion(int r)
{
    if (r != rotacion)
    {
        velocidadRotacion = r;
        cambioRuedas = true;
    }
}
void setPosicionMotorBrazo(int motor, int posicion)
{
    if (posicion != motoresBrazo[motor])
    {
        if (motor == 0)
            motoresBrazo[motor] = map(posicion, -100, 100, 0, 360);
        else
            motoresBrazo[motor] = map(posicion, -100, 100, 0, 180);
        cambioBrazo = true;
    }
}
void controlBrazo(void *params)
{
    static float _m1 = -1, _m2 = -1, _m3 = -1, _m4 = -1;
    float _v1, _v2, _v3, _v4;
    bool changed;
    Serial.println("Iniciando hilo de brazo");
    while (true)
    {
        // float m1 = map(motoresBrazo[0], -100, 100, 0, 180);
        // float m2 = map(motoresBrazo[1], -100, 100, 0, 180);
        // float m3 = map(motoresBrazo[2], -100, 100, 0, 180);
        // float m4 = map(motoresBrazo[3], -100, 100, 0, 180);
        float m1 = motoresBrazo[0];
        float m2 = motoresBrazo[1];
        float m3 = motoresBrazo[2];
        float m4 = motoresBrazo[3];
        float m5 = motoresBrazo[4];
        changed = false;
        if (_m1 == -1)
        {
            _m1 = m1;
            changed = true;
        }
        if (_m2 == -1)
        {
            _m2 = m2;
            changed = true;
        }
        if (_m3 == -1)
        {
            _m3 = m3;
            changed = true;
        }
        if (_m4 == -1)
        {
            _m4 = m4;
            changed = true;
        }
        if (m1 != _m1 || m2 != _m2 || m3 != _m3 || m4 != _m4 || cambioBrazo == true)
        {
            changed = true;
            cambioBrazo = false;
        }

        if (changed)
        {
            Serial.println("Enviando nueva posicion del brazo");
            for (int i = 0; i <= 100; i++)
            {
                float cOrigen = (cos((100 - i) / 100.0 * PI - PI) + 1) / 2.0;
                float cDestino = (cos(i / 100.0 * PI - PI) + 1) / 2.0;
                _v1 = _m1 * cOrigen + m1 * cDestino;
                _v2 = _m2 * cOrigen + m2 * cDestino;
                _v3 = _m3 * cOrigen + m3 * cDestino;
                _v4 = _m4 * cOrigen + m4 * cDestino;
                // Serial.printf("_m1=%f, m1=%f, _v1=%f, i=%f, cos=%f\n", _m1, m1, _v1, i / 100.0, cOrigen);
                pwm.writeMicroseconds(0, map(_v1, 0, 360, 400, 1920));
                pwm.writeMicroseconds(1, map(_v2, 0, 180, 400, 2200));
                pwm.writeMicroseconds(2, map(_v3, 180, 0, 500, 2350));
                pwm.writeMicroseconds(3, map(_v4, 180, 0, 620, 2480));
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            _m1 = m1;
            _m2 = m2;
            _m3 = m3;
            _m4 = m4;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
void motores(void *parametros)
{
    Wire.begin();
    pwm.begin();

    /*
     * In theory the internal oscillator (clock) is 25MHz but it really isn't
     * that precise. You can 'calibrate' this by tweaking this number until
     * you get the PWM update frequency you're expecting!
     * The int.osc. for the PCA9685 chip is a range between about 23-27MHz and
     * is used for calculating things like writeMicroseconds()
     * Analog servos run at ~50 Hz updates, It is importaint to use an
     * oscilloscope in setting the int.osc frequency for the I2C PCA9685 chip.
     * 1) Attach the oscilloscope to one of the PWM signal pins and ground on
     *    the I2C PCA9685 chip you are setting the value for.
     * 2) Adjust setOscillatorFrequency() until the PWM update frequency is the
     *    expected value (50Hz for most ESCs)
     * Setting the value here is specific to each individual I2C PCA9685 chip and
     * affects the calculations for the PWM update frequency.
     * Failure to correctly set the int.osc value will cause unexpected PWM results
     */
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(SERVO_FREQ); // Analog servos run at ~50 Hz updates
    // pwm.writeMicroseconds(0, 1500);
    // pwm.writeMicroseconds(1, 1500);
    // pwm.writeMicroseconds(2, 1500);
    // pwm.writeMicroseconds(3, 1500);
    // pwm.writeMicroseconds(4, map(270, 0, 360, 400, 1920));
    // pwm.writeMicroseconds(5, map(90, 0, 180, 400, 2200));
    // pwm.writeMicroseconds(6, map(90, 180, 0, 500, 2350));
    // pwm.writeMicroseconds(7, map(90, 180, 0, 620, 2480));
    /*  vTaskDelay(1000 / portTICK_PERIOD_MS);
      double d0 = 5;     // Longitud base a primer motor
      double d1 = 17;    // Longitud segmento 1 del brazo (primer motor a segundo motor)
      double d2 = 15;    // Longitud segmento 2 del brazo (segundo motor a tercer motor)
      double d3 = 9;     // Longitud segmento 3 del brazo (tercer motor a muñeca)
      double beta = 45;  // grados (rotacion de la base)
      double gamma = 60; // grados (inclinacion del vector del brazo)
      double D = 30;     // cm (magnitud del vector resultante del brazo)

      double Dx = -16; // D * cos(gamma * DEG_TO_RAD);
      double Dy = 16;  // D * sin(gamma * DEG_TO_RAD);
      double phy = atan(Dy / Dx) * RAD_TO_DEG;
      double alpha = atan(Dy / d1) * RAD_TO_DEG;
      double theta = atan(Dx / d0) * RAD_TO_DEG;
      Serial.printf("Dx %f Dy %f phy %f alpha %f theta %f \n", Dx, Dy, phy, alpha, theta);
      pwm.writeMicroseconds(5, map(theta, 90, -90, 400, 2200));
      pwm.writeMicroseconds(6, map(alpha, 90, -90, 500, 2350));
      pwm.writeMicroseconds(7, map(phy, 90, -90, 620, 2480));*/
    // controlbrazo(90, 180, 180, 0);

    // miservo.write(FRONTAL_DERECHO, map(35, -100, 100, 0, 180));
    // miservo.write(FRONTAL_IZQUIERDO, map(35, -100, 100, 180, 0));
    // miservo.write(TRASERO_DERECHO , map(35, -100, 100, 0, 180));
    // miservo.write(TRASERO_IZQUIERDO, map(35, -100, 100, 180, 0));
    xTaskCreatePinnedToCore(controlBrazo, "controlBrazo", 4096, NULL, 1, NULL, 1);
    while (true)
    {
        if (Serial.available() > 0)
        {
            String v = Serial.readStringUntil('\n');
            Serial.println("Vale " + v);
            String m = v.substring(0, v.indexOf('.'));
            String p = v.substring(v.indexOf('.') + 1);
            Serial.printf("Motor %d Posicion %d\n", m.toInt(), p.toInt());
            setPosicionMotorBrazo(m.toInt(), p.toInt());
        }
        if (cambioRuedas)
        {
            cambioRuedas = false;
            miservo.write(FRONTAL_DERECHO, map(velocidad * direccion + rotacion * velocidadRotacion, -100, 100, 180, 0));
            miservo.write(FRONTAL_IZQUIERDO, map(velocidad * direccion - rotacion * velocidadRotacion, -100, 100, 0, 180));
            miservo.write(TRASERO_DERECHO, map(velocidad * direccion + rotacion * velocidadRotacion, -100, 100, 180, 0));
            miservo.write(TRASERO_IZQUIERDO, map(velocidad * direccion - rotacion * velocidadRotacion, -100, 100, 0, 180));
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// You can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. It's not precise!
void setServoPulse(uint8_t n, double pulse)
{
    double pulselength;

    pulselength = 1000000;     // 1,000,000 us per second
    pulselength /= SERVO_FREQ; // Analog servos run at ~60 Hz updates
    Serial.print(pulselength);
    Serial.println(" us per period");
    pulselength /= 4096; // 12 bits of resolution
    Serial.print(pulselength);
    Serial.println(" us per bit");
    pulse *= 1000000; // convert input seconds to us
    pulse /= pulselength;
    Serial.println(pulse);
    pwm.setPWM(n, 0, pulse);
}