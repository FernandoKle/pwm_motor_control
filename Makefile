# @=======================================================@
# @================= Makefile para uC AVR ================@
# @=======================================================@

# Escrito por: Fernando Kleinubing

# Usar "make" para compilar
# Usar "make load" para cargar a la placa

####################### Configuración ########################

# Que es lo que se va a compilar
TARGET = ejercicio-2

all: $(TARGET).elf $(TARGET).hex

# Preparado para placas Arduino Uno
# Para usar Arduino MEGA cambiar:
# AVR_IO_H AVRDUDE_PART FLASH_PROTOCOL

# Path de los programas
CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
AVR_ARCH = atmega64
LDAVR_ARCH = avrmega64

# Clock del uC (microcontrolador)
F_CPU = 16000000L

# Especifica la placa en uso a la librería de IO
AVR_IO_H = __AVR_ATmega328P__  # Arduino Uno
#AVR_IO_H = __AVR_ATmega2560__ # Mega (creo)

###################### Parámetros para AVRDUDE ######################
FLASH_PROTOCOL = arduino  # Arduino Uno
#FLASH_PROTOCOL = wiring  # Mega (creo)

PORT = /dev/arduino # COMx si Windows

AVRDUDE_PART = atmega328p  # Arduino Uno
#AVRDUDE_PART = atmega2560 # Mega (creo)

######################### Flags para compilar #######################

CFLAGS = -DF_CPU=$(F_CPU) -D$(AVR_IO_H) -Wall 

# Flags de optimización 
# para debugging cambiar -Os (small binary) por -Og (debug mode)
# si hay problemas, quitar lto (link time optimization)
CFLAGS += -Os -flto

# Estos flags capas que no necesitamos
CFLAGS += -mcall-prologues -fshort-enums -fpack-struct
CFLAGS += -ffunction-sections -DAVR -I. -mmcu=$(AVR_ARCH)
CXXFLAGS = $(CFLAGS)

####################### Fin de la configuración ####################

# Compila el binario
$(TARGET).elf: $(TARGET).c
	echo "Compilando"
	$(CC) $(CFLAGS) $< -o $@

# Pasa el binario a .hex
$(TARGET).hex: $(TARGET).elf
	echo "Generando .hex"
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

# carga el binario en la placa
load: $(TARGET).hex
	echo "Cargando .hex a la placa"
	avrdude -c $(FLASH_PROTOCOL) -P $(PORT) -b 115200 -p $(AVRDUDE_PART) -D -U flash:w:"$@":i
