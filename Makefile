CC = avr-gcc
CXX = avr-g++
OBJCOPY = avr-objcopy
AVR_ARCH = atmega64
LDAVR_ARCH = avrmega64

# Para usar Arduino MEGA cambiar:
# AVR_IO_H AVRDUDE_PART FLASH_PROTOCOL

# Clock del uC (microcontrolador)
F_CPU = 16000000L

# Especifica la placa en uso a la librería de IO
AVR_IO_H = __AVR_ATmega328P__

# Programador para avrdude
FLASH_PROTOCOL = arduino
PORT = /dev/arduino # COMx si Windows
AVRDUDE_PART = atmega328p

# Flags para compilar
CFLAGS = -Os -flto -DF_CPU=$(F_CPU) -D$(AVR_IO_H) -Wall 
# Estos flags capas que no necesitamos
CFLAGS += -mcall-prologues -fshort-enums -fpack-struct
CFLAGS += -ffunction-sections -DAVR -I. -mmcu=$(AVR_ARCH)
CXXFLAGS = $(CFLAGS)

# Que es lo que se va a compilar
TARGET = ejercicio-2

all: $(TARGET).elf $(TARGET).hex

# Compila el binario
$(TARGET).elf: $(TARGET).c
	echo "Compilando"
	$(CC) $(CFLAGS) $< -o $@

# Pasa el binario a .hex
$(TARGET).hex: $(TARGET).elf
	echo "Generando .hex"
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

load: $(TARGET).hex
	echo "Cargando .hex a la placa"
	avrdude -c $(FLASH_PROTOCOL) -P $(PORT) -b 115200 -p $(AVRDUDE_PART) -D -U flash:w:"$@":i
