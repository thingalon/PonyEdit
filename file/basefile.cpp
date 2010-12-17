#include <QDebug>
#include <QCryptographicHash>

#include "file/sshfile.h"
#include "file/basefile.h"
#include "main/tools.h"
#include "main/globaldispatcher.h"
#include "file/openfilemanager.h"
#include "editor/editor.h"

const char* BaseFile::sStatusLabels[] =  { "Closed", "Loading...", "Error while loading", "Ready", "Disconnected", "Reconnecting...", "Lost Synchronization; Repairing", "Syncronization Error", "Closing" };

BaseFile* BaseFile::getFile(const Location& location)
{
	//	See if the specified location is already open...
	BaseFile* existingFile = gOpenFileManager.getFile(location);
	if (existingFile)
		return existingFile;

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
		gOpenFileManager.registerFile(newFile);

	return newFile;
}

BaseFile::~BaseFile()
{
	//	Tell every attached editor
	foreach (Editor* editor, mAttachedEditors)
		editor->fileClosed();

	if (mDocument) delete mDocument;
}

BaseFile::BaseFile(const Location& location)
{
	mLoadingPercent = 0;
	mOpenStatus = BaseFile::Closed;
	mLocation = location;

	mIgnoreChanges = 0;

	mDocument = new QTextDocument();
	mDocumentLayout = new QPlainTextDocumentLayout(mDocument);
	mDocument->setDocumentLayout(mDocumentLayout);

	connect(mDocument, SIGNAL(contentsChange(int,int,int)), this, SLOT(documentChanged(int,int,int)));
	connect(this, SIGNAL(fileOpenedRethreadSignal(QByteArray)), this, SLOT(fileOpened(QByteArray)), Qt::QueuedConnection);
	connect(this, SIGNAL(closeCompletedRethreadSignal()), this, SLOT(closeCompleted()), Qt::QueuedConnection);
}

void BaseFile::documentChanged(int position, int removeChars, int charsAdded)
{
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
	qDebug() << "handleDocumentChange " << mIgnoreChanges;
	if (mIgnoreChanges)
		return;

	mRevision++;

	mContent.replace(position, removeChars, insert);
	mChanged = true;

	qDebug() << "Checksum: " << getChecksum();
	emit unsavedStatusChanged();
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

	QTime t;
	t.start();

	//  Detect line ending mode, then convert it to unix-style. Use unix-style line endings everywhere, only convert to DOS at save time.
	mDosLineEndings = mContent.contains("\r\n");
	if (mDosLineEndings)
		mContent.replace("\r\n", "\n");

	qDebug() << "Time taken scanning document: " << t.elapsed();
	t.restart();

	mChanged = false;
	mRevision = 0;
	mLastSavedRevision = 0;

	ignoreChanges();
	mDocument->setPlainText(content);
	unignoreChanges();

	qDebug() << "Time taken calling setPlainText: " << t.elapsed();
	t.restart();

	setOpenStatus(Ready);
}

void BaseFile::openError(const QString& error)
{
	mError = error;
	setOpenStatus(LoadError);
}

void BaseFile::setOpenStatus(OpenStatus newStatus)
{
	mOpenStatus = newStatus;
	emit openStatusChanged(newStatus);
}

void BaseFile::editorAttached(Editor* editor)	//	Call only from Editor constructor.
{
	mAttachedEditors.append(editor);
}

void BaseFile::editorDetached(Editor* editor)	//	Call only from Editor destructor.
{
	mAttachedEditors.removeOne(editor);
}

QString BaseFile::getChecksum() const
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(mContent);
	return QString(hash.result().toHex().toLower());
}

void BaseFile::savedRevision(int revision, const QByteArray& checksum)
{
	mLastSaveChecksum = checksum;
	setLastSavedRevision(revision);
	gDispatcher->emitGeneralStatusMessage(QString("Finished saving ") + mLocation.getLabel() + " at revision " + QString::number(revision));
	qDebug() << "Saved revision " << revision;
	emit unsavedStatusChanged();
}

void BaseFile::setLastSavedRevision(int lastSavedRevision)
{
	this->mLastSavedRevision = lastSavedRevision;
}

void BaseFile::fileOpenProgressed(int percent)
{
	mLoadingPercent = percent;
	emit fileOpenProgress(percent);
}

const Location& BaseFile::getDirectory() const
{
	return mLocation.getDirectory();
}

void BaseFile::closeCompleted()
{
	//	If this is not the main thread, move to the main thread.
	if (!Tools::isMainThread())
	{
		emit closeCompletedRethreadSignal();
		return;
	}

	setOpenStatus(Closed);
	gOpenFileManager.deregisterFile(this);
	delete this;
}

