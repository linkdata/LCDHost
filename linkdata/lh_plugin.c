/**
  \file     lh_plugin.c
  @author   Johan Lindh <johan@linkdata.se>
  \brief    Provides some utility functions for handling the lh_blob type.
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "lh_plugin.h"

#if 0
/**
	lh_binaryfile_to_blob()

	Reads a file and converts it into a lh_blob structure.

	\param filename
		Name of the file to load into a blob.
	\return
		A pointer to a malloc()'d lh_blob. The caller must
		free this when done with it.
*/
lh_blob *lh_binaryfile_to_blob( const char *filename )
{
    lh_blob *blob = NULL;
    long filesize = 0;
    FILE *f;

    if( filename == NULL ) return NULL;

    f = fopen( filename, "rb" );
    if( f != NULL )
    {
        if( fseek( f, 0, SEEK_END ) == 0 ) filesize = ftell( f );
        if( filesize > 0 )
        {
            fseek( f, 0, SEEK_SET );
            blob = malloc( sizeof(lh_blob) + filesize );
            if( blob )
            {
                blob->sign = 0xDEADBEEF;
                blob->len = filesize;
                if( fread( & blob->data, 1, filesize, f ) != (size_t) filesize )
                {
                    blob->sign = 0;
                    blob->len = 0;
                    free( blob );
                    blob = NULL;
                }
            }
        }
        fclose( f );
    }

    return blob;
}

/**
	lh_blob_to_headerfile()

	Turns a lh_blob structure into a C/C++ headerfile, suitable for inclusion.

	\param blob
		The blob to convert.
	\param filename
		Name of the file to write to. Will be created if it doesn't exist,
		truncated and overwritten if it does exist.
	\param varname
		Name of the lh_blob variable inside the created headerfile.

*/
void lh_blob_to_headerfile( lh_blob *blob, const char *filename, const char *varname )
{
    unsigned n, m;
    FILE *f;

    if( filename == NULL || blob == NULL ) return;

    f = fopen( filename, "w" );
    if( f != NULL )
    {
        fprintf( f,
                "/* %s */\n"
                "\n"
                "struct _%s_s { const unsigned len; const unsigned sign; const unsigned char data[%u]; };\n"
                "struct _%s_s const _%s_d = \n"
                "{\n"
                "    %u, 0x%X,\n"
                "    {\n"
                "        "
                "", filename, varname, blob->len, varname, varname, blob->len, blob->sign
                );

        for( m=n=0; n<blob->len; n++ )
        {
            m += fprintf( f, "%u", blob->data[n] );
            if( n<blob->len-1 ) m += fprintf( f, "," );
            if( m > 60 )
            {
                fprintf( f, "\n        " );
                m = 0;
            }
        }

        fprintf( f,
                 "\n    }\n"
                 "};\n"
                 "const void *%s = &_%s_d;\n"
                 "\n"
                 "/* eof */\n"
                 "\n", varname, varname );
        fclose( f );
    }
    return;
}
#endif

/**
  Given two lh_cpudata, calculate the average CPU load between them.
  Return the load in hundredths of a percent (0...10000).
  */
int lh_cpuload( lh_cpudata *from, lh_cpudata *to )
{
    qint64 result = 0;
    if( from && to && to->total != from->total )
    {
        if( to->total > from->total )
            result = (((to->system - from->system)+(to->user - from->user)) * 10000) / (to->total - from->total);
        else
            result = (((from->system - to->system)+(from->user - to->user)) * 10000) / (from->total - to->total);
    }
    return (int) result;
}

