/**
  \file     LH_QtPlugin_Graph.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh.

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

#include "LH_QtPlugin_Graph.h"

LH_PLUGIN(LH_QtPlugin_Graph)

DataLineCollection* cpu_histogram_;
DataLineCollection* cpu_average_;
DataLineCollection* mem_physical_;
DataLineCollection* mem_virtual_;
DataLineCollection* net_in_;
DataLineCollection* net_out_;

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Graph</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" STRINGIZE(VERSION) "\nr" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Andy \"Triscopic\" Bridges</author>"
  "<homepageurl><a href=\"http://www.codeleap.co.uk\">CodeLeap</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Plots graphs of data items over time"
  "</shortdesc>"
  "<longdesc>"
    "<p>Plots graphs of data items such as CPU, memory and network load. "
    "Graphs can span 5 seconds to 2 hours by taking up to 600 samples at intervals of up to 12 seconds.</p>"
"</longdesc>"
"</lcdhostplugin>";

#define GRAPH_POINT_CACHE 2048

const char *LH_QtPlugin_Graph::userInit() {
    cpu_histogram_ = new DataLineCollection(GRAPH_POINT_CACHE);
    cpu_average_ = new DataLineCollection(GRAPH_POINT_CACHE);
    mem_physical_ = new DataLineCollection(GRAPH_POINT_CACHE);
    mem_virtual_ = new DataLineCollection(GRAPH_POINT_CACHE);
    net_in_ = new DataLineCollection(GRAPH_POINT_CACHE);
    net_out_ = new DataLineCollection(GRAPH_POINT_CACHE);

    cpu_.smoothingHidden(true);
    net_.smoothingHidden(true);
    cpu_timer_.start();
    mem_timer_.start();
    net_timer_.start();
    return 0;
}

void LH_QtPlugin_Graph::userTerm()
{
    delete cpu_histogram_;
    delete cpu_average_;
    delete mem_physical_;
    delete mem_virtual_;
    delete net_in_;
    delete net_out_;
}
