#include <QtGui/QApplication>
#include "mainwindow.h"
#include "sshconnection.h"
#include "location.h"
#include <QString>
#include <QDebug>
#include <QMetaType>

int main(int argc, char *argv[])
{
	int result = 1;

	try
	{
		SshConnection::initializeLib();
		qRegisterMetaType<Location>("Location");

		QApplication a(argc, argv);
		MainWindow w;
		w.show();

		result = a.exec();
	}
	catch (const char* err)
	{
		qDebug() << "FATAL ERROR: " << err;
	}

	Location::cleanup();
	return result;
}
