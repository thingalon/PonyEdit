#include "licence.h"
#include "website/sitemanager.h"

#include "licencecheckdialog.h"
#include "ui_licencecheckdialog.h"

#include <QMessageBox>
#include <QCloseEvent>
#include "licence.h"

LicenceCheckDialog::LicenceCheckDialog(QWidget* parent, Licence* currentLicence) :
	QDialog(parent),
    ui(new Ui::LicenceCheckDialog)
{
    ui->setupUi(this);

	if (currentLicence->isValid())
	{
		ui->trialActivate->setEnabled(false);
		if (currentLicence->hasExpired())
		{
			//	Change the dialog to suit expired demo licences
			ui->headline->setText(tr("Your PonyEdit trial has expired!"));
			ui->actionline->setText(tr("We hope you have enjoyed using PonyEdit. If you would like to keep using it, please visit PonyEdit.com and buy a full licence!"));
		}
		else if (!currentLicence->getExpiry().isNull())
		{
			int daysLeft = currentLicence->getDaysLeft();
			ui->headline->setText(tr("Trial Licence"));
			ui->actionline->setText(tr("You have %1 days left on your PonyEdit trial licence. Once your time expires, you will need to buy PonyEdit to keep using it.").arg(daysLeft));
		}
	}

	ui->stackedWidget->setCurrentWidget(ui->mainPage);

	connect(ui->buyPonyEditButton, SIGNAL(clicked()), gSiteManager, SLOT(purchase()));
	connect(ui->trialActivate, SIGNAL(clicked()), this, SLOT(getTrial()));
	connect(gSiteManager, SIGNAL(gotTrial(QString)), this, SLOT(saveTrial(QString)));
}

LicenceCheckDialog::~LicenceCheckDialog()
{
    delete ui;
}

void LicenceCheckDialog::getTrial()
{
	ui->stackedWidget->setCurrentWidget(ui->statusPage);
	ui->statusWidget->setStatus(QPixmap(":/icons/loading.png"), tr("Fetching a trial licence..."));

	gSiteManager->getTrial();
}

void LicenceCheckDialog::getTrialFailed(const QString& error)
{
	QMessageBox::critical(this, tr("Failed to fetch licence"), error);
	ui->stackedWidget->setCurrentWidget(ui->mainPage);
}

void LicenceCheckDialog::saveTrial(const QString& key)
{
	try
	{
		Licence l(key.toUtf8());
		if (!l.isValid()) throw("Invalid licence key!");
		if (l.hasExpired()) throw("Licence key has already expired!");

		l.save();

		QMessageBox::information(this, tr("Thanks!"), tr("Thanks for trying PonyEdit. We hope you like it. Please don't hesitate to contact us to let us know what you think!"));
		accept();
	}
	catch (QString error)
	{
		ui->stackedWidget->setCurrentWidget(ui->mainPage);
		QMessageBox::critical(this, tr("Error"), error);
	}
}
