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

#ifndef RAWINPUTLOGITECH_H
#define RAWINPUTLOGITECH_H

#define LG_G1		1
#define LG_G2		2
#define LG_G3		3
#define LG_G4		4
#define LG_G5		5
#define LG_G6		6
#define LG_G7		7
#define LG_G8		8
#define LG_G9		9
#define LG_G10		10
#define LG_G11		11
#define LG_G12		12
#define LG_G13		13
#define LG_G14		14
#define LG_G15		15
#define LG_G16		16
#define LG_G17		17
#define LG_G18		18
#define LG_G19		19
#define LG_G20		20
#define LG_G21		21
#define LG_G22		22
#define LG_BL		24
#define LG_S1		25
#define LG_S2		26
#define LG_S3		27
#define LG_S4		28
#define LG_S5		29
#define LG_M1		30
#define LG_M2		31
#define LG_M3		32
#define LG_MR		33
#define LG_B1		34
#define LG_B2		35
#define LG_B3		36


typedef struct
{
    BYTE bReportId;     // G13_InputReportId_Gaming == 1

    BYTE X;             // Stick X
    BYTE Y;             // Stick Y

    BYTE G1     : 1;
    BYTE G2     : 1;
    BYTE G3     : 1;
    BYTE G4     : 1;
    BYTE G5     : 1;
    BYTE G6     : 1;
    BYTE G7     : 1;
    BYTE G8     : 1;

    BYTE G9     : 1;
    BYTE G10    : 1;
    BYTE G11    : 1;
    BYTE G12    : 1;
    BYTE G13    : 1;
    BYTE G14    : 1;
    BYTE G15    : 1;
    BYTE G16    : 1;

    BYTE G17    : 1;
    BYTE G18    : 1;
    BYTE G19    : 1;
    BYTE G20    : 1;
    BYTE G21    : 1;
    BYTE G22    : 1;
    BYTE FB     : 1;    // Flash Busy
    BYTE BL     : 1;    // Back lighting Button

    BYTE S1     : 1;    // LCD Button1
    BYTE S2     : 1;    // LCD Button2
    BYTE S3     : 1;    // LCD Button3
    BYTE S4     : 1;    // LCD Button4
    BYTE S5     : 1;    // LCD Button5
    BYTE M1     : 1;
    BYTE M2     : 1;
    BYTE M3     : 1;

    BYTE MR     : 1;
    BYTE B1     : 1;    // Stick - Left Button
    BYTE B2     : 1;    // Stick - Bottom Button
    BYTE B3     : 1;    // Stick - Stick Button
    BYTE ZP     : 1;    // ESD zap
    BYTE Resv   : 3;    // Reserved

} G13_InputReport_Gaming;

typedef struct G15v1InputReport_Gaming {
    BYTE   	bReportId; // == 2

	BYTE    G1      : 1;
	BYTE    res1    : 1;
	BYTE    G13     : 1;
	BYTE    res2    : 4;
	BYTE    BL      : 1;

	BYTE    G7      : 1;
	BYTE    G2      : 1;
	BYTE    res3    : 1;
	BYTE    G14     : 1;
	BYTE    res4    : 3;
	BYTE    S2      : 1;

	BYTE    res5    : 1;
	BYTE    G8      : 1;
	BYTE    G3      : 1;
	BYTE    res6    : 1;
	BYTE    G15     : 1;
	BYTE    res7    : 2;
	BYTE    S3      : 1;

	BYTE    res8    : 2;
	BYTE    G9      : 1;
	BYTE    G4      : 1;
	BYTE    res9    : 1;
	BYTE    G16     : 1;
	BYTE    res10   : 1;
	BYTE    S4      : 1;

	BYTE    res11   : 3;
	BYTE    G10     : 1;
	BYTE    G5      : 1;
	BYTE    res12   : 1;
	BYTE    G17     : 1;
	BYTE    S5      : 1;

	BYTE    M1      : 1;
	BYTE    res13   : 3;
	BYTE    G11     : 1;
	BYTE    G6      : 1;
	BYTE    res14   : 2;

	BYTE    res15   : 1;
	BYTE    M2      : 1;
	BYTE    res16   : 3;
	BYTE    G12     : 1;
	BYTE    MR      : 1;
	BYTE    res17   : 1;

	BYTE    res18   : 2;
	BYTE    M3      : 1;
	BYTE    res19   : 3;
	BYTE    G18     : 1;
	BYTE    S1      : 1;

} G15v1InputReport_Gaming;

typedef struct
{
    BYTE        bReportId;
    union {
        struct {
            struct {
                BYTE    GKey1       : 1;
                BYTE    GKey2       : 1;
                BYTE    GKey3       : 1;
                BYTE    GKey4       : 1;
                BYTE    GKey5       : 1;
                BYTE    GKey6       : 1;
                BYTE    M1Key       : 1;
                BYTE    M2Key       : 1;
            } R0;
            struct {
                BYTE    bLight      : 1;
                BYTE    bSoftKey1   : 1;
                BYTE    bSoftKey2   : 1;
                BYTE    bSoftKey3   : 1;
                BYTE    bSoftKey4   : 1;
                BYTE    M3Key       : 1;
                BYTE    MRKey       : 1;
                BYTE    bSoftKey0   : 1;
            } R1;
            BYTE    Reserved[2];
        } KeyMatrix;

        BYTE    bData[2];
    } GamingReport;
} org_G15v2InputReport_Gaming;

typedef struct
{
    BYTE    bReportId;
	BYTE    G1   : 1;
	BYTE    G2   : 1;
	BYTE    G3   : 1;
	BYTE    G4   : 1;
	BYTE    G5   : 1;
	BYTE    G6   : 1;
	BYTE    M1   : 1;
	BYTE    M2   : 1;
	BYTE    BL   : 1;
	BYTE    S2   : 1;
	BYTE    S3   : 1;
	BYTE    S4   : 1;
	BYTE    S5   : 1;
	BYTE    M3   : 1;
	BYTE    MR   : 1;
	BYTE    S1   : 1;
	BYTE    resvd[2];
} G15v2InputReport_Gaming;

typedef struct
{
    BYTE bReportId;     // This will be G19InputReportId_Gaming == 2

    BYTE G1     : 1;
    BYTE G2     : 1;
    BYTE G3     : 1;
    BYTE G4     : 1;
    BYTE G5     : 1;
    BYTE G6     : 1;
    BYTE G7     : 1;
    BYTE G8     : 1;

    BYTE G9     : 1;
    BYTE G10    : 1;
    BYTE G11    : 1;
    BYTE G12    : 1;
    BYTE M1     : 1;
    BYTE M2     : 1;
    BYTE M3     : 1;
    BYTE MR     : 1;

    BYTE Resv1  : 3;
    BYTE BL     : 1;  // Light Button
    BYTE Resv2  : 4;

} G19InputReport_Gaming;


#endif // RAWINPUTLOGITECH_H
