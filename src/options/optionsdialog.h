#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QList>
#include <QMap>
#include <QToolButton>
#include "optionsdialogpage.h"

class FontOptionsWidget;

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog {
	Q_OBJECT

	public:
		enum Options { Editor, SshServers, FontsAndColors, Startup, Logging, NumOptions };

		explicit OptionsDialog( QWidget *parent = 0 );
		~OptionsDialog();

	private slots:
		void buttonClicked( QAbstractButton *button );
		void saveOptions();
		void pageClicked();
		void pageClicked( QToolButton *page );

	private:
		void addPage( QToolButton *button, OptionsDialogPage *page );

		Ui::OptionsDialog *ui;
		QList< OptionsDialogPage * > mPages;
		QMap< QToolButton *, OptionsDialogPage * > mPageMap;
};

#endif  // OPTIONSDIALOG_H
