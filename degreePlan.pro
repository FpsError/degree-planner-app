QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    coursedetails.cpp \
    editcoursedetails.cpp \
    global_objects.cpp \
    main.cpp \
    mainwindow.cpp \
    newsemester.cpp \
    welcomewindow.cpp

HEADERS += \
    course.h \
    coursedetails.h \
    editcoursedetails.h \
    global_objects.h \
    mainwindow.h \
    newsemester.h \
    welcomewindow.h

FORMS += \
    coursedetails.ui \
    editcoursedetails.ui \
    mainwindow.ui \
    newsemester.ui \
    welcomewindow.ui

TRANSLATIONS += \
    degreePlan_ar_MA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
