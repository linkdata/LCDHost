/**
  \file     LH_QuickAction.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010,2011 Andy Bridges
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

 **/

#include "LH_QuickAction.h"

LH_PLUGIN_CLASS(LH_QuickAction)

lh_class *LH_QuickAction::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Cursor",
        "QuickAction",
        "Quick Action",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_QuickAction::LH_QuickAction()
{
    timer_ = new QTimer(this);
    timer_->setInterval(1000);

    setup_fire_ = new LH_Qt_InputState(this,"Trigger key","",0);
    setup_fire_ ->setHelp("Binding a key to this object will allow LCDHost to load a specified layout when the key is pressed, e.g. binding the G19's \"Menu\" key and configuring it to load a menu layout.");

    setup_enable_timeout_ = new LH_Qt_bool(this,"Enable Auto Trigger",false,0);
    setup_enable_timeout_ ->setHelp("Enabling this will cause the specified to automatically load after the specified delay.<br/></br><span style='color:red'>WARNING: Once enabled the selected layout WILL BE LOADED after the delay WHETHER THIS LAYOUT HAS BEEN SAVED OR NOT!");

    setup_timeout_ = new LH_Qt_int(this,"Auto Trigger Timeout", 30, 5, 600);
    setup_timeout_->setHelp("Number of seconds after which to load the specified layout.");

    setup_countdown_ = new LH_Qt_QString(this,"^CountDown","", LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK|LH_FLAG_READONLY|LH_FLAG_HIDDEN);

    setup_layout_ = new LH_Qt_QFileInfo(this, "Layout", QFileInfo(), 0);
    setup_layout_->setHelp("The layout to load when the associated key is pressed.<br/>"
                           "<br/>"
                           "<b>Layout Designers Beware!</b> firing off a \"load layout\" command will not give you the option of saving any changes you may have made to the current layout! Make sure you haved saved your layout before testing layout loading!");

    connect( setup_fire_, SIGNAL(input(QString,int,int)), this, SLOT(doFire(QString,int,int)) );
    connect( setup_timeout_, SIGNAL(changed()), this, SLOT(changeTimeout()) );
    connect( setup_enable_timeout_, SIGNAL(changed()), this, SLOT(changeTimeout()) );
    connect( timer_, SIGNAL(timeout()), this, SLOT(doCountdown()) );
}

void LH_QuickAction::updateCountdown()
{
    if(!setup_layout_->value().isFile())
        setup_countdown_->setValue("Cannot count down trigger: target layout is not valid.");
    else
        setup_countdown_->setValue(QString("Auto-triggering in %1 second%2. MAKE SURE LAYOUT IS SAVED!").arg(counter_).arg(counter_==1?"":"s"));
}

void LH_QuickAction::doCountdown()
{
    if(setup_enable_timeout_->value())
    {
        if(setup_layout_->value().isFile())
            counter_ --;
        updateCountdown();
    }
    if(counter_<=0)
        doFire();
}

void LH_QuickAction::doFire(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);

    if(setup_layout_->value().isFile())
    {
        static QByteArray ary;
        ary = setup_layout_->value().absoluteFilePath().toUtf8();
        callback(lh_cb_load_layout, ary.data() );
    }
}

void LH_QuickAction::changeTimeout()
{
    setup_countdown_->setFlag(LH_FLAG_HIDDEN, !setup_enable_timeout_->value());
    counter_ = setup_timeout_->value();

    if(setup_enable_timeout_->value()) {
        updateCountdown();
        timer_->start();
    }
    else
        timer_->stop();
}
