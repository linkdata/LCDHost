/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

  This file is part of LCDHost.

  LCDHost is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LCDHost is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LCDHost.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDate>
#include <QSettings>
#include <QFile>
#include <QApplication>
#include <QDesktopWidget>

#include "LCDHost.h"
#include "AppWelcomeDialog.h"
#include "ui_AppWelcomeDialog.h"

AppWelcomeDialog::AppWelcomeDialog(QWidget *parent) :
    QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint),
    ui(new Ui::AppWelcomeDialog)
{
    QString versionText;
    QSettings settings;
    setModal(true);
    setWindowTitle( tr("Welcome") );
    setAttribute( Qt::WA_DeleteOnClose, true );
    ui->setupUi(this);
    versionText = QString("LCDHost version %1").arg(VERSION);
    ui->versionLabel->setText( versionText );

    QString text =
            "<html><head></head><body>"
            "<p align=\"center\"><h2>Thank you for using LCDHost!</h2></p>"
            "<p align=\"justify\">If you like LCDHost, please help me out by spreading the word. "
            "Blog about it, <a href=\"http://digg.com/submit?phase=2&amp;url=http://www.linkdata.se/software/lcdhost&amp;title=\">digg it</a>, "
            "<a href=\"http://twitter.com/home?status=http://www.linkdata.se/software/lcdhost&amp;title=\">twitter</a> or "
            "<a href=\"http://www.facebook.com/sharer.php?u=http://www.linkdata.se/software/lcdhost&amp;title=\">facebook</a> "
            "about it, or simply tell your friends! If you need help, please don't hesitate to ask on the "
            "<a href=\"http://www.linkdata.se/forum/lcdhost\">forum</a>.</p></body></html>";

    QFile whatsnew(":/lcdhost/whatsnew.html");
    if( whatsnew.open(QIODevice::ReadOnly) )
    {
        text = QString::fromLatin1(whatsnew.readAll());
        whatsnew.close();
    }

    ui->welcomeText->setText(text);
    ui->welcomeText->setTextFormat(Qt::RichText);
    ui->welcomeText->setOpenExternalLinks(true);

    ui->dontShowAgainCheckBox->setChecked( settings.value("dontShowWelcome",false).toBool() );
    adjustSize();
    move( QApplication::desktop()->availableGeometry(
              QApplication::desktop()->primaryScreen() ).center() -
          rect().center() );
    show();
}

AppWelcomeDialog::~AppWelcomeDialog()
{
    delete ui;
}

void AppWelcomeDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void AppWelcomeDialog::on_okButton_clicked()
{
    close();
}

void AppWelcomeDialog::on_dontShowAgainCheckBox_stateChanged(int state)
{
    QSettings settings;
    settings.setValue( "dontShowWelcome", state == Qt::Checked );
    return;
}
