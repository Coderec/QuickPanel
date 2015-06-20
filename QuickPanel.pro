#-------------------------------------------------
#
# Project created by QtCreator 2013-11-15T19:33:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuickPanel
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    myButton.cpp \
    labelButton.cpp

HEADERS  += widget.h \
    myButton.h \
    labelButton.h

FORMS    += widget.ui

RESOURCES += \
    img.qrc

OTHER_FILES += \
    myapp.rc

RC_FILE += myapp.rc
#QMAKE_LFLAGS += /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'"
