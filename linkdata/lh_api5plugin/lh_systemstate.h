/**
  \file     lh_systemstate.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#ifndef LH_SYSTEMSTATE_H
#define LH_SYSTEMSTATE_H

/* 64 bits integers for the common platforms */
/* Consider including <QtGlobal> for the real deal */
#if !defined(Q_INT64_C)
# if defined(_MSC_VER) && defined(_UI64_MAX)
#  define Q_INT64_C(c) c ## i64    /* signed 64 bit constant */
#  define Q_UINT64_C(c) c ## ui64   /* unsigned 64 bit constant */
typedef __int64 qint64;            /* 64 bit signed */
typedef unsigned __int64 quint64;  /* 64 bit unsigned */
# else
#  define Q_INT64_C(c) static_cast<long long>(c ## LL)     /* signed 64 bit constant */
#  define Q_UINT64_C(c) static_cast<unsigned long long>(c ## ULL) /* unsigned 64 bit constant */
typedef long long qint64;           /* 64 bit signed */
typedef unsigned long long quint64; /* 64 bit unsigned */
# endif
#endif

typedef struct lh_cpudata_t
{
    qint64 when;        /**< millisecond when this data was captured */
    qint64 idle;
    qint64 work;
} lh_cpudata;

typedef struct lh_netdata_t
{
    qint64 device;      /**< device identifier (or zero if all devices) */
    qint64 when;        /**< millisecond when this data was captured */
    qint64 in;          /**< bytes received (system wide cumulative) */
    qint64 out;         /**< bytes sent (system wide cumulative) */
} lh_netdata;

typedef struct lh_memdata_t
{
    qint64 tot_phys;    /**< amount of total physical memory present */
    qint64 tot_virt;    /**< amount of total virtual memory present (swap file) */
    qint64 free_phys;   /**< free physical memory */
    qint64 free_virt;   /**< free virtual memory */
} lh_memdata;

/**
  Provides basic information about the current system state. Strings are UTF-8
  encoded. Paths are absolute, canonical and with forward slashes, and they all
  end in a forward slash.
  */
typedef struct lh_systemstate_t
{
    int size;               /**< sizeof(lh_systemstate) */

    int dev_width;          /**< LCD device width in pixels */
    int dev_height;         /**< LCD device height in pixels */
    int dev_depth;          /**< LCD bit depth, usually 1 (monochrome) or 32 (ARGB) */
    int dev_fps;            /**< current frames per second output to the LCD device */

    int run_time;           /**< LCDHost runtime in seconds (realtime, not CPU usage) */
    int run_idle;           /**< idle time in seconds (screensaver active, or similar) */

    int net_max_in;         /**< max expected bytes/sec inbound (user specified) */
    int net_max_out;        /**< max expected bytes/sec outbound (user specified) */
    int net_cur_in;         /**< current inbound traffic over last second, bytes/sec */
    int net_cur_out;        /**< current outbound traffic over last second, bytes/sec */
    lh_netdata net_data;    /**< raw network load data */

    lh_memdata mem_data;    /**< memory load data */

    int cpu_count;          /**< number of CPU cores detected */
    int cpu_load;           /**< average cpu core load over last second (0...10000) */
    int cpu_self_now;       /**< current LCDHost cpu usage over last second (0...10000) */
    int cpu_self_avg;       /**< average LCDHost cpu usage since start (0...10000) */
    lh_cpudata *cpu_cores;  /**< points to an array lh_cpudata[cpu_count] */

    char *dir_binaries;     /**< where the LCDHost executables are stored, may not be writable */
    char *dir_plugins;      /**< where the LCDHost plugins are stored, may not be writable */
    char *dir_data;         /**< where layouts, logs and other data are stored, will be writable */
    char *dir_layout;       /**< directory where the current layout is stored */

    char *layout_file;      /**< file name of the current layout */
    int layout_rps;         /**< renders per second generated by the current layout */
} lh_systemstate;

#endif /* LH_SYSTEMSTATE_H */
