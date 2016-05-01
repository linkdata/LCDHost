/**
  \file     LH_QtPlugin_Weather.h
  @author   Andy "Triscopic" Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andrew Bridges

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

#ifndef LH_QTPLUGIN_DATAVIEWER_H
#define LH_QTPLUGIN_DATAVIEWER_H

#include "LH_QtPlugin.h"

class LH_QtPlugin_DataViewer : public LH_QtPlugin
{
public:
    const char * lh_name() { return "DataViewer"; }
    const char * lh_shortdesc() { return "Extracts data from a source file and reformats it for display."; }
    const char * lh_author() { return "Andy \"Triscopic\" Bridges"; }
    const char * lh_homepage() { return "<a href=\"http://www.codeleap.co.uk\">CodeLeap</a>"; }
    const char * lh_longdesc()
    {
        return "This plugin provides classes that acquire and display information from a source file.\n\n"
               "It is primarily designed to be used with games that provide the ability via an internal scripting "
               "language to export text. This plugin can then be used to read in that data and display it on screen "
               "in various formats to create a layout that displays useful in-game stats.";
    }
};

#endif // LH_QTPLUGIN_DATAVIEWER_H
