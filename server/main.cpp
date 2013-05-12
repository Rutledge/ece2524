/****************************************************************************
  From Qt examples
****************************************************************************/
// Name: Sam Darius Emrani
// ID: 9054-00931
// Email: demrani@vt.edu
// Class: ECE 3574
// Assignment: HW7
// Submission Date: 5/5/2013

#include <QApplication>
#include <QtCore>
#include <stdlib.h>
#include "server.h"

// Creates the main QT window
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Server server;
    server.show();
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    return server.exec();
}

