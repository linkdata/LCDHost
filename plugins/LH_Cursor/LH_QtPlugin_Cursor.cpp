/**
  \file     LH_QtPlugin_Cursor.cpp
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

#include "LH_QtPlugin_Cursor.h"
#include <QDebug>

LH_PLUGIN(LH_QtPlugin_Cursor)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Cursor</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" STRINGIZE(VERSION) "\nr" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Andy \"Triscopic\" Bridges</author>"
  "<homepageurl><a href=\"http://www.codeleap.co.uk\">CodeLeap</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Allows some complex cursor-like and tab-like behaviours"
  "</shortdesc>"
  "<longdesc>"
    "This plugin allows the user to: <ul>"
    "<li>Create menu layouts that can load other layouts when an item is selected by a cursor.</li>"
    "<li>Allows non-menu layouts to load another (menu) layout at the press of a button.</li>"
    "<li>Create tabs or pages in a layout that a user can navigate through.</li>"
    "<li>Create layouts which have \"drill down\" effects, in which panels can be selected to reveal "
    "(or hide) another panel.</li>"
    "<li>Have one layout load another after a certain number of seconds.</li>"
    "</ul>"
    "Note that pages that are added will only be visible when the cursor is on them; when building your layout "
    "make sure you have assigned the page a suitable coordinate and move the cursor to it. New pages start with "
    "a coordintate of 1,1 so might not always be visible when first added (i.e. if the cursor is not at 1,1 "
    "newly added pages will be hidden from view immediately)."
"</longdesc>"
"</lcdhostplugin>";

//LH_QtPlugin_Cursor::LH_QtPlugin_Cursor() : setup_keys_(){}

const char *LH_QtPlugin_Cursor::userInit() {
    keyDelay.start();

    setup_enable_favourite_shortcut_ = new LH_Qt_bool(this, "^Enable Favourite Layout Shortcut", false, LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_enable_favourite_shortcut_->setHelp("Enabling this box will allow you to assign a series of key presses that will always load a certain layout, e.g. a menu layout.");

    setup_favourite_layout_ = new LH_Qt_QFileInfo(this, "Favourite Layout", QFileInfo(), LH_FLAG_NOSINK | LH_FLAG_NOSOURCE | LH_FLAG_READONLY);
    setup_favourite_layout_->setHelp("This is the layout the key sequence will load");

    setup_key_presses_ = new LH_Qt_int (this, "Key Presses", 2, 1, MAX_KEYS, LH_FLAG_NOSINK | LH_FLAG_NOSOURCE | LH_FLAG_READONLY);
    setup_key_presses_->setHelp("How many key presses the sequence contains.");

    setup_key_press_timout_ = new LH_Qt_QSlider(this, "Press Delay", 250, 50, 1000, LH_FLAG_NOSINK | LH_FLAG_NOSOURCE | LH_FLAG_READONLY);
    setup_key_press_timout_->setHelp("The maximum delay between key presses to trigger the sequence.");

    for(int i=1; i<=MAX_KEYS; i++)
    {
        setup_keys_.append(new LH_Qt_InputState(this,QString("Key #%1").arg(i),"",LH_FLAG_NOSINK | LH_FLAG_NOSOURCE | LH_FLAG_READONLY | (i<=setup_key_presses_->value()? 0 : LH_FLAG_HIDDEN)));
        connect(setup_keys_.at(i-1), SIGNAL(input(QString,int,int)), this, SLOT(keyPressed(QString,int,int)));
    }

    connect(setup_enable_favourite_shortcut_, SIGNAL(changed()), this, SLOT(enableFavouriteShortcut()));
    connect(setup_key_presses_, SIGNAL(changed()), this, SLOT(changeKeyPresses()));

    return 0;
}

void LH_QtPlugin_Cursor::enableFavouriteShortcut()
{
    bool enabled = setup_enable_favourite_shortcut_->value();
    setup_favourite_layout_->setReadonly(!enabled);
    setup_key_presses_->setReadonly(!enabled);
    setup_key_press_timout_->setReadonly(!enabled);
    for(int i=1; i<=MAX_KEYS; i++)
        setup_keys_.at(i-1)->setReadonly(!enabled);
}

void LH_QtPlugin_Cursor::changeKeyPresses()
{
    for(int i=1; i<=MAX_KEYS; i++)
        setup_keys_.at(i-1)->setVisible(i<=setup_key_presses_->value());
}


void LH_QtPlugin_Cursor::keyPressed(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    if( !setup_enable_favourite_shortcut_->value() )
        return;
    if(keyDelay.elapsed() < 25)
        return;
    bool ok;
    int keyID = ((LH_Qt_InputState*)QObject::sender())->objectName().remove("Key #").toInt(&ok);
    if(!ok)
        return;
    if(keyDelay.elapsed() > setup_key_press_timout_->value())
        favourite_combo_step_ = 0;
    if(favourite_combo_step_ == keyID-1)
    {
        //qDebug() << keyID << ":" << key;
        favourite_combo_step_ = keyID;
        if(favourite_combo_step_ == setup_key_presses_->value())
        {
            favourite_combo_step_ = 0;
            if(setup_favourite_layout_->value().isFile())
            {
                static QByteArray ary;
                ary = setup_favourite_layout_->value().absoluteFilePath().toUtf8();
                callback(lh_cb_load_layout, ary.data() );
            }
        }
        else
            keyDelay.restart();
    }
}
