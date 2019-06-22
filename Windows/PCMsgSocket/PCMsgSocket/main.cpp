#include "PCMsgSocket.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

int main(int argc, char *argv[])
{
	QApplication qa(argc, argv);
	// QApplication::setStyle(QStyleFactory::create("fusion"));

	auto w = new PCMsgSocketDialog();
	w->show();
	return qa.exec();
}
