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

#ifndef LH_DATAVIEWERDATANODE_H
#define LH_DATAVIEWERDATANODE_H

#include <QString>
#include <QHash>
#include <QMutex>
#include <QList>
#include <QDebug>
#include <QStringList>

#define polling_rate 200

#ifdef Q_OS_WIN
#include <windows.h>
#include <winbase.h>
#include <Psapi.h>
#include <tlhelp32.h>
#else
    typedef size_t SIZE_T;
    typedef void* HANDLE;
    typedef quint32 DWORD;
#endif

extern QHash<QString, uint> modules_;

enum MemoryDataType
{
    MEMTYPE_NONE = 0,
    MEMTYPE_4BYTE,
    MEMTYPE_8BYTE,
    MEMTYPE_FLOAT,
    MEMTYPE_DOUBLE,
    MEMTYPE_TEXT_ASCII,
    MEMTYPE_TEXT_UNICODE
};

struct itemDefinition
{
    QString name;
    QString address;
    int x;
    int y;
    QString formatting;
    bool hasData;
    QString startAddress;
    QList<uint> offsets;
    MemoryDataType memoryDataType;
    itemDefinition() :
        x(-1), y(-1), hasData(false), memoryDataType(MEMTYPE_NONE)
    {}
    itemDefinition(
        const QString & a, const QString & b,
            int c, int d, const QString & e, bool f,
            const QString & g, const QList<uint> & h,
            MemoryDataType i
            ) :
        name(a), address(b), x(c), y(d),
        formatting(e), hasData(f), startAddress(g),
        offsets(h), memoryDataType(i)
    {}
};

struct sharedItem
{
    QString name;
    QString value;
    sharedItem() {}
    sharedItem(const QString & a, const QString & b) :
        name(a), value(b) {}
};

class sharedCollection
{
    QList<sharedItem> items_;
public:
    bool valid;
    QString lastUpdated;
    QString expiresAt;

    void clear(int createCount = 0)
    {
        valid = false;
        lastUpdated = "";
        expiresAt = "N/A";
        items_.clear();

        for(int i=0; i<createCount; i++)
            items_.append(sharedItem());
    }

    int count()
    {
        return items_.count();
    }

    sharedItem item(int index)
    {
        return items_[index];
    }

    sharedItem operator[](int index)
    {
        return items_[index];
    }

    void setItem(int index, QString name, QString value = "" )
    {
        items_[index].name = name;
        items_[index].value = value;
    }
};

class dataNode
{
    QString defaultItem_;

    itemDefinition definition_;
    QString value_;

    QHash<QString, QList<dataNode*> > childNodes_;
    dataNode* parentNode_;

    QHash<QString, int> cursorPositions_;

    HANDLE processHandle_;
    DWORD processID_;

    QString getProcessValue();

    bool getProcessValue(uint address, QList<uint> offsets, void *dest, size_t len);

    bool getProcessValue(uint address, QList<uint> offsets, QString *dest, bool unicode = true);

    bool validatePID(DWORD pid, QString exeFile);

    DWORD getProcessId(QString exeFile);

    void indexModules(DWORD pid);

    QString getProcessVersion(QString exeFile);

protected:
    QMutex* mutex;

    void divorce();
    void clear();

public:
    QHash<QString,QString> attributes;

    void clearValues();

    void resetCursors();

    dataNode(dataNode* parentNode = 0, itemDefinition def = itemDefinition(), QString nodeValue = "" );

    ~dataNode();

    dataNode *parentNode() const { return parentNode_; }

    QString value() const { return value_; }

    QString name() const { return definition_.name; }

    itemDefinition definition();

    void setValue(QString val);

    dataNode* addChild(QString name, QString val = "");
    dataNode* addChild(itemDefinition def, QString val = "");

    dataNode* openChild(QString name, QString val = "");

    QList<dataNode*> operator[](QString name);

    QList<dataNode*> child(QString name);

    QString defaultItem();

    bool contains(QString name);

    QList<QString> keys();

    bool hasData();

    bool hasChildren();

    QString address(QString childNode = "");

    void debugTree();

    bool refreshProcessValues();

    bool openProcess(QString exeFile, QString targetVersion, QString &feedbackMessage);

    QList<uint> memoryOffsets();

    bool getModuleAddress(QString moduleName, uint &moduleAddress);

    HANDLE processHandle() const
    {
        if( parentNode() )
            return parentNode()->processHandle();
        return processHandle_;
    }

    uint memoryAddress();
};




#endif // LH_DATAVIEWERDATANODE_H
