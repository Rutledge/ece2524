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
#include "client.h"

// Creates the main Qt window
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Client client;
    client.show();
    return client.exec();
}

