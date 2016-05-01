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

#include "LH_CursorReceiver.h"

LH_CursorReceiver::LH_CursorReceiver(LH_QtInstance *parent, const char *amember) : QObject(parent)
{
    setup_json_data_ = new LH_Qt_QString(parent, "Cursor Data", QString(), LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSOURCE | LH_FLAG_FIRST); // | LH_FLAG_READONLY | LH_FLAG_HIDEVALUE
    setup_json_data_->setHelp("<p>This field holds the data for this cursor; all cursor objects by default link themselves to \"Primary Cursor\".</p>"
                              "<p>You only need change the name of this link if your layout needs more than one cursor.");
    setup_json_data_->setSubscribePath("/Cursors/Primary Cursor");
    setup_json_data_->setMimeType("x-cursor/x-cursor");

    setup_json_postback_ = new LH_Qt_QString(parent, "Cursor Postback", "", LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSINK | LH_FLAG_FIRST | LH_FLAG_HIDDEN);
    setup_json_postback_->setPublishPath("/Cursors/Postback");
    setup_json_postback_->setMimeType("x-cursor/x-postback");

    new LH_Qt_QString(parent, "hr2", "<hr>", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_FIRST | LH_FLAG_HIDETITLE, lh_type_string_html);

    setup_coordinate_ = new LH_Qt_QString(parent, "Coordinate", "1,1", LH_FLAG_AUTORENDER | LH_FLAG_FIRST);
    setup_coordinate_->setHelp("This is the coordinate of this object, i.e. when the cursor is at the point specified here this object is selected. <br/>"
                               "<br/>"
                               "Note that many objects can have the same coordinate if the user requires.<br/>"
                               "<br/>"
                               "The format is [x],[y] <br/>"
                               "e.g.: 1,1"
                               );

    setup_cursor_state_ = new LH_Qt_QStringList( parent, ("Cursor State"), QStringList()<<"OFF"<<"OFF_SEL"<<"ON"<<"ON_SEL", LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSINK|LH_FLAG_NOSOURCE|LH_FLAG_READONLY|LH_FLAG_FIRST );

    connect(setup_json_data_,SIGNAL(changed()),this,SLOT(updateState()));

    if(amember)
        connect(this, SIGNAL(stateChanged(bool,bool)), parent, amember);

    return;
}

bool LH_CursorReceiver::updateState()
{
    bool newSelected;
    bool newActive;
    QString newStatusCode = data().getState(setup_coordinate_->value().split(';'),newSelected,newActive);
    if(setup_cursor_state_->valueText() != newStatusCode)
    {
        setup_cursor_state_->setValue(newStatusCode);
        emit stateChanged(newSelected, newActive);
        return true;
    }
    else
        return false;
}

cursorData LH_CursorReceiver::data()
{
    return cursorData(setup_json_data_->value());
}

void LH_CursorReceiver::postback(cursorData data)
{
    QString key = setup_json_data_->link().remove(QRegExp("^(@|=)"));
    if(postback_data.contains(key))
        postback_data.remove(key);
    postback_data.insert(key, data.serialize());
    qDebug() << "Postback:" << key << "  ::  " << data.serialize();
    setup_json_postback_->setValue( QString::number(QDateTime::currentMSecsSinceEpoch()) );
}
