/**
  \file     LH_QtPlugin_LCoreReboot.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#include "LH_QtPlugin_LCoreReboot.h"

#include <QDebug>
#include <QDir>
// #include <QtCore/QLibrary>
// #include <QFileInfo>

LH_PLUGIN(LH_QtPlugin_LCoreReboot)

char __lcdhostplugin_xml[] =
        "<?xml version=\"1.0\"?>"
        "<lcdhostplugin>"
        "<id>LCoreReboot</id>"
        "<rev>" STRINGIZE(REVISION) "</rev>"
        "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
        "<ver>" STRINGIZE(VERSION) "\nr" STRINGIZE(REVISION) "</ver>"
        "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
        "<author>Andy \"Triscopic\" Bridges</author>"
        "<homepageurl><a href=\"http://www.codeleap.co.uk\">CodeLeap</a></homepageurl>"
        "<logourl></logourl>"
        "<shortdesc>"
        "Reboots the Logitech Gaming Software"
        "</shortdesc>"
        "<longdesc>"
        "<p>This plugin provides a facility to restart the Logitech Gaming Software (LGS). "
        "It simply locates the running LGS process (LCore.exe), kills it and then relaunches the Logitech Gaming Software (sending it directly to the system tray, rather than allowing the main LGS window to open).</p>"
        "<p>The purpose of this is to help the user quickly recover from a crashed LGS instance (i.e. when the lcd display appears frozen).</p>"
        "</longdesc>"
        "</lcdhostplugin>";

#ifdef Q_OS_WIN

static bool TranslateDeviceNameToDriveLetter(const QString& exename, QString* out_path) {

    // Translate path with device name to drive letters.
    wchar_t szTemp[MAX_PATH];
    szTemp[0] = L'\0';

    if (GetLogicalDriveStringsW(MAX_PATH - 1, szTemp))
    {
        wchar_t szName[MAX_PATH];
        wchar_t szDrive[3];
        szDrive[0] = L' ';
        szDrive[1] = L':';
        szDrive[2] = L'\0';
        wchar_t* p = szTemp;

        qDebug() << "Exename" << exename;

        do {
            // Copy the drive letter to the template string
            *szDrive = *p;

            // Look up each device name
            if (QueryDosDeviceW(szDrive, szName, MAX_PATH))
            {
                size_t uNameLen = wcslen(szName);
                qDebug() << "DOS Drive" << QString::fromWCharArray(szDrive, 2) << uNameLen << QString::fromWCharArray(szName, uNameLen);
                if (uNameLen && uNameLen < MAX_PATH && (size_t)exename.size() >= uNameLen && exename.at(uNameLen) == QChar('\\')) {
                    QString drivename(QString::fromWCharArray(szName, uNameLen));
                    qDebug() << exename << drivename;
                    if (exename.startsWith(drivename)) {
                        if (out_path) {
                            out_path->append(QChar(szDrive[0]));
                            out_path->append(QChar(szDrive[1]));
                            out_path->append(exename.midRef(uNameLen));
                        }
                        return true;
                    }
                }
            }
            // Go to the next NULL character.
            while (*p++)
                ;
        } while (*p);
    }
    return false;
}

static DWORD GetLCoreProcessId(QString* p_path = 0) {
    DWORD idProcess[4096];
    DWORD cbNeeded = 0;
    if (!EnumProcesses(idProcess, sizeof(idProcess), &cbNeeded))
        return 0;
    if (cbNeeded > sizeof(idProcess))
        return 0;
    DWORD numProcesses = cbNeeded / sizeof(DWORD);
    for (DWORD i = 0; i < numProcesses; ++i) {
        if (HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, idProcess[i])) {
            wchar_t szExeName[MAX_PATH];
            if (DWORD dwSize = GetProcessImageFileNameW(hProcess, szExeName, MAX_PATH)) {
                QString exename(QString::fromWCharArray(szExeName, dwSize));
                if (exename.endsWith(QLatin1String("LCore.exe"), Qt::CaseInsensitive)) {
                    TranslateDeviceNameToDriveLetter(exename, p_path);
                    CloseHandle(hProcess);
                    return idProcess[i];
                }
            }
            CloseHandle(hProcess);
        }
    }
    return 0;
}
#endif

const char *LH_QtPlugin_LCoreReboot::userInit()
{
    if( const char *err = LH_QtPlugin::userInit() ) return err;

#ifndef Q_OS_WIN
    return "Not supported on this OS";
#else
    QString filePath;
    if (!GetLCoreProcessId(&filePath))
        return "Logitech drivers are not loaded";
    if(filePath.isEmpty())
        return "Cannot read LCore.exe image path";

#if 0
    QDir lg_dir(filePath);
    lg_dir.cdUp();
    if (lg_dir.exists(QLatin1String("RestartLCore.exe"))) {
        restartlcore_path_ = lg_dir.filePath(QLatin1String("RestartLCore.exe"));
        qDebug() << "restartlcore_path_" << restartlcore_path_;
    }
    if(restartlcore_path_.isEmpty() && !enableDebugPrivileges())
        return "LCDHost not running as admin";
#endif

    setup_reboot_ = new LH_Qt_QString(
                this, "Reboot",
                "Reboot Logitech Gaming Software",
                LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE | LH_FLAG_HIDETITLE, lh_type_string_button);
    connect(setup_reboot_, SIGNAL(changed()), this, SLOT(rebootLCore()));

    setup_lcore_path_ = new LH_Qt_QString(
                this, "LGS Path",
                filePath,
                LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE | LH_FLAG_HIDDEN);
#endif
    return 0;
}

void LH_QtPlugin_LCoreReboot::rebootLCore()
{
    if (restartlcore_path_.isEmpty()) {
        if(killLCore())
            launchLCore();
    } else {
#ifdef Q_OS_WIN
        QString param;
        HINSTANCE hInst = ShellExecuteW(
                    NULL,
                    NULL,
                    reinterpret_cast<const WCHAR*>(restartlcore_path_.utf16()),
                    reinterpret_cast<const WCHAR*>(param.utf16()),
                    NULL,
                    SW_SHOWMINNOACTIVE
                    );
        qDebug() << "LCoreReboot:" << restartlcore_path_ << "code" << (int)hInst;
#endif
    }
}

void LH_QtPlugin_LCoreReboot::launchLCore()
{
#ifdef Q_OS_WIN
    QString param; //(QLatin1String("/minimized"));
    QString filename = setup_lcore_path_->value();
    HINSTANCE hInst = ShellExecuteW(
                NULL,
                NULL,
                reinterpret_cast<const WCHAR*>(filename.utf16()),
                reinterpret_cast<const WCHAR*>(param.utf16()),
                NULL,
                SW_SHOWMINNOACTIVE
                );
    qDebug("LCoreReboot: LCore restart %s code %ld\n", ((int)hInst > 32) ? "succeeded" : "failed", (long int)hInst);
#endif
}

bool LH_QtPlugin_LCoreReboot::killLCore()
{
#ifdef Q_OS_WIN
    QString path;
    if (DWORD dwProcessId = GetLCoreProcessId(&path)) {
        if (!path.isEmpty()) {
            if (HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, false, dwProcessId)) {
                UINT uExitCode = 0;
                bool terminated = TerminateProcess(hProcess, uExitCode);
                if(terminated) {
                    switch (WaitForSingleObject(hProcess, 1000)) {
                    case WAIT_ABANDONED:
                        qDebug("LCoreReboot: WAIT_ABANDONED\n");
                        break;
                    case WAIT_OBJECT_0:
                        break;
                    case WAIT_TIMEOUT:
                        qDebug("LCoreReboot: WAIT_TIMEOUT\n");
                        terminated = false;
                        break;
                    case WAIT_FAILED:
                        qDebug("LCoreReboot: WAIT_FAILED\n");
                        break;
                    default:
                        qDebug("LCoreReboot: WaitForSingleObject error\n");
                        terminated = false;
                        break;
                    }
                } else {
                    qDebug("LCoreReboot: Unable to kill LCore\n");
                }
                CloseHandle(hProcess);
                return terminated;
            }
            qDebug() << "LCoreReboot: Could not open process:" << getLastErrorMessage();
        } else {
            qDebug("LCoreReboot: LCore path is empty");
        }
    }
#endif
    return false;
}

QString LH_QtPlugin_LCoreReboot::getLastErrorMessage()
{
#ifdef Q_OS_WIN
    DWORD dwError;
    wchar_t errBuf[256];

    dwError = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)errBuf, sizeof(errBuf),NULL);

    return QString("Err Code: %1 - %2").arg(dwError).arg(QString::fromWCharArray(errBuf));
#endif
}

#if 0
bool LH_QtPlugin_LCoreReboot::enableDebugPrivileges()
{
#ifdef Q_OS_WIN
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if ( !LookupPrivilegeValue(NULL, L"SeDebugPrivilege", &luid ) )
    {
        qDebug() << "LookupPrivilegeValue error: " <<  getLastErrorMessage();
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Enable the privilege or disable all privileges.
    HANDLE currProc = GetCurrentProcess();
    HANDLE procToken;
    if(!OpenProcessToken(currProc,TOKEN_ADJUST_PRIVILEGES,&procToken))
    {
        qDebug() << "OpenProcessToken failed with error: " << getLastErrorMessage();
        return false;
    }
    if(GetLastError())
    {
        qDebug() << "OpenProcessToken succeeded with error: " << getLastErrorMessage();
        return false;
    }

    if ( !AdjustTokenPrivileges(procToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL) )
    {
        qDebug() << "AdjustTokenPrivileges failed with error: " << getLastErrorMessage();
        return false;
    }
    if(GetLastError())
    {
        qDebug() << "AdjustTokenPrivileges succeeded with error: " << getLastErrorMessage();
        return false;
    }

    CloseHandle(procToken);
    CloseHandle(currProc);

    return true;
#endif
}
#endif
