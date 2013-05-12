/****************************************************************************
  From the Qt examples
  Modified for secure socket connection
****************************************************************************/
// Name: Sam Darius Emrani
// ID: 9054-00931
// Email: demrani@vt.edu
// Class: ECE 3574
// Assignment: HW7
// Submission Date: 5/5/2013

#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QSslError>
#include <QSslCertificate>
#include <QSslKey>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QSslSocket;
class QSslCertificate;
QT_END_NAMESPACE

class Client : public QDialog
{
	Q_OBJECT

	public:
		Client(QWidget *parent = 0);

	private slots:
		// Inital connection
		void connectToChatServer();
		void messageComing();
		// Errors in being unable to connect to the server
		void displayError(QAbstractSocket::SocketError socketError);
		// Enable the connect button once the fields are filled
		void enableConnectButton();
		// Particular issues with SSL
		void handleSSLError(QList<QSslError>);
		// Choosing a client to chat with
		void clientSelected(QListWidgetItem* item);
		void clientSelected(const QString& name);
		// Sending messages to the server
		void sendToServer(const QString& message);
		void sendMessage();
		// Close the chat window
		void closeChat();

	private:
		// Get and display information about server certificate
		QString getCertificateString(const QSslCertificate&);
		void displayCertificateWindow();

		QLabel *hostLabel;
		QLabel *portLabel;
		QLabel *nameLabel;
		QLineEdit *hostLineEdit;
		QLineEdit *portLineEdit;
		QLineEdit *clientNameLineEdit;
		QPushButton *connectButton;
		QPushButton *quitButton;
		QDialogButtonBox *buttonBox;
		QTextEdit certificateWindow;
		QPlainTextEdit  chatLog;
		QListWidget     userList;
		QPushButton     closeChatWindow;

		QPlainTextEdit  clientChatLog;
		QLineEdit       clientMessage;
		QWidget         chatWindow;
		QString         partnerName;

		QSslSocket *secureSocket;
		QString currentMessage;
		quint16 blockSize;
};

#endif
