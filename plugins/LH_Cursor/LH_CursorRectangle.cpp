/**
  \file     LH_CursorRectangle.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2010,2011 Andy Bridges & Johan Lindh
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

#include "LH_CursorRectangle.h"

LH_PLUGIN_CLASS(LH_CursorRectangle)

lh_class *LH_CursorRectangle::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Cursor",
        "CursorRectangle",
        "Cursor Rectangle",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

const char *LH_CursorRectangle::userInit()
{
    if( const char *err = LH_Rectangle::userInit() ) return err;
    rcvr_ = new LH_CursorReceiver(this, SLOT(stateChangeAction(bool,bool)));

    new LH_Qt_QString(this, "hr1", "<hr>", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_FIRST | LH_FLAG_HIDETITLE, lh_type_string_html );

    setup_layout_trigger_ = new LH_Qt_bool(this,"Layout Trigger",false,0);
    setup_layout_trigger_->setHelp("Cursor Rectangles can be used to create a simple menu. Simply check this box and when selected the selected layout will be opened.<br/>"
                                   "<br/>"
                                   "<b>Layout Designers Beware!</b> firing off a \"load layout\" command will not give you the option of saving any changes you may have made to the current layout! Make sure you haved saved your layout before testing layout loading!");
    setup_layout_ = new LH_Qt_QFileInfo(this, "Layout", QFileInfo(), LH_FLAG_HIDDEN);
    setup_layout_trigger_->setHelp("The layout to load when the rectangle is selected.");

    connect(setup_layout_trigger_, SIGNAL(changed()), this, SLOT(changeLayoutTrigger()));

    add_cf_target(setup_penwidth_);
    add_cf_target(setup_pencolor_);
    add_cf_target(setup_bgcolor1_);
    add_cf_target(setup_bgcolor2_);
    add_cf_target(setup_gradient_);
    add_cf_target(setup_horizontal_);
    add_cf_source(rcvr_->setup_cursor_state_);

    cf_set_rules("<rules>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>OFF</value></condition></conditions>"
                 "<actions><action type='property'><target>Fill color 1</target><value>FFFFFFFF</value></action></actions>"
                 "</rule>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>OFF_SEL</value></condition></conditions>"
                 "<actions><action type='property'><target>Fill color 1</target><value>FF0000FF</value></action></actions>"
                 "</rule>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>ON</value></condition></conditions>"
                 "<actions><action type='property'><target>Fill color 1</target><value>FFFF0000</value></action></actions>"
                 "</rule>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>ON_SEL</value></condition></conditions>"
                 "<actions><action type='property'><target>Fill color 1</target><value>FF800080</value></action></actions>"
                 "</rule>"
                 "</rules>");
    return 0;
}

void LH_CursorRectangle::stateChangeAction(bool newSelected, bool newActive)
{
    Q_UNUSED(newActive);
    if(newSelected && setup_layout_trigger_->value() && setup_layout_->value().isFile())
    {
        static QByteArray ary;
        ary = setup_layout_->value().absoluteFilePath().toUtf8();
        callback(lh_cb_load_layout, ary.data() );
    }
}

void LH_CursorRectangle::changeLayoutTrigger()
{
    setup_layout_->setFlag(LH_FLAG_HIDDEN, !setup_layout_trigger_->value());
}
