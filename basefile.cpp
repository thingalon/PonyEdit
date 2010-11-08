#include <QDebug.h>

#include "sshfile.h"
#include "basefile.h"
#include "tools.h"
#include "globaldispatcher.h"

QList<BaseFile*> BaseFile::sActiveFiles;

BaseFile* BaseFile::getFile(const Location& location)
{
	const QString& locationPath = location.getPath();

	//	See if the specified location is already open...
	foreach (BaseFile* file, sActiveFiles)
		if (file->getLocation().getPath() == locationPath)
			return file;

	//	If not, create a new file object.
	BaseFile* newFile = NULL;
	Location::Protocol protocol = location.getProtocol();
	switch (protocol)
	{
	case Location::Ssh:
		newFile = new SshFile(location);
		break;

	default:
		throw(QString("Opening local files is not yet supported."));
	}

	if (newFile)
	{
		sActiveFiles.append(newFile);
		gDispatcher->emitActiveFilesUpdated();
	}

	return newFile;
}

BaseFile::BaseFile(const Location& location)
{
	mOpenStatus = BaseFile::NotOpen;
	mLocation = location;

	mDocument = new QTextDocument();
	mDocumentLayout = new QPlainTextDocumentLayout(mDocument);
	mDocument->setDocumentLayout(mDocumentLayout);

	connect(mDocument, SIGNAL(contentsChange(int,int,int)), this, SLOT(documentChanged(int,int,int)));
	connect(this, SIGNAL(fileOpenedRethreadSignal(QByteArray)), this, SLOT(fileOpened(QByteArray)), Qt::QueuedConnection);
}

void BaseFile::documentChanged(int position, int removeChars, int charsAdded)
{
	if (mOpenStatus != Open)
		return;

	QByteArray added = "";
	for (int i = 0; i < charsAdded; i++)
	{
		QChar c = mDocument->characterAt(i + position);
		if (c == QChar::ParagraphSeparator || c == QChar::LineSeparator)
			c = QLatin1Char('\n');
		else if (c == QChar::Nbsp)
			c = QLatin1Char(' ');

		added += c;
	}

	this->handleDocumentChange(position, removeChars, added);
}

void BaseFile::handleDocumentChange(int position, int removeChars, const QByteArray &insert)
{
	mRevision++;
	mContent.replace(position, removeChars, insert);
	mChanged = true;
}

void BaseFile::fileOpened(const QByteArray& content)
{
	//	If this is not the main thread, move to the main thread.
	if (!Tools::isMainThread())
	{
		emit fileOpenedRethreadSignal(content);
		return;
	}

	mContent = content;

	//	Detect line ending mode, then convert it to unix-style. Use unix-style line endings everywhere, only convert to DOS at save time.
	mDosLineEndings = mContent.contains("\r\n");
	if (mDosLineEndings)
		mContent.replace("\r\n", "\n");

	mChanged = false;
	mRevision = 0;
	mLastSavedRevision = 0;

	mDocument->setPlainText(content);

	setOpenStatus(Open);
}

void BaseFile::openError(const QString& error)
{
	mError = error;
	setOpenStatus(Error);
}

void BaseFile::setOpenStatus(OpenStatus newStatus)
{
	mOpenStatus = newStatus;
	emit openStatusChanged(newStatus);
}

const QList<BaseFile*>& BaseFile::getActiveFiles()
{
	return sActiveFiles;
}

void BaseFile::editorAttached(Editor* editor)	//	Call only from Editor constructor.
{
	mAttachedEditors.append(editor);
}

void BaseFile::editorDetached(Editor* editor)	//	Call only from Editor destructor.
{
	mAttachedEditors.removeOne(editor);
}








