/**
  \file     LH_Qt_QFileInfo.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#ifndef LH_QT_QFILEINFO_H
#define LH_QT_QFILEINFO_H

#include <QFileInfo>
#include <QByteArray>
#include <QDir>
#include "LH_QtSetupItem.h"

class LH_Qt_QFileInfo : public LH_QtSetupItem
{
    QFileInfo fi_;
    QByteArray array_;

public:
    LH_Qt_QFileInfo( LH_QtObject *parent, QString name, QFileInfo value, int flags = 0 )
        : LH_QtSetupItem( parent, name, lh_type_string_filename, flags ), fi_(value), array_(value.filePath().toUtf8())
    {
        item_.param.size = array_.capacity();
        item_.data.s = array_.data();
        return;
    }

    virtual void setup_resize( size_t needed )
    {
        array_.resize((int) needed);
        item_.param.size = array_.capacity();
        item_.data.s = array_.data();
        return;
    }

    virtual void setup_change()
    {
        const char *path = "";
        if( parent() && parent()->state() ) path = parent()->state()->dir_layout;
        fi_ = QFileInfo( QString::fromUtf8(path), QString::fromUtf8(item_.data.s) );
        LH_QtSetupItem::setup_change();
        return;
    }

    QFileInfo& value()
    {
        return fi_;
    }

    void setValue(const QFileInfo &fi)
    {
        if( fi != fi_ )
        {
            fi_ = fi;
            array_ = fi_.filePath().toUtf8();
            item_.param.size = array_.capacity();
            item_.data.s = array_.data();
            refresh();
            emit set();
        }
    }
};

#endif // LH_QT_QFILEINFO_H
