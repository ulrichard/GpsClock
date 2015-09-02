SET(atmega328.build.f_cpu 16000000) # running on external 16MHz crystal

set(CMAKE_TOOLCHAIN_FILE ../../arduino-cmake/cmake/ArduinoToolchain.cmake)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(GpsClock)

#print_board_list()

SET(AVR_DEVICE     atmega328)
SET(AVRDUDE_DEVICE atmega328p)
SET(AVRDUDE_PORT   /dev/ttyACM0)   # diamex AVR von www.reichelt.de
SET(AVRDUDE_PROGRAMMER   stk500v2) # diamex AVR von www.reichelt.de
#SET(AVR_DEVICE     nano328)
#SET(AVRDUDE_DEVICE nano328)
#SET(AVRDUDE_PORT   /dev/ttyUSB0)    # Arduino
#SET(AVRDUDE_PROGRAMMER    arduino)  # Arduino


generate_arduino_library(Adafruit_GFX
    BOARD ${AVR_DEVICE}
	SRCS  Adafruit-GFX-Library-1.0.1/Adafruit_GFX.cpp
	HDRS  Adafruit-GFX-Library-1.0.1/Adafruit_GFX.h
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-GFX-Library-1.0.1)


generate_arduino_library(Adafruit_SSD1306
    BOARD ${AVR_DEVICE}
	SRCS  Adafruit_SSD1306-1.0.0/Adafruit_SSD1306.cpp
	HDRS  Adafruit_SSD1306-1.0.0/Adafruit_SSD1306.h
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Adafruit_SSD1306-1.0.0)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(/usr/share/arduino/libraries/SPI)

generate_arduino_firmware(gps_clock
    BOARD ${AVR_DEVICE}
    SKETCH ../gps_clock
	LIBS Adafruit_GFX Adafruit_SSD1306
#    ARDLIBS SoftwareSerial
	PROGRAMMER stk500v2
    PORT  ${AVRDUDE_PORT}
)

IF(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")
	ADD_CUSTOM_TARGET(flash
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -b 57600 -F -u -Uflash:w:gps_clock.hex
		DEPENDS gps_clock
	)
ELSE(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")
	ADD_CUSTOM_TARGET(flash
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -Uflash:w:gps_clock.hex
		DEPENDS gps_clock
	)
ENDIF(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")

# for fuse settings see http://www.engbedded.com/fusecalc/
IF(atmega8 STREQUAL ${AVR_DEVICE})
	SET(LFUSE 0xbf) # ext crystal
	SET(HFUSE 0xd9)

	ADD_CUSTOM_TARGET(fuses
			avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -U lfuse:w:${LFUSE}:m -Uhfuse:w:${HFUSE}:m
	)

	ADD_CUSTOM_TARGET(readchip
			avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -U flash:r:dumpflash.hex:i -U eeprom:r:dumpee.hex:i -U lfuse:r:dumplfuse.hex:i -U hfuse:r:dumphfuse.hex:i
	)
ELSE()
	SET(LFUSE 0xff) # ext crystal
	SET(HFUSE 0xda)
	SET(EFUSE 0x05)

	ADD_CUSTOM_TARGET(fuses
			avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -U lfuse:w:${LFUSE}:m -Uhfuse:w:${HFUSE}:m -Uefuse:w:${EFUSE}:m
	)

	ADD_CUSTOM_TARGET(readchip
			avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c stk500v2 -U flash:r:dumpflash.hex:i -U eeprom:r:dumpee.hex:i -U lfuse:r:dumplfuse.hex:i -U hfuse:r:dumphfuse.hex:i -U efuse:r:dumpefuse.hex:i
	)
ENDIF()

