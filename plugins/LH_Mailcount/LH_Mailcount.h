/**
  \file     LH_Mailcount.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#ifndef LH_MAILCOUNT_H
#define LH_MAILCOUNT_H

#include <QDebug>
#include <QTime>
#include <QImage>

#include "LH_QtInstance.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_int.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_bool.h"

class LH_Mailcount : public LH_QtInstance
{
    Q_OBJECT

    LH_Qt_int *email_count_;
    LH_Qt_QFileInfo *mail_image_;
    LH_Qt_bool *flashing_;
    LH_Qt_bool *smoothflash_;

    int envelope_count_;
    QImage envelope_;
    bool flash_on_;

public:
    LH_Mailcount();

    int count() const { return email_count_->value(); }
    int polling();
    int notify(int n,void*p);
    void prerender();
    int width(int h=-1);
    int height(int w=-1);
    QImage *render_qimage( int w, int h );
    lh_blob *render_blob(int,int) { return NULL; }

    static lh_class *classInfo();

public slots:
    void makeEnvelope();
};

#endif // LH_MAILCOUNT_H
