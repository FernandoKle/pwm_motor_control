/*! Control de arranque de motor con PWM (por código)

@=== Entradas ===@
* PB1 --> Pulsador, cambia entre encendido y apagado
	  usa el arranque por PWM.

* PB2 --> Pulsador, cambia la constante de tiempo usada
	  por el PWM

* PB3 --> Pulsador, cambia entre encendido y apagado
	  usa arranque inmediato, por escalón.

@=== Salidas ===@
* PIN 8 --> Salida conectada al motor.
	    un LED estará conectado al mismo PIN.

* PIN 0 al 4 --> LEDs deben encenderse uno a 
	         uno durante la etapa PWM.
*/

// @====== Librerias y defines =======@
// Comentado, fue agragdo al Makefile
//#define F_CPU 16000000UL // clock a 16 MHz
// Especifica que estamos usando el 328P (Arduino Uno)
//#define __AVR_ATmega328P__ 

#include <avr/io.h>
#include <util/delay.h> // _delay_ms(tiempo_en_ms)
#include <avr/interrupt.h>

// @=================== Macros ===================@
// CORRE 00000001  "b" veces a la izquierda
#define _BV(b) = (1 << b) 
// SET (1) bit "b" de la posición "p"
#define sbi(p,b) p |= _BV(b)
// CLEAR (0) bit "b" de la posición "p"
#define cbi(p,b) p &= ~(_BV(b))
// TOGGLE (~x) bit "b" de la posición "p"
#define tbi(p,b) p ^= _BV(b)

// RECORDATORIO 0 es False y diferente de 0 es True

// RETORNA 1 si bit "b" de "p" es 1
#define is_high(p,b) (p & _BV(b)) == _BV(b)
// RETORNA 1 si bit "b" de "p" es 1
#define is_low(p,b) (p & _BV(b)) == 0
// RETORNA 1+ se es uno, diferente si es cero
// tener CUIDADO con asignar esto a un numero con signo
#define bit_read(p,b) (p & _BV(b))

// @====== Variables y constantes globales =======@
#define T1 5000 // ms - Default
#define T2 8000 // ms
#define T3 11000 // ms
#define T4 14000 // ms

const int * time_base_array = {T1, T2, T3, T4};
volatile int time_base_index = 0 ; // Debe estar entre 0 y 3
volatile int time_base = T1 ;

// Cantidad de pasos en los cuales aumenta la tension
#define PASOS 100 

// Cantidad de ms a esperar antes del siguiente paso 
// paso --> cambio en el nivel de tension
#define T1_PASO (T1 / PASOS)
#define T2_PASO (T2 / PASOS)
#define T3_PASO (T3 / PASOS)
#define T4_PASO (T4 / PASOS)

const int * time_step_array = {T1_PASO, T2_PASO, T3_PASO, T4_PASO};
volatile int time_step_index = 0 ;
volatile int time_step = T1_PASO ;

// Tiempo por instruccion asm
#define T_ASM 62 // us

// Estado del motor
enum estado_del_motor {apagado, encendido, en_pwm};
volatile enum estado_del_motor estado_motor = apagado ;

// @======= Definiciones de Funciones =======@
void arranque_pwm ();

// @============= Interrupciones ============@

// Crea una funcion void vector_de_interrupcion(void)
// ISR (vector_de_interrupcion){ codigo; }

/* Detalles
   El ejercicio pide manejar 2 botones con interrupciones
   y 1 con pooling.
   Sugiero usar el arranque por PWM con una interrupción,
   de esa manera no va a poder ser interrumpido por otra,
   ya que eso rompería el bucle del PWM.

   Sugiero dejar el pooling para el boton de cambio de velocidad.
*/

// @================== Main =================@
void 
main (void)
{
	// @===== Setup =====@

	sei(); // activar interrupciones cli() para apagar

	// @====== Loop =====@
	while(1)
	{
		// Codigo
	}
}

// @======= Funciones =======@
void 
arranque_pwm ()
{
	// WRITE ME
}
