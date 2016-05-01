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


#include <QSettings>
#include <QNetworkProxy>
#include <QDebug>

#include "WebKitServerWindow.h"
#include "ui_WebKitServerWindow.h"
#include "WebKitRequest.h"


void WebKitHeart::run()
{
    while( alive )
    {
        if( rate < 1 ) rate = 1;
        QCoreApplication::postEvent( parent(), new EventWebKitHeartbeat(), Qt::HighEventPriority );
        int delay = (1000/rate) - (QTime::currentTime().msec()%(1000/rate));
        if( delay < 10 ) delay = 10;
        if( delay > 1000 ) delay = 1000;
        msleep( delay );
    }
    return;
}

WebKitServerWindow::WebKitServerWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WebKitServerWindow)
{
    QSettings settings;

    ui->setupUi(this);

    setWindowTitle("WebKitServer");

    manager_ = new QNetworkAccessManager();

    QNetworkProxy proxy = QNetworkProxyFactory::systemProxyForQuery().first();
    if( proxy.type() != QNetworkProxy::NoProxy )
    {
        QNetworkProxy::setApplicationProxy( proxy );
        manager_->setProxy( proxy );
        qDebug() << "HTTP proxy" << proxy.hostName() << ":" << proxy.port();
    }
    else
        qDebug() << "HTTP proxy not detected";

    connect( manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)) );
    server_ = new QLocalServer();
    connect( server_, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    if (!server_->listen("LCDHost_WebKitServer")) {
      if (QLocalServer::removeServer("LCDHost_WebKitServer")) {
        if (!server_->listen("LCDHost_WebKitServer")) {
          qCritical() << "Failed to start listening";
        }
      }
    }

    rps_ = 0;
    heart_ = new WebKitHeart(this);
    heart_->rate = settings.value("heartrate",30).toInt();
    heart_->start( QThread::HighPriority );
    lastbeat_ = QTime::currentTime();
    ui->spinBoxMaxFPS->setValue(heart_->rate);
}

WebKitServerWindow::~WebKitServerWindow()
{
    QSettings settings;
    if( heart_ )
    {
        settings.setValue("heartrate",heart_->rate);
        heart_->alive = false;
        for( int waited = 0; heart_->isRunning(); ++ waited )
        {
            QCoreApplication::processEvents();
            if( ! heart_->wait(100) && waited > 50 )
            {
                waited = 40;
                qWarning( "WebKitServer: waiting for heartbeat thread to stop" );
            }
        }
        delete heart_;
        heart_ = 0;
    }
    if( manager_ )
    {
        delete manager_;
        manager_ = NULL;
    }
    if( server_ )
    {
        delete server_;
        server_ = NULL;
    }
    delete ui;
}

bool WebKitServerWindow::paused() const
{
    return ui ? ui->pushButton_2->isChecked() : false;
}

void WebKitServerWindow::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void WebKitServerWindow::closeEvent(QCloseEvent *event)
{
    int still_alive = 0;
    foreach( WebKitRequest* req, findChildren<WebKitRequest*>() )
    {
        if( req )
        {
            if( req->isDead() ) delete req;
            else
            {
                req->dying();
                ++ still_alive;
            }
        }
    }
    if( !still_alive ) event->accept();
    return;
}

void WebKitServerWindow::finished(QNetworkReply *reply)
{
    if( reply->error() != QNetworkReply::NoError &&
        reply->error() != QNetworkReply::OperationCanceledError )
        qWarning() << reply->url().toString() << ": " << reply->errorString();
    return;
}

void WebKitServerWindow::newConnection()
{
    if( paused() ) return;

    while( server_->hasPendingConnections() )
    {
        QLocalSocket *sock = server_->nextPendingConnection();
        if( sock->state() == QLocalSocket::ConnectingState ||
            sock->state() == QLocalSocket::ConnectedState )
        {
            WebKitRequest *req = new WebKitRequest( sock, this );
            connect( req, SIGNAL(dead()), this, SLOT(refreshList()) );
            connect( req, SIGNAL(destroyed()), this, SLOT(refreshList()) );
            refreshList();
        }
    }
}

void WebKitServerWindow::refreshList()
{
    ui->listWidget->clear();
    ui->labelHtml->clear();
    ui->labelImage->clear();
    ui->labelUrl->clear();
    ui->labelSize->clear();

    foreach( WebKitRequest* req, findChildren<WebKitRequest*>() )
    {
        if( req )
        {
            QListWidgetItem *item = new QListWidgetItem();
            item->setText( QString::number( (qptrdiff)req, 16 ) );
            item->setData( Qt::UserRole, (qlonglong)(qptrdiff)(void*)req );
            ui->listWidget->addItem( item );
            item->setSelected( true );
            if( req->isDead() ) item->setTextColor( Qt::red );
            else item->setTextColor( Qt::green );
        }
    }

    return;
}

bool WebKitServerWindow::event(QEvent *e)
{
    if( e->type() == EventWebKitHeartbeat::type() )
    {
        // if( lastbeat_.elapsed() > 3*(1000/heart_->rate) ) qWarning() << lastbeat_.elapsed() << "ms since last heartbeat, interval" << (1000/heart_->rate);
        lastbeat_ = QTime::currentTime();
        updateRequests();
        return true;
    }

    return QDialog::event(e);
}

void WebKitServerWindow::updateRequests()
{
    static int last_ms = 0;
    int now_ms = QTime::currentTime().msec();
    qptrdiff current_req = 0;
    QListWidgetItem *item = ui->listWidget->item( ui->listWidget->currentRow() );

    if( paused() ) return;

    if( item ) current_req = item->data( Qt::UserRole ).toInt();

    foreach( WebKitRequest* req, findChildren<WebKitRequest*>() )
    {
        if( req )
        {
            if( req->isDirty() )
            {
                rps_ ++;
                req->repaint();
                if( current_req == (qptrdiff)req )
                {
                    ui->labelSize->setText( QString("%1x%2").arg(req->size().width()).arg(req->size().height()) );
                    ui->labelUrl->setText( req->url().toString() );
                    ui->labelHtml->setText( req->html().isEmpty() ? QString("<none>") : QString("%1 characters").arg(req->html().size()) );
                    ui->labelImage->setPixmap( QPixmap::fromImage(req->image()) );
                }
            }
            if( req->isDead() )
                delete req;
        }
    }

    if( last_ms > now_ms )
    {
        ui->labelRPS->setText( QString("%1 renders/second").arg(rps_) );
        rps_ = 0;
    }
    last_ms = now_ms;
}

void WebKitServerWindow::on_spinBoxMaxFPS_valueChanged(int newrate)
{
    heart_->rate = newrate;
}

void WebKitServerWindow::on_pushButton_clicked()
{
    hide();
}

void WebKitServerWindow::on_listWidget_currentRowChanged(int row)
{
    QListWidgetItem *item = ui->listWidget->item(row);
    if( item )
    {
        qptrdiff itemInt = item->data( Qt::UserRole ).toLongLong();
        foreach( WebKitRequest* req, findChildren<WebKitRequest*>() )
        {
            if( req && ( (qptrdiff)(void*)req == itemInt ) )
            {
                ui->labelSize->setText( QString("%1x%2").arg(req->size().width()).arg(req->size().height()) );
                ui->labelUrl->setText( req->url().toString() );
                ui->labelHtml->setText( req->html().isEmpty() ? QString("<none>") : QString("%1 characters").arg(req->html().size()) );
                ui->labelImage->setPixmap( QPixmap::fromImage(req->image()) );
                return;
            }
        }
        qDebug() << "could not find WebKitRequest for row" << row << (void*)itemInt;
    }
}

void WebKitServerWindow::on_pushButton_2_clicked(bool checked)
{
    if( checked )
        ui->pushButton_2->setText(tr("Resume"));
    else {
      ui->pushButton_2->setText(tr("Pause"));

    }
}
