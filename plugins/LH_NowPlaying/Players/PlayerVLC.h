/*
  Copyright (C) 2011 Birunthan Mohanathas (www.poiru.net)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __PLAYERVLC_H__
#define __PLAYERVLC_H__

#include "Player.h"
#include <QXmlStreamReader>

class CPlayerVLC : public CPlayer
{
public:
    virtual ~CPlayerVLC();

    static CPlayer* Create(std::wstring port);
    static HWND FindVLCWindow();
    static bool TestUrl(std::wstring port);

	virtual void UpdateData();

    virtual void Pause();
	virtual void Play();
    virtual void Stop();
	virtual void Next();
	virtual void Previous();
    /*virtual void SetPosition(int position);
	virtual void SetRating(int rating);
    virtual void SetVolume(int volume);*/
	virtual void SetShuffle(bool state);
	virtual void SetRepeat(bool state);
	virtual void ClosePlayer();
    //virtual void OpenPlayer(std::wstring& path);

protected:
    CPlayerVLC(std::wstring port);

private:
    static CPlayer* c_Player;
    static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
    static HWND m_VLCHandle;

	DWORD m_LastCheckTime;
    std::wstring m_Port;
    QXmlStreamReader xml_;

};

#endif
