/*! Control de arranque de motor con PWM (por código)
Escrito por: Fernando Kleinubing

@=== Entradas ===@
* PB1 --> PIN2, INT0, Pulsador, cambia entre encendido y apagado
	  usa el arranque por PWM.

* PB2 --> Pooling, Pulsador, cambia la constante de tiempo usada
	  por el PWM

* PB3 --> Pooling, Pulsador, cambia entre encendido y apagado
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
// RETORNA 1 si bit "b" de "p" es 0
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
#define LED4 PORTD3

#define BUTTON_PORT PINB // para evitar errores
#define BUTTON_PIN 	PINB // deben ser iguales

#define PWM_PIN  PIND // ISR (INT0_vect)

#define TIME_BUTTON PINB3 // Pooling
#define PWM_BUTTON  PIND2 // ISR (INT0_vect)
#define STEP_BUTTON PINB2 // ISR (INT1_vect) (PIND3) (ya no)

// @====== Variables y constantes globales =======@

const int8_t led_array[] = {LED0, LED1, LED2, LED3};

#define T1 5000 // ms - Default
#define T2 8000 // ms
#define T3 11000 // ms
#define T4 14000 // ms

const int16_t time_base_array[] = {T1, T2, T3, T4};
volatile int8_t time_index = 0 ; // Debe estar entre 0 y 3
volatile int16_t time_base = T1 ;

// Cantidad de pasos por segundo
#define PASOS 100 // 50
#define PERIODO 10 // 20 // ms

// Cantidad de pasos de PWM totales
// para cada segundo
#define T1_PASOS (T1 / 1000 * PASOS)
#define T2_PASOS (T2 / 1000 * PASOS)
#define T3_PASOS (T3 / 1000 * PASOS)
#define T4_PASOS (T4 / 1000 * PASOS)

const int16_t time_step_array[] = {T1_PASOS, T2_PASOS, T3_PASOS, T4_PASOS};
volatile int16_t time_step = T1_PASOS ;

// 1/5 parte de los pasos
#define T1_PASOS_1_QUINTO ( T1_PASOS / 5 )
#define T2_PASOS_1_QUINTO ( T2_PASOS / 5 )
#define T3_PASOS_1_QUINTO ( T3_PASOS / 5 )
#define T4_PASOS_1_QUINTO ( T4_PASOS / 5 )

const int16_t un_quinto_step_array[] = {
	T1_PASOS_1_QUINTO, 
	T2_PASOS_1_QUINTO, 
	T3_PASOS_1_QUINTO, 
	T4_PASOS_1_QUINTO
};
volatile int16_t step_un_quinto = T1_PASOS_1_QUINTO ;

// Calculo de pendientes
// 1/50 = 20 ms = T (periodo)
// Ton + Toff = T
// 20 [ms] / (cantidad de pasos)

// Comentado y calculado a mano ya que el
// compilador los resolvia como 0
//#define T1_STEP (PERIODO / T1_PASOS)
//#define T2_STEP (PERIODO / T2_PASOS)
//#define T3_STEP (PERIODO / T3_PASOS)
//#define T4_STEP (PERIODO / T4_PASOS)

//const float step_inc_array[] = {T1_STEP, T2_STEP, T3_STEP, T4_STEP};
const float step_inc_array[] = {
	0.02,  //0.08,  // T1_STEP = PERIODO / T1_PASOS, //0.002,  //
	0.0125,//0.05,  // T2_STEP = PERIODO / T2_PASOS, //0.00125,//
	0.0090,//0.036, // T3_STEP = PERIODO / T3_PASOS, //0.0009, //
	0.0071 //0.028  // T4_STEP = PERIODO / T4_PASOS  //0.0007  //
};

// Estado del motor
enum estado_del_motor {apagado, encendido, en_pwm, casi_apagado};
volatile enum estado_del_motor estado_motor = apagado ;

// Flag pulsadores
enum estado_pulsador {bajo, alto};
volatile enum estado_pulsador pul_step = alto ;
volatile enum estado_pulsador pul_time = alto ;

// @======= Definiciones de Funciones =======@
void arranque_pwm ();
void alterar_tiempo ();
void apagar_leds ();
void encender_leds ();
void alt_delay_ms(uint16_t);
void alt_delay_100_u(uint16_t);

// @============= Interrupciones ============@

// Crea una funcion void vector_de_interrupcion(void)
// ISR (vector_de_interrupcion){ codigo; }

/* Detalles
   El ejercicio pide manejar 1 botones con interrupciones
   y 2 con pooling.
   Sugiero usar el arranque por PWM con una interrupción,
   de esa manera no va a poder ser interrumpido por otra,
   ya que eso rompería el bucle del PWM.
*/

// @==== Definiciones de Interrupciones =====@

ISR (INT0_vect) // Boton de arranque por PWM
{
	if (estado_motor == apagado)
	{
		estado_motor = en_pwm ;
	}

	if (estado_motor == encendido)
	{
		estado_motor = casi_apagado ;
	}

	return;
}

//ISR (INT1_vect) // Boton de arranque por Escalon
void
arranque_escalon()
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

	//EIFR = 0x00 ; // Apaga los flags, por si acaso

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

	// Habilita interrupcion INT0
	EICRA = 0b00000010 ; // Flanco desendente
	EIMSK = 0b00000001 ; // Habilita interrupciones
	EIFR = 0x00 ; // Apaga los flags, por si acaso

	// Activar bit de interrupciones global, cli() para apagar
	sei(); 

	// @====== Loop =====@
	while(1)
	{
		// Hacer Encendido por PWM
		if ( estado_motor == en_pwm )
		{
			arranque_pwm();
			estado_motor = encendido ;
		}
		if ( (estado_motor == casi_apagado) && is_high(PWM_PIN, PWM_BUTTON) )
		{
			_delay_ms(10);
			MOTOR_OFF ;
			apagar_leds();
			estado_motor = apagado ;
		}
		// Hacer pooling del TIME_BUTTON
		if ( is_low(BUTTON_PIN, TIME_BUTTON) && pul_time == alto )
		{
			pul_time = bajo ;
			_delay_ms(10); // Espera
			alterar_tiempo();
		}
		if ( is_high(BUTTON_PIN, TIME_BUTTON)) // vuelve a medir
		{
			pul_time = alto ;
		}

		// Hacer pooling del STEP_BUTTON
		if ( is_low(BUTTON_PIN, STEP_BUTTON) && pul_step == alto )
		{
			pul_step = bajo ;
			_delay_ms(10); // Espera
			arranque_escalon();
		}
		if ( is_high(BUTTON_PIN, STEP_BUTTON)) // vuelve a medir
		{
			pul_step = alto ;
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
alt_delay_ms (uint16_t ms)
{
  while (0 < ms)
  {  
    _delay_ms(1);
    --ms;
  }
  return;
}

void 
alt_delay_100_u (uint16_t ms)
{
  while (0 < ms)
  {  
    _delay_ms(0.1);
    --ms;
  }
  return;
}

void 
arranque_pwm ()
{
	// THE BIG ONE
	sbi(LED_PORT, LED0);

	float pendiente = step_inc_array[time_index];
	
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

		MOTOR_ON ;

		//uint16_t i_step = i * pendiente + 1 ;
		float i_step = i * pendiente + 1 ;

		//alt_delay_ms ( i_step ) ;
		_delay_ms ( i_step ) ;

		MOTOR_OFF ;

		//alt_delay_ms ( PERIODO - i_step ) ;
		_delay_ms ( PERIODO - i_step ) ;
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

	// Parpeadea un LED
	tbi(LED_PORT, led_array[time_index]);
	_delay_ms(100);
	tbi(LED_PORT, led_array[time_index]);
	_delay_ms(100);

	return ;
}
