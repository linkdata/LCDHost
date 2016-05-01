/**
  \file     LH_TextNumber.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2010 Johan Lindh

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


  */

#ifndef LH_TEXTNUMBER_H
#define LH_TEXTNUMBER_H

#include "LH_Text.h"

class LH_TextNumber : public LH_Text
{
    Q_OBJECT
    qreal value_;
    qreal max_;

    bool makeText();
    bool bytes_;

protected:
    LH_Qt_bool *setup_showsuffix_;
    LH_Qt_bool *setup_showunits_;
    LH_Qt_bool *setup_bits_;
    LH_Qt_QStringList *setup_scale_;

public:
    LH_TextNumber(bool bytes = false, LH_QtObject* parent = 0);

    const char *userInit();
    void prerender() { makeText(); }

    bool showUnits() const { return setup_showunits_->value() && (setup_scale_->value() != 1); }

    bool setMax( qreal m ); // return true if text changed and we need rendering
    qreal max() const { return max_; }

    bool setValue( qreal v ); // return true if text changed and we need rendering
    qreal value() const { return value_; }

    bool isBytes() const { return bytes_; }
    void setBytes( bool b ) { bytes_ = b; }
};

#endif // LH_TEXTNUMBER_H
