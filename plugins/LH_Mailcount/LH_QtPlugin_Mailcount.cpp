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

#include <QFile>
#include <QDebug>

#include "LH_QtPlugin_Mailcount.h"

#ifdef Q_OS_WIN
#include <windows.h>
/* Make sure unread mail function is declared */
typedef HRESULT (WINAPI *SHGetUnreadMailCountW_t)(
    HKEY hKeyUser,
    LPCTSTR pszMailAddress,
    DWORD *pdwCount,
    FILETIME *pFileTime,
    LPCTSTR pszShellExecuteCommand,
    int cchShellExecuteCommand
);
static HANDLE hShell32Dll = (HANDLE)0;
static SHGetUnreadMailCountW_t SHGetUnreadMailCountW_p = NULL;
#endif

LH_PLUGIN(LH_QtPlugin_Mailcount)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Mailcount</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Unread mail indicator."
  "</shortdesc>"
  "<longdesc>"
  "Show unread e-mails. Requires a running e-mail client with shell support, "
  "like <a href=\"http://www.mozillamessaging.com/\">Mozilla Thunderbird</a>."
  "</longdesc>"
"</lcdhostplugin>";

LH_QtPlugin_Mailcount::LH_QtPlugin_Mailcount()
{
    email_count_ = new LH_Qt_int(this,tr("Mail count"),0,LH_FLAG_READONLY|LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK|LH_FLAG_NOSINK);
    email_count_->setHelp("This is the number of waiting e-mails, as reported by the "
                          "operating system.");
    email_count_->setPublishPath("/system/Mail count");
    email_addr_ = new LH_Qt_QString(this,tr("Only check address"),QString());
    email_addr_->setHelp("If not empty, limits the search for waiting email to the "
                         "given e-mail address. <strong>Leave it empty</strong> for normal use.");
    email_days_ = new LH_Qt_int(this,tr("Days back to check"),7);
    email_days_->setHelp("How far back to look for unread e-mails.");
    check_interval_ = new LH_Qt_int(this,tr("Check interval (seconds)"),2);
    check_interval_->setHelp("How often to look for new email, in seconds.");
    manual_adjust_ = new LH_Qt_int(this,tr("Manual adjustment"),0);
    manual_adjust_->setHelp("If the number of mails reported by the system is "
                            "always wrong by a fixed amount, you may adjust for it here.");
    connect( manual_adjust_, SIGNAL(changed()), this, SLOT(getUnreadMailcount()) );
}

LH_QtPlugin_Mailcount::~LH_QtPlugin_Mailcount()
{
}

const char *LH_QtPlugin_Mailcount::userInit()
{
#ifdef Q_OS_WIN
    hShell32Dll = LoadLibraryW( L"SHELL32.DLL" );
    if( hShell32Dll != NULL )
        SHGetUnreadMailCountW_p = (SHGetUnreadMailCountW_t) GetProcAddress( (HMODULE) hShell32Dll, "SHGetUnreadMailCountW" );
#endif
    return NULL;
}

int LH_QtPlugin_Mailcount::notify( int code, void *param )
{
    if( !code || (code & LH_NOTE_SECOND) )
    {
        int delta = last_check_.secsTo( QTime::currentTime() );
        if( !last_check_.isValid() ||
            delta < 0 ||
            delta > check_interval_->value() )
        {
            last_check_ = QTime::currentTime();
            getUnreadMailcount();
        }
    }

    return LH_QtPlugin::notify( code, param ) | LH_NOTE_SECOND;
}

void LH_QtPlugin_Mailcount::userTerm()
{
#ifdef Q_OS_WIN
    if( hShell32Dll )
    {
            FreeLibrary( (HMODULE) hShell32Dll );
            hShell32Dll = NULL;
            SHGetUnreadMailCountW_p = 0;
    }
#endif
    return;
}

void LH_QtPlugin_Mailcount::getUnreadMailcount()
{
    int total = manual_adjust_->value();

#ifdef Q_OS_WIN
    if( SHGetUnreadMailCountW_p )
    {
        HRESULT retv;
        SYSTEMTIME st;
        FILETIME ft;
        ULARGE_INTEGER mailtime;
        DWORD dwMail = 0;

        // Get unread mail count
        GetLocalTime( &st );
        SystemTimeToFileTime( &st, &ft );
        Q_ASSERT( sizeof(mailtime) == sizeof(ft) );
        memcpy( &mailtime, &ft, sizeof(mailtime) );
        mailtime.QuadPart -= (ULONGLONG)email_days_->value()*24*60*60*10000000; // subtract wanted number of days
        memcpy( &ft, &mailtime, sizeof(mailtime) );
        if( !email_addr_->value().isEmpty() )
        {
            retv = SHGetUnreadMailCountW_p( NULL, (LPCTSTR)(void*)email_addr_->value().utf16(), &dwMail, &ft, NULL, 0 );
        }
        else
            retv = SHGetUnreadMailCountW_p( NULL, NULL, &dwMail, &ft, NULL, 0 );
        if( retv != S_OK ) dwMail = 0;

        total += dwMail;
    }
#endif

    email_count_->setValue(total);
    return;
}
