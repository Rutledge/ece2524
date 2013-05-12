/****************************************************************************
  From the Qt examples
  Modified to do a secure SSL connection
****************************************************************************/
// Name: Sam Darius Emrani
// ID: 9054-00931
// Email: demrani@vt.edu
// Class: ECE 3574
// Assignment: HW7
// Submission Date: 5/5/2013

#include <QtGui>
#include <QtNetwork>
#include <QDebug>
#include "client.h"

Client::Client(QWidget *parent) : QDialog(parent)
{
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));
    nameLabel = new QLabel(tr("&Client name:"));

    // Find out which IP to connect to
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

    hostLineEdit = new QLineEdit(ipAddress);
    portLineEdit = new QLineEdit;
    clientNameLineEdit = new QLineEdit;
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);
    nameLabel->setBuddy(clientNameLineEdit);

    connectButton = new QPushButton(tr("Connect"));
    connectButton->setDefault(true);
    connectButton->setEnabled(false);

// Make the quit button exit chat sessions
    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    secureSocket = new QSslSocket(this);

    // Special slot to handle errors with the certificates
    // in particular the fact that they are self-signed
    connect(secureSocket, SIGNAL(sslErrors(QList<QSslError>)), this,
            SLOT(handleSSLError(QList<QSslError>)));

// Connect signals to slots for the chat window
    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableConnectButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableConnectButton()));
    connect(clientNameLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableConnectButton()));
    connect(connectButton, SIGNAL(clicked()),
            this, SLOT(connectToChatServer()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(secureSocket, SIGNAL(readyRead()), this, SLOT(messageComing()));
    connect(secureSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(nameLabel, 2, 0);
    mainLayout->addWidget(clientNameLineEdit, 2, 1);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(mainLayout);

	QVBoxLayout* vlay = new QVBoxLayout();
	vlay->addWidget(&clientChatLog);
	vlay->addWidget(&clientMessage);
	closeChatWindow.setText("Quit");
	vlay->addWidget(&closeChatWindow);
	chatWindow.setLayout(vlay);
	connect(&clientMessage, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
	connect(&userList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(clientSelected(QListWidgetItem*)));

    setWindowTitle(tr("Secure Chatp2p Client"));
    portLineEdit->setFocus();

	connect(&closeChatWindow, SIGNAL(clicked()), this, SLOT(closeChat()));
}

void Client::closeChat()
{
	partnerName = "";
	chatWindow.hide();
}

void Client::sendMessage()
{
	sendToServer(tr("$to %1 %2").arg(partnerName).arg(clientMessage.text()));
	clientChatLog.appendPlainText(tr("%1> %2").arg(clientNameLineEdit->text()).arg(clientMessage.text()));
	clientMessage.clear();
}

void Client::sendToServer(const QString& message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;
    out << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
	secureSocket->write(block);
}

void Client::connectToChatServer()
{
    blockSize = 0;
    secureSocket->abort();
    secureSocket->setPeerVerifyMode(QSslSocket::QueryPeer);
    secureSocket->connectToHostEncrypted(hostLineEdit->text(),
                             portLineEdit->text().toInt());
    if (!secureSocket->waitForEncrypted(1000)) {
         QMessageBox::critical(this, "ERROR", "ERROR: Could not connect to host");
         return;
    }
	sendToServer(tr("$name %1").arg(clientNameLineEdit->text()));
    displayCertificateWindow();
    //QSslCertificate peerCertificate = secureSocket->peerCertificate();
    //qDebug() << "Peer Certificate is: " << endl;
    //qDebug() << peerCertificate << endl;
    //QSslCertificate localCertificate = secureSocket->localCertificate();
    //qDebug() << "Local Certificate is: " << endl;
    //qDebug() << localCertificate << endl;
}

QString Client::getCertificateString(const QSslCertificate &cert)
{
// Grab the fields and append to certInfo
    QString certInfo;
    certInfo += "Issuer Org: ";
    certInfo += cert.issuerInfo(QSslCertificate::Organization) + "\n";
    certInfo += "Common Name: ";
    certInfo += cert.issuerInfo(QSslCertificate::CommonName) + "\n";
    certInfo += "Effective Date: ";
    certInfo += cert.effectiveDate().toString() + "\n";
    certInfo += "Expiry Date: ";
    certInfo += cert.expiryDate().toString() + "\n";
    certInfo += "Public Key: ";
    certInfo += cert.publicKey().toPem() + "\n";
    certInfo += "Serial Number: ";
    certInfo += cert.serialNumber() + "\n";
    return certInfo;
}

void Client::displayCertificateWindow()
{// Parsing getcertificate for actual display
    certificateWindow.setWindowTitle("Certificate Information");
    certificateWindow.setMinimumSize(400,300);
    QString peerCertificateInformation("Peer Certificate Information:\n");
    QSslCertificate peerCertificate = secureSocket->peerCertificate();
    peerCertificateInformation += getCertificateString(peerCertificate);
    certificateWindow.append(peerCertificateInformation);
    QString localCertificateInformation("Local Certificate Information:\n");
    QSslCertificate localCertificate = secureSocket->localCertificate();
    localCertificateInformation += getCertificateString(localCertificate);
    certificateWindow.append(localCertificateInformation);
    certificateWindow.show();
}

void Client::messageComing()
{
	QDataStream in(secureSocket);
	in.setVersion(QDataStream::Qt_4_0);

	do
	{
		qDebug() << "message coming";
		if (blockSize == 0) {
			if (secureSocket->bytesAvailable() < (int)sizeof(quint16))
				return;
			in >> blockSize;
		}

		qDebug() << "\tblocksize: " << blockSize << "\tavailable: " << secureSocket->bytesAvailable();
		if (secureSocket->bytesAvailable() < blockSize)
			return;
		QString nextMessage;
		in >> nextMessage;
		qDebug() << "\tmessage: " << nextMessage;
		blockSize = 0;

		currentMessage = nextMessage;
		if (currentMessage == "connected")
		{
			connectButton->setEnabled(false);
			hostLineEdit->setEnabled(false);
			portLineEdit->setEnabled(false);
			clientNameLineEdit->setEnabled(false);
			userList.setWindowTitle(tr("User list of %1").arg(clientNameLineEdit->text()));
			chatLog.setWindowTitle(tr("Chat log of %1").arg(clientNameLineEdit->text()));
			layout()->addWidget(&chatLog);
			layout()->addWidget(&userList);
			chatLog.setReadOnly(true);
		}
		else if (currentMessage.startsWith("$reconnect"))
		{
			connectButton->setEnabled(true);
			hostLineEdit->setEnabled(true);
			portLineEdit->setEnabled(true);
			clientNameLineEdit->setEnabled(true);
		}
		else if (currentMessage.startsWith("$addclient "))
		{
			userList.addItem(currentMessage.right(currentMessage.length() - tr("$addclient ").length()));
		}
		else if (currentMessage.startsWith("$delclient "))
		{
			QString name = currentMessage.right(currentMessage.length() - tr("$delclient ").length());
			foreach (QListWidgetItem* item, userList.findItems(name, 0))
				userList.takeItem(userList.row(item));
		}
		else if (currentMessage.startsWith("$open "))
		{
			clientSelected(currentMessage.right(currentMessage.length() - tr("$open ").length()));
		}
		else if (currentMessage.contains(QRegExp("^\\S+>")))
		{
			clientChatLog.appendPlainText(currentMessage);
		}
		else
			chatLog.appendPlainText(currentMessage);

		qDebug() << "bytes in queue: " << secureSocket->bytesAvailable();
	} while (secureSocket->bytesAvailable() > 0);
}

void Client::clientSelected(const QString& name)
{
	if (partnerName == name)
		return;
	partnerName = name;
	qDebug() << "client selected: " << partnerName;
	chatWindow.setWindowTitle(tr("<%1> chat with %2").arg(clientNameLineEdit->text()).arg(partnerName));
	chatWindow.setMinimumSize(300,400);
	chatWindow.show();
	sendToServer(tr("$open %1").arg(partnerName));
	chatWindow.setFocus();
}

void Client::clientSelected(QListWidgetItem* item)
{
	clientSelected(item->text());
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("ChatP2p Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("ChatP2p Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("ChatP2p Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(secureSocket->errorString()));
    }

    connectButton->setEnabled(true);
}

void Client::enableConnectButton()
{ //Check the lines
    connectButton->setEnabled(!hostLineEdit->text().isEmpty()
		 && !portLineEdit->text().isEmpty()
		 && !clientNameLineEdit->text().isEmpty());
}

// Special slot to handle SSL errors
void Client::handleSSLError(QList<QSslError> errorList)
{
    foreach ( QSslError error, errorList) {
       qDebug() << "Need to handle SSL error:" << error;
    }
    secureSocket->ignoreSslErrors();
}

