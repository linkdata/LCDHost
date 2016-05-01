/**
  \file     LH_QtPlugin_DriveStats.h
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#ifndef LH_QTPLUGIN_DRIVESTATS_H
#define LH_QTPLUGIN_DRIVESTATS_H

#include <QDateTime>

#include "LH_QtMonitoringPlugin.h"
#include "LH_MonitoringSources.h"

#include "LH_MonitoringSource_DriveStats.h"

#include <QDebug>

class LH_QtPlugin_DriveStats : public LH_QtMonitoringPlugin
{
    Q_OBJECT

public:
    const char *userInit();

};

#endif // LH_QTPLUGIN_DRIVESTATS_H
