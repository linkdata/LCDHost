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

#include "AppDownloadUpdateDialog.h"
#include "ui_AppDownloadUpdateDialog.h"
#include "MainWindow.h"

#include <QProcess>
#include <QMessageBox>
#include <QDir>

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#define MINIZ_HEADER_FILE_ONLY
#include "../miniz/miniz.c"

AppDownloadUpdateDialog::AppDownloadUpdateDialog(MainWindow *parent)
    : QDialog(parent)
    , ui(new Ui::AppDownloadUpdateDialog)
{
    reply_ = 0;
    ui->setupUi(this);

    ui->label->setTextFormat( Qt::RichText );
    ui->label->setOpenExternalLinks( true );
    ui->label->setText(
                "<html>"
                "A new version of LCDHost is available. "
                "Do you want to download and open the installation package? "
                "You may also download it from <a href=\"http://www.linkdata.se/\">Link Data</a>."
                "</html>"
                );

    setModal(false);
    setWindowTitle( tr("New version available") );
    setAttribute( Qt::WA_DeleteOnClose, true );

    show();
}

MainWindow* AppDownloadUpdateDialog::mainWindow() const
{
    return static_cast<MainWindow*>(QDialog::parent());
}

AppDownloadUpdateDialog::~AppDownloadUpdateDialog()
{
    delete ui;
}

void AppDownloadUpdateDialog::on_downloadButton_clicked()
{
    AppPluginVersion apv;

    apv = mainWindow()->versionCache().get("LCDHost");
    if( apv.isValid() )
    {
        ui->downloadButton->setEnabled(false);
        ui->currentActionText->setText( tr("Sending request") );
        QUrl url = apv.downloadurl();
        qDebug() << "Requesting" << url.toString();
        QNetworkRequest req(url);
        req.setRawHeader(QString("User-Agent").toLatin1(),mainWindow()->webUserAgent().toLatin1());
        reply_ = mainWindow()->getNAM().get( req );
        if( reply_ )
        {
            connect( reply_, SIGNAL(finished()), this, SLOT(downloadComplete()) );
            connect( reply_, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)) );
        }
    }
}

void AppDownloadUpdateDialog::on_pushButton_clicked()
{
    if( reply_ )
    {
        reply_->abort();
        reply_ = 0;
        return;
    }
    close();
}

void AppDownloadUpdateDialog::downloadComplete()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if( reply )
    {
        Q_ASSERT( reply == reply_ );

        if( reply->error() != QNetworkReply::NoError )
        {
            ui->currentActionText->setText( reply->errorString() );
            qWarning() << "Network error:" << reply->url().toString() << reply->errorString();
            ui->downloadButton->setEnabled(true);
            reply_ = 0;
            return;
        }

        if( reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isValid() )
        {
            ui->currentActionText->setText( tr("Redirected...") );
            QUrl url = reply->url().resolved(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
            qDebug() << "Redirected to" << url.toString();
            if( url.isValid() )
            {
                QNetworkRequest req(url);
                req.setRawHeader(QString("User-Agent").toLatin1(),mainWindow()->webUserAgent().toLatin1());
                reply_ = mainWindow()->getNAM().get( req );
                if( reply_ )
                {
                    connect( reply_, SIGNAL(finished()), this, SLOT(downloadComplete()) );
                    connect( reply_, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)) );
                }
            }
            return;
        }

        if( reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200 )
        {
            // "Content-Disposition" "inline;filename="LCDHost-0-0-16.dmg""
            QDir datadir( AppState::instance()->dir_data() );
            QString ctype = QString::fromLatin1( reply->rawHeader("Content-Type") ).trimmed();
            QString disp = QString::fromLatin1( reply->rawHeader("Content-Disposition") ).trimmed();
            QByteArray content = reply->readAll();
            QString dlfilename;

            dlfilename = QFileInfo(reply->url().path()).fileName();

            if( disp.contains("filename=\"") )
            {
                dlfilename = disp.mid( disp.indexOf("filename=\"")+10 );
                if( dlfilename.endsWith('\"') ) dlfilename.chop(1);
            }

            qDebug() << "Received" << dlfilename << ":" << content.size() << "bytes of" << ctype;

            datadir.mkdir("downloads");

            dlfilename.prepend(AppState::instance()->dir_data() + "downloads/");
            QByteArray zipname_array = dlfilename.toLocal8Bit();
            QFile dlfile( dlfilename );
            if( dlfile.open(QIODevice::WriteOnly|QIODevice::Truncate) )
            {
                qint64 written = dlfile.write( content );
                if( written == content.size() )
                    qDebug() << "Wrote" << written << "bytes into" << dlfilename;
                else
                    qWarning() << "Failed to write all" << content.size() << "bytes into" << dlfilename << dlfile.errorString();
                dlfile.close();
            }
            else
            {
                qWarning() << "Can't open output file" << dlfilename << dlfile.errorString();
            }
            ui->currentActionText->setText( tr("Download complete") );
            ui->downloadButton->setEnabled(true);
            reply_ = 0;

            // unzip if applicable
            if(dlfilename.endsWith(".zip", Qt::CaseInsensitive))
            {
                qDebug() << "Unzipping download";
                ui->currentActionText->setText( tr("Unzipping download") );

                mz_zip_archive zip_archive;
                memset(&zip_archive, 0, sizeof(zip_archive));
                if(mz_zip_reader_init_file(
                            &zip_archive,
                            zipname_array.constData(),
                            MZ_ZIP_FLAG_CASE_SENSITIVE | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
                {
                    mz_uint file_count = mz_zip_reader_get_num_files(&zip_archive);
                    for(mz_uint file_index = 0; file_index < file_count; ++ file_index)
                    {
                        mz_zip_archive_file_stat file_stat;
                        if(mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat))
                        {
                            QString outfilename(QDir::fromNativeSeparators(QString::fromLocal8Bit(file_stat.m_filename)));
                            outfilename.prepend(AppState::instance()->dir_data() + "downloads/");
                            if(QFile::exists(outfilename))
                                QFile::remove(outfilename);
                            if(! mz_zip_reader_extract_to_file(
                                        &zip_archive,
                                        file_index,
                                        outfilename.toLocal8Bit().constData(),
                                        MZ_ZIP_FLAG_CASE_SENSITIVE))
                            {
                                qCritical() << "Failed to extract" << outfilename;
                                if(QFile::exists(outfilename))
                                    QFile::remove(outfilename);
                            }
                        }
                    }
                    mz_zip_reader_end(&zip_archive);
                }
                else
                    qWarning() << "Could not open" << QString::fromLocal8Bit(zipname_array);
            }

            dlfilename = QDir::fromNativeSeparators(dlfilename);

#ifdef Q_OS_MAC
            hide();

            QMessageBox::information( mainWindow(),
                                     tr("Manual update required"),
                                     tr("You now need to close LCDHost and select the Finder window "
                                        "with the disk image %1 and drag the new LCDHost into the "
                                        "Applications folder."
                                        ).arg(QFileInfo(dlfilename).filePath()) );

#endif
#ifdef Q_OS_WIN
            qDebug() << "Starting" << dlfilename;
            QProcess::startDetached(dlfilename);
#endif
            QCoreApplication::postEvent( this, new QCloseEvent() );
            return;
        }

        ui->currentActionText->setText( tr("Download failed!") );
        qDebug() << "Download failed:"
                 << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()
                 << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        ui->downloadButton->setEnabled(true);
        reply_ = 0;
    }
}

void AppDownloadUpdateDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal )
{
    ui->currentActionText->setText( tr("Downloaded %1 bytes").arg(bytesReceived) );
    ui->progressBar->setMaximum( bytesTotal );
    ui->progressBar->setValue( bytesReceived );
}
