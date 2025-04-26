#ifndef _MOTORES_
#define _MOTORES_

#define FRONTAL_DERECHO GPIO_NUM_13
#define FRONTAL_IZQUIERDO GPIO_NUM_12
#define TRASERO_DERECHO GPIO_NUM_14
#define TRASERO_IZQUIERDO GPIO_NUM_27
void setRotacion(int r);
void setvelocidad(int r);
void setdireccion(int r);
void setvelocidadRotacion(int r);
void setPosicionMotorBrazo(int motor, int posicion);

void motores(void *parametros);

#endif