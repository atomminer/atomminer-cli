QT -= gui
QT       += core network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# compiler if fussy about CryptoPP's headers. Following will disable unused var warning
QMAKE_CXXFLAGS -= -Wno-unused-parameter
QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-parameter

INCLUDEPATH += /usr/include/libusb-1.0

LIBS		+= -lcurl -lusb-1.0 -lz


SOURCES += \
        main.cpp \
    stratum/algos.cpp \
    stratum/stratum.cpp \
    json/dump.c \
    json/error.c \
    json/hashtable.c \
    json/load.c \
    json/memory.c \
    json/pack_unpack.c \
    json/strbuffer.c \
    json/strconv.c \
    json/utf.c \
    json/value.c \
    utils/log.cpp \
    utils/format.cc \
    api/api.cpp \
    utils/u.cpp \
    g.cpp \
    device/device.cpp \
    device/devicemanager.cpp \
    device/hotplug.cpp \
    firmware/firmware.cpp \
    settings/conf.cpp \
    utils/gz.cpp \
    device/miningmanager.cpp \
    device/miner.cpp \
    stratum/stratumwork.cpp \
    stratum/stratumtest.cpp

HEADERS += \
    stratum/stratum.h \
    utils/log.h \
    utils/types.h \
    build.h \
    json/config.h \
    json/hashtable.h \
    json/jansson-impl.hpp \
    json/jansson.h \
    json/jansson.hpp \
    json/jansson_config.h \
    json/jansson_private.h \
    json/strbuffer.h \
    json/utf.h \
    json/util.h \
    utils/fmt/core.h \
    utils/fmt/format-inl.h \
    utils/fmt/format.h \
    utils/fmt/locale.h \
    utils/fmt/ostream.h \
    utils/fmt/posix.h \
    utils/fmt/printf.h \
    utils/fmt/time.h \
    api/api.h \
    utils/u.h \
    g.h \
    device/device.h \
    device/devicemanager.h \
    device/hotplug.h \
    firmware/firmware.h \
    settings/conf.h \
    utils/gz.h \
    device/miningmanager.h \
    device/miner.h \
    stratum/stratumwork.h \
    stratum/stratumtest.h

RESOURCES += \
    resource.qrc
