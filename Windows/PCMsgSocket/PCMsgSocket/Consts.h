#ifndef CONSTS_H
#define CONSTS_H

#include <QDebug>
#include <QDataStream>

class Consts {
public:
	const static int Qt_Version = QDataStream::Qt_5_5;
};

class SocketStatus : public QObject {
	Q_OBJECT

public:
	enum Status {
		notConnect = 0, asClient, asServer, ServerGetClient
	};
	Q_ENUM(Status)

private:
	Status statusflag;

signals:
	void statusChange(SocketStatus::Status newStatus);

public:
	SocketStatus() : statusflag(notConnect) {}

	void operator=(Status newStatus) {
		this->statusflag = newStatus;
		emit statusChange(newStatus);
	}

	bool operator==(Status newStatus) {
		return this->statusflag == newStatus;
	}
};

#endif // CONSTS_H