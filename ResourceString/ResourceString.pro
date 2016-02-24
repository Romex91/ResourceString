QT += core
QT -= gui

CONFIG += c++11

TARGET = ResourceString
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += C:/projects/boost_1_59_0/
LIBS += "-LC:/projects/boost_1_59_0/stage/lib/"

mingw:QMAKE_CXXFLAGS += -march=i686 -m32
mingw:LIBS += \
    -lboost_log-mgw49-mt-s-1_59 \
    -lboost_log_setup-mgw49-mt-s-1_59 \
    -lboost_thread-mgw49-mt-s-1_59 \
    -lboost_locale-mgw49-mt-s-1_59 \
    -lboost_wserialization-mgw49-mt-s-1_59 \
    -lboost_serialization-mgw49-mt-s-1_59 \
    -lboost_system-mgw49-mt-s-1_59\
    -liconv

SOURCES += \
    Sample.cpp

HEADERS += \
    rstring.h \
    rstring/Defines.h \
    rstring/EditableResource.h \
    rstring/Resource.h \
    rstring/String.h

