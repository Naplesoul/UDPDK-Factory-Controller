QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Master.cpp \
    arm.cpp \
    belt.cpp \
    block.cpp \
    camera.cpp \
    label.cpp \
    main.cpp \
    object.cpp \
    scada.cpp \
    widgets.cpp

HEADERS += \
    Master.h \
    arm.h \
    belt.h \
    block.h \
    camera.h \
    label.h \
    object.h \
    scada.h \
    widgets.h

LIBS += /usr/lib/x86_64-linux-gnu/libjsoncpp.a

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
