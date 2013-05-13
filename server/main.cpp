/****************************************************************************
  From Qt examples
****************************************************************************/

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

