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

#include "PlayerVLC.h"
#include "internet.h"
#include <QDebug>
#include <QString>
#include <QUrl>
#include <QStringList>

CPlayer* CPlayerVLC::c_Player = NULL;
HWND CPlayerVLC::m_VLCHandle = NULL;

// This player retrieves data through the VLC Web interface.

BOOL CALLBACK CPlayerVLC::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    Q_UNUSED(lParam);

    if (!hWnd)
        return TRUE;		// Not a window

    /*qDebug() << hWnd;
    char String[255];

    if (!SendMessage(hWnd, WM_GETTEXT, sizeof(String), (LPARAM)String))
        return TRUE;		// No window title

    qDebug() << String;*/

    WCHAR winClass[255];
    GetClassName(hWnd,winClass,255);

    WCHAR winTitle[511];
    GetWindowText(hWnd,winTitle,511);

    if (QString::fromWCharArray(winTitle).endsWith("VLC media player"))
    {
        m_VLCHandle = hWnd;
        qDebug() << QString::fromWCharArray(winClass) << " // " << QString::fromWCharArray(winTitle);
    }

    return TRUE;
}

HWND CPlayerVLC::FindVLCWindow()
{
    m_VLCHandle = NULL;
    qDebug() << "Finding VLC...";
    EnumWindows(EnumWindowsProc, 0);
    return m_VLCHandle;
}

bool CPlayerVLC::TestUrl(std::wstring port)
{
    return CInternet::TestUrl(L"http://127.0.0.1:" + port + L"/requests/status.xml");
}
/*
** CPlayerVLC
**
** Constructor.
**
*/
CPlayerVLC::CPlayerVLC(std::wstring port) : CPlayer(),
	m_LastCheckTime(0),
    m_Port(port),
    xml_()
{
}

/*
** ~CPlayerVLC
**
** Destructor.
**
*/
CPlayerVLC::~CPlayerVLC()
{
	c_Player = NULL;
}

/*
** Create
**
** Creates a shared class object.
**
*/
CPlayer* CPlayerVLC::Create(std::wstring port)
{
	if (!c_Player)
	{
        c_Player = new CPlayerVLC(port);
	}

	return c_Player;
}

/*
** UpdateData
**
** Called during each update of the main measure.
**
*/
void CPlayerVLC::UpdateData()
{
    xml_.clear();
    xml_.addData(QString::fromStdWString(CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml", CP_UTF8)));

    QStringList currentPosition;
    while (!xml_.atEnd())
    {
        xml_.readNext();
        if (xml_.isStartElement())
        {
            currentPosition.push_back(xml_.name().toString());
        }
        else if (xml_.isEndElement())
        {
            currentPosition.pop_back();
        }
        else if (xml_.isCharacters() && !xml_.isWhitespace())
        {
            QString nodeAddress = currentPosition.join(".");
            QString nodeValue = xml_.text().toString();
            if(nodeAddress == "root.information.meta-information.artist")
                m_Artist = nodeValue.toStdWString();
            if(nodeAddress == "root.information.meta-information.title")
                m_Title = nodeValue.toStdWString();
            if(nodeAddress == "root.information.meta-information.album")
                m_Album = nodeValue.toStdWString();
            if(nodeAddress == "root.length")
                m_Duration = nodeValue.toInt();
            if(nodeAddress == "root.time")
                m_Position = nodeValue.toInt();
            if(nodeAddress == "root.volume")
                m_Volume = nodeValue.toInt();
            if(nodeAddress == "root.state")
            {
                if(nodeValue == "paused") m_State = PLAYER_PAUSED;
                if(nodeValue == "playing") m_State = PLAYER_PLAYING;
                if(nodeValue == "stop") m_State = PLAYER_STOPPED;
            }
            if(nodeAddress == "root.random")
                m_Shuffle = (nodeValue.toInt()==1);
            if(nodeAddress == "root.repeat")
                m_Repeat = (nodeValue.toInt()==1);
        }
    }
    if (xml_.error() && xml_.error() != QXmlStreamReader::PrematureEndOfDocumentError)
    {
        qWarning() << "XML ERROR:" << xml_.lineNumber() << ": " << xml_.errorString();
    }

    m_CoverPath.clear();
    m_FilePath.clear();
    if((m_Measures & MEASURE_COVER) && m_State != PLAYER_STOPPED)
    {
        xml_.clear();
        xml_.addData(QString::fromStdWString(CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/playlist.xml", CP_UTF8)));
        while (!xml_.atEnd())
        {
            xml_.readNext();
            if (xml_.isStartElement())
            {
                QString nodeName = xml_.name().toString();
                if(currentPosition.count()!=0 && xml_.attributes().value("name")!="" && nodeName=="node")
                    nodeName = QString("%1:%2").arg(nodeName).arg(xml_.attributes().value("name").toString());
                if(currentPosition.count()!=0 && xml_.attributes().value("current")!="" && nodeName=="leaf")
                    nodeName = QString("%1:%2").arg(nodeName).arg(xml_.attributes().value("current").toString());
                currentPosition.push_back(nodeName);

                if(currentPosition.join(".") == "node.node:Playlist.leaf:current")
                    m_FilePath = QUrl::fromEncoded(xml_.attributes().value("uri").toString().toUtf8()).toString().remove("file:///").toStdWString();
            }
            else if (xml_.isEndElement())
                currentPosition.pop_back();
            else if (xml_.isCharacters() && !xml_.isWhitespace())
            {
                QString nodeAddress = currentPosition.join(".");
                QString nodeValue = xml_.text().toString();
                if(nodeAddress == "node.node:Playlist.leaf:current.art_url")
                    m_CoverPath = QUrl::fromEncoded(nodeValue.toUtf8()).toString().remove("file:///").toStdWString();
            }
        }
        if (xml_.error() && xml_.error() != QXmlStreamReader::PrematureEndOfDocumentError)
        {
            qWarning() << "XML ERROR:" << xml_.lineNumber() << ": " << xml_.errorString();
        }

    }

    if (m_Measures & MEASURE_LYRICS)
    {
        FindLyrics();
    }
}

/*
** Pause
**
** Handles the Pause bang.
**
*/
void CPlayerVLC::Pause()
{
    CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_pause", CP_UTF8);
}

/*
** Play
**
** Handles the Play bang.
**
*/
void CPlayerVLC::Play()
{
    if(m_State == PLAYER_PAUSED)
        CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_pause", CP_UTF8);
    else
        CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_play", CP_UTF8);
}

/*
** Stop
**
** Handles the Stop bang.
**
*/
void CPlayerVLC::Stop()
{
    CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_stop", CP_UTF8);
}

/*
** Next
**
** Handles the Next bang.
**
*/
void CPlayerVLC::Next()
{
    CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_next", CP_UTF8);
}

/*
** Previous
**
** Handles the Previous bang.
**
*/
void CPlayerVLC::Previous()
{
    CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_previous", CP_UTF8);
}

#ifdef DISABLED_CONTROLS
/*
** SetPosition
**
** Handles the SetPosition bang.
**
*/
void CPlayerVLC::SetPosition(int position)
{
    //position *= 1000; // To milliseconds
    //SendMessage(m_Window, WM_WA_IPC, position, IPC_JUMPTOTIME);
}

/*
** SetRating
**
** Handles the SetRating bang.
**
*/
void CPlayerVLC::SetRating(int rating)
{
    /*if (rating < 0)
	{
		rating = 0;
	}
	else if (rating > 5)
	{
		rating = 5;
	}

	SendMessage(m_Window, WM_WA_IPC, rating, IPC_SETRATING);
    m_Rating = rating;*/
}

/*
** SetVolume
**
** Handles the SetVolume bang.
**
*/
void CPlayerVLC::SetVolume(int volume)
{
    /*if (volume > 0 && volume < 100) ++volume;	// For proper scaling

	// Winamp accepts volume in 0 - 255 range
	volume *= 255;
	volume /= 100;
    SendMessage(m_Window, WM_WA_IPC, volume, IPC_SETVOLUME);*/
}
#endif
/*
** SetShuffle
**
** Handles the SetShuffle bang.
**
*/
void CPlayerVLC::SetShuffle(bool state)
{
    Q_UNUSED(state);
    CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_random", CP_UTF8);
    /*if (!m_PlayingStream)
	{
		m_Shuffle = state;
		SendMessage(m_Window, WM_WA_IPC, (WPARAM)m_Shuffle, IPC_SET_SHUFFLE);
	}
    */
}

/*
** SetRepeat
**
** Handles the SetRepeat bang.
**
*/
void CPlayerVLC::SetRepeat(bool state)
{
    Q_UNUSED(state);
    CInternet::DownloadUrl(L"http://127.0.0.1:"+m_Port+L"/requests/status.xml?command=pl_repeat", CP_UTF8);
    /*if (!m_PlayingStream)
	{
		m_Repeat = state;
		SendMessage(m_Window, WM_WA_IPC, (WPARAM)m_Repeat, IPC_SET_REPEAT);
	}
    */
}

/*
** ClosePlayer
**
** Handles the ClosePlayer bang.
**
*/
void CPlayerVLC::ClosePlayer()
{
    HWND wnd = CPlayerVLC::FindVLCWindow();
    if (wnd)
    {
        SendMessage(wnd, WM_CLOSE, 0, 0);
    }
}
#ifdef DISABLED_CONTROLS
/*
** OpenPlayer
**
** Handles the OpenPlayer bang.
**
*/
void CPlayerVLC::OpenPlayer(std::wstring& path)
{
    /*if (m_WinampType == WA_WINAMP)
	{
		ShellExecute(NULL, L"open", path.empty() ? L"winamp.exe" : path.c_str(), NULL, NULL, SW_SHOW);
	}
	else // if (m_WinampType == WA_MEDIAMONKEY)
	{
		if (path.empty())
		{
			// Gotta figure out where Winamp is located at
			HKEY hKey;
			RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						 L"SOFTWARE\\Clients\\Media\\MediaMonkey\\shell\\open\\command",
						 0,
						 KEY_QUERY_VALUE,
						 &hKey);

			DWORD size = 512;
			WCHAR* data = new WCHAR[size];
			DWORD type = 0;

			if (RegQueryValueEx(hKey,
								NULL,
								NULL,
								(LPDWORD)&type,
								(LPBYTE)data,
								(LPDWORD)&size) == ERROR_SUCCESS)
			{
				if (type == REG_SZ)
				{
					ShellExecute(NULL, L"open", data, NULL, NULL, SW_SHOW);
					path = data;
				}
			}

			delete [] data;
			RegCloseKey(hKey);
		}
		else
		{
			ShellExecute(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOW);
		}
	}
*/
}
#endif
