/****************************************************************************
  From the Qt examples
  Modified to do a secure SSL connection
****************************************************************************/

#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include "server.h"
#include <iostream>
using namespace std;

Server::Server(QWidget *parent) : QDialog(parent)
{
	blockSize = 0;
    logWindow = new QPlainTextEdit(this);
	logWindow->setReadOnly(true);
    quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);

// Creates the new server
    sslServer = new SSLServer(this);
    if (!sslServer->listen()) {
        QMessageBox::critical(this, tr("Secure Fortune Server"),
                              tr("Unable to start the server: %1.")
                              .arg(sslServer->errorString()));
        close();
        return;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // Use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // If we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    log(tr("The server is running on\n\nIP: %1\nport: %2\n\n")
                         .arg(ipAddress).arg(sslServer->serverPort()));

// Connect signals for quit and new client connection
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(sslServer, SIGNAL(newConnection()), this, SLOT(clientConnected()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(logWindow);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
// Create the server dialog
    setWindowTitle(tr("Secure Chat P2P Server"));
}

void Server::clientConnected()
{// Messsages on new client connection
	log("clientConnected");
    QSslSocket *clientConnection = sslServer->nextPendingConnection();
    if (!clientConnection->waitForEncrypted(1000)){
        log("Waited for 1 second for encryption handshake without success");
        return;
    }
    log("Successfully waited for secure handshake...");
    connect(clientConnection, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
	connect(clientConnection, SIGNAL(readyRead()), this, SLOT(clientMsg()));
	send(clientConnection, "connected");
}

void Server::clientDisconnected()
{// Log client disconnect
	QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
	log(tr("client '%1' disconnected").arg(getName(socket)));
    socket->disconnectFromHost();
	socket->deleteLater();
	QString name = getName(socket);
// Notify all clients about disconnect
	clients.remove(name);
	broadcast(tr("$delclient %1").arg(name));
}

QString Server::getName(QSslSocket* socket)
{
	return clients.key(socket, "");
}

QSslSocket* Server::getClient(const QString& name)
{
	return clients.contains(name) ? clients[name] : NULL;
}

// Creates the server log
void Server::log(const QString& text)
{
	logWindow->appendPlainText(text);
}

void Server::clientMsg()
{
	QSslSocket* secureSocket = qobject_cast<QSslSocket*>(sender());
    QDataStream in(secureSocket);
    in.setVersion(QDataStream::Qt_4_0);

    if (blockSize == 0) {
        if (secureSocket->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> blockSize;
    }

    if (secureSocket->bytesAvailable() < blockSize)
        return;

	blockSize = 0;
    QString message;
    in >> message;
	log(tr("%1: %2").arg(getName(secureSocket)).arg(message));
// Special messages
	if (message.startsWith("$"))
	{
		QStringList splitted = message.split(QRegExp("\\s+"));
		QString command = splitted.takeFirst();
		QString args = message.right(message.length() - command.length() - 1).trimmed();
		if (command == "$name")
		{
			if (args.trimmed().isEmpty())
			{
				QString errmsg = "ERROR: invalid client name!";
				log (errmsg);
				send(secureSocket, "$reconnect");
				send(secureSocket, errmsg);
			}
// Name repeats
			else if (clients.contains(args))
			{
				QString errmsg = tr("ERROR: client '%1' is already connected").arg(args);
				log (errmsg);
				send(secureSocket, errmsg);
			}
// Welcome new clients
			else
			{
				log(tr("*** client connected: '%1'").arg(args));
				foreach(QString s, clients.keys())
					send(secureSocket, tr("$addclient %1").arg(s));
				broadcast(tr("$addclient %1").arg(args));
				clients[args] = secureSocket;
				send(secureSocket, tr("welcome %1").arg(args));
			}
		}
// Opening new chat sessions
		else if (command == "$open")
		{
			send(args, tr("$open %1").arg(getName(secureSocket)));
			log(tr("chat open from %1 to %2").arg(getName(secureSocket)).arg(args));
		}
		else if (command == "$to")
		{
			QString target = args.split(QRegExp("\\s+"))[0];
			QString msg = args.right(args.length() - target.length() - 1);
			send(target, tr("%1> %2").arg(getName(secureSocket)).arg(msg));
			log(tr("from %1 to %2: %3").arg(getName(secureSocket)).arg(target).arg(msg));
		}
	}
	else
		send(secureSocket, "ERROR: Unknown command");
}

void Server::broadcast(const QString& message)
{
	foreach (QSslSocket* socket, clients)
		send(socket, message);
}

void Server::send(const QString& name, const QString& message)
{
	if (!getClient(name))
	{
		log(tr("ERROR: can't send to %1: socket not found").arg(name));
		log("clients known:");
		foreach(QString s, clients.keys())
			log(tr("'%1'").arg(s));
		return;
	}
	send(getClient(name), message);
}

void Server::send(QSslSocket* socket, const QString& message)
{
	if (!socket)
	{
		log("ERROR: NULL socket");
		return;
	}
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;
    out << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
	qDebug() << "message to socket: " << socket << ": " << message;
	log(tr("TO '%1': %2").arg(getName(socket)).arg(message));
	log(tr("  message size: %1 written: %2").arg(block.size()).arg(socket->write(block)));
}
