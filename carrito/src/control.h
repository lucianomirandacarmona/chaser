#ifndef _CONTROL_
#define _CONTROL_
#define ARRIBA 'w'
#define ABAJO 's'
#define DERECHA 'd'
#define IZQUIERDA 'a'

#define ARRIBA_ALTO 'W'
#define ABAJO_ALTO 'S'
#define DERECHA_ALTO 'D'
#define IZQUIERDA_ALTO 'A'
#define TERMINA_COMANDO 'x'

#define CAMBIA_LUCES 'ñ'
#define LUCES_VERDE 'Ñ'


#define I2C_DEV_ADDR 0x55

void xd();
void control(void *parametros);

#endif