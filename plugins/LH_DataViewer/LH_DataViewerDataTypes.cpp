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

#include "LH_DataViewerDataTypes.h"

#define MAX_STRING 256

QHash<QString, uint> modules_;

QString dataNode::getProcessValue() {
    int val_int;
    qreal val_flt;
    double val_dbl;
    qlonglong val_lng;
    QString val_string;

    bool ok;
    switch(definition_.memoryDataType)
    {
    case MEMTYPE_4BYTE:
        ok = getProcessValue(memoryAddress(), definition_.offsets, &val_int, 4);
        if(ok)
            return QString::number(val_int);
        break;
    case MEMTYPE_8BYTE:
        ok = getProcessValue(memoryAddress(), definition_.offsets, &val_lng, 8);
        if(ok)
            return QString::number(val_lng);
        break;
    case MEMTYPE_FLOAT:
        ok = getProcessValue(memoryAddress(), definition_.offsets, &val_flt, 4);
        if(ok)
            return QString::number(val_flt);
        break;
    case MEMTYPE_DOUBLE:
        ok = getProcessValue(memoryAddress(), definition_.offsets, &val_dbl, 8);
        if(ok)
            return QString::number(val_dbl);
        break;
    case MEMTYPE_TEXT_UNICODE:
        ok = getProcessValue(memoryAddress(), definition_.offsets, &val_string);
        if(ok)
            return val_string;
        break;
    case MEMTYPE_TEXT_ASCII:
        ok = getProcessValue(memoryAddress(), definition_.offsets, &val_string, false);
        if(ok)
            return val_string;
        break;
    case MEMTYPE_NONE:
        return "??";
    }
    return "??";
}

bool dataNode::getProcessValue(uint address, QList<uint> offsets, void *dest, size_t len)
{
    SIZE_T r = 0;
    if(offsets.count()>0)
    {
        //int offsetCount = sizeof(offsets)/sizeof(uint);
        for(int i = 0; i<offsets.count(); i++)
        {
            if(!getProcessValue(address, QList<uint>(), &address, 4))
               return false;
            address += offsets.at(i);
        }
    }
    if( len == 0 ) len = sizeof(dest);
#ifdef Q_OS_WIN
    ReadProcessMemory( processHandle(), (BYTE *) address, dest, len, &r );
#endif
    return (r == len);
}

bool dataNode::getProcessValue(uint address, QList<uint> offsets, QString *dest, bool unicode)
{
    if( HANDLE hProcess = processHandle() )
    {
        if(offsets.count()>0)
        {
            for(int i = 0; i<offsets.count(); i++)
            {
                if(!getProcessValue(address, QList<uint>(), &address, 4))
                   return false;
                address += offsets.at(i);
            }
        }

        QByteArray srcdata;
        while( srcdata.size() < MAX_STRING )
        {
            char ch = '\0';
#ifdef Q_OS_WIN
            if( ! ReadProcessMemory(hProcess, (BYTE *) address ++, &ch, 1, 0) )
                return false;
#else
            Q_UNUSED( hProcess );
#endif
            if( ch == '\0' ) break;
            srcdata.append( ch );
        }
        if( dest )
        {
            if( unicode ) dest->append( QString::fromUtf8( srcdata ) );
            else dest->append( QString::fromLocal8Bit( srcdata ) );
        }
    }
    return false;
}

bool dataNode::validatePID( DWORD pid, QString exeFile )
{
#ifdef Q_OS_WIN
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( processesSnapshot == INVALID_HANDLE_VALUE )
        return false;

    bool searching = Process32First(processesSnapshot, &processInfo);
    while(searching)
    {
        if ( pid == processInfo.th32ProcessID )
        {
            CloseHandle(processesSnapshot);
            return ( exeFile == QString::fromWCharArray(processInfo.szExeFile) );
        }
        searching = Process32Next(processesSnapshot, &processInfo);
    }

    CloseHandle(processesSnapshot);
#else
    Q_UNUSED( pid );
    Q_UNUSED( exeFile );
#endif
    return false;
}

DWORD dataNode::getProcessId(QString exeFile)
{
#ifdef Q_OS_WIN
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( processesSnapshot == INVALID_HANDLE_VALUE )
        return 0;

    bool searching = Process32First(processesSnapshot, &processInfo);
    while(searching)
    {
        if ( exeFile == QString::fromWCharArray(processInfo.szExeFile) )
        {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
        searching = Process32Next(processesSnapshot, &processInfo);
    }

    CloseHandle(processesSnapshot);
#else
    Q_UNUSED( exeFile );
#endif
    return 0;
}

void dataNode::indexModules( DWORD pid )
{
#ifdef Q_OS_WIN
    MODULEENTRY32 moduleInfo;
    moduleInfo.dwSize = sizeof(moduleInfo);

    modules_.clear();

    HANDLE modulesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if ( modulesSnapshot == INVALID_HANDLE_VALUE )
        return;

    bool searching = Module32First(modulesSnapshot, &moduleInfo);
    while(searching)
    {
        QString moduleName = QString::fromWCharArray(moduleInfo.szModule);
        uint moduleAddr = reinterpret_cast<UINT_PTR>(moduleInfo.modBaseAddr);
        modules_.insert(moduleName, moduleAddr);
        modules_.insert(QString("\"%1\"").arg(moduleName), moduleAddr);
        //qDebug() << moduleName << ":" << moduleAddr;
        searching = Module32Next(modulesSnapshot, &moduleInfo);
    }

    CloseHandle(modulesSnapshot);
#else
    Q_UNUSED( pid );
#endif
}

QString dataNode::getProcessVersion(QString exeFile)
{
#ifdef Q_OS_WIN
    QString version = "";

    HANDLE moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID_);
    if ( moduleSnapshot == INVALID_HANDLE_VALUE )
        return "";

    MODULEENTRY32 moduleInfo;
    moduleInfo.dwSize = sizeof(moduleInfo);
    bool searching = Module32First(moduleSnapshot, &moduleInfo);
    while(searching)
    {
        if(QString::fromWCharArray(moduleInfo.szExePath).endsWith("\\" + exeFile))
        {
            DWORD verLength;
            DWORD verSize = GetFileVersionInfoSize(moduleInfo.szExePath, &verLength);
            if(verSize)
            {
                LPBYTE lpBuffer = NULL;
                UINT size = 0;
                LPSTR verData = new char[verSize];
                if( GetFileVersionInfo(moduleInfo.szExePath, verLength, verSize, verData) )
                    if (VerQueryValueA(verData,QString("\\").toLatin1().data(),(VOID FAR* FAR*)&lpBuffer,&size) && size)
                    {
                        VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                        if (verInfo->dwSignature == 0xfeef04bd)
                        {
                                int major = HIWORD(verInfo->dwFileVersionMS);
                                int minor = LOWORD(verInfo->dwFileVersionMS);
                                int build = HIWORD(verInfo->dwFileVersionLS);
                                int revision = LOWORD(verInfo->dwFileVersionLS);

                                version = QString("%1.%2.%3.%4").arg(major).arg(minor).arg(build).arg(revision);
                                break;
                        }
                    }
                delete verData;
            }
        }
        searching = Module32Next(moduleSnapshot, &moduleInfo);
    }
    CloseHandle(moduleSnapshot);
    return version;
#else
    Q_UNUSED( exeFile );
    return "";
#endif
}

void dataNode::divorce()
{
    parentNode_ = 0;
}

void dataNode::clear()
{
    foreach(QString key, childNodes_.keys())
    {
        while (childNodes_[key].count()>0)
        {
            dataNode* temp = childNodes_[key].last();
            childNodes_[key].removeLast();
            delete temp;
        }
    }
}

void dataNode::clearValues()
{
    setValue("");
    foreach(QString key, childNodes_.keys())
        for(int i = 0; i < childNodes_[key].count(); i++)
            childNodes_[key][i]->clearValues();
}

void dataNode::resetCursors()
{
    cursorPositions_.clear();
}

dataNode::dataNode(dataNode* parentNode, itemDefinition def, QString nodeValue )
{
    mutex = new QMutex(QMutex::Recursive);
    value_ = nodeValue;
    definition_ = def;
    parentNode_ = parentNode;
    childNodes_ = QHash<QString, QList<dataNode*> >();
    processHandle_ = NULL;
    processID_ = -1;
}

dataNode::~dataNode()
{
    mutex->lock();
#ifdef Q_OS_WIN
    if(processHandle_)
        CloseHandle(processHandle_);
#endif
    clear();
    divorce();
    mutex->unlock();
    delete mutex;
}

itemDefinition dataNode::definition()
{
    return definition_;
}

void dataNode::setValue(QString val)
{
    mutex->lock();
    value_ = val;
    mutex->unlock();
}

dataNode* dataNode::addChild(QString name, QString val)
{
    return addChild(itemDefinition(name,"",-1,-1,"",false,"",QList<uint>(),MEMTYPE_NONE), val);
}

dataNode* dataNode::addChild(itemDefinition def, QString val)
{
    mutex->lock();
    //remove the * if present and set as default item
    if(def.name.startsWith("*"))
    {
        Q_ASSERT(defaultItem_ == "");
        def.name = def.name.remove(0,1);
        defaultItem_ = def.name.toLower();
    }

    //if new, add a new entry in the children collection
    if(!childNodes_.contains(def.name.toLower()))
        childNodes_.insert(def.name.toLower(), QList<dataNode*>());

    //add a new node to the collection for this named child
    childNodes_[def.name.toLower()].append(new dataNode(this,def,val));
    return childNodes_[def.name.toLower()].last();
    mutex->unlock();
}

dataNode* dataNode::openChild(QString name, QString val)
{
    name = name.toLower();

    if(name.startsWith("*"))
    {
        name = name.remove(0,1);
        Q_ASSERT(defaultItem_ == "" || defaultItem_ == name);
        defaultItem_ = name.toLower();
    }

    //if new, add a new entry in the children collection
    if(!childNodes_.contains(name))
        childNodes_.insert(name, QList<dataNode*>());

    if(!cursorPositions_.contains(name))
        cursorPositions_.insert(name,0);

    if(cursorPositions_[name] >= childNodes_[name].count())
        childNodes_[name].append(new dataNode(this,itemDefinition(name,"",-1,-1,"",false,"",QList<uint>(),MEMTYPE_NONE), val));
    else
        childNodes_[name][cursorPositions_[name]]->setValue(val);

    cursorPositions_[name]++;
    childNodes_[name][cursorPositions_[name]-1]->resetCursors();
    return childNodes_[name][cursorPositions_[name]-1];
}

QList<dataNode*> dataNode::operator[](QString name)
{
    return childNodes_[name.toLower()];
}

QList<dataNode*> dataNode::child(QString name)
{
    return childNodes_[name.toLower()];
}

QString dataNode::defaultItem()
{
    return defaultItem_;
}

bool dataNode::contains(QString name)
{
    return childNodes_.contains(name.toLower());
}

QList<QString> dataNode::keys()
{
    return childNodes_.keys();
}

bool dataNode::hasData()
{
    return definition_.hasData;
}

bool dataNode::hasChildren()
{
    return (childNodes_.count()!=0);
}

QString dataNode::address(QString childNode)
{
    QString nodeAddress = "";
    dataNode* currentNode = this;
    while (currentNode != 0 && currentNode->parentNode()!=0)
    {
        nodeAddress = QString("%1%2%3").arg(currentNode->name()).arg(nodeAddress==""?"":".").arg(nodeAddress);
        currentNode = currentNode->parentNode();
    }
    if(childNode!="")
        nodeAddress = QString("%1%2%3").arg(nodeAddress).arg(nodeAddress==""?"":".").arg(childNode);
    return nodeAddress.toLower();
}

void dataNode::debugTree()
{
    qDebug() << this->address() << " = " << this->value();
    for(int i = 0; i < childNodes_.count(); i++)
    {
        QList<dataNode*> childList = child(keys().at(i));
        for(int j = 0; j < childList.count(); j++)
            childList[j]->debugTree();
    }
}

bool dataNode::refreshProcessValues()
{
    bool changed = false;
#ifdef Q_OS_WIN
    QString newVal = getProcessValue();
    if(newVal!=value_)
    {
        setValue( newVal );
        changed |= true;
    }

    foreach(QString key, childNodes_.keys())
        for(int i = 0; i < childNodes_[key].count(); i++)
            changed |= childNodes_[key][i]->refreshProcessValues();
#endif
    return changed;
}

bool dataNode::openProcess(QString exeFile, QString targetVersion, QString &feedbackMessage)
{
#ifdef Q_OS_WIN
    DWORD pid = processID_;
    feedbackMessage = "";
    if(processHandle_)
    {
        if(validatePID(processID_, exeFile))
            return true;
        else
        {
            feedbackMessage = "LH_DataViewer: Process ID no longer valid.";
            processID_ = 0;
            CloseHandle(processHandle_);
            processHandle_ = NULL;
        }
    }

    if (exeFile=="")
    {
        feedbackMessage = "No process specified";
        return false;
    }

    processID_ = getProcessId(exeFile);
    if(processID_ == pid)
        return (processID_!=0);
    else
    {
        if ( !processID_ )
        {
            feedbackMessage = QString("Could not find process \"%1\"").arg(exeFile);
            return false;
        }

        if(targetVersion!="")
        {
            QString processVersion = getProcessVersion(exeFile);
            if(targetVersion != processVersion)
            {
                feedbackMessage = QString("Incorrect version (layout is for v%1, not v%2)").arg(targetVersion).arg(processVersion);
                return false;
            }
        }

        indexModules(processID_);

        processHandle_ = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processID_);
        if ( !processHandle_ )
        {
            feedbackMessage = QString("Could not open process \"%1\"").arg(exeFile);
            return false;
        }

        refreshProcessValues();
        return true;
    }
#else
    Q_UNUSED(exeFile);
    Q_UNUSED(targetVersion);
    feedbackMessage = "Direct memory access is unsupported on this operating system";
    return false;
#endif
}

QList<uint> dataNode::memoryOffsets()
{
    return definition_.offsets;
}

bool dataNode::getModuleAddress(QString moduleName, uint &moduleAddress)
{
    moduleAddress = 0;
    if (modules_.contains(moduleName))
    {
        moduleAddress = modules_[moduleName];
        return true;
    }
    if (modules_.contains(QString("%1").arg(moduleName)))
    {
        moduleAddress = modules_[QString("%1").arg(moduleName)];
        return true;
    }
    return false;
}

uint dataNode::memoryAddress()
{
    uint addressVal = 0;
#ifdef Q_OS_WIN
    //return definition_.memory;
    if(definition_.startAddress.contains("+"))
    {
        uint moduleAddress;
        QStringList baseParts = definition_.startAddress.split('+',QString::SkipEmptyParts);
        if(getModuleAddress(baseParts.at(0), moduleAddress))
        {
            sscanf(baseParts.at(1).toLatin1().data(), "%x", &addressVal);
            addressVal += moduleAddress;
        } else
            addressVal = 0;
    } else
        sscanf(definition_.startAddress.toLatin1().data(), "%x", &addressVal);
#endif
    return addressVal;
}

