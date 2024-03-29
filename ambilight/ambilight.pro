HEADERS       = window.h \
    delegate.h \
    version.h \
    grab/grab_api.h
SOURCES       = main.cpp \
                window.cpp \
    delegate.cpp
RESOURCES     = \
    ambilight.qrc

include( globalshortcut/globalshortcut.pri)

OTHER_FILES += \
    ambilight.rc

TRANSLATIONS += translate/Ambilight_ru_RU.ts

QT           += xml svg

RC_FILE = ambilight.rc

INCLUDEPATH +=../src/qserialdeviceenumerator \
../src/qserialdevice


OBJECTS_DIR = ./obj
MOC_DIR = ./moc
TEMPLATE = app

CONFIG(debug, debug|release) {
    DESTDIR = ./build/debug
    QMAKE_LIBDIR += ../src/build/debug
    LIBS += -lqserialdeviced

} else {
    DESTDIR = ./build/release
    QMAKE_LIBDIR += ../src/build/release
    LIBS += -lqserialdevice

}

win32 {
    LIBS += -lsetupapi -luuid -ladvapi32
    LIBS    += -lgdi32
    SOURCES += grab/grab_winapi.cpp
    SOURCES += grab/grab_qt.cpp
}
unix:!macx {
    LIBS += -ludev
    SOURCES += grab/grab_qt.cpp
}
