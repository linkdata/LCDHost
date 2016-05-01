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

#include "LH_AidaWriter.h"

#include <QDebug>

LH_PLUGIN_CLASS(LH_AidaWriter)

LH_AidaWriter::LH_AidaWriter()
{
    createAidaMemory();
}

const char *LH_AidaWriter::userInit()
{
    if( const char *err = LH_QtInstance::userInit() ) return err;
    setup_xml_ = new LH_Qt_QTextEdit(this,"Aida XML","<sys><id>SDATE</id> <label>Date</label><value>11/01/2011</value></sys><sys><id>STIME</id> <label>Time</label><value>17:35:28</value></sys><sys><id>SUPTIME</id> <label>UpTime</label><value>03:25:30</value></sys><sys><id>SCPUCLK</id> <label>CPU Clock</label><value>3810</value></sys><sys><id>SCPUMUL</id> <label>CPU Multiplier</label><value>20.0</value></sys><sys><id>SCPUFSB</id> <label>CPU FSB</label><value>191</value></sys><sys><id>SMEMCLK</id> <label>Memory Clock</label><value>762</value></sys><sys><id>SMEMSPEED</id> <label>Memory Speed</label><value>DDR3-1524</value></sys><sys><id>SDRAMFSB</id> <label>DRAM:FSB Ratio</label><value>4:1</value></sys><sys><id>SMEMTIM</id> <label>Memory Timings</label><value>9-9-9-24 CR1</value></sys><sys><id>SCPUUTI</id> <label>CPU Utilization</label><value>23</value></sys><sys><id>SCPU1UTI</id> <label>CPU1 Utilization</label><value>10</value></sys><sys><id>SCPU2UTI</id> <label>CPU2 Utilization</label><value>26</value></sys><sys><id>SCPU3UTI</id> <label>CPU3 Utilization</label><value>34</value></sys><sys><id>SCPU4UTI</id> <label>CPU4 Utilization</label><value>26</value></sys><sys><id>SMEMUTI</id> <label>Memory Utilization</label><value>71</value></sys><sys><id>SUSEDMEM</id> <label>Used Memory</label><value>2914</value></sys><sys><id>SFREEMEM</id> <label>Free Memory</label><value>1172</value></sys><sys><id>SGPU1CLK</id> <label>GPU1 Clock</label><value>500</value></sys><sys><id>SGPU1MEMCLK</id> <label>GPU1 Memory Clock</label><value>975</value></sys><sys><id>SGPU1UTI</id> <label>GPU1 Utilization</label><value>12</value></sys><sys><id>SGPU2CLK</id> <label>GPU2 Clock</label><value>500</value></sys><sys><id>SGPU2MEMCLK</id> <label>GPU2 Memory Clock</label><value>975</value></sys><sys><id>SGPU2UTI</id> <label>GPU2 Utilization</label><value>0</value></sys><sys><id>SBATT</id> <label>Battery</label><value>No Battery</value></sys><temp><id>TMOBO</id> <label>Motherboard</label><value>49</value></temp><temp><id>TCPU</id> <label>CPU</label><value>50</value></temp><temp><id>TCC-1-1</id><label>CPU #1 / Core #1</label><value>56</value></temp><temp><id>TCC-1-2</id> <label>CPU #1 / Core #2</label><value>57</value></temp><temp><id>TCC-1-3</id> <label>CPU #1 / Core #3</label><value>59</value></temp><temp><id>TCC-1-4</id> <label>CPU #1 / Core #4</label><value>63</value></temp><temp><id>TGPU1DIOD</id> <label>GPU1 Diode (DispIO)</label><value>69</value></temp><temp><id>TGPU1DIOM</id> <label>GPU1 Diode (MemIO)</label><value>72</value></temp><temp><id>TGPU1DIOS</id> <label>GPU1 Diode (Shader)</label><value>68</value></temp><temp><id>TGPU2DIOD</id> <label>GPU2 Diode (DispIO)</label><value>67</value></temp><temp><id>TGPU2DIOM</id> <label>GPU2 Diode (MemIO)</label><value>71</value></temp><temp><id>TGPU2DIOS</id> <label>GPU2 Diode (Shader)</label><value>62</value></temp><temp><id>THDD1</id> <label>SAMSUNG HD103SJ</label><value>35</value></temp><fan><id>FCPU</id> <label>CPU</label><value>1965</value></fan><fan><id>FGPU1</id> <label>GPU1</label><value>2470</value></fan><fan><id>FGPU2</id> <label>GPU2</label><value>4444</value></fan><duty><id>DGPU1</id> <label>GPU1</label><value>57</value></duty><duty><id>DGPU2</id> <label>GPU2</label><value>57</value></duty><volt><id>VCPU</id> <label>CPU Core</label><value>1.400</value></volt><volt><id>V33V</id> <label>+3.3 V</label><value>3.312</value></volt><volt><id>VP5V</id> <label>+5 V</label><value>5.213</value></volt><volt><id>VP12V</id> <label>+12 V</label><value>11.880</value></volt><volt><id>VBAT</id> <label>VBAT Battery</label><value>3.264</value></volt>");
    hide();
    return 0;
}

LH_AidaWriter::~LH_AidaWriter()
{
    destroyAidaMemory();
}

lh_class *LH_AidaWriter::classInfo()
{
#ifndef QT_NO_DEBUG
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Test",
        "AidaWriter",
        "Aida64 DataWriter",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
#else
    return NULL;
#endif
}

int LH_AidaWriter::notify(int code,void* param)
{
    Q_UNUSED(code);
    Q_UNUSED(param);
    updateAidaMemory();
    return LH_NOTE_SECOND;
}


bool LH_AidaWriter::updateAidaMemory()
{
    destroyAidaMemory();
    createAidaMemory();
    return true;
}

bool LH_AidaWriter::createAidaMemory()
{
#ifdef Q_OS_WIN
    const char* mapname_aida = "AIDA64_SensorValues";
    QByteArray bytXML  = setup_xml_->value().toLatin1();

    sz = bytXML.length();
    filemap_aida = (HANDLE)CreateFileMappingA(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sz,mapname_aida);
    // Get pointer
    if(filemap_aida != NULL)
    {
        aidaData = (char*)MapViewOfFile(filemap_aida, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
        if (aidaData) {
            memset(aidaData, '\0', sz);
            strncpy(aidaData, (char*) bytXML.data(), sz);

            return true;
        } else
            CloseHandle(filemap_aida);
    }
#endif
    return false;
}

void LH_AidaWriter::destroyAidaMemory()
{
#ifdef Q_OS_WIN
    if (aidaData)
    {
        memset(aidaData, '\0', sz);
        UnmapViewOfFile(aidaData);
    }
    if (filemap_aida) CloseHandle(filemap_aida);
#endif
}
