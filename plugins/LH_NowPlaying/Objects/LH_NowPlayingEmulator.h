/**
  \file     LH_NowPlayingEmulator.h
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

#ifndef LH_NOWPLAYINGEMULATOR_H
#define LH_NOWPLAYINGEMULATOR_H

#include <QFont>
#include <QFontMetrics>
#include <QTime>
#include <QRegExp>

#include "LH_Text/LH_Text.h"

class LH_NowPlayingEmulator : public LH_Text
{
protected:
    LH_Qt_QString* setup_player_;
    LH_Qt_bool* setup_enabled_;
    LH_Qt_QString* setup_artist_;
    LH_Qt_QString* setup_track_;
    LH_Qt_QString* setup_album_;

    void sendMessage(bool enabled);

public:
    LH_NowPlayingEmulator();
    ~LH_NowPlayingEmulator();

    const char *userInit();

    int notify(int code, void *param);
    static lh_class *classInfo();
};

#endif // LH_NOWPLAYINGEMULATOR_H
