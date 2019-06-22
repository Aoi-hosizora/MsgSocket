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

class ClientServer;
class ServerSocket;

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
	void SocketStatus_StatusChange(SocketStatus::Status);

	void TCPSocket_ConnectedFromServer();
	void TCPSocket_DisconnectedByServer();
	void TCPSocket_ErrorFromServer(QAbstractSocket::SocketError);
	void TCPSocket_ReadyRead();

private:
	Ui::PCMsgSocketDialog ui;
	SocketStatus nowStatus;
	QTcpSocket *clientSocket;
	ClientServer *serverSocketHandler;

	void setupUIData();
	void setupConnect();

	void appendSent(QString msg);
	void appendRcvd(QString msg);
};

#endif // PCMSGSOCKETDIALOG_H

//////////////////////////////////////////////////////////////////////////

#ifndef CLIENTSERVER_H
#define CLIENTSERVER_H

class ClientServer : public QTcpServer {
	Q_OBJECT

public:
	ClientServer(PCMsgSocketDialog* pCMsgSocketDialog, void (PCMsgSocketDialog::*appendSent)(QString), void (PCMsgSocketDialog::*appendRcvd)(QString), QObject *parent = nullptr) 
		: pCMsgSocketDialog(pCMsgSocketDialog), appendRcvd(appendRcvd), appendSent(appendSent) {}
	ServerSocket *serversocket;
	void sendMsg(QString msg);

private:
	void incomingConnection(int) override;

	PCMsgSocketDialog* pCMsgSocketDialog;
	void (PCMsgSocketDialog::*appendSent)(QString);
	void (PCMsgSocketDialog::*appendRcvd)(QString);
};

#endif // CLIENTSERVER_H

//////////////////////////////////////////////////////////////////////////

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QtNetwork/QTcpSocket>

class ServerSocket : public QTcpSocket {
	Q_OBJECT

public:
	ServerSocket(PCMsgSocketDialog*, void (PCMsgSocketDialog::*appendSent)(QString), void (PCMsgSocketDialog::*appendRcvd)(QString), QObject *parent = nullptr);
	void sendMsg(QString msg);
private slots:
	void readClient();

private:
	PCMsgSocketDialog* pCMsgSocketDialog;
	void (PCMsgSocketDialog::*appendSent)(QString);
	void (PCMsgSocketDialog::*appendRcvd)(QString);

};

#endif // CLIENTSOCKET_H