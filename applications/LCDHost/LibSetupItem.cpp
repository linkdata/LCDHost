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


#include <QDebug>
#include "AppObject.h"
#include "LibSetupItem.h"
#include "AppSetupItem.h"

LibSetupItem::LibSetupItem( const AppSetupItem *asi ) :
    val_(asi->value()),
    type_((lh_setup_type)asi->type()),
    publish_(asi->publishPath()),
    subscribe_(asi->subscribePath()),
    flags_(asi->flags()),
    extra_str_(asi->extraStr()),
    extra_int_(asi->extraInt()),
    help_(asi->help()),
    paramlist_(asi->paramList())
{
    name_ = asi->objectName();
    memcpy( &param_, asi->param(), sizeof(param_) );
}

LibSetupItem::LibSetupItem( const lh_setup_item * item ) :
    mimetype_(QString::fromLatin1(item->link.mime)),
    publish_(QString::fromUtf8(item->link.publish)),
    subscribe_(QString::fromUtf8(item->link.subscribe))
{
    extra_str_ = QString();
    extra_int_ = 0;

    if( item->name ) name_ = QString::fromUtf8( item->name );
    type_ = item->type;
    flags_ = item->flags;

    if( item->help )
    {
        help_.append( "<html>" );
        help_.append( item->help );
        help_.append( "</html>" );
    }
    memcpy( &param_, &item->param, sizeof(param_) );
    switch( type_ )
    {
    case lh_type_none:
    case lh_type_last:
        break;
    case lh_type_integer:
    case lh_type_integer_color:
    case lh_type_integer_slider:
    case lh_type_integer_progress:
        val_ = item->data.i;
        break;
    case lh_type_integer_list:
    case lh_type_integer_listbox:
        val_ = item->data.i;
        if( param_.list ) paramlist_ = QString(param_.list).split('\t',QString::SkipEmptyParts);
        param_.list = NULL; // make sure we don't touch unsafe data
        break;
    case lh_type_integer_boolean:
        val_ = (item->data.i) ? true : false;
        break;
    case lh_type_fraction:
        val_ = item->data.f;
        break;
    case lh_type_string:
    case lh_type_string_html:
    case lh_type_string_button:
    case lh_type_string_script:
    case lh_type_string_filename:
    case lh_type_string_font:
    case lh_type_string_inputstate:
    case lh_type_string_inputvalue:
        if( item->data.s && item->param.size > 0 ) val_ = QString::fromUtf8( item->data.s );
        else val_ = QString();
        break;
    case lh_type_image_png:
        val_ = QImage::fromData( (uchar*) item->data.s, item->param.size, "PNG" );
        break;
    case lh_type_image_qimage:
        if( item->data.s ) val_ = *(static_cast<QImage*>((void*)item->data.s));
        else val_ = QImage();
        break;
    }
}

void LibSetupItem::checkDefaults()
{
    // make sure default values provided from LibObject are reasonable
    // things where a blank or zero data item isn't
    // a reasonable default needs a better default
    switch( type() )
    {
    case lh_type_none:
    case lh_type_last:
    case lh_type_integer_color:
    case lh_type_integer_boolean:
        // these are fine with any default
        break;
    case lh_type_integer:
    case lh_type_integer_slider:
    case lh_type_integer_progress:
        // make sure we're in correct range
        if( val_.toInt() < param_.slider.min ) val_.setValue( param_.slider.min );
        if( val_.toInt() > param_.slider.max ) val_.setValue( param_.slider.max );
        break;
    case lh_type_integer_list:
    case lh_type_integer_listbox:
        // if outside valid range, reset
        if( val_.toInt() < 0 || val_.toInt() > paramlist_.size() ) val_.setValue( (int) 0 );
        break;
        break;
    case lh_type_fraction:
        if( val_.toFloat() < param_.range.min ) val_.setValue( param_.range.min );
        if( val_.toFloat() > param_.range.max ) val_.setValue( param_.range.max );
        break;
    case lh_type_string:
    case lh_type_string_html:
    case lh_type_string_button:
    case lh_type_string_script:
    case lh_type_string_filename:
    case lh_type_string_inputstate:
    case lh_type_string_inputvalue:
        break;
    case lh_type_string_font:
        if( val_.toString().isEmpty() ) val_.setValue( QFont().toString() );
        break;
    case lh_type_image_png:
    case lh_type_image_qimage:
        break;
    }
}

size_t LibSetupItem::fillSize( lh_setup_item *item ) const
{
    if( item == NULL ) return 0;
    if( name_ != item->name ) return 0;

    switch( type() )
    {
    case lh_type_none:
    case lh_type_last:
    case lh_type_integer:
    case lh_type_integer_color:
    case lh_type_integer_slider:
    case lh_type_integer_progress:
    case lh_type_integer_list:
    case lh_type_integer_listbox:
    case lh_type_integer_boolean:
    case lh_type_fraction:
        break;
    case lh_type_string:
    case lh_type_string_html:
    case lh_type_string_button:
    case lh_type_string_script:
    case lh_type_string_filename:
    case lh_type_string_font:
    case lh_type_string_inputstate:
    case lh_type_string_inputvalue:
        {
            QByteArray ary = val_.toString().toUtf8();
            return ary.size() + 1;
        }
        break;
    case lh_type_image_png:
    case lh_type_image_qimage:
        break;
    }

    return 0;
}

bool LibSetupItem::fillItem( lh_setup_item *item ) const
{
    bool retv = false;

    if( item == NULL ) return false;
    if( name_ != item->name ) return false;
    if( type_ != item->type ) return false;

    if( !subscribe().isNull() )
    {
        QByteArray source( subscribe().toUtf8() );
        if( source != item->link.subscribe )
        {
            memset(item->link.subscribe, 0, sizeof(item->link.subscribe));
            qstrncpy(item->link.subscribe, source.constData(), sizeof(item->link.subscribe));
            retv = true;
        }
    }

    // note that flags are not
    // transferred TO the plugin
    // just values!
    if( val_.isValid() )
    {
        switch( item->type )
        {
        case lh_type_none:
        case lh_type_last:
            break;
        case lh_type_integer:
        case lh_type_integer_color:
        case lh_type_integer_slider:
        case lh_type_integer_progress:
        case lh_type_integer_list:
        case lh_type_integer_listbox:
            {
                int new_i = val_.toInt();
                if( item->data.i != new_i )
                {
                    item->data.i = new_i;
                    retv = true;
                }
            }
            break;
        case lh_type_integer_boolean:
            {
                int new_i = val_.toBool() ? 1 : 0;
                if( item->data.i != new_i )
                {
                    item->data.i = new_i;
                    retv = true;
                }
            }
            break;
        case lh_type_fraction:
            {
                qreal new_f = val_.toDouble();
                if( item->data.f != new_f )
                {
                    item->data.f = new_f;
                    retv = true;
                }
            }
            break;
        case lh_type_string:
        case lh_type_string_html:
        case lh_type_string_button:
        case lh_type_string_script:
        case lh_type_string_filename:
        case lh_type_string_font:
        case lh_type_string_inputstate:
        case lh_type_string_inputvalue:
            if( item->data.s && item->param.size > 0 )
            {
                QByteArray ary = val_.toString().toUtf8();
                size_t n;
                n = (size_t) ary.size();
                if( n >= item->param.size )
                {
                    qWarning() << item->name << "allows" << item->param.size << "bytes, but" << n << "was needed";
                    n = item->param.size - 1;
                }
                if( memcmp(item->data.s, ary.constData(), n+1) )
                {
                    memcpy( item->data.s, ary.constData(), n );
                    *(item->data.s + n) = '\0';
                    retv = true;
                }
            }
            break;
        case lh_type_image_png:
        case lh_type_image_qimage:
            break;
        }
    }


    if( extra_int_ )
        retv = true; // always propagate active controls

    return retv;
}
