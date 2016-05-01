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

#ifndef APPDEBUGDIALOG_H
#define APPDEBUGDIALOG_H

#include "AppId.h"

#include <QDialog>

class AppInstance;

namespace Ui {
    class AppDebugDialog;
}

class MainWindow;
class QTreeWidgetItem;
class AppObject;
class AppSetupItem;

class AppDebugDialog : public QDialog
{
    Q_OBJECT

public:
    AppDebugDialog(MainWindow *parent);
    ~AppDebugDialog();

    MainWindow* mainWindow() const;

    void refreshState();

    bool event(QEvent *);

    bool updateAppObject( AppObject * obj, QTreeWidgetItem * parentitem );
    bool updateTree( QObject * parentobj, QTreeWidgetItem * parentitem );
    void updateInfo( QTreeWidgetItem * item );

public slots:
    void libraryStateChanged(AppId);
    void signalLibraryStates();
    void loadLayout();
    void saveLayout();
    void layoutSaved( QString dir, QString name );
    void layoutChanged( QString dir, QString name );
    void onceASecond();
    void currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous );

protected:
    void changeEvent(QEvent *e);
    QString location(AppInstance *app_inst, QModelIndex idx);

private:
    Ui::AppDebugDialog *ui;

private slots:
    void on_logLoad_clicked(bool checked);
};

#endif // APPDEBUGDIALOG_H
