TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    rs232.c \
    main.cpp \
    ../testje/RingBuffer.cpp \
    ../testje/SerialCommand.cpp \
    ../testje/BufferedSerial.cpp

HEADERS += \
    rs232.h \
    ../testje/RingBuffer.h \
    ../testje/BufferedSerial.h \
    ../testje/SerialCommand.h

