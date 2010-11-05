#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QByteArray>
#include <QTextDocument>

#include "location.h"

class BaseFile : public QObject
{
	Q_OBJECT

public:
	enum OpenStatus { NotOpen, Opening, Open, Error };

	static BaseFile* getFile(const Location& location);
	static const QList<BaseFile*>& getActiveFiles();

	inline const QByteArray& getContent() const { return mContent; }
	inline const Location& getLocation() const { return mLocation; }
	inline QTextDocument* getTextDocument() { return &mDocument; }
	inline const QString& getError() const { return mError; }

	virtual void open() = 0;
	virtual void save() = 0;
	void openError(const QString& error);

public slots:
	void fileOpened(const QByteArray& content);
	void documentChanged(int position, int removeChars, int added);

signals:
	void fileOpenedRethreadSignal(const QByteArray& content);
	void openStatusChanged(int newStatus);

protected:
	BaseFile(const Location& location);
	void setOpenStatus(OpenStatus newStatus);

	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);

	Location mLocation;
	QByteArray mContent;
	QString mError;

	QTextDocument mDocument;

	bool mChanged;
	bool mDosLineEndings;
	int mRevision;
	int mLastSavedRevision;

private:
	OpenStatus mOpenStatus;

	static QList<BaseFile*> sActiveFiles;
};

#endif // FILE_H
