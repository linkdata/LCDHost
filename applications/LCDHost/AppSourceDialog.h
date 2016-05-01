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

#ifndef APPSOURCEDIALOG_H
#define APPSOURCEDIALOG_H

#include <QDialog>
#include "AppSetupItem.h"

namespace Ui {
    class AppSourceDialog;
}

class MainWindow;

class AppSourceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppSourceDialog( AppSetupItem *asi, MainWindow *parent);
    ~AppSourceDialog();

public slots:
    void evaluate();
    void setSubscribePath( const QString & path );
    void setPublishPath( const QString & path );

signals:
    void subscribePathChanged( QString s );
    void publishPathChanged( QString s );

private:
    Ui::AppSourceDialog *ui;
};

#endif // APPSOURCEDIALOG_H
