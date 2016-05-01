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

#ifndef LH_HIDWORKER_H
#define LH_HIDWORKER_H

#include <QObject>
#include <QAtomicPointer>
#include <QTime>

class QEvent;
class QTimerEvent;
class LH_HidThread;
class LH_HidDevice;
class LH_HidRequest;

class LH_HidWorker : public QObject
{
    Q_OBJECT

    static QAtomicPointer<LH_HidWorker> instance_;
    static int event_publish_;

    int timer_id_;
    bool hidapi_inited_;

public:
    static bool subscribe(QObject *recipient, const char *slot);
    virtual ~LH_HidWorker();

signals:
    void publishStart() const;
    void publish(LH_HidDevice *hd) const;
    void onlineChanged(LH_HidDevice *hd, bool state) const;

protected:
    friend class LH_HidThread;
    explicit LH_HidWorker();
    void timerEvent(QTimerEvent *ev); // enumerates HID devices
    void enumerateDevices();
    void setOnline(LH_HidDevice *hd, bool state) const;

    void customEvent(QEvent *ev);
    void processRequest(LH_HidRequest *hidreq);
    bool checkOnline(LH_HidDevice *hd) const;
    void closeDevice(LH_HidDevice *hd) const;
    void stopHidApi();
};

#endif // LH_HIDWORKER_H
