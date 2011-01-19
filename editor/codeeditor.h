#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextBlock>

class LineNumberWidget;
class SyntaxHighlighter;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
	CodeEditor(QWidget *parent = 0);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

	int firstNonWhiteSpace(const QTextBlock& block);

protected:
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent* event);
	void applyIndent(QTextCursor& cursor, bool outdent);

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(const QRect &, int);
	void highlightCurrentLine();

private:
	LineNumberWidget *mLineNumberWidget;
	SyntaxHighlighter* mSyntaxHighlighter;

signals:
public slots:

};

#endif // CODEEDITOR_H
