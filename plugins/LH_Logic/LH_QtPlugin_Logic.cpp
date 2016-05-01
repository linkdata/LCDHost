/**
  \file     LH_QtPlugin_Logic.cpp
  @author   Johan Lindh <johan@linkdata.se>
  @author   Andy Bridges <andy@bridgesuk.com>
  Copyright (c) 2010 Johan Lindh, Andy Bridges

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  **/

#include "LH_QtPlugin_Logic.h"

LH_PLUGIN(LH_QtPlugin_Logic)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Logic</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" STRINGIZE(VERSION) "\nr" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Andy \"Triscopic\" Bridges</author>"
  "<homepageurl><a href=\"http://www.codeleap.co.uk\">CodeLeap</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Advanced Logic Objects."
  "</shortdesc>"
  "<longdesc>"
    "<p><b>The Logic Box</b></p>"
    "<p>The Logic Box object is not designed to render any data in itself. Instead it allows you to read two different values and, via rules, combine them into an output, allowing layout designers to perform more complex activities.</p>"
    "<p>How to use it: "
    "<ol>"
        "<li>Firstly, use data linking to link the inputs to fields on other objects, e.g. Team Speak connection status and the Now Playing music player name.<br /></li>"
        "<li>With that done, write a set of rules which set the value of the \"Output\" depending on the values of those inputs.<br /></li>"
        "<li>Finally link the output value to a field on another object or objects, e.g. the Text field of a StaticText object. <br /><br />"
            "You could now choose to write more rules to affect this object, or just display this new value.</li> "
    "</ol></p>"
    "<p>N.B.: If you need more than two inputs, you can add multiple Logic Boxes, linking the output of one to the input of the next to combine as many pieces of data as needed.</p>"
    "<p>If none of this makes sense to you, you probably don't need to use this object. If you  still think you do, check out the Eos layout (installed with LCDHost) for an example of how it can work. Eos uses Logic Boxes to display either the Team Speak panel or the music player panel (and hide the 5-day weather and large clock) when the Team Speak 3 client or a recognised music player is run.</p>"
  "</longdesc>"
"</lcdhostplugin>";

