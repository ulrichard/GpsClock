SET(atmega328.build.f_cpu 16000000) # running on external 16MHz crystal

set(CMAKE_TOOLCHAIN_FILE arduino-cmake/cmake/ArduinoToolchain.cmake)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(GpsClock)

#print_board_list()

SET(IS_SOLDERED TRUE)
IF(${IS_SOLDERED})
    SET(AVR_DEVICE     atmega328)
    SET(AVRDUDE_DEVICE atmega328p)
    SET(AVRDUDE_PORT   /dev/ttyACM0)   # diamex AVR von www.reichelt.de
    SET(AVRDUDE_PROGRAMMER   stk500v2) # diamex AVR von www.reichelt.de
ELSE()
#    SET(AVR_DEVICE     nano328)
    SET(AVR_DEVICE     atmega328)
    SET(AVRDUDE_DEVICE atmega328p)
    SET(AVRDUDE_PORT   /dev/ttyUSB0)    # Arduino
    SET(AVRDUDE_PROGRAMMER    arduino)  # Arduino
ENDIF()

ADD_DEFINITIONS(-std=c++11)

generate_arduino_library(Adafruit_GFX
    BOARD ${AVR_DEVICE}
	SRCS  Adafruit-GFX-Library/Adafruit_GFX.cpp
	HDRS  Adafruit-GFX-Library/Adafruit_GFX.h
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-GFX-Library)


generate_arduino_library(Adafruit_SSD1306
    BOARD ${AVR_DEVICE}
	SRCS  Adafruit_SSD1306/Adafruit_SSD1306.cpp
	HDRS  Adafruit_SSD1306/Adafruit_SSD1306.h
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Adafruit_SSD1306)


generate_arduino_library(Adafruit-GPS-Library
    BOARD ${AVR_DEVICE}
	SRCS  Adafruit-GPS-Library/Adafruit_GPS.cpp
	HDRS  Adafruit-GPS-Library/Adafruit_GPS.h
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Adafruit-GPS-Library)


generate_arduino_library(MemoryFree
    BOARD ${AVR_DEVICE}
	SRCS  MemoryFree/MemoryFree.cpp
	HDRS  MemoryFree/MemoryFree.h
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/MemoryFree)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(/usr/share/arduino/libraries/SPI)
INCLUDE_DIRECTORIES(/usr/share/arduino/libraries/SoftwareSerial)

generate_arduino_firmware(gps_board
    BOARD ${AVR_DEVICE}
    SKETCH GpsBoard
	LIBS
        Adafruit-GPS-Library
        MemoryFree
	PROGRAMMER stk500v2
    PORT  ${AVRDUDE_PORT}
)

generate_arduino_firmware(display_board
    BOARD ${AVR_DEVICE}
    SKETCH DisplayBoard
	LIBS
        Adafruit_GFX
        Adafruit_SSD1306
        MemoryFree
	PROGRAMMER stk500v2
    PORT  ${AVRDUDE_PORT}
)


IF(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")
	ADD_CUSTOM_TARGET(flash_display
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -b 57600 -F -u -Uflash:w:display_board.hex
		DEPENDS display_board
	)
	ADD_CUSTOM_TARGET(flash_gps
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -b 57600 -F -u -Uflash:w:gps_board.hex
		DEPENDS gps_board
	)
ELSE(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")
	ADD_CUSTOM_TARGET(flash_display
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -Uflash:w:display_board.hex
		DEPENDS display_board
	)
	ADD_CUSTOM_TARGET(flash_gps
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -Uflash:w:gps_board.hex
		DEPENDS gps_board
	)
ENDIF(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")



