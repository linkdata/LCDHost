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

#ifndef APPDOWNLOADUPDATEDIALOG_H
#define APPDOWNLOADUPDATEDIALOG_H

#include <QDialog>
#include <QNetworkReply>

class MainWindow;

namespace Ui {
    class AppDownloadUpdateDialog;
}

class AppDownloadUpdateDialog : public QDialog
{
    Q_OBJECT
    QNetworkReply *reply_;

public:
    explicit AppDownloadUpdateDialog(MainWindow *parent);
    ~AppDownloadUpdateDialog();

    MainWindow* mainWindow() const;

public slots:
    void downloadComplete();
    void downloadProgress(qint64,qint64);

private slots:
    void on_downloadButton_clicked();
    void on_pushButton_clicked();

private:
    Ui::AppDownloadUpdateDialog *ui;
};

#endif // APPDOWNLOADUPDATEDIALOG_H
