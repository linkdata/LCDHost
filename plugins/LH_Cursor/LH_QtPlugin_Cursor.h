/**
  \file     LH_QtPlugin_Cursor.h
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

#ifndef LH_QTPLUGIN_CURSOR_H
#define LH_QTPLUGIN_CURSOR_H

#include "LH_QtPlugin.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_int.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_QFileInfo.h"

#include <QList>
#include <QTime>

#define MAX_KEYS 4

class LH_QtPlugin_Cursor: public LH_QtPlugin
{
    Q_OBJECT
    int favourite_combo_step_;
    QTime keyDelay;

protected:
    LH_Qt_bool *setup_enable_favourite_shortcut_;
    LH_Qt_int *setup_key_presses_;
    LH_Qt_QSlider *setup_key_press_timout_;
    QList<LH_Qt_InputState*> setup_keys_;
    LH_Qt_QFileInfo *setup_favourite_layout_;

public:
    //LH_QtPlugin_Cursor();
    const char *userInit();

public slots:
    void enableFavouriteShortcut();
    void changeKeyPresses();
    void keyPressed(QString key,int flags,int value);
};

#endif // LH_QTPLUGIN_CURSOR_H

