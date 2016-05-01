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

#ifndef LIBSETUPITEM_H
#define LIBSETUPITEM_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include "lh_plugin.h"

class AppSetupItem;
class LibObject;

class LibSetupItem
{
    QString name_;
    QVariant val_;
    lh_setup_type type_; // original type, or lh_type_none if not verified by LibInstance
    lh_setup_param param_; // plain copy of params, note param_.list is not safe
    QString mimetype_;
    QString publish_;
    QString subscribe_;
    unsigned flags_;
    QString extra_str_;
    int extra_int_;
    QString help_;
    QStringList paramlist_; // exploded param.list, if applicable

public:
    LibSetupItem( const AppSetupItem *asi );
    LibSetupItem( const lh_setup_item *item );

    QString name() const { return name_; }
    QVariant value() const { return val_; }
    lh_setup_type type() const { return type_; }
    unsigned flags() const { return flags_; }
    const lh_setup_param *param() const { return &param_; }
    QString extraStr() const { return extra_str_; }
    int extraInt() const { return extra_int_; }
    const QString & mimetype() const { return mimetype_; }
    const QString & publish() const { return publish_; }
    const QString & subscribe() const { return subscribe_; }

    void clearValue() { val_.clear(); }
    void clearSubscribe() { subscribe_.clear(); }
    void clearPublish() { publish_.clear(); }

    QString help() const { return help_; }
    QStringList paramList() const { return paramlist_; }

    void checkDefaults();
    size_t fillSize( lh_setup_item *item ) const;
    bool fillItem( lh_setup_item *item ) const;
};

#endif // LIBSETUPITEM_H
