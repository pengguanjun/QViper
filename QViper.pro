QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AppConfig.cpp \
    CommLog.cpp \
    CommThread.cpp \
    CommandParser.cpp \
    GuiMotorConfig.cpp \
    GuiMotorControl.cpp \
    GuiMotorView.cpp \
    ModuleParameters.cpp \
    MotorAxis.cpp \
    MotorManagement.cpp \
    MotorParameters.cpp \
    OMS_MAXv.cpp \
    VMEInterface.cpp \
    VMEInterface_XXUSB.cpp \
    VMEModule.cpp \
    VMEMotor.cpp \
    lib/libxxusb.c \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    AppConfig.h \
    AppDef.h \
    CommLog.h \
    CommThread.h \
    CommandParser.h \
    GuiMotorConfig.h \
    GuiMotorControl.h \
    GuiMotorView.h \
    MainWindow.h \
    ModuleParameters.h \
    MotorAxis.h \
    MotorManagement.h \
    MotorParameters.h \
    OMS_MAXv.h \
    VMEInterface.h \
    VMEInterface_XXUSB.h \
    VMEModule.h \
    VMEMotor.h \
    lib/libxxusb.h

FORMS += \
    CommLog.ui \
    GuiMotorConfig.ui \
    GuiMotorControl.ui \
    MainWindow.ui

INCLUDEPATH += lib /usr/include/libusb-1.0

LIBS+= /usr/lib64/libusb-1.0.so

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    module.cfg \
    motor.cfg
