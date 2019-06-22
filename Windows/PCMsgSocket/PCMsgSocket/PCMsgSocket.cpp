#include "PCMsgSocket.h"
#include "Consts.h"

#include <string>
using namespace std;

#include <QtWidgets/QMessageBox>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QHostInfo>
#include <QtCore/QMetaEnum>

PCMsgSocketDialog::PCMsgSocketDialog(QWidget *parent) : QDialog(parent) {
	setupUIData();
	setupConnect();
}

void PCMsgSocketDialog::setupUIData() {
	ui.setupUi(this);

	setWindowFlags(Qt::Window
		| Qt::WindowCloseButtonHint
		| Qt::WindowMinimizeButtonHint 
		| Qt::WindowStaysOnTopHint);

	tcpSocket = new QTcpSocket(this);

	nowStatus = SocketStatus::notConnect;

	QString srcip = QHostInfo::fromName(QHostInfo::localHostName()).addresses().last().toString();
	ui.LineEdit_SrcIP->setText(srcip);
	ui.LineEdit_DestIP->setText("127.0.0.1");
}

void PCMsgSocketDialog::setupConnect() {
	connect(ui.Button_Connect, SIGNAL(clicked()), this, SLOT(Button_Connect_Clicked()));
	connect(ui.Button_Listen, SIGNAL(clicked()), this, SLOT(Button_Listen_Clicked()));
	connect(ui.Button_SendMsg, SIGNAL(clicked()), this, SLOT(Button_SendMsg_Clicked()));

	connect(ui.LineEdit_SendMsg, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	connect(ui.LineEdit_DestIP, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	connect(ui.LineEdit_DestPort, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	connect(ui.LineEdit_SrcPort, SIGNAL(textChanged(QString)), this, SLOT(LineEdit_TextChanged()));
	
	connect(tcpSocket, SIGNAL(connected()), this, SLOT(TCPSocket_ConnectedFromServer()));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(TCPSocket_DisconnectedByServer()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCPSocket_ErrorFromServer(QAbstractSocket::SocketError)));
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(TCPSocket_ReadyRead()));
}

PCMsgSocketDialog::~PCMsgSocketDialog() {
	tcpSocket->close();
}

#pragma region ClientSocket

// 请求连接
void PCMsgSocketDialog::Button_Connect_Clicked() {
	qDebug() << "Button_Connect_Clicked():" << "connectToHost";
	tcpSocket->connectToHost(QHostAddress::LocalHost, ui.LineEdit_DestPort->text().toInt());
}

// 服务器连接成功返回信息
void PCMsgSocketDialog::TCPSocket_ConnectedFromServer() {
	nowStatus = SocketStatus::isClient;
	QMessageBox::information(this, "Connect", "Connect Success");
}

// 服务器主动断开连接
void PCMsgSocketDialog::TCPSocket_DisconnectedByServer() {
	nowStatus = SocketStatus::notConnect;
	QMessageBox::information(this, "Connect", "Disconnected");
}

// 从服务器端收到的错误信息
void PCMsgSocketDialog::TCPSocket_ErrorFromServer(QAbstractSocket::SocketError err) {
	qDebug() << "TCPSocket_ErrorFromServer():" << err;
	QString errStr = QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(err);
	QMessageBox::critical(this, "Connect", "连接失败: " + errStr);
}

// 从服务器端收到正常信息
void PCMsgSocketDialog::TCPSocket_ReadyRead() {
	QDataStream in(tcpSocket);
	in.setVersion(Consts::Qt_Version);

	if (tcpSocket->bytesAvailable() == 0)
		return;

	QString rcvdStr;
	in >> rcvdStr;
	qDebug() << "TCPSocket_ReadyRead():" << rcvdStr;
}

// 发信息给服务器
void PCMsgSocketDialog::Button_SendMsg_Clicked() {
	if (nowStatus == SocketStatus::notConnect)
		return;

	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(Consts::Qt_Version);
	
	QString sentStr = ui.LineEdit_SendMsg->text();
	if (sentStr.isEmpty())
		return;

	qDebug() << "Button_SendMsg_Clicked():" << sentStr;
	out << sentStr;
	tcpSocket->write(block);
}

#pragma endregion ClientSocket

#pragma region ServerSocket

void PCMsgSocketDialog::Button_Listen_Clicked() {
	static ClientServer server;
	if (!server.listen(QHostAddress::Any, ui.LineEdit_SrcPort->text().toInt()))
		qDebug() << "Button_Listen_Clicked():" << "Error for listen port";
	else 
		qDebug() << "Button_Listen_Clicked():" << "Listening port:" << ui.LineEdit_SrcPort->text().toInt();
}

#pragma endregion ServerSocket

void PCMsgSocketDialog::LineEdit_TextChanged() {
	ui.Button_Connect->setEnabled(!ui.LineEdit_DestIP->text().isEmpty() &&
								  !ui.LineEdit_DestPort->text().isEmpty());

	ui.Button_SendMsg->setEnabled(!ui.LineEdit_SendMsg->text().isEmpty());

	ui.Button_Listen->setEnabled(!ui.LineEdit_SrcPort->text().isEmpty());
}

ClientSocket::ClientSocket(QObject *parent) : QTcpSocket(parent) {
	connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
}

// 从客户端读取数据
void ClientSocket::readClient() {
	QDataStream in(this);
	in.setVersion(Consts::Qt_Version);

	if (bytesAvailable() <= 0)
		return;

	QString rcvdStr;
	in >> rcvdStr;
	qDebug() << "readClient():" << rcvdStr;
}

ClientServer::ClientServer(QObject *parent) {

}

void ClientServer::incomingConnection(int socketId) {
	ClientSocket *socket = new ClientSocket(this);
	socket->setSocketDescriptor(socketId);
}