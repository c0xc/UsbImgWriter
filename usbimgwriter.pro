TARGET = usbimgwriter
DESTDIR = bin/

HEADERS = inc/*.hpp
SOURCES = src/*.cpp
OBJECTS_DIR = obj/
INCLUDEPATH = inc/

RESOURCES += res/res.qrc

# missing return statement should be fatal: -Werror=return-type
QMAKE_CXXFLAGS += -Werror=return-type

QT += widgets
QT += dbus
QT += concurrent
# network feature is optional - comment this module out to disable the feature
QT += network

