#include "PCMsgSocket.h"
#include "Consts.h"

#include <string>
using namespace std;

#include <QtWidgets/QMessageBox>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QHostInfo>
#include <QtCore/QMetaEnum>
#include <QtCore/QDateTime>

#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif

PCMsgSocketDialog::PCMsgSocketDialog(QWidget *parent) : QDialog(parent) {
	setupUIData();
	setupConnect();
}

PCMsgSocketDialog::~PCMsgSocketDialog() {
	clientSocket->close();
}

void PCMsgSocketDialog::setupUIData() {
	ui.setupUi(this);

	setWindowFlags(Qt::Window
		| Qt::WindowCloseButtonHint
		| Qt::WindowMinimizeButtonHint 
		| Qt::WindowStaysOnTopHint);

	clientSocket = new QTcpSocket(this);
	serverSocketHandler = new ClientServer(this, &PCMsgSocketDialog::appendSent, &PCMsgSocketDialog::appendRcvd, this);

	nowStatus = SocketStatus::notConnect;

	QString srcip = QHostInfo::fromName(QHostInfo::localHostName()).addresses().last().toString();
	ui.LineEdit_SrcIP->setText(srcip);
	ui.LineEdit_DestIP->setText("127.0.0.1");
	ui.LineEdit_DestIP->setValidator(new QRegExpValidator(*new QRegExp("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])"), this));
}

void PCMsgSocketDialog::setupConnect() {
	connect(ui.Button_Connect, SIGNAL(clicked()), this, SLOT(Button_Connect_Clicked()));
	connect(ui.Button_Listen, SIGNAL(clicked()), this, SLOT(Button_Listen_Clicked()));
	connect(ui.Button_SendMsg, SIGNAL(clicked()), this, SLOT(Button_SendMsg_Clicked()));
	connect(ui.Button_DisConnect, SIGNAL(clicked()), this, SLOT(Button_DisConnect_Clicked()));
	connect(ui.Button_DisListen, SIGNAL(clicked()), this, SLOT(Button_DisListen_Clicked()));

	connect(ui.LineEdit_SendMsg, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	connect(ui.LineEdit_DestIP, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	connect(ui.LineEdit_DestPort, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	connect(ui.LineEdit_SrcPort, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	
	connect(clientSocket, SIGNAL(connected()), this, SLOT(TCPSocket_ConnectedFromServer()));
	connect(clientSocket, SIGNAL(disconnected()), this, SLOT(TCPSocket_DisconnectedByServer()));
	connect(clientSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCPSocket_ErrorFromServer(QAbstractSocket::SocketError)));
	connect(clientSocket, SIGNAL(readyRead()), this, SLOT(TCPSocket_ReadyRead()));

	connect(ui.List_Text, SIGNAL(itemSelectionChanged()), this, SLOT(List_Text_SelectionChanged()));

	connect(&nowStatus, SIGNAL(statusChange(SocketStatus::Status)), this, SLOT(SocketStatus_StatusChange(SocketStatus::Status)));
}

// 判断各种有效性
void PCMsgSocketDialog::LineEdit_TextChanged() {
	if (nowStatus == SocketStatus::notConnect) {
		ui.Button_Connect->setEnabled(!ui.LineEdit_DestIP->text().isEmpty() &&
			!ui.LineEdit_DestPort->text().isEmpty());

		ui.Button_Listen->setEnabled(!ui.LineEdit_SrcPort->text().isEmpty());
	}

	ui.Button_SendMsg->setEnabled(!ui.LineEdit_SendMsg->text().isEmpty());
}

void PCMsgSocketDialog::Button_DisConnect_Clicked() {
	clientSocket->close();
	nowStatus = SocketStatus::notConnect;
}

void PCMsgSocketDialog::Button_DisListen_Clicked() {
	serverSocketHandler->disconnect();
	nowStatus = SocketStatus::notConnect;
}

void PCMsgSocketDialog::List_Text_SelectionChanged() {
	QString currLine = ui.List_Text->selectedItems().at(0)->data(0).toString();
	ui.LineEdit_SendMsg->setText(currLine.mid(2, currLine.length() - 11 - 2)); // < 34 (07:48:37)
}

// 根据是客户端还是服务器判断操作
void PCMsgSocketDialog::SocketStatus_StatusChange(SocketStatus::Status newStatus) {
	QString statusStr = QMetaEnum::fromType<SocketStatus::Status>().valueToKey(newStatus);
	qDebug() << "SocketStatus: Now is" << statusStr;
	switch (newStatus) {
	case SocketStatus::asClient:
		ui.Label_Status->setText(QString("当前状态(客户端): 连接至服务器 %0:%1 中...").arg(ui.LineEdit_DestIP->text()).arg(ui.LineEdit_DestPort->text().toInt()));
		ui.LineEdit_DestIP->setReadOnly(true);
		ui.LineEdit_DestPort->setReadOnly(true);
		ui.LineEdit_SrcIP->setEnabled(false);
		ui.LineEdit_SrcPort->setEnabled(false);
		ui.Button_Listen->setEnabled(false);
		ui.Button_Connect->setEnabled(false);
		ui.Button_DisConnect->setEnabled(true);
	break;
	case SocketStatus::asServer:
		ui.Label_Status->setText(QString("当前状态(服务器): 监听本地端口 %0 中...").arg(ui.LineEdit_SrcPort->text().toInt()));
		ui.LineEdit_SrcIP->setReadOnly(true);
		ui.LineEdit_SrcPort->setReadOnly(true);
		ui.LineEdit_DestIP->setEnabled(false);
		ui.LineEdit_DestPort->setEnabled(false);
		ui.Button_Connect->setEnabled(false);
		ui.Button_Listen->setEnabled(false);
		ui.Button_DisListen->setEnabled(true);
	break;
	case SocketStatus::notConnect:
		ui.Label_Status->setText("当前状态: 无");
		ui.LineEdit_DestIP->setReadOnly(false);
		ui.LineEdit_DestIP->setEnabled(true);
		ui.LineEdit_DestPort->setReadOnly(false);
		ui.LineEdit_DestPort->setEnabled(true);
		ui.LineEdit_SrcIP->setEnabled(true);
		ui.LineEdit_SrcPort->setReadOnly(false);
		ui.LineEdit_SrcPort->setEnabled(true);
		ui.Button_DisConnect->setEnabled(false);
		ui.Button_DisListen->setEnabled(false);
		LineEdit_TextChanged();
	break;
	}
}

//////////////////////////////////////////////////////////////////////////

#pragma region ClientSocket

// 请求连接
void PCMsgSocketDialog::Button_Connect_Clicked() {
	ui.Label_Status->setText(QString("当前状态(客户端): 连接至 %0:%1 中...").arg(ui.LineEdit_DestIP->text()).arg(ui.LineEdit_DestPort->text().toInt()));
	// QHostAddress::LocalHost
	clientSocket->connectToHost(ui.LineEdit_DestIP->text(), ui.LineEdit_DestPort->text().toInt());
}

// 服务器连接成功返回信息
void PCMsgSocketDialog::TCPSocket_ConnectedFromServer() {
	nowStatus = SocketStatus::asClient;
	// QMessageBox::information(this, "连接", "与服务器连接成功。");
}

// 服务器主动断开连接
void PCMsgSocketDialog::TCPSocket_DisconnectedByServer() {
	nowStatus = SocketStatus::notConnect;
	QMessageBox::information(this, "连接", "与服务器连接断开。");
}

// 从服务器端收到的错误信息
void PCMsgSocketDialog::TCPSocket_ErrorFromServer(QAbstractSocket::SocketError err) {
	if (err != QAbstractSocket::RemoteHostClosedError) {
		QString errStr = QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(err);
		nowStatus = SocketStatus::notConnect;
		QMessageBox::critical(this, "连接", QString("连接失败: %0。").arg(errStr));
	}
}

// 从服务器端收到正常信息
void PCMsgSocketDialog::TCPSocket_ReadyRead() {
	QDataStream in(clientSocket);
	in.setVersion(Consts::Qt_Version);

	if (clientSocket->bytesAvailable() == 0)
		return;

	QString rcvdStr;
	in >> rcvdStr;
	qDebug() << "客户端接收" << rcvdStr;

	appendRcvd(rcvdStr);
}

// 发信息给服务器/服务器
void PCMsgSocketDialog::Button_SendMsg_Clicked() {
	if (nowStatus == SocketStatus::notConnect)
		return;

	QString sentStr = ui.LineEdit_SendMsg->text();
	if (sentStr.isEmpty())
		return;

	if (nowStatus == SocketStatus::asClient) {
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out.setVersion(Consts::Qt_Version);

		
		out << sentStr;

		qDebug() << "客户端发送" << sentStr;
		clientSocket->write(block);

		appendSent(sentStr);
	}
	else {
		// nowStatus == SocketStatus::asServer
		serverSocketHandler->sendMsg(sentStr);
	}
}

#pragma endregion ClientSocket

//////////////////////////////////////////////////////////////////////////

#pragma region ServerSocket

// 监听端口，作为服务器
void PCMsgSocketDialog::Button_Listen_Clicked() {
	// static ClientServer serverSocketHandler;
	QString port = ui.LineEdit_SrcPort->text();
	if (!serverSocketHandler->listen(QHostAddress::Any, port.toInt())) {
		QString errStr = QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(serverSocketHandler->serverError());
		QMessageBox::critical(this, "监听", QString("端口 %0 监听失败：%1。").arg(port).arg(errStr));
	}
	else {
		nowStatus = SocketStatus::asServer;
	}
}

// 接收到对端口的请求连接
void ClientServer::incomingConnection(int socketId) {
	serversocket = new ServerSocket(pCMsgSocketDialog, appendSent, appendRcvd, this);
	serversocket->setSocketDescriptor(socketId);
}

void ClientServer::sendMsg(QString msg) {
	if (serversocket != nullptr)
		serversocket->sendMsg(msg);
}

void ClientServer::disconnect() {
	if (serversocket != nullptr)
		serversocket->disconnect();
	close();
}

ServerSocket::ServerSocket(PCMsgSocketDialog *pCMsgSocketDialog, void (PCMsgSocketDialog::*appendSent)(QString), void (PCMsgSocketDialog::*appendRcvd)(QString), QObject *parent) 
	: QTcpSocket(parent) {
	connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
	// connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater())); 
	this->appendSent = appendSent;
	this->appendRcvd = appendRcvd;
	this->pCMsgSocketDialog = pCMsgSocketDialog;

}

// 从客户端收到数据
void ServerSocket::readClient() {
	QDataStream in(this);
	in.setVersion(Consts::Qt_Version);

	if (bytesAvailable() <= 0)
		return;

	QString rcvdStr;
	in >> rcvdStr;
	qDebug() << "服务器收到" << rcvdStr;
	(pCMsgSocketDialog->*appendRcvd)(rcvdStr);
}

// 向客户端发送数据
void ServerSocket::sendMsg(QString msg) {
	if (state() != QAbstractSocket::ConnectedState)
		return;

	qDebug() << "sendMsg";
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(Consts::Qt_Version);
	out << msg;

	qDebug() << "服务器发送" << msg;
	
	write(block); // ServerSocket
	(pCMsgSocketDialog->*appendSent)(msg);
}

void ServerSocket::disconnect() {
	if (state() != QAbstractSocket::ConnectedState)
		return;

	close();
}

#pragma endregion ServerSocket

// 本机发送
void PCMsgSocketDialog::appendSent(QString msg) {
	QString now = QDateTime::currentDateTime().toString("hh:mm:ss");
	QListWidgetItem *item = new QListWidgetItem(QString("< %0 (%1)").arg(msg).arg(now));
	item->setTextAlignment(Qt::AlignLeft);
	ui.List_Text->addItem(item);
	ui.List_Text->scrollToBottom();
}

// 本机接收
void PCMsgSocketDialog::appendRcvd(QString msg) {
	QString now = QDateTime::currentDateTime().toString("hh:mm:ss");
	QListWidgetItem *item = new QListWidgetItem(QString("> %0 (%1)").arg(msg).arg(now));
	item->setTextAlignment(Qt::AlignRight);
	ui.List_Text->addItem(item);
	ui.List_Text->scrollToBottom();
}