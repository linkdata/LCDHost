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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <QDateTime>

#ifdef TAGLIB
#include "fileref.h"
#include "tag.h"
#include "Cover.h"
#endif

//#include <Psapi.h>
#include <string>
#include <map>
#include <process.h>
#include <windows.h>
#include <winuser.h>

#include <QString>
#include <QRegExp>
#include <QTime>

//#include "Cover.h"
//#include "Internet.h"
//#include "Lyrics.h"

struct ArtworkCache
{
    QString cachePath;
    QString cacheFile;
    QString album;
    QString artist;
};

enum PLAYSTATE
{
	PLAYER_STOPPED,
	PLAYER_PLAYING,
	PLAYER_PAUSED
};

enum MEASURETYPE
{
	MEASURE_NONE     = 0x00000000,
	MEASURE_ARTIST   = 0x00000001,
	MEASURE_TITLE    = 0x00000002,
	MEASURE_ALBUM    = 0x00000004,
	MEASURE_LYRICS   = 0x00000008,
	MEASURE_COVER    = 0x00000010,
	MEASURE_FILE     = 0x00000020,
	MEASURE_DURATION = 0x00000040,
	MEASURE_POSITION = 0x00000080,
	MEASURE_PROGRESS = 0x00000100,
	MEASURE_RATING   = 0x00000200,
	MEASURE_VOLUME   = 0x00000400,
	MEASURE_STATE    = 0x00000800,
	MEASURE_STATUS   = 0x00001000,
	MEASURE_SHUFFLE  = 0x00002000,
	MEASURE_REPEAT   = 0x00004000
};

class CPlayer
{
    void replace_token(QString &str, QString token, QString val);
    void replace_token(QString &str, QString token, uint seconds, uint totalSeconds);

public:
	CPlayer();
	virtual ~CPlayer() = 0;

	void AddInstance();
	void RemoveInstance();
	void UpdateMeasure();
	void AddMeasure(INT type);
	virtual void UpdateData() = 0;

	bool IsInitialized() { return m_Initialized; }
	UINT GetTrackCount() { return m_TrackCount; }

	void FindCover();
	void FindLyrics();

	virtual void Pause() {}
	virtual void Play() {}
	virtual void Stop() {}
	virtual void Next() {}
	virtual void Previous() {}
    virtual void SetPosition(int /*position*/) {}		// position: position in seconds
    virtual void SetRating(int /*rating*/) {}			// rating: 0 - 5
    virtual void SetVolume(int /*volume*/) {}			// volume: 0 - 100
    virtual void SetShuffle(bool /*state*/) {}			// state: off = 0, on = 1
    virtual void SetRepeat(bool /*state*/) {}			// state: off = 0, on = 1
    virtual void OpenPlayer(std::wstring& /*path*/) {}
	virtual void ClosePlayer() {}

    PLAYSTATE GetState()   { return m_State; }
    QString GetArtist()    { return QString::fromStdWString( m_Artist ); }
    QString GetAlbum()     { return QString::fromStdWString( m_Album ); }
    QString GetTitle()     { return QString::fromStdWString( m_Title ); }
    QString GetLyrics()    { return QString::fromStdWString( m_Lyrics ); }
    QString GetCoverPath() { return QString::fromStdWString( m_CoverPath ); }
    QString GetFilePath()  { return QString::fromStdWString( m_FilePath ); }
    virtual QString GetPlayer()    { return m_PlayerName; }
    UINT GetDuration() { return m_Duration; }
	UINT GetPosition() { return m_Position; }
	UINT GetRating() { return m_Rating; }
	UINT GetVolume() { return m_Volume; }
	bool GetShuffle() { return m_Shuffle; }
	bool GetRepeat() { return m_Repeat; }
    void SetPlayer(QString s) { m_PlayerName = s; }
    void setArtworkCachePath( ArtworkCache *artworkCache ) { m_artworkCache = artworkCache; }

    QString replace_tokens(QString str, bool hidePlayingState = false);

protected:
	void ClearData();
    bool CachedArtworkValid() { if(!m_artworkCache) return true; return m_artworkCache->album == GetAlbum() && m_artworkCache->artist == GetArtist(); }
    void UpdateCachedArtwork(QString NewFilePath) { if(!m_artworkCache) return; m_artworkCache->album = GetAlbum(); m_artworkCache->artist = GetArtist(); m_artworkCache->cacheFile = NewFilePath; }

	bool m_Initialized;
	UINT m_InstanceCount;
	UINT m_UpdateCount;
	UINT m_TrackCount;
	std::wstring m_TempCoverPath;

	INT m_Measures;
	PLAYSTATE m_State;
    std::wstring m_Artist;
    std::wstring m_Title;
	std::wstring m_Album;
	std::wstring m_Lyrics;
    std::wstring m_CoverPath;		// Path to cover art image
	std::wstring m_FilePath;		// Path to playing file
	UINT m_Duration;				// Track duration in seconds
	UINT m_Position;				// Current position in seconds
	UINT m_Rating;					// Track rating from 0 to 100
	UINT m_Volume;					// Volume from 0 to 100
	bool m_Shuffle;
	bool m_Repeat;
    ArtworkCache *m_artworkCache;
    QString m_PlayerName;

private:
	static unsigned __stdcall LyricsThreadProc(void* pParam);

	HANDLE m_InternetThread;
};

#endif
