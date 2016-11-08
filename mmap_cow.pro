TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

QMAKE_CXXFLAGS += -O0

SOURCES += main.cpp
LIBS += -lrt

include(deployment.pri)
qtcAddDeployment()

