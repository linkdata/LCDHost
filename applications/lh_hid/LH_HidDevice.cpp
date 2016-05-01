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


#include "LH_HidDevice.h"
#include "LH_HidWorker.h"
#include "LH_HidRequest.h"

#include <ctype.h>
#include <errno.h>

#include <QCoreApplication>
#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>

void LH_HidDevice::subscribe(QObject *recipient, const char *slot)
{
    LH_HidWorker::subscribe(recipient, slot);
}

LH_HidDevice::LH_HidDevice(
        const QByteArray &device_path,
        int vendor_id,
        int product_id,
        int device_version,
        const QString &serial_text,
        const QString &vendor_text,
        const QString &product_text,
        int usage_page,
        int usage,
        QObject *parent) :
    QObject(parent),
    handle_(0),
    unused_(true),
    online_(false),
    device_path_(device_path),
    vendor_id_(vendor_id & 0xFFFF),
    product_id_(product_id & 0xFFFF),
    device_version_(device_version & 0xFFFF),
    serial_text_(serial_text),
    vendor_text_(vendor_text),
    product_text_(product_text),
    usage_page_(usage_page & 0xFFFF),
    usage_(usage & 0xFFFF)
{
    setObjectName(QString::fromUtf8(device_path.constData()));
}

bool LH_HidDevice::online()
{
    if(online_ && (unused_ || handle_))
        return true;
    LH_HidRequest *hid_data = new LH_HidRequest(this, LH_HidRequest::HidCheckOnline);
    QCoreApplication::postEvent(parent(), new LH_HidRequestEvent(hid_data));
    if(! hid_data->wait())
    {
        hid_data->abandon();
        return false;
    }
    int result_code = hid_data->resultCode();
    delete hid_data;
    return result_code == 0;
}

QByteArray LH_HidDevice::read(int max_size)
{
    LH_HidRequest *hid_data = new LH_HidRequest(this, LH_HidRequest::HidRead, max_size);
    QCoreApplication::postEvent(parent(), new LH_HidRequestEvent(hid_data));
    if(! hid_data->wait())
    {
        hid_data->setError("timeout");
        qDebug("HID: %s", hid_data->error());
        hid_data->abandon();
        return QByteArray();
    }
    QByteArray report(hid_data->report());
    if(hid_data->error())
        qDebug("HID: %s", hid_data->error());
    delete hid_data;
    return report;
}

int LH_HidDevice::write(const QByteArray &report)
{
    LH_HidRequest *hid_data = new LH_HidRequest(this, LH_HidRequest::HidWrite, report);
    QCoreApplication::postEvent(parent(), new LH_HidRequestEvent(hid_data));
    if(! hid_data->wait())
    {
        hid_data->setError("timeout");
        qDebug("HID: %s", hid_data->error());
        hid_data->abandon();
        return -1;
    }
    int result_code = hid_data->resultCode();
    if(hid_data->error())
        qDebug("HID: %s", hid_data->error());
    delete hid_data;
    return result_code;
}

QByteArray LH_HidDevice::readFeature(int n, int max_size)
{
    LH_HidRequest *hid_data = new LH_HidRequest(this, LH_HidRequest::HidReadFeature, max_size);
    *(hid_data->data()) = (unsigned char) n;
    QCoreApplication::postEvent(parent(), new LH_HidRequestEvent(hid_data));
    if(! hid_data->wait())
    {
        hid_data->setError("timeout");
        qDebug("HID: %s", hid_data->error());
        hid_data->abandon();
        return QByteArray();
    }
    QByteArray report(hid_data->report());
    if(hid_data->error())
        qDebug("HID: %s", hid_data->error());
    delete hid_data;
    return report;
}

int LH_HidDevice::writeFeature(const QByteArray &report)
{
    LH_HidRequest *hid_data = new LH_HidRequest(this, LH_HidRequest::HidWriteFeature, report);
    QCoreApplication::postEvent(parent(), new LH_HidRequestEvent(hid_data));
    if(! hid_data->wait())
    {
        hid_data->setError("timeout");
        qDebug("HID: %s", hid_data->error());
        hid_data->abandon();
        return -1;
    }
    int result_code = hid_data->resultCode();
    if(hid_data->error())
        qDebug("HID: %s", hid_data->error());
    delete hid_data;
    return result_code;
}
