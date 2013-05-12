// Name: Sam Darius Emrani
// ID: 9054-00931
// Email: demrani@vt.edu
// Class: ECE 3574
// Assignment: HW7
// Submission Date: 5/5/2013

#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>
#include <QList>

class SSLServer : public QTcpServer
{
    Q_OBJECT

	public:
	    SSLServer(QObject *parent = 0);
	    // Since we have a new version of incomingComing connection, we have
	    // to keep track of the SSL sockets created and return them with a
	    // a revised version of nextPendingConnection.
	    QSslSocket *nextPendingConnection();
	protected:
	    // Override of QTcpServer::incomingConnection(), see documentation 
	    // for QSslSocket.  
	    void incomingConnection(int socketDescriptor);
	private:
	    // A list to keep track of the sockets that we have created
	    QList<QSslSocket *> m_secureSocketList;
};

#endif // SSLSERVER_H
