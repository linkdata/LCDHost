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

#include "LH_HidWorker.h"
#include "LH_HidRequest.h"
#include "LH_HidDevice.h"
#include "LH_HidPublisher.h"
#include "LH_HidThread.h"
#include "hidapi/hidapi.h"

#include <errno.h>

#include <QThread>
#include <QEvent>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>

QAtomicPointer<LH_HidWorker> LH_HidWorker::instance_ = 0;
int LH_HidWorker::event_publish_ = -1;

#if QT_VERSION < 0x050000
# define HIDINSTANCE static_cast<LH_HidWorker *>(LH_HidWorker::instance_)
#else
# define HIDINSTANCE static_cast<LH_HidWorker *>(LH_HidWorker::instance_.load())
#endif

bool LH_HidWorker::subscribe(QObject *recipient, const char *slot)
{
    LH_HidPublisher *publisher = new LH_HidPublisher(recipient);
    if(connect(publisher, SIGNAL(onlineChanged(LH_HidDevice*,bool)), recipient, slot, Qt::QueuedConnection))
    {
        connect(HIDINSTANCE, SIGNAL(publish(LH_HidDevice*)), publisher, SLOT(publish(LH_HidDevice*)));
        connect(HIDINSTANCE, SIGNAL(publishStart()), publisher, SLOT(publishStart()));
        QCoreApplication::postEvent(HIDINSTANCE, new QEvent((QEvent::Type) event_publish_));
        connect(HIDINSTANCE, SIGNAL(onlineChanged(LH_HidDevice*,bool)), recipient, slot);
        return true;
    }
    delete publisher;
    qCritical("HID: subscribe failed: %s::%s",
              recipient->metaObject()->className(),
              (slot && isdigit(*slot)) ? slot + 1 : slot);
    return false;
}

LH_HidWorker::LH_HidWorker() :
    QObject(0),
    timer_id_(0),
    hidapi_inited_(false)
{
    if(instance_.testAndSetOrdered(0, this))
    {
        qDebug("Starting HID\n");
        event_publish_ = QEvent::registerEventType(event_publish_);
        hidapi_inited_ = !hid_init();
        timer_id_ = startTimer(2000);
    }
}

LH_HidWorker::~LH_HidWorker()
{
    if(instance_.testAndSetOrdered(this, 0))
    {
        qDebug("Stopping HID\n");
        if (timer_id_) {
          killTimer(timer_id_);
          timer_id_ = 0;
        }
        stopHidApi();
    }
}

bool LH_HidWorker::checkOnline(LH_HidDevice *hd) const
{
    Q_ASSERT(hd->unused_ == false);
    if(hidapi_inited_ == false || hd->device_path_.isEmpty())
        return false;
    if(hd->handle_ == 0)
    {
        hd->handle_ = hid_open_path(hd->device_path());
        if(hd->handle_)
            hid_set_nonblocking((hid_device*) hd->handle_, 1);
    }
    return hd->handle_ != 0;
}

void LH_HidWorker::closeDevice(LH_HidDevice *hd) const
{
    if(hd->handle_)
    {
        hid_close((hid_device*) hd->handle_);
        hd->handle_ = 0;
    }
}

void LH_HidWorker::stopHidApi()
{
    if(hidapi_inited_)
    {
        hidapi_inited_ = false;
        // qDebug("HID: stopping hidapi\n");
        foreach(LH_HidDevice *hd, findChildren<LH_HidDevice *>())
            closeDevice(hd);
        hid_exit();
    }
}

void LH_HidWorker::setOnline(LH_HidDevice *hd, bool state) const
{
    if(! state)
        closeDevice(hd);

    if(hd->online_ != state)
    {
#if 0
        qDebug("HID: %s '%s': %s",
               hd->device_path(),
               hd->product_text().toLocal8Bit().constData(),
               state ? "online" : "offline"
               );
#endif
        hd->online_ = state;
        emit onlineChanged(hd, state);
        emit hd->onlineChanged(state);
    }
}


void LH_HidWorker::processRequest(LH_HidRequest *hidreq)
{
    LH_HidDevice *hd = hidreq->device();
    int retv = -1;
    const char *error_source = "hid_open_path";
    errno = 0;
    hd->unused_ = false;

    if(checkOnline(hd))
    {
        switch(hidreq->request())
        {
        case LH_HidRequest::HidCheckOnline:
            retv = 0;
            break;
        case LH_HidRequest::HidRead:
            error_source = "hid_read";
            retv = hid_read((hid_device*) hd->handle_, hidreq->data(), hidreq->size());
            hidreq->report().truncate(retv < 0 ? 0 : retv);
            break;
        case LH_HidRequest::HidWrite:
            error_source = "hid_write";
            retv = hid_write((hid_device*) hd->handle_, hidreq->constData(), hidreq->size());
            if(retv > -1 && (size_t)retv != hidreq->size())
                qDebug("HID: %s '%s': short write (%d of %u)",
                       hd->device_path(),
                       hd->product_text_.toLocal8Bit().constData(),
                       retv, (unsigned int)hidreq->size()
                       );
            break;
        case LH_HidRequest::HidReadFeature:
            error_source = "hid_get_feature_report";
            retv = hid_get_feature_report((hid_device*) hd->handle_, hidreq->data(), hidreq->size());
            hidreq->report().truncate(retv < 0 ? 0 : retv);
            break;
        case LH_HidRequest::HidWriteFeature:
            error_source = "hid_send_feature_report";
            retv = hid_send_feature_report((hid_device*) hd->handle_, hidreq->constData(), hidreq->size());
            break;
        default:
            error_source = "unknown_request_type";
            break;
        }

        if(retv < 0)
            closeDevice(hd);
    }

    hidreq->result_code_ = retv;
    if(retv < 0)
        hidreq->setError(error_source, errno);

    if(hidreq->semaphore_.tryAcquire())
    {
        if(hidreq->error())
            qDebug("HID: abandoned %s", hidreq->error());
        delete hidreq;
    }
    else
        hidreq->semaphore_.release();

    return;
}

void LH_HidWorker::enumerateDevices()
{
    QSet<LH_HidDevice *> enumerated_set;
    bool need_reset = false;

    if(struct hid_device_info *hdi_head = hid_enumerate(0, 0))
    {
        for(struct hid_device_info *hdi = hdi_head; hdi; hdi = hdi->next)
        {
            if(hdi->path && *hdi->path)
            {
                QByteArray device_path(hdi->path);
                LH_HidDevice *hd = findChild<LH_HidDevice *>(QString::fromUtf8(device_path));
                if(hd == 0)
                {
                    hd = new LH_HidDevice(
                                device_path,
                                hdi->vendor_id,
                                hdi->product_id,
                                hdi->release_number,
                                QString::fromWCharArray(hdi->serial_number),
                                QString::fromWCharArray(hdi->manufacturer_string),
                                QString::fromWCharArray(hdi->product_string),
                                hdi->usage_page,
                                hdi->usage,
                                this
                                );
                }
#if 0
                qDebug("HID: %s '%s': %04x:%04x:%04x:%04x %s%s%s [%p]",
                       hd->device_path(),
                       hd->product_text().toLocal8Bit().constData(),
                       hdi->vendor_id, hdi->product_id, hdi->usage_page, hdi->usage,
                       enumerated_set.contains(hd) ? "(duplicate)" : "enumerated",
                       hd->online_ ? " online" : "",
                       hd->unused_ ? " unused" : "",
                       hd->handle_
                       );
#endif
                if(enumerated_set.contains(hd))
                {
                    if((! hd->unused_) &&
                            hd->usage_page() == hdi->usage_page &&
                            hd->usage() == hdi->usage &&
                            hd->device_version() == hdi->release_number &&
                            hd->serial_text() == QString::fromWCharArray(hdi->serial_number))
                    {
                        qDebug("HID: %s '%s': already enumerated",
                                  hd->device_path(),
                                  hd->product_text().toLocal8Bit().constData()
                                  );
                        need_reset = true;
                    }
                }
                else
                {
                    enumerated_set.insert(hd);
                }
            }
        }
        hid_free_enumeration(hdi_head);
    }
    else
    {
        qDebug("HID: enumeration failed");
        need_reset = true;
    }

    if(need_reset)
    {
        stopHidApi();
    }
    else
    {
        foreach(LH_HidDevice *hd, findChildren<LH_HidDevice *>())
        {
            bool is_online = enumerated_set.contains(hd);
            if(is_online && ! hd->unused_)
                is_online = checkOnline(hd);
#if 0
            qDebug("HID: %s '%s': %s%s [%p] %d",
                   hd->device_path(),
                   hd->product_text().toLocal8Bit().constData(),
                   (hd->online_ == is_online) ?
                       (is_online ? "online" : "offline") :
                       (is_online ? "came online " : "went offline "),
                   (hd->unused_ ? " unused" : ""),
                   hd->handle_,
                   hd->error_count_
                   );
#endif
            setOnline(hd, is_online);
        }
    }
}

void LH_HidWorker::timerEvent(QTimerEvent *ev)
{
    if(timer_id_ && ev->timerId() == timer_id_)
    {
        if (QCoreApplication::startingUp() || QCoreApplication::closingDown())
            return;
        if(! hidapi_inited_)
        {
            hidapi_inited_ = !hid_init();
            if(!hidapi_inited_)
                qDebug("HID: failed to start hidapi");
        }
        else
        {
            enumerateDevices();
        }
    }
    return;
}

void LH_HidWorker::customEvent(QEvent *ev)
{
    if(ev->type() == LH_HidRequestEvent::type())
    {
        processRequest((static_cast<LH_HidRequestEvent *>(ev))->request());
        return;
    }

    if(ev->type() == event_publish_)
    {
        emit publishStart();
        foreach(LH_HidDevice *hd, findChildren<LH_HidDevice *>())
            emit publish(hd);
        emit publish(0);
        return;
    }
    return;
}
