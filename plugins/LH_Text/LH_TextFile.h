/**
  \file     LH_TextFile.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

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

#ifndef LH_TEXTFILE_H
#define LH_TEXTFILE_H

#include <QDateTime>

#include "LH_Text.h"
#include "LH_Qt_QFileInfo.h"

class LH_TextFile : public LH_Text
{
    Q_OBJECT
    QDateTime last_read_;

protected:
    LH_Qt_QFileInfo *setup_file_;

public:
    LH_TextFile();
    const char* userInit();

    int notify( int, void* );
    void checkFile();

    static lh_class *classInfo();

public slots:
    void fileChanged();
};

#endif // LH_TEXTFILE_H
