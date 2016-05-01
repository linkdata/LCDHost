/**
  \file     LH_LgLcdMan.h
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

/*
  The Logitech drivers make the (fairly reasonable) assumption that
  an application will only connect to one device at a time. So their
  drivers try to 'simplify' by only exposing one B/W or QVGA device
  even if more than one is available.

  This class will provide access to a B/W or QVGA device as exposed
  by the Logitech drivers. If you have multiple B/W Logitech devices,
  you need to use another method than Logitech drivers to access them
  individually.
  */

#ifndef LH_LGLCDMAN_H
#define LH_LGLCDMAN_H

#include <QImage>
#include "LH_QtPlugin.h"
#include "LH_LgLcdThread.h"


class LogitechDevice;

class LH_LgLcdMan : public LH_QtPlugin
{
    Q_OBJECT
    LH_LgLcdThread *thread_;
    LogitechDevice *bw_;
    LogitechDevice *qvga_;

public:
    LH_LgLcdMan() : LH_QtPlugin(), thread_(NULL), bw_(NULL), qvga_(NULL) {}
    ~LH_LgLcdMan();

    virtual const char *userInit();
    virtual void userTerm();

    virtual int notify(int code,void *param);

    bool event( QEvent * e );

    void setBW( QImage img ) { if( thread_ ) thread_->setBW(img); }
    void setQVGA( QImage img ) { if( thread_ ) thread_->setQVGA(img); }

    static const char *lglcd_Err( int result, const char *filename = NULL, unsigned line = 0 );
};

#endif // LH_LGLCDMAN_H
