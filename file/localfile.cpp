#include <QDebug>

#include "localfile.h"

LocalFile::LocalFile(const Location& location) : BaseFile(location)
{
	connect(this, SIGNAL(localFileOpened(QByteArray)), this, SLOT(fileOpened(QByteArray)), Qt::QueuedConnection);
}

void LocalFile::newFile()
{
}

void LocalFile::open()
{
	QFile fileHandle(mLocation.getPath());
	fileHandle.open(QIODevice::ReadOnly);

	QTextStream stream(&fileHandle);

	QString content = stream.readAll();

	fileHandle.close();

	//BaseFile::fileOpened(content.toUtf8());
	emit localFileOpened(content.toUtf8());
}

void LocalFile::save()
{
	QFile fileHandle(mLocation.getPath());
	fileHandle.open(QIODevice::WriteOnly);

	QTextStream stream(&fileHandle);

	stream << mContent;
	stream.flush();

	fileHandle.close();

	mLastSavedRevision = mRevision;

	setOpenStatus(Ready);
}

void LocalFile::close()
{
	setOpenStatus(Closing);
	BaseFile::closeCompleted();
}
