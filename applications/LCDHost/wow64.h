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

#ifndef WOW64_H
#define WOW64_H

#ifndef KEY_WOW64_64KEY
# define KEY_WOW64_64KEY		(0x0100)
#endif
#ifndef KEY_WOW64_32KEY
# define KEY_WOW64_32KEY		(0x0200)
#endif

int isWoW64();
#endif // WOW64_H
