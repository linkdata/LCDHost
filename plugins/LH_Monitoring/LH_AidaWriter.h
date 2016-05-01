/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

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

#ifndef LH_AIDAWRITER_H
#define LH_AODAWRITER_H

#include "LH_Text/LH_Text.h"
#include "LH_Qt_QTextEdit.h"

#ifdef Q_OS_WIN
# include <windows.h>
#else
# ifndef DWORD
#  define DWORD quint32
# endif
# ifndef HANDLE
#  define HANDLE void*
# endif
#endif

class LH_AidaWriter : public LH_QtInstance
{
    Q_OBJECT

    HANDLE filemap_aida;
    char* aidaData;
    DWORD sz;
protected:
    LH_Qt_QTextEdit* setup_xml_;

public:
    LH_AidaWriter();
    ~LH_AidaWriter();
    const char *userInit();

    static lh_class *classInfo();

    int notify(int code,void* param);

    bool updateAidaMemory();
    bool createAidaMemory();
    void destroyAidaMemory();
};

#endif // LH_AIDAWRITER_H
