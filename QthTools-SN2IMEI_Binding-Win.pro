QT       += core gui
QT       += sql
QT       += axcontainer
QT       += concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include ($$PWD/third/QSimpleUpdater/QSimpleUpdater.pri)
#DEFINES += QSU_INCLUDE_MOC=1
#INCLUDEPATH += $$PWD/include
#greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG += c++11
RC_FILE += image/quectel.rc
#DESTDIR     = $$PWD/third

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $$PWD/third
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    csvfilemange.cpp \
    kernel/mysqlite.cpp \
    kernel/newtablewidget.cpp \
    kernel/pagenavigator.cpp \
    main.cpp \
    mainwindow.cpp \
    mythead.cpp \

HEADERS += \
    csvfilemange.h \
    kernel/mysqlite.h \
    kernel/newtablewidget.h \
    kernel/pagenavigator.h \
    mainwindow.h \
    mythead.h \
    newMessageBox.h \
    third/include/progresswater.h \
    user_configure.h

FORMS += \
    csvfilemange.ui \
    kernel/pagenavigator.ui \
    mainwindow.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#不同的构建套件 debug release 依赖不同的链接库
CONFIG(debug, debug|release){
LIBS += -L$$PWD/third/ -lqucd
} else {
LIBS += -L$$PWD/third/ -lquc
}


RESOURCES += \
    resource.qrc \

DISTFILES += \
    image/quectel.ico \
    image/quectel.rc
