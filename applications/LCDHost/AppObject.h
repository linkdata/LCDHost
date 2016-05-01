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

#ifndef APPOBJECT_H
#define APPOBJECT_H

#include <QtGlobal>
#include <QObject>
#include <QList>

#include "AppId.h"
#include "LibObject.h"
#include "AppState.h"
#include "AppSetupItem.h"
#include "EventRawInput.h"

class AppObject : public QObject
{
    Q_OBJECT

    AppId id_;
    unsigned eventcount_;
    bool appsetup_done_;
    bool libobject_created_;
    bool ready_; // ready to recieve LibObject setup item values (as opposed to metadata only)

protected:
    QList<AppSetupItem*> setup_items_;

public:
    explicit AppObject(QObject *parent);
    virtual ~AppObject();

    virtual void init();
    virtual void ready() {}
    virtual void term();
    virtual QString path() const;

    AppId id() const { return id_; }
    void setId( AppId id );
    QString url() const { return "lcdhostobject:"+id_.toString(); }

    void input( EventRawInput * );
    void setObjectName( const QString name );

    void setupComplete(); // call when ASI's are all loaded from storage
    QWidget *setupBuildUI(QWidget *parent = 0);
    void setupFocusUI();
    QList<AppSetupItem*>& setupItems() { return setup_items_; }

    bool event( QEvent * );

public slots:
    // void setupItemChanged( AppSetupItem* );
};

#endif // APPOBJECT_H
