/**
  \file     LH_QtDevice.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011 Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#ifndef LH_QTDEVICE_H
#define LH_QTDEVICE_H

#include <QImage>

#include "LH_QtPlugin.h"
#include "LH_QtObject.h"

/**
  Suggested base class for LCD devices reported by your driver. You
  don't need to inherit from this, but it does provide a good starting
  point.
  */
class LH_API5PLUGIN_EXPORT LH_QtDevice : public LH_QtObject
{
    Q_OBJECT
    QString devid_;
    QByteArray devid_latin1_;
    QByteArray name_utf8_;
    lh_device lh_dev_;

public:
    LH_QtDevice( LH_QtObject *parent );
    ~LH_QtDevice();

    void userTerm();

    lh_device *lh_dev() { return &lh_dev_; }

    /**
      These methods are the ones you need to override in your
      descendant classes. And the constructor is usually a good
      idea to override as well.
      */
    virtual const char* open() { return NULL; }
    virtual const char* render_qimage(QImage*) { return NULL; }
    virtual const char* render_argb32(int,int,const void*) { return NULL; }
    virtual const char* render_mono(int,int,const void*) { return NULL; }
    virtual const char* get_backlight(lh_device_backlight*) { return NULL; }
    virtual const char* set_backlight(lh_device_backlight*) { return NULL; }
    virtual const char* close() { return NULL; }

    void arrive();
    void leave();

    QString devid() const { return devid_; }
    void setDevid(const QString &);
    QString name() const { return objectName(); }
    void setObjectName(const QString &name);
    void setName(const QString &name) { setObjectName(name); }
    QSize size() const { return QSize( lh_dev_.width, lh_dev_.height); }
    void setSize( QSize s ) { lh_dev_.width = s.width(); lh_dev_.height = s.height(); }
    void setSize( int w, int h ) { lh_dev_.width = w; lh_dev_.height = h; }
    int depth() const { return lh_dev_.depth; }
    void setDepth( int n ) { lh_dev_.depth = n; }
    void setAutoselect(bool b) { lh_dev_.noauto = !b; }
    bool autoselect() const { return lh_dev_.noauto == 0; } /* available for autoselection by LCDHost */
    bool monochrome() const { return lh_dev_.depth == 1; }
};

#endif // LH_QTDEVICE_H
