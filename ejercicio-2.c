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
#define F_CPU 16000000UL // clock a 16 MHz
#define __AVR_ATmega328P__ // Especifica que estamos usando el 328P (Arduino Uno)

#include <avr/io.h>
#include <util/delay.h> // _delay_ms(tiempo_en_ms)
#include <avr/interrupt.h>

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
#define T1_PASO (T1 / PASO)
#define T2_PASO (T2 / PASO)
#define T3_PASO (T3 / PASO)
#define T4_PASO (T4 / PASO)

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

// @================== Main =================@
void 
main (void)
{
	// CODE GOES HERE
}

// @======= Funciones =======@
void 
arranque_pwm ()
{
	// WRITE ME
}
