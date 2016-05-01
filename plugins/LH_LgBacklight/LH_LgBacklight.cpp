/**
  \file     LH_Lg160x43.cpp
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

#include "LH_LgBacklight.h"
#include "LgBacklightDevice.h"
#include "LH_HidDevice.h"
#include <QDebug>

LH_PLUGIN(LH_LgBacklight)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Backlight</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Logitech device backlight control via HID"
  "</shortdesc>"
  "<longdesc>"
"<p>HID-level driver for Logitech backlit devices, such as G13, G15 or G19. "
"Note that to use this driver you may need to uninstall existing drivers for these devices.</p>"
"<p>To set the backlight for all connected devices from a layout, create a <i>data source</i> named "
"<tt>/plugin/backlight/all/set</tt> using the link button <img src=\":/LH_Backlight/source.png\"></img> at any color item.</p>"
  "</longdesc>"
"</lcdhostplugin>";

const char *LH_LgBacklight::userInit()
{
    if( const char *err = LH_QtPlugin::userInit() ) return err;
    devselect_ = new LH_Qt_QStringList( this, "Device", QStringList(), LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK|LH_FLAG_NOSOURCE|LH_FLAG_NOSINK|LH_FLAG_HIDDEN );
    devselect_->setHelp( "The device which backlight you want to control." );
    connect( devselect_, SIGNAL(changed()), this, SLOT(changeDev()) );

    devcolor_ = new LH_Qt_QColor( this, "Color", Qt::white, LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK|LH_FLAG_HIDDEN );
    devcolor_->setHelp("The color you want to see used as backlight.");
    connect( devcolor_, SIGNAL(changed()), this, SLOT(changeColor()) );

    allcolor_ = new LH_Qt_QColor( this, "SetAllColor", Qt::white, LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK|LH_FLAG_HIDDEN|LH_FLAG_NOSOURCE );
    allcolor_->setSubscribePath("/plugin/backlight/all/set");
    connect( allcolor_, SIGNAL(changed()), this, SLOT(setAllColor()) );

    poll_interval_ = new LH_Qt_int(this, "Poll interval", 500, 100, 5000);
    poll_interval_->setHelp("Interval in milliseconds to check for device color changes");
    connect(poll_interval_, SIGNAL(changed()), this, SLOT(pollIntervalChanged()));

    LH_HidDevice::subscribe(this, SLOT(onlineChanged(LH_HidDevice*,bool)));
    pollIntervalChanged();
    return 0;
}

void LH_LgBacklight::userTerm()
{
}

void LH_LgBacklight::timerEvent(QTimerEvent*) {
    if(devselect_->index() >= 0 && devselect_->index() < dev_list_.size())
        if(LgBacklightDevice *bld = dev_list_.at(devselect_->index()))
            if (bld->hd()->online())
                devcolor_->setValue(bld->getDeviceColor());
}

void LH_LgBacklight::pollIntervalChanged() {
    startTimer(poll_interval_->value());
}

void LH_LgBacklight::onlineChanged(LH_HidDevice *hd, bool b)
{
    Q_UNUSED(b);
    LgBacklightDevice *bld = 0;
    if(hd->vendor_id() == 0x046d)
    {
        switch(hd->product_id())
        {
        case 0xC225: /* G11 */
        case 0xC227: /* G15v2 */
        case 0xC21C: /* G13 */
        case 0xC22D: /* G510 without audio */
        case 0xC22E: /* G510 with audio */
        case 0xC229: /* G19 */
            bld = findChild<LgBacklightDevice *>(hd->objectName());
            if(bld == 0) {
                bld = new LgBacklightDevice(hd, this);
                connect(bld, SIGNAL(colorChanged()), this, SLOT(colorChanged()));
            }
            break;
        }
    }
    if(bld)
        refreshList();
}

void LH_LgBacklight::colorChanged() {
    if(devselect_->index() >= 0 && devselect_->index() < dev_list_.size())
        if(LgBacklightDevice *bld = dev_list_.at(devselect_->index()))
            devcolor_->setValue(bld->color());
    return;
}

void LH_LgBacklight::refreshList()
{
    LgBacklightDevice *current_bld = 0;
    if(devselect_->index() >= 0 && devselect_->index() < dev_list_.size())
        current_bld = dev_list_.value(devselect_->index());

    dev_list_.clear();
    foreach(LgBacklightDevice *bld, findChildren<LgBacklightDevice *>())
    {
        if(bld->hd()->online())
            dev_list_.append(bld);
    }

    if(! dev_list_.contains(current_bld))
        current_bld = dev_list_.isEmpty() ? 0 : dev_list_.first();

    int bld_index = 0;
    QStringList sl;
    foreach(LgBacklightDevice *bld, dev_list_)
    {
        if(bld == current_bld)
            bld_index = sl.size();
        sl.append(bld->hd()->product_text());
    }

    devselect_->setVisible(sl.size() > 0);
    devcolor_->setVisible(sl.size() > 0);
    devselect_->setList(sl);
    devselect_->setIndex(bld_index);
    devcolor_->setValue(current_bld ? current_bld->color() : QColor(Qt::transparent));

    return;
}

void LH_LgBacklight::changeDev()
{
    if(devselect_->index() >= 0 && devselect_->index() < dev_list_.size())
        devcolor_->setValue(dev_list_.at(devselect_->index())->color());
}

void LH_LgBacklight::changeColor()
{
    if(devselect_->index() >= 0 && devselect_->index() < dev_list_.size())
        if(LgBacklightDevice *bld = dev_list_.at(devselect_->index()))
            if(bld->setDeviceColor(devcolor_->value()))
                bld->setColor(devcolor_->value());
}

void LH_LgBacklight::setAllColor()
{
    foreach(LgBacklightDevice *bld, dev_list_)
        if(bld->setDeviceColor(allcolor_->value()))
            bld->setColor(allcolor_->value());
}
