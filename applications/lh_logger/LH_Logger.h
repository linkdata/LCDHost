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


#include <QtGlobal>
#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QString>
#include <QHash>
#include <QEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
typedef QtMessageHandler lh_log_handler_t;
#else
typedef QtMsgHandler lh_log_handler_t;
#endif

class LH_LoggerEvent : public QEvent
{
public:
  static QEvent::Type type();
  explicit LH_LoggerEvent(uint msgtime, QtMsgType msgtype, const QString& msgtext)
    : QEvent(type())
    , m_time(msgtime)
    , m_type(msgtype)
    , m_text(msgtext)
  {}
  QDateTime msgTime() const { return QDateTime::fromTime_t(m_time); }
  QtMsgType msgType() const { return m_type; }
  QString msgText() const { return m_text; }

protected:
  uint m_time;
  QtMsgType m_type;
  QString m_text;
};

class LH_Logger : public QObject
{
  Q_OBJECT

public:
#if defined(Q_OS_WIN)
  static QString Win32Message(int dw);
#endif
  static LH_Logger *lock();

  LH_Logger(const QString &app_name = QString(), QObject *parent = 0);
  virtual ~LH_Logger();

  QString fileName() const;
  void removeOld(int days_old = 30) const;
  void log(uint when, QtMsgType type, const QString &msg);
  void unlock() const;

  //signals:
  // void logged(uint msgtime, int msgtype, QString msgtext) const;

protected:
  static QMutex mutex_;
  static LH_Logger *instance_;

  QString app_name_;
  QString log_path_;
  QByteArray file_name_;
};

QString lh_data_dir();
QString lh_log_dir();

