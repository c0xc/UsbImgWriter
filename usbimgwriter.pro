TARGET = usbimgwriter
DESTDIR = bin/

HEADERS = inc/*.hpp
SOURCES = src/*.cpp
OBJECTS_DIR = obj/
INCLUDEPATH = inc/

# missing return statement should be fatal: -Werror=return-type
QMAKE_CXXFLAGS += -Werror=return-type

QT += widgets
QT += dbus
QT += concurrent

RESOURCES += res/res.qrc

