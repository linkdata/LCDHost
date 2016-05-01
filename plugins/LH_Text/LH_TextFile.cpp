/**
  \file     LH_TextFile.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

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

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QUrl>

#include "LH_TextFile.h"

LH_PLUGIN_CLASS(LH_TextFile)

lh_class *LH_TextFile::classInfo()
{
    static lh_class classinfo =
    {
        sizeof(lh_class),
        "Static",
        "StaticTextFile",
        "Text (file)",
        96, 32,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classinfo;
}

LH_TextFile::LH_TextFile()
    : LH_Text()
    , setup_file_(0)
{
}

const char* LH_TextFile::userInit()
{
    if (const char* msg = LH_Text::userInit()) return msg;
    setup_file_ = new LH_Qt_QFileInfo( this, tr("File"), QFileInfo(), LH_FLAG_AUTORENDER );
    setup_file_->setOrder(-1);
    setup_file_->setHelp("<p>Select the file whose contents will be shown. "
                         "The file will be checked for updates regularly.</p>");
    connect( setup_file_, SIGNAL(changed()), this, SLOT(fileChanged()) );
    setup_text_->setName("~HiddenText");
    setup_text_->setFlag(LH_FLAG_HIDDEN|LH_FLAG_READONLY,true);
    return 0;
}

void LH_TextFile::fileChanged()
{
    last_read_ = QDateTime();
    checkFile();
    return;
}

void LH_TextFile::checkFile()
{
    setup_file_->value().refresh();
    if( !setup_file_->value().isFile() )
    {
        setup_text_->setValue(tr("No such file."));
        setup_text_->setFlag(LH_FLAG_HIDDEN,false);
        return;
    }
    else
    {
        setup_text_->setFlag(LH_FLAG_HIDDEN,true);
    }

    if( !last_read_.isValid() || setup_file_->value().lastModified() > last_read_ )
    {
        QFile file( setup_file_->value().filePath() );
        if( file.open( QIODevice::ReadOnly) )
        {
            QTextStream stream(&file);
            setText( stream.readAll() );
            doc().setMetaInformation( QTextDocument::DocumentTitle, setup_file_->value().fileName() );
            QString docpath = setup_file_->value().canonicalPath();
            if( !docpath.endsWith('/') ) docpath.append('/');
            doc().setMetaInformation( QTextDocument::DocumentUrl, QUrl::fromLocalFile(docpath).toString() );
            requestRender(); // Always render, external resources may have changed
            last_read_ = setup_file_->value().lastModified();
        }
        else
        {
            if( setText( tr("%1:<br> %2").arg(setup_file_->value().canonicalFilePath() ).arg(file.errorString()) ) )
                requestRender();
        }
    }
}

int LH_TextFile::notify( int code, void* param )
{
    if( code & LH_NOTE_SECOND ) checkFile();
    return LH_NOTE_SECOND | LH_Text::notify(code,param);
}
