/****************************************************************************
  From Qt Examples
****************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <SSLServer.h>
#include <QDialog>
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QTcpServer;
QT_END_NAMESPACE

class Server : public QDialog
{
	Q_OBJECT

	public:
		Server(QWidget *parent = 0);

	private slots:
		// New client connects to server
		void clientConnected();
		// Commands from the clients
		void clientMsg();
		// Connected client disconnects
		void clientDisconnected();

	private:
		// Get socket for client with given name (NULL if not found)
		QSslSocket*  getClient(const QString& name);
		// Get name for client socket, empty if not found
		QString      getName(QSslSocket* socket);
		// Generates server log
		void log(const QString& text);
		void send(const QString& name, const QString& message);
		void send(QSslSocket* socket, const QString& message);
		// Sends messages to individual clients
		void broadcast(const QString& message);

		QPlainTextEdit *logWindow;
		QPushButton *quitButton;
		SSLServer *sslServer;
		quint16  blockSize;
		QMap<QString,QSslSocket*> clients;
};

#endif
