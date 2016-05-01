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

#ifndef WEBKITSERVERWINDOW_H
#define WEBKITSERVERWINDOW_H

#include <QtNetwork>

#include <QApplication>
#include <QDialog>
#include <QImage>

#include "EventWebKitHeartbeat.h"

namespace Ui
{
    class WebKitServerWindow;
}

class WebKitHeart : public QThread
{
public:
    bool alive;
    int rate;
    WebKitHeart(QObject *parent = 0 ) : QThread(parent), alive(true), rate(2) {}
    void run();
};

class WebKitServerWindow : public QDialog
{
    Q_OBJECT

public:
    WebKitServerWindow(QWidget *parent = 0);
    ~WebKitServerWindow();

    // void timerEvent(QTimerEvent *);
    void closeEvent(QCloseEvent *);
    QNetworkAccessManager *manager() { return manager_; }

    bool event(QEvent *);
    bool paused() const;
    void updateRequests();

public slots:
    void finished(QNetworkReply*);
    void newConnection();
    void refreshList();

protected:
    void changeEvent(QEvent *e);

private:
    QNetworkAccessManager *manager_;
    QLocalServer *server_;
    QTimer timer_;

private:
    Ui::WebKitServerWindow *ui;
    int rps_;
    WebKitHeart *heart_;
    QTime lastbeat_;

private slots:
    void on_pushButton_2_clicked(bool checked);
    void on_listWidget_currentRowChanged(int currentRow);
    void on_pushButton_clicked();
    void on_spinBoxMaxFPS_valueChanged(int );
};

#endif // WEBKITSERVERWINDOW_H
