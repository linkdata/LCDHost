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

#include "NetworkEngine_TDI.h"

#ifdef __MINGW32__

#if (__GNUC__ >= 4 && (__GNUC__ > 4 || __GNUC_MINOR__ >= 7))
# include <winternl.h>
# include <tdiinfo.h>
#else
# include <ntdef.h>
# include <ddk/ntddk.h>
# include <ddk/tdiinfo.h>
#endif

# define IF_MIB_STATS_ID                 1
# define FSCTL_TCP_BASE                  FILE_DEVICE_NETWORK
# define _TCP_CTL_CODE(function, method, access) CTL_CODE(FSCTL_TCP_BASE, function, method, access)
# define IOCTL_TCP_QUERY_INFORMATION_EX  _TCP_CTL_CODE(0, METHOD_NEITHER, FILE_ANY_ACCESS)
# define MAX_PHYSADDR_SIZE   8

# ifndef uchar
#  define uchar UCHAR
#  define ushort USHORT
#  define ulong ULONG
# endif

typedef struct IFEntry {
    ulong           if_index;
    ulong           if_type;
    ulong           if_mtu;
    ulong           if_speed;
    ulong           if_physaddrlen;
    uchar           if_physaddr[MAX_PHYSADDR_SIZE];
    ulong           if_adminstatus;
    ulong           if_operstatus;
    ulong           if_lastchange;
    ulong           if_inoctets;
    ulong           if_inucastpkts;
    ulong           if_innucastpkts;
    ulong           if_indiscards;
    ulong           if_inerrors;
    ulong           if_inunknownprotos;
    ulong           if_outoctets;
    ulong           if_outucastpkts;
    ulong           if_outnucastpkts;
    ulong           if_outdiscards;
    ulong           if_outerrors;
    ulong           if_outqlen;
    ulong           if_descrlen;
    uchar           if_descr[1];
} IFEntry;

#else
# if defined(Q_OS_WIN64)
#  include <winternl.h>
#  include <tdiinfo.h>
#  ifndef uchar
#   define uchar UCHAR
#   define ushort USHORT
#   define ulong ULONG
#  endif
#  include <tcpioctl.h>
# else
#  include <winternl.h>
#  include <api/tdiinfo.h>
#  ifndef uchar
#   define uchar UCHAR
#   define ushort USHORT
#   define ulong ULONG
#  endif
#  include <tcpioctl.h>
# endif
#endif

#ifndef FILE_OPEN_IF
# define FILE_OPEN_IF                    0x00000003
#endif
#ifndef FILE_SYNCHRONOUS_IO_NONALERT
# define FILE_SYNCHRONOUS_IO_NONALERT    0x00000020
#endif
#ifndef OBJ_CASE_INSENSITIVE
# define OBJ_CASE_INSENSITIVE            0x00000040L
#endif
#ifndef DD_TCP_DEVICE_NAME
# define DD_TCP_DEVICE_NAME              L"\\Device\\Tcp"
#endif

typedef NTSTATUS (NTAPI *P_NT_CREATE_FILE)(
    OUT PHANDLE              FileHandle,
    IN  ACCESS_MASK          DesiredAccess,
    IN  POBJECT_ATTRIBUTES   ObjectAttributes,
    OUT PIO_STATUS_BLOCK     IoStatusBlock,
    IN  PLARGE_INTEGER       AllocationSize OPTIONAL,
    IN  ULONG                FileAttributes,
    IN  ULONG                ShareAccess,
    IN  ULONG                CreateDisposition,
    IN  ULONG                CreateOptions,
    IN  PVOID                EaBuffer OPTIONAL,
    IN  ULONG                EaLength );

#define MAX_STATS			32
typedef struct tdi_rawstats_
{
        unsigned long if_index;
        unsigned long if_inoctets;
        unsigned long if_outoctets;
} tdi_rawstats;

static tdi_rawstats statslist[MAX_STATS];

class TDI_Entity
{
public:
    TDI_Entity(TDIEntityID *p) :
        index(0),
        in(0),
        out(0),
        last_in(0),
        last_out(0),
        loopback(false),
        working(false)
    {
        memcpy(&id,p,sizeof(id) );
    }
    TDIEntityID id;
    unsigned index;
    qint64 in;
    qint64 out;
    unsigned long last_in;
    unsigned long last_out;
    QString description;
    bool loopback;
    bool working;
};


NetworkEngine_TDI::NetworkEngine_TDI()
{
    HINSTANCE               ntdll;
    P_NT_CREATE_FILE        pNtCreateFile;
    WCHAR TCPDriverName[] = DD_TCP_DEVICE_NAME;
    OBJECT_ATTRIBUTES       objectAttributes;
    IO_STATUS_BLOCK         ioStatusBlock;
    UNICODE_STRING          UnicodeStr;

    tcpdriver_ = INVALID_HANDLE_VALUE;

    ntdll = LoadLibrary( L"NTDLL.DLL" );
    if( ntdll )
    {
        pNtCreateFile = (P_NT_CREATE_FILE) GetProcAddress( ntdll, "NtCreateFile" );
        if( pNtCreateFile )
        {
            UnicodeStr.Buffer = TCPDriverName;
            UnicodeStr.Length = (USHORT)( wcslen(TCPDriverName) * sizeof(WCHAR) );
            UnicodeStr.MaximumLength = UnicodeStr.Length + sizeof(UNICODE_NULL);

            objectAttributes.Length = sizeof( OBJECT_ATTRIBUTES );
            objectAttributes.ObjectName = &UnicodeStr;
            objectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
            objectAttributes.RootDirectory = NULL;
            objectAttributes.SecurityDescriptor = NULL;
            objectAttributes.SecurityQualityOfService = NULL;

            if( pNtCreateFile(
                    &tcpdriver_,
                    SYNCHRONIZE | GENERIC_EXECUTE,
                    &objectAttributes,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0 ) >= 0 )
            {
                TCP_REQUEST_QUERY_INFORMATION_EX req;
                DWORD arrayLen;
                DWORD bufferLen;
                TDIEntityID * pEntity = NULL;

                memset( statslist, 0, sizeof(statslist) );

                memset( &req, 0, sizeof(req) );
                req.ID.toi_entity.tei_entity    = GENERIC_ENTITY;
                req.ID.toi_entity.tei_instance  = 0;
                req.ID.toi_class                = INFO_CLASS_GENERIC;
                req.ID.toi_type                 = INFO_TYPE_PROVIDER;
                req.ID.toi_id                   = ENTITY_LIST_ID;

                arrayLen = bufferLen = MAX_TDI_ENTITIES * sizeof (TDIEntityID);
                pEntity = (TDIEntityID *) malloc( bufferLen );
                if( pEntity )
                {
                    if( DeviceIoControl( tcpdriver_, IOCTL_TCP_QUERY_INFORMATION_EX,
                                         &req, sizeof(req), pEntity, bufferLen, &arrayLen, NULL ) )
                    {
                        /* call succeeded */
                        if( arrayLen )
                        {
                            for( unsigned n=0; n<arrayLen / sizeof(TDIEntityID); n++ )
                                list_.append( new TDI_Entity( &pEntity[n] ) );
                            sample( NULL );
                        }
                        else
                        {
                            CloseHandle( tcpdriver_ );
                            tcpdriver_ = INVALID_HANDLE_VALUE;
                        }
                    }
                    else
                    {
                        CloseHandle( tcpdriver_ );
                        tcpdriver_ = INVALID_HANDLE_VALUE;
                    }
                    free( pEntity );
                }
            }
            else
                tcpdriver_ = INVALID_HANDLE_VALUE;
        }
        FreeLibrary( ntdll );
    }
    return;
}

NetworkEngine_TDI::~NetworkEngine_TDI()
{
    while( !list_.isEmpty() )
        delete list_.takeFirst();
    if( tcpdriver_ != INVALID_HANDLE_VALUE )
    {
        CloseHandle( tcpdriver_ );
        tcpdriver_ = INVALID_HANDLE_VALUE;
    }
    return;
}

QStringList NetworkEngine_TDI::list() const
{
    QStringList list;
    if( tcpdriver_ != INVALID_HANDLE_VALUE )
    {
        foreach( TDI_Entity *ent, list_ ) if( !ent->loopback && ent->working ) list.append( ent->description );
    }
    return list;
}

bool NetworkEngine_TDI::working() const
{
    int count = 0;
    if( tcpdriver_ == INVALID_HANDLE_VALUE ) return false;
    foreach( TDI_Entity *ent, list_ ) if( ent->working ) count ++;
    return count ? true : false;
}

void NetworkEngine_TDI::sample( lh_netdata *data, QString which )
{
    TCP_REQUEST_QUERY_INFORMATION_EX tcpRequestQueryInfoEx;
    ULONG entityType;
    DWORD entityTypeSize;
    DWORD ifEntrySize;
    IFEntry *ifEntry;
    char ifEntryBuf[ sizeof(IFEntry) + 128 + 1 ];

    if( tcpdriver_ == INVALID_HANDLE_VALUE ) return;

    if( data )
    {
        data->when = QDateTime::currentDateTime().toMSecsSinceEpoch();
        data->device = 0;
        data->in = 0;
        data->out = 0;
    }

    ifEntry = (IFEntry*) &ifEntryBuf;

    foreach( TDI_Entity *ent, list_ )
    {
        if( ent->id.tei_entity == IF_ENTITY )
        {
            /* see if the interface supports snmp mib-2 info */
            memset( &tcpRequestQueryInfoEx, 0, sizeof(tcpRequestQueryInfoEx) );
            tcpRequestQueryInfoEx.ID.toi_entity = ent->id;
            tcpRequestQueryInfoEx.ID.toi_class = INFO_CLASS_GENERIC;
            tcpRequestQueryInfoEx.ID.toi_type = INFO_TYPE_PROVIDER;
            tcpRequestQueryInfoEx.ID.toi_id = ENTITY_TYPE_ID;

            if( DeviceIoControl( tcpdriver_, IOCTL_TCP_QUERY_INFORMATION_EX,
                                 &tcpRequestQueryInfoEx, sizeof(tcpRequestQueryInfoEx),
                                 &entityType, sizeof( entityType ), &entityTypeSize, NULL ) )
            {
                if( entityType == IF_MIB )
                {
                    /* Supports MIB-2 interface. Get snmp mib-2 info. */
                    tcpRequestQueryInfoEx.ID.toi_class = INFO_CLASS_PROTOCOL;
                    tcpRequestQueryInfoEx.ID.toi_type = INFO_TYPE_PROVIDER;
                    tcpRequestQueryInfoEx.ID.toi_id = IF_MIB_STATS_ID;

                    memset( ifEntry, 0, sizeof(ifEntryBuf) );

                    if( DeviceIoControl( tcpdriver_, IOCTL_TCP_QUERY_INFORMATION_EX,
                                         &tcpRequestQueryInfoEx, sizeof(tcpRequestQueryInfoEx),
                                         ifEntry, sizeof(ifEntryBuf), &ifEntrySize, NULL ) )
                    {
                        Q_ASSERT( ent->index ? (ent->index == ifEntry->if_index) : true );
                        ent->description = QString::fromLatin1( (const char*) ifEntry->if_descr );
                        ent->index = ifEntry->if_index;
                        ent->loopback = (ifEntry->if_type == 24);
                        ent->working = true;

                        if( ifEntry->if_inoctets < ent->last_in )
                            ent->in += ifEntry->if_inoctets + ( ULONG_MAX - ent->last_in );
                        else
                            ent->in += ifEntry->if_inoctets - ent->last_in;
                        ent->last_in = ifEntry->if_inoctets;

                        if( ifEntry->if_outoctets < ent->last_out )
                            ent->out += ifEntry->if_outoctets + ( ULONG_MAX - ent->last_out );
                        else
                            ent->out += ifEntry->if_outoctets - ent->last_out;
                        ent->last_out = ifEntry->if_outoctets;

                        /* if filter on, only collect if it matches */
                        if( data && ( (which.isEmpty() && !ent->loopback) || which == ent->description) )
                        {
                            data->in = ent->in;
                            data->out = ent->out;
                        }
                    }
                }
            }
        }
    }
}

