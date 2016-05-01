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

#include "LibDevicePointer.h"
#include "lh_plugin.h"

LibDevicePointer::LibDevicePointer(lh_device *lh_dev, const QString &object_name, QObject *parent) :
    QObject(parent),
    lh_dev_(lh_dev)
{
    setObjectName(object_name);
}

lh_device *LibDevicePointer::lock()
{
    mutex_.lock();
    if(lh_dev_)
        return lh_dev_;
    mutex_.unlock();
    return 0;
}

void LibDevicePointer::unlock()
{
    mutex_.unlock();
}

void LibDevicePointer::clear()
{
    lh_dev_ = 0;
}

LibDevicePointer::~LibDevicePointer()
{
    if(lh_dev_)
    {
        mutex_.lock();
        lh_dev_ = 0;
        mutex_.unlock();
    }
}
