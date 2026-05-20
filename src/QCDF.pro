QT       += core gui serialport charts concurrent

CONFIG += c++17
CONFIG += optimize_full

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    collector.cpp \
    doubleslider.cpp \
    gr_doa.cpp \
    gr_music.cpp \
    main.cpp \
    mainwindow.cpp \
    music.cpp \
    node.cpp \
    recorder.cpp \
    settingsdialog.cpp

HEADERS += \
    collector.h \
    crc32.h \
    doubleslider.h \
    frame.h \
    gr_doa.h \
    gr_music.h \
    mainwindow.h \
    metaTypes.h \
    music.h \
    node.h \
    physics.h \
    plot.h \
    recorder.h \
    settingsdialog.h

FORMS += \
    doubleslider.ui \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    res.qrc

DISTFILES += \
    .gitignore \
    $$OUT_PWD/COM.txt

include(qt_zoomable_chart_widget/zoomable_chart_widget.pri)
INCLUDEPATH += eigen-3.4.1/

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
