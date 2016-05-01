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

#include "LH_HidRequest.h"
#include "LH_HidDevice.h"

#include <errno.h>

void LH_HidRequest::setError(const char *text, int code)
{
    if(error_.isEmpty())
    {
        error_.append(device_->objectName().toLocal8Bit())
                .append(QLatin1String(" '"))
                .append(device_->product_text().toLocal8Bit())
                ;
        switch(request_)
        {
        case HidCheckOnline:
            error_.append("' checkOnline:");
            break;
        case HidRead:
            error_.append("' read:");
            break;
        case HidWrite:
            error_.append("' write:");
            break;
        case HidReadFeature:
            error_.append("' readFeature:");
            break;
        case HidWriteFeature:
            error_.append("' writeFeature:");
            break;
        }
    }
    else
        error_.append(';');
    if(text)
        error_.append(' ').append(text);
    if(code != -1)
    {
        if(code)
            error_.append(' ').append(strerror(code));
        else
            error_.append(" failed");
#ifdef Q_OS_LINUX
        if(code == EACCES)
        {
            qDebug("Try adding the following line to <strong><tt>/etc/udev/rules.d/99-lcdhost.rules</tt></strong>");
            qDebug("SUBSYSTEM==\"hidraw\", ATTRS{idVendor}==\"%04x\", ATTRS{idProduct}==\"%04x\", MODE=\"0666\"",
                   device_->vendor_id(), device_->product_id());
        }
#endif
    }
}

bool LH_HidRequest::wait(int timeout)
{
    return semaphore_.tryAcquire(1, timeout);
}

void LH_HidRequest::abandon()
{
    semaphore_.release();
}

QEvent::Type LH_HidRequestEvent::type()
{
    static QEvent::Type eventType_ = (QEvent::Type) QEvent::registerEventType();
    return eventType_;
}
