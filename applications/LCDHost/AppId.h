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

#ifndef APPID_H
#define APPID_H

#include <QDebug>
#include <QAtomicInt>
#include <QVariant>
#include <QMetaType>
#include <QString>
#include <QMutex>
#include <QVector>
#include <QEvent>

class AppObject;
class LibObject;

class AppId
{
    static QAtomicInt masterid_;
    static QMutex mutex_;
    static QVector<AppObject*> app_objects_;
    static QVector<LibObject*> lib_objects_;

    int id_;

public:
    AppId() : id_(0) {}
    AppId( int n ) : id_(n) {}
    AppId( QObject * ) : id_(masterid_.fetchAndAddOrdered(1)) {}
    AppId( const AppId& src ) : id_(src.id_) {}
    AppId( const QVariant& v ) { *this = v.value<AppId>(); }
    ~AppId() {}

    bool isValid() const { return id_ > 0 && id_ < masterid_.fetchAndAddRelaxed(0); }
    bool isEmpty() const { return !id_; }
    void clear() { id_ = 0; }

    operator QVariant() const { return QVariant::fromValue<AppId>(*this); }
    bool operator==( const AppId &other ) const { return id_ == other.id_; }
    bool operator==( const QVariant &other ) const { return other.value<AppId>() == *this; }

    AppId& operator=(const AppId& s)
    {
        if( this != &s)
        {
            id_=s.id_;
        }
        return *this;
    }

    int toInt() const { return id_; }
    QString toString() const;

    bool hasAppObject() const;
    bool hasLibObject() const;

    void setAppObject( AppObject *obj );
    void setLibObject( LibObject *obj );
    void postAppEvent( QEvent *event, int priority = Qt::NormalEventPriority );
    void postLibEvent( QEvent *event, int priority = Qt::NormalEventPriority );

    AppObject *appObject() const;

    LibObject *lock() const;
    void unlock() const;

    static int countAppObjects();
    static int countLibObjects();
    static QList<AppObject*> appObjects();
    static QList<LibObject*> libObjects();
    static AppId fromString(QString s);
    static int master() { return masterid_.fetchAndAddRelaxed(0); }
    static void notify( int n, void *p );
    // static void callback( LibObject *parent, const void *obj, int code, void *param );
};


Q_DECLARE_METATYPE(AppId)

QDebug operator<<(QDebug dbg, AppId id);

#endif // APPID_H
