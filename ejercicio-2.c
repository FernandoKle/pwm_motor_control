/*! Control de arranque de motor con PWM (por código)
Escrito por: Fernando Kleinubing

@=== Entradas ===@
* PB1 --> PIN2, INT0, Pulsador, cambia entre encendido y apagado
	  usa el arranque por PWM.

* PB2 --> Pooling, Pulsador, cambia la constante de tiempo usada
	  por el PWM

* PB3 --> PIN3, INT1, Pulsador, cambia entre encendido y apagado
	  usa arranque inmediato, por escalón.

@=== Salidas ===@
* PIN 9 --> Salida conectada al motor.
	    un LED estará conectado al mismo PIN.

* LEDs deben encenderse uno a 
	         uno durante la etapa PWM.
*/

// @====== Librerias y defines =======@
// Comentado, fue agragdo al Makefile
//#define F_CPU 16000000UL // clock a 16 MHz
// Especifica que estamos usando el 328P (Arduino Uno)
//#define __AVR_ATmega328P__ 

// delay.h: Permite el uso de variables para los delays
#define __DELAY_BACKWARD_COMPATIBLE__ // los delays usan floats...

#include <inttypes.h> // uint8_t int8_t entre otros tipos
#include <avr/io.h>
#include <util/delay.h> // _delay_ms(tiempo_en_ms)
#include <avr/interrupt.h>

// @=================== Macros ===================@
// p --> Puerto
// b --> bit

// CORRE 00000001  "b" veces a la izquierda
//#define _BV(b) ( (1 << b) )  // Incluido en las librerias
// SET (1) bit "b" de la posición "p"
#define sbi(p,b) ( p |= _BV(b) )
// CLEAR (0) bit "b" de la posición "p"
#define cbi(p,b) ( p &= ~(_BV(b)) )
// TOGGLE (~x) bit "b" de la posición "p"
#define tbi(p,b) ( p ^= _BV(b) )

// RECORDATORIO: 0 es False y diferente de 0 es True

// RETORNA 1 si bit "b" de "p" es 1
#define is_high(p,b) ( (p & _BV(b)) == _BV(b) )
// RETORNA 1 si bit "b" de "p" es 1
#define is_low(p,b) ( (p & _BV(b)) == 0 )
// RETORNA 1+ se es uno, diferente si es cero
// tener CUIDADO con asignar esto a un numero con signo
#define bit_read(p,b) ( (p & _BV(b)) )

// @=================== Pines ====================@
// Ver pinout para relacion pin del uC y la placa arduino

// DDR  --> Configuracion: 0 entrada, 1 salida
// PORT --> Escritura
// PIN  --> Lectura
#define MOTOR  		PORTB1
#define MOTOR_PORT  PORTB
#define MOTOR_DDR   DDRB
#define MOTOR_ON  sbi(MOTOR_PORT, MOTOR) // Encender motor
#define MOTOR_OFF cbi(MOTOR_PORT, MOTOR) // Apagar motor

#define LED_PORT PORTD
#define LED_DDR DDRD

#define LED0 PORTD4
#define LED1 PORTD5
#define LED2 PORTD6
#define LED3 PORTD7
#define LED4 PORTD0

#define BUTTON_PORT PIND
#define BUTTON_PIN BUTTON_PORT // para evitar errores

#define TIME_BUTTON PIND1 // Pooling
#define PWM_BUTTON  PIND2 // ISR (INT0_vect)
#define STEP_BUTTON PIND3 // ISR (INT1_vect)

// @====== Variables y constantes globales =======@
#define T1 5000 // ms - Default
#define T2 8000 // ms
#define T3 11000 // ms
#define T4 14000 // ms

const int16_t * time_base_array = {T1, T2, T3, T4};
volatile int8_t time_index = 0 ; // Debe estar entre 0 y 3
volatile int16_t time_base = T1 ;

// Cantidad de pasos por segundo
#define PASOS 1000

// Cantidad de pasos de PWM totales
// para cada tiempo
#define T1_PASOS (T1 / 1000 * PASOS)
#define T2_PASOS (T2 / 1000 * PASOS)
#define T3_PASOS (T3 / 1000 * PASOS)
#define T4_PASOS (T4 / 1000 * PASOS)

const int16_t * time_step_array = {T1_PASOS, T2_PASOS, T3_PASOS, T4_PASOS};
volatile int16_t time_step = T1_PASOS ;

// 1/5 parte de los pasos
#define T1_PASOS_1_QUINTO ( T1_PASOS / 5 )
#define T2_PASOS_1_QUINTO ( T2_PASOS / 5 )
#define T3_PASOS_1_QUINTO ( T3_PASOS / 5 )
#define T4_PASOS_1_QUINTO ( T4_PASOS / 5 )

const int16_t * un_quinto_step_array = {
	T1_PASOS_1_QUINTO, 
	T2_PASOS_1_QUINTO, 
	T3_PASOS_1_QUINTO, 
	T4_PASOS_1_QUINTO
};
volatile int16_t step_un_quinto = T1_PASOS_1_QUINTO ;

#define T1_STEP ( PASOS / T1 )
#define T2_STEP ( PASOS / T2 )
#define T3_STEP ( PASOS / T3 )
#define T4_STEP ( PASOS / T4 )

const float * step_inc_array = {T1_STEP, T2_STEP, T3_STEP, T4_STEP};

// Tiempo por instruccion asm
#define T_ASM 62 // us (esto deberia depender de F_CPU)

// Estado del motor
enum estado_del_motor {apagado, encendido, en_pwm};
volatile enum estado_del_motor estado_motor = apagado ;

// @======= Definiciones de Funciones =======@
void arranque_pwm ();
void alterar_tiempo ();
void apagar_leds ();
void encender_leds ();
void alt_delay_ms(int);

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

// @==== Definiciones de Interrupciones =====@
ISR (INT0_vect) // Boton de arranque por PWM
{
	// Para Debugging
	//arranque_pwm();

	if (estado_motor == apagado)
	{
		estado_motor = en_pwm ;
		arranque_pwm();
		estado_motor = encendido ;
	}
	else
	{
		MOTOR_OFF ;
		estado_motor = apagado ;
		apagar_leds();
	}

	EIFR = 0x00 ; // Apaga los flags, por si acaso

	return;
}

ISR (INT1_vect) // Boton de arranque por Escalon
{
	if (estado_motor == apagado)
	{
		encender_leds ();
		MOTOR_ON ;
		estado_motor = encendido ;
	}
	else
	{
		MOTOR_OFF ;
		estado_motor = apagado ;
		apagar_leds();
	}

	EIFR = 0x00 ; // Apaga los flags, por si acaso

	return;
}

	/*    .                    .    */
	/*   /-\                  /-\   */
	/*  /---\ @=== Main ===@ /---\  */
	/* /-----\              /-----\ */

int 
main (void)
{
	// @===== Setup =====@

	// Puertos y Pines
	// Deberian estar todos en Entrada por defecto
	// Asi que solo activo los que quiero como salida

	// Primero, todos los puertos a 0
	PORTD = 0x00 ;
	PORTB = 0x00 ;
	PORTC = 0x00 ;

	// Motor
	sbi(MOTOR_DDR, MOTOR); // Habilita la salida
	
	// LEDs
	sbi(LED_DDR, LED0);
	sbi(LED_DDR, LED1);
	sbi(LED_DDR, LED2);
	sbi(LED_DDR, LED3);
	sbi(LED_DDR, LED4);

	// Habilita interrupcion INT0 e INT1
	EICRA = 0b00001111 ; // Flanco asendente para ambos
	EIMSK = 0b00000011 ; // Habilita ambas interrupciones
	EIFR = 0x00 ; // Apaga los flags, por si acaso

	// Activar bit de interrupciones global, cli() para apagar
	sei(); 

	uint8_t time_button_counter = 0 ;

	// Debuging
	//arranque_pwm();

	// @====== Loop =====@
	while(1)
	{
		// Debugging
		//tbi(LED_PORT, LED4);
		//_delay_ms(300);

		// Hacer pooling del TIME_BUTTON
		if ( is_high(BUTTON_PIN, TIME_BUTTON) && (time_button_counter > 5) )
		{
			alterar_tiempo();
			time_button_counter = 0 ;

			// Debugging
			tbi(LED_PORT, LED4);
			_delay_ms(300);
			tbi(LED_PORT, LED4);
			_delay_ms(300);
		}
		else
		{
			_delay_ms(2); // espera 1 ms
			time_button_counter ++ ;
		}
	}
}
	/*   \-----/                  \-----/  */
	/*    \---/                    \---/   */
	/*     \-/  @=== FIN MAIN ===@  \-/    */
	/*      *                        *     */

/* @======= Funciones =======@ */

// _delay_ms(constante de compilacion)
void 
alt_delay_ms(int ms)
{
  while (0 < ms)
  {  
    _delay_ms(1);
    --ms;
  }
}

void 
arranque_pwm ()
{
	// THE BIG ONE
	sbi(LED_PORT, LED0);
	
	for ( int16_t i = 1 ; i < time_step ; i++ )
	{
		// LEDs
		if ( i > step_un_quinto )
			sbi(LED_PORT, LED1);

		if ( i > step_un_quinto * 2 )
			sbi(LED_PORT, LED2);

		if ( i > step_un_quinto * 3 )
			sbi(LED_PORT, LED3);

		if ( i > step_un_quinto * 4 )
			sbi(LED_PORT, LED4);

		// PWM
		// SI F = 1000 Hz --> T = 1 ms
		// --> 1000 ejecuciones de este bucle por segundo
		// al principio debe tener mas tiempo apagado
		// y al terminar mas tiempo encendido
		// Entonces, el tiempo encendido es
		// proporcional a el PASO actual
		// y el de apagado es lo que queda para completar el periodo.
		// T = Ton + Toff = 1/PASOS

		int16_t i_step  = i * step_inc_array[time_index] ;

		MOTOR_ON ;
		_delay_us ( i_step ) ;
		MOTOR_OFF ;
		_delay_us ( PASOS - i_step ) ;
	}
	
	// Dejar encendido al final de la secuencia
	MOTOR_ON ;

	return ;
}

void 
apagar_leds ()
{
	cbi(LED_PORT, LED0);
	cbi(LED_PORT, LED1);
	cbi(LED_PORT, LED2);
	cbi(LED_PORT, LED3);
	cbi(LED_PORT, LED4);
}

void 
encender_leds ()
{
	sbi(LED_PORT, LED0);
	sbi(LED_PORT, LED1);
	sbi(LED_PORT, LED2);
	sbi(LED_PORT, LED3);
	sbi(LED_PORT, LED4);
}

void 
alterar_tiempo ()
{
	time_index ++ ;

	if (time_index > 3)
		time_index = 0 ;

	time_base = time_base_array [time_index] ;
	time_step = time_step_array [time_index] ;
	step_un_quinto = un_quinto_step_array [time_index] ;

	return ;
}
