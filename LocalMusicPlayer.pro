QT       += core gui sql
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TEMPLATE = app

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addmusicdialog.cpp \
    controlstyle.cpp \
    main.cpp \
    musictablewidget.cpp \
    sqlmanager.cpp \
    widget.cpp

HEADERS += \
    addmusicdialog.h \
    controlstyle.h \
    musictablewidget.h \
    sqlmanager.h \
    widget.h

FORMS += \
    addmusicdialog.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc \
    qss.qrc

RC_ICONS = app.ico

INCLUDEPATH += "E:/Program Files/FFmpeg/ffmpeg-n4.3.1-26-gca55240b8c-win64-gpl-shared-4.3/include"

LIBS += "E:/Program Files/FFmpeg/ffmpeg-n4.3.1-26-gca55240b8c-win64-gpl-shared-4.3/lib/libavformat.dll.a" \
        "E:/Program Files/FFmpeg/ffmpeg-n4.3.1-26-gca55240b8c-win64-gpl-shared-4.3/lib/libavutil.dll.a"   \
#        "E:/Program Files/FFmpeg/ffmpeg-n4.3.1-26-gca55240b8c-win64-gpl-shared-4.3/bin/avformat-58.dll"  \
#        "E:/Program Files/FFmpeg/ffmpeg-n4.3.1-26-gca55240b8c-win64-gpl-shared-4.3/bin/avutil-56.dll"
