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

#ifndef LH_HIDREQUEST_H
#define LH_HIDREQUEST_H

#include <QEvent>
#include <QByteArray>
#include <QString>
#include <QSemaphore>

class LH_HidWorker;
class LH_HidDevice;

class LH_HidRequest
{
public:
    enum Request {
        HidCheckOnline,
        HidRead,
        HidWrite,
        HidReadFeature,
        HidWriteFeature
    };

    explicit LH_HidRequest(LH_HidDevice *device, LH_HidRequest::Request request, const QByteArray &report) :
        device_(device),
        request_(request),
        report_(report),
        result_code_(-1)
    {}

    explicit LH_HidRequest(LH_HidDevice *device, LH_HidRequest::Request request, int max_size = 256) :
        device_(device),
        request_(request),
        report_(max_size, '\0'),
        result_code_(-1)
    {}

    LH_HidDevice *device() const { return device_; }
    Request request() const { return request_; }
    QByteArray &report() { return report_; }
    const unsigned char *constData() const { return (const unsigned char *) report_.constData(); }
    unsigned char *data() { return (unsigned char *) report_.data(); }
    size_t size() const { return (size_t) report_.size(); }
    int resultCode() const { return result_code_; }
    const char * error() const { return error_.isNull() ? 0 : error_.constData(); }
    void setError(const char *text, int code = -1);

    bool wait(int timeout = 1000);
    void abandon();

protected:
    friend class LH_HidWorker;

    LH_HidDevice *device_;
    Request request_;
    QByteArray report_;
    int result_code_;
    QByteArray error_;
    QSemaphore semaphore_;
};

class LH_HidRequestEvent : public QEvent
{
public:
    static Type type();

    explicit LH_HidRequestEvent(LH_HidRequest *r) :
        QEvent(type()),
        request_(r)
    {}

    LH_HidRequest *request() const { return request_; }

protected:
    LH_HidRequest *request_;
};

#endif // LH_HIDREQUEST_H
