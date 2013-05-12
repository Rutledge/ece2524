######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT           += network

# Input
HEADERS += server.h SSLServer.h
SOURCES += main.cpp server.cpp SSLServer.cpp
