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

#ifndef LH_HIDDEVICE_H
#define LH_HIDDEVICE_H

#include <QObject>
#include <QByteArray>
#include <QString>

#if defined(LH_HID_LIBRARY)
# define LH_HID_EXPORT Q_DECL_EXPORT
#else
# define LH_HID_EXPORT Q_DECL_IMPORT
#endif

class LH_HidWorker;

class LH_HID_EXPORT LH_HidDevice : public QObject
{
    Q_OBJECT

public:
    // slot signature must match "onlineChanged(LH_HidDevice*,bool)"
    static void subscribe(QObject *recipient, const char *slot);

    const char *device_path() const { return device_path_.constData(); }
    int vendor_id() const { return vendor_id_; }
    int product_id() const { return product_id_; }
    int device_version() const { return device_version_; }
    QString serial_text() const { return serial_text_; }
    QString vendor_text() const { return vendor_text_; }
    QString product_text() const { return product_text_; }
    int usage_page() const { return usage_page_; }
    int usage() const { return usage_; }

    bool online();
    QByteArray read(int max_size = 256);
    int write(const QByteArray &report);
    QByteArray readFeature(int n, int max_size = 256);
    int writeFeature(const QByteArray &report);

signals:
    void onlineChanged(bool online);

protected:
    friend class LH_HidWorker;

    explicit LH_HidDevice(
            const QByteArray &device_path,
            int vendor_id,
            int product_id,
            int device_version,
            const QString &serial_text,
            const QString &vendor_text,
            const QString &product_text,
            int usage_page,
            int usage,
            QObject *parent = 0);

    void *handle_;
    bool unused_;
    bool online_;
    QByteArray device_path_;
    int vendor_id_;
    int product_id_;
    int device_version_;
    QString serial_text_;
    QString vendor_text_;
    QString product_text_;
    int usage_page_;
    int usage_;
};

#endif // LH_HIDDEVICE_H
