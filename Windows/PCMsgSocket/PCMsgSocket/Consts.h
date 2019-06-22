#ifndef CONSTS_H
#define CONSTS_H

#include <QtNetwork/QTcpSocket>

enum SocketStatus {
	isClient, isServer, notConnect,
};

class Consts {
public:
	const static int Qt_Version = QDataStream::Qt_5_5;
};

#endif // CONSTS_H