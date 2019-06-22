#ifndef PCMSGSOCKETDIALOG_H
#define PCMSGSOCKETDIALOG_H

#include "GeneratedFiles/ui_PCMsgSocket.h"
#include "Consts.h"

#include <QtWidgets/QDialog>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>

#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif

class PCMsgSocketDialog : public QDialog {
	Q_OBJECT

public:
	PCMsgSocketDialog(QWidget *parent = nullptr);
	~PCMsgSocketDialog();

private slots:
	void Button_Connect_Clicked();
	void Button_Listen_Clicked();
	void Button_SendMsg_Clicked();

	void LineEdit_TextChanged();

	void TCPSocket_ConnectedFromServer();
	void TCPSocket_DisconnectedByServer();
	void TCPSocket_ErrorFromServer(QAbstractSocket::SocketError);
	void TCPSocket_ReadyRead();

private:
	Ui::PCMsgSocketDialog ui;
	SocketStatus nowStatus;
	QTcpSocket *tcpSocket;

	void setupUIData();
	void setupConnect();
};

#endif // PCMSGSOCKETDIALOG_H


#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QtNetwork/QTcpSocket>

class ClientSocket : public QTcpSocket {
	Q_OBJECT

public:
	ClientSocket(QObject *parent = nullptr);

private slots:
	void readClient();

};

#endif // CLIENTSOCKET_H

#ifndef CLIENTSERVER_H
#define CLIENTSERVER_H

class ClientServer : public QTcpServer {
	Q_OBJECT

public:
	ClientServer(QObject *parent = nullptr);

private:
	void incomingConnection(int);
};

#endif // CLIENTSERVER_H