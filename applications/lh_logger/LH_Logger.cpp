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


#include <stdio.h>
#include <time.h>
#include <limits.h>

#include "LH_Logger.h"

#include <QtGlobal>
#include <QDebug>
#include <QCoreApplication>
#include <QMutex>
#include <QFileInfo>
#include <QString>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
QString LH_Logger::Win32Message(int dw) {
  QString s;
  LPVOID lpMsgBuf;

  FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD) dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

  s = QString::fromUtf16( (const ushort*) lpMsgBuf ).trimmed();
  LocalFree(lpMsgBuf);
  return s;
}
#endif

QMutex LH_Logger::mutex_;
LH_Logger *LH_Logger::instance_ = 0;
static lh_log_handler_t lh_log_old_handler = 0;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
static lh_log_handler_t (*lh_log_install_message_handler)(lh_log_handler_t) = qInstallMessageHandler;

QString lh_data_dir()
{
  QString data_dir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
  if(data_dir.isEmpty())
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  return data_dir.append(QLatin1String("/LCDHost"));
}

static void lh_log_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (lh_log_old_handler) {
    if (lh_log_old_handler == lh_log_handler) {
      lh_log_old_handler = 0;
      Q_ASSERT(!"lh_log_old_handler == lh_log_handler");
    } else {
      lh_log_old_handler(type, context, msg);
    }
  }
  if(LH_Logger *logger = LH_Logger::lock())
  {
    time_t when;
    time(&when);
    logger->log(when, type, msg);
    if (logger->parent()) {
      try {
        QCoreApplication::postEvent(logger->parent(), new LH_LoggerEvent(when, type, msg), Qt::LowEventPriority);
      } catch(...) {
        logger->log(when, QtFatalMsg, QLatin1String("new LH_LoggerEvent() caused exception"));
        /* no-op */
      }
    }
    logger->unlock();
  }
  return;
}
#else
#include <QDesktopServices>
static lh_log_handler_t (*lh_log_install_message_handler)(lh_log_handler_t) = qInstallMsgHandler;

QString lh_data_dir()
{
  QString data_dir(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
  if(data_dir.isEmpty())
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
  return data_dir.append(QLatin1String("/LCDHost"));
}
static void lh_log_handler(QtMsgType type, const char *msg_p)
{
  if(lh_log_old_handler)
    lh_log_old_handler(type, msg_p);
#ifndef QT_NO_DEBUG
  else
  {
    // Need this to get log output in Qt Creator with Qt 4
    qInstallMsgHandler(0);
    switch(type)
    {
      case QtDebugMsg:
        qDebug("%s", msg_p);
        break;
      case QtWarningMsg:
        qWarning("%s", msg_p);
        break;
      default:
        qCritical("%s", msg_p);
        break;
    }
    qInstallMsgHandler(lh_log_handler);
  }
#endif
  if(LH_Logger *logger = LH_Logger::lock())
  {
    logger->log(type, QString::fromLatin1(msg_p));
    logger->unlock();
  }
  return;
}
#endif

QString lh_log_dir()
{
  return lh_data_dir().append(QLatin1String("/logs"));
}

QEvent::Type LH_LoggerEvent::type()
{
  static QEvent::Type event_type = (QEvent::Type) QEvent::registerEventType();
  return event_type;
}

LH_Logger *LH_Logger::lock()
{
  if(!mutex_.tryLock(1000)) {
    if (lh_log_old_handler) {
      QMessageLogContext ctx;
      lh_log_old_handler(
            QtCriticalMsg,
            ctx,
            QString("LH_Logger::lock() failed to aquire mutex")
            );
    } else {
      qCritical("LH_Logger::lock() failed to aquire mutex");
    }
    return 0;
  }
  if(!instance_) {
    mutex_.unlock();
    return 0;
  }
  return instance_;
}

LH_Logger::LH_Logger(const QString &app_name, QObject *parent)
  : QObject(parent)
{
  Q_ASSERT(instance_ == 0);
  Q_ASSERT(lh_log_old_handler == 0);
  mutex_.lock();
  app_name_ = app_name.isEmpty() ? QCoreApplication::applicationName() : app_name;
  QDir logdir(lh_log_dir());
  log_path_ = logdir.absolutePath();
  logdir.mkpath(log_path_);
  file_name_ = QString("%1/%2-%3.log")
      .arg(log_path_)
      .arg(app_name_)
      .arg(QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss"))
      .toLocal8Bit()
      ;
  instance_ = this;
  mutex_.unlock();
  lh_log_old_handler = lh_log_install_message_handler(lh_log_handler);
  return;
}

LH_Logger::~LH_Logger()
{
  Q_ASSERT(instance_ == this);
  lh_log_handler_t old_handler = lh_log_old_handler;
  lh_log_old_handler = 0;
  lh_log_install_message_handler(old_handler);
  mutex_.lock();
  instance_ = 0;
  mutex_.unlock();
  // qDebug("LH_Logger: %d filtered, %d known", filter_count_, last_seen_.size());
  return;
}

QString LH_Logger::fileName() const
{
  return QString::fromLocal8Bit(file_name_);
}

void LH_Logger::removeOld(int days_old) const
{
  QDir dir(log_path_);
  foreach(const QFileInfo &info, dir.entryInfoList(QDir::Files | QDir::Writable))
  {
    QString name(info.fileName());
    if( name.startsWith(app_name_) && name.endsWith(".log") )
    {
      if( info.lastModified().daysTo( QDateTime::currentDateTime() ) > days_old )
        dir.remove( name );
    }
  }
  return;
}

void LH_Logger::log(uint when, QtMsgType type, const QString &msg)
{
  if(!when || msg.isEmpty())
    return;

  if(struct tm *lt = localtime((time_t*) &when))
  {
    if(FILE *f = fopen(file_name_.constData(), "a"))
    {
      const char *msg_type = "???";
      const QByteArray msg_text(msg.toLocal8Bit());

      switch(type)
      {
        case QtInfoMsg:     msg_type = "..."; break;
        case QtDebugMsg:    msg_type = "D.."; break;
        case QtWarningMsg:  msg_type = ".W."; break;
        case QtCriticalMsg: msg_type = "..E"; break;
        case QtFatalMsg:    msg_type = "***"; break;
      }

      fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d | %s | %s\n",
              lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
              lt->tm_hour, lt->tm_min, lt->tm_sec,
              msg_type, msg_text.constData());
#ifdef QT_NO_DEBUG
      if(type == QtCriticalMsg)
#endif
        fflush(f);
      fclose(f);
    }
  }

  return;
}

void LH_Logger::unlock() const
{
  mutex_.unlock();
}

