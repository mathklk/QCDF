QT       += core gui serialport charts concurrent

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    collector.cpp \
    core.cpp \
    main.cpp \
    mainwindow.cpp \
    node.cpp \
    settingsdialog.cpp

HEADERS += \
    collector.h \
    core.h \
    crc32.h \
    frame.h \
    mainwindow.h \
    node.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

DISTFILES += \
    .gitignore \
    $$OUT_PWD/COM.txt

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
