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

#include "LH_MonitoringObject.h"
#include <limits>
#include <typeinfo>

LH_MonitoringObject::LH_MonitoringObject(LH_QtObject *object, monitoringDataMode dataMode, bool includeGroups, bool adaptiveUnitsAllowed)
    : obj_(object)
    , dataMode_(dataMode)
    , includeGroups_(includeGroups)
    , isReset_(false)
    , changeAppSelectionSlot_(0)
    , changeTypeSelectionSlot_(0)
    , changeGroupSelectionSlot_(0)
    , changeItemSelectionSlot_(0)
    , refreshMonitoringOptionsSlot_(0)
    , dataValidityChangedSlot_(0)
    , renderRequiredSlot_(0)
    , setup_value_str_(0)
    , setup_value_units_(0)
    , adaptiveUnitsAllowed_(adaptiveUnitsAllowed)
    , setup_value_ptr_(0)
    , setup_value_valid_(0)
    , setup_monitoring_app_(0)
    , setup_monitoring_app_name_(0)
    , setup_value_type_(0)
    , setup_value_type_name_(0)
    , setup_value_group_(0)
    , setup_value_group_name_(0)
    , setup_value_item_(0)
    , setup_value_item_name_(0)
    , setup_unit_selection_(0)
    , setup_value_offset_(0)
    , setup_value_format_(0)
    , setup_monitoring_options_(0)
{
    int LH_FLAG_SAVEOBJECT_VISIBILITY = LH_FLAG_HIDDEN; //for debugging, set this to LH_FLAG_READONLY, or LH_FLAG_HIDDEN for normal operation
    int LH_FLAG_LINKOBJECT_VISIBILITY = LH_FLAG_HIDDEN; //for debugging, set this to LH_FLAG_READONLY, or LH_FLAG_HIDDEN for normal operation

    setup_value_units_ = new LH_Qt_QString(object, "Value (Units)", "",  LH_FLAG_LINKOBJECT_VISIBILITY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_value_units_->setOrder(-3);
    setup_value_str_ = new LH_Qt_QString(object, "Value (String)", "N/A", LH_FLAG_LINKOBJECT_VISIBILITY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_value_str_->setOrder(-3);
    setup_value_ptr_ = new LH_Qt_int(object, "Value (ArrayPtr)", 0, std::numeric_limits<qreal>::min(), std::numeric_limits<qreal>::max(), LH_FLAG_LINKOBJECT_VISIBILITY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_value_ptr_->setOrder(-3);
    setup_value_valid_ = new LH_Qt_bool(object, "Value (Valid)", false, LH_FLAG_LINKOBJECT_VISIBILITY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_value_valid_->setOrder(-3);
    setup_monitoring_options_ = new LH_Qt_QString(object,"Monitoring Options", "", LH_FLAG_LINKOBJECT_VISIBILITY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSOURCE);
    setup_monitoring_options_->setSubscribePath(getDataSources()->getOptionsLinkPath());
    setup_monitoring_options_->setOrder(-3);


    setup_monitoring_app_ = new LH_Qt_QStringList(object, "Application", getDataSources()->getApplications(), LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);

//    setup_monitoring_app_->setHidden(setup_monitoring_app_->list().count()==1);

    setup_monitoring_app_->setHelp( "<p>The 3rd party application you are using used to monitor your system.</p>");
    setup_monitoring_app_->setOrder(-4);

    setup_monitoring_app_name_ = new LH_Qt_QString(object,"(Application Name)", "", LH_FLAG_SAVEOBJECT_VISIBILITY);
    setup_monitoring_app_name_->setOrder(-4);

    setup_value_type_ = new LH_Qt_QStringList(object, "Value Type", QStringList(), LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_value_type_->setHelp( "<p>The type of value to display.</p>");
    setup_value_type_->setOrder(-4);

    setup_value_type_name_ = new LH_Qt_QString(object,"(Value Type Name)", "", LH_FLAG_SAVEOBJECT_VISIBILITY);
    setup_value_type_name_->setOrder(-4);

    setup_value_group_ = new LH_Qt_QStringList(object, "Value Group", QStringList(), LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_value_group_->setHelp( "<p>The value to display.</p>");
    setup_value_group_->setOrder(-4);

    setup_value_group_name_ = new LH_Qt_QString(object,"(Value Group Name)", "", LH_FLAG_SAVEOBJECT_VISIBILITY);
    setup_value_group_name_->setOrder(-4);

    setup_value_item_ = new LH_Qt_QStringList(object,"Value Item", QStringList(), LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_value_item_->setHelp( "<p>The value to display.</p>");
    setup_value_item_->setOrder(-4);

    setup_value_item_name_ = new LH_Qt_QString(object,"(Value Item Name)", "", LH_FLAG_SAVEOBJECT_VISIBILITY);
    setup_value_item_name_->setOrder(-4);

    setup_value_offset_ = new LH_Qt_bool(object,"Apply Offsets", true, LH_FLAG_HIDDEN);
    setup_value_offset_->setHelp( "<p>Add any user defined offsets to the value (defined within RivaTuner).</p>");
    setup_value_offset_->setOrder(-4);

    setup_value_format_ = new LH_Qt_bool(object,"Use Formatted Data", false, LH_FLAG_HIDDEN);
    setup_value_format_->setHelp( "<p>Relates to RivaTuner's \"raw data transforming mode\" or Afterburner's \"Formatted Data\".</p><p>(If you don't know what this is, leave it disabled and ignore it.)</p>");
    setup_value_format_->setOrder(-4);

    (new LH_Qt_QString(object,("image-hr-data"), QString("<hr>"), LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_HIDETITLE,lh_type_string_html ))->setOrder(-4);

    setup_unit_selection_ = new LH_Qt_QStringList(object, "Units", QStringList(), LH_FLAG_HIDDEN);
    setup_unit_selection_->setHelp( "<p>The units used by this control.</p>");
    setup_unit_selection_->setOrder(-3);

}

void LH_MonitoringObject::monitoringInit(
        const char* refreshMonitoringOptionsSlot, const char* connectChangeEventsSlot, const char* changeAppSelectionSlot,
        const char* changeTypeSelectionSlot, const char* changeGroupSelectionSlot, const char* changeItemSelectionSlot,
        const char* dataValidityChangedSlot, const char* renderRequiredSlot)
{
    changeAppSelectionSlot_ = changeAppSelectionSlot;
    changeTypeSelectionSlot_ = changeTypeSelectionSlot;
    changeGroupSelectionSlot_ = changeGroupSelectionSlot;
    changeItemSelectionSlot_ = changeItemSelectionSlot;
    refreshMonitoringOptionsSlot_ = refreshMonitoringOptionsSlot;
    dataValidityChangedSlot_ = dataValidityChangedSlot;
    renderRequiredSlot_ = renderRequiredSlot;

    obj_->connect(obj_, SIGNAL(initialized()), obj_, connectChangeEventsSlot);
}

void LH_MonitoringObject::connectChangeEvents()
{
    setAppSelection();
    setTypeSelection();
    setGroupSelection();
    setItemSelection();

    setup_monitoring_app_->setFlag(LH_FLAG_READONLY, false);

    obj_->connect(setup_monitoring_options_, SIGNAL(changed()), obj_, refreshMonitoringOptionsSlot_);
    obj_->connect(setup_monitoring_app_, SIGNAL(changed()), obj_, changeAppSelectionSlot_ );
    obj_->connect(setup_value_type_, SIGNAL(changed()), obj_, changeTypeSelectionSlot_ );
    obj_->connect(setup_value_group_, SIGNAL(changed()), obj_, changeGroupSelectionSlot_ );
    obj_->connect(setup_value_item_, SIGNAL(changed()), obj_, changeItemSelectionSlot_ );
    obj_->connect(setup_value_valid_, SIGNAL(changed()), obj_, dataValidityChangedSlot_ );

    obj_->connect(setup_unit_selection_, SIGNAL(changed()), obj_, renderRequiredSlot_ );
}

void LH_MonitoringObject::refreshMonitoringOptions()
{
    setup_monitoring_app_->setList(getDataSources()->getApplications());
    setAppSelection();
}

void LH_MonitoringObject::dataValidityChanged()
{
    updateLinkPaths();
}

void LH_MonitoringObject::setAppSelection()
{
    QString val = setup_monitoring_app_name_->value();
    if(val!="" && setup_monitoring_app_->list().contains(val))
        setup_monitoring_app_->setValue(val);
    else
        setup_monitoring_app_->setIndex(0);
    reset();
    updateUnitOptions();
}

void LH_MonitoringObject::changeAppSelection()
{
    if (setup_monitoring_app_->list().length()!=0)
    {
        QString val = setup_monitoring_app_->valueText();
        if(val!="") setup_monitoring_app_name_->setValue(val);
    }

    reset();
    updateUnitOptions();
}

void LH_MonitoringObject::setSelection(LH_Qt_QStringList* list_obj, LH_Qt_QString* name_obj, int et)
{
    QString val = name_obj->value();
    if(val!="" && list_obj->list().contains(val))
        list_obj->setValue(val);
    else
        list_obj->setValue(0);
    clear(et);
    refresh(et);
    updateUnitOptions();
    updateLinkPaths();
}

void LH_MonitoringObject::changeSelection(LH_Qt_QStringList* list_obj, LH_Qt_QString* name_obj, int et)
{
    if (list_obj->list().length()!=0)
    {
        QString val = list_obj->valueText();
        if(val!="") name_obj->setValue(val);
    }
    clear(et);
    refresh(et);
    updateUnitOptions();
    updateLinkPaths();
}

void LH_MonitoringObject::setTypeSelection()
{
    setSelection(setup_value_type_, setup_value_type_name_, mon_group | mon_item);
}

void LH_MonitoringObject::changeTypeSelection()
{
    changeSelection(setup_value_type_, setup_value_type_name_, mon_group | mon_item);
}

void LH_MonitoringObject::setGroupSelection()
{
    setSelection(setup_value_group_, setup_value_group_name_, mon_item);
}

void LH_MonitoringObject::changeGroupSelection()
{
    changeSelection(setup_value_group_, setup_value_group_name_, mon_item);
}

void LH_MonitoringObject::setItemSelection()
{
    setSelection(setup_value_item_, setup_value_item_name_, 0);
}

void LH_MonitoringObject::changeItemSelection()
{
    changeSelection(setup_value_item_, setup_value_item_name_, 0);
}

void LH_MonitoringObject::reset()
{
    isReset_ = true;

    setup_value_units_->setValue("");

    setup_value_str_->setSubscribePath("");
    setup_value_str_->setValue("N/A");

    setup_value_ptr_->setSubscribePath("");
    setup_value_ptr_->setValue(0);

    setup_value_valid_->setSubscribePath("");
    setup_value_valid_->setValue(false);

    setVisible(mon_type | mon_group | mon_item, false);
    setReadonly(mon_type | mon_group | mon_item, false);
    clear(mon_type | mon_group | mon_item);
    refresh(mon_type | mon_group | mon_item);

    showOffset(false);
    showFormat(false);
}

QString LH_MonitoringObject::value_str() {
    return setup_value_str_->value();
}

double LH_MonitoringObject::value_num(bool *ok, QString *units) {
    bool _ok;
    double val = setup_value_str_->value().toDouble(&_ok);
    if(ok) *ok = _ok;
    if(units) *units = setup_value_units_->value();
    if(!_ok)
        return 0;
    if(adaptiveUnitsAllowed_ && setup_unit_selection_->list().count() > 1)
    {
        SensorItem itm = selectedSensor(&_ok);
        if(ok) *ok = _ok;
        if(_ok)
            val = itm.adaptToUnits(val, (setup_unit_selection_->value()==0), setup_value_units_->value(), setup_unit_selection_->valueText(), units);
    }
    return val;
}

QString LH_MonitoringObject::value_units(bool allowAdaptation)
{
    if(allowAdaptation)
        return value_units();
    else
        return setup_value_units_->value();
}

QString LH_MonitoringObject::value_units() {
    if(adaptiveUnitsAllowed_ && setup_unit_selection_->list().count() > 1)
    {
        if(setup_unit_selection_->value()!=0)
            return setup_unit_selection_->valueText();
        else
        {
            bool ok;
            QString units;
            value_num(&ok, &units);
            if(ok)
                return units;
            else
                return setup_value_units_->value();
        }
    }
    else
        return setup_value_units_->value();
}


void LH_MonitoringObject::showOffset(bool b)
{
    setup_value_offset_->setFlag(LH_FLAG_HIDDEN, !b);
}

void LH_MonitoringObject::showFormat(bool b)
{
    setup_value_format_->setFlag(LH_FLAG_HIDDEN, !b);
}

void LH_MonitoringObject::setVisible(int et, bool b)
{
    if((et & mon_type)==mon_type)
        setup_value_type_->setFlag(LH_FLAG_HIDDEN, !b);
    if((et & mon_group)==mon_group)
        setup_value_group_->setFlag(LH_FLAG_HIDDEN, !b);
    if((et & mon_item)==mon_item)
        setup_value_item_->setFlag(LH_FLAG_HIDDEN, !b);
}

void LH_MonitoringObject::setReadonly(int et, bool b)
{
    if((et & mon_type)==mon_type)
        setup_value_type_->setFlag(LH_FLAG_READONLY, b);
    if((et & mon_group)==mon_group)
        setup_value_group_->setFlag(LH_FLAG_READONLY, b);
    if((et & mon_item)==mon_item)
        setup_value_item_->setFlag(LH_FLAG_READONLY, b);
}

void LH_MonitoringObject::clear(int et)
{
    if((et & mon_type)==mon_type)
        setup_value_type_->list().clear();
    if((et & mon_group)==mon_group)
        setup_value_group_->list().clear();
    if((et & mon_item)==mon_item)
        setup_value_item_->list().clear();
}

void LH_MonitoringObject::refresh(int et)
{
    bool ok;
    LH_MonitoringSource* source = selectedMonitoringApp(&ok);
    if(ok)
    {
        if(et & mon_type)
        {
            if(source)
                setup_value_type_->setList(source->getTypes());
            else
                setup_value_type_->list().clear();
            setup_value_type_->setVisible(
                    setup_value_type_->list().length()>1 ||
                    (
                        setup_value_type_->list().length()==1 &&
                        setup_value_type_->list()[0]!="" &&
                        source->alwaysShowAllSelectors()
                    )
                );
            setTypeSelection();
        }

        if(et & mon_group)
        {
            if(source)
                setup_value_group_->setList(source->getGroups(setup_value_type_->valueText()));
            else
                setup_value_type_->list().clear();
            setup_value_group_->setVisible(
                    setup_value_group_->list().length()>1 ||
                    (
                        setup_value_group_->list().length()==1 &&
                        setup_value_group_->list()[0]!="" &&
                        source->alwaysShowAllSelectors()
                    )
                );
            setGroupSelection();
        }

        if(et & mon_item)
        {
            if(source)
            {
                int i = setup_value_item_->value();
                setup_value_item_->setList(source->getItems(setup_value_type_->valueText(), setup_value_group_->valueText(), includeGroups_));
                i = qMin(i, setup_value_item_->list().count()-1);
                setup_value_item_->setValue(i);
            }
            else
                setup_value_type_->list().clear();

            if(setup_value_item_->list().length()==1)
                setup_value_item_->setVisible(
                        (
                            setup_value_item_->valueText() != setup_value_group_->valueText() &&
                            setup_value_item_->valueText() != setup_value_type_->valueText()
                        ) ||
                        source->alwaysShowAllSelectors()
                    );
            else
                setup_value_item_->setVisible(setup_value_item_->list().length()>0);
            setItemSelection();
        }
    }
}

void LH_MonitoringObject::updateUnitOptions()
{
    // update the list of possible options for the adaptive units
    bool ok;
    SensorItem itm = selectedSensor(&ok);
    if(ok)
    {
        setup_unit_selection_->list().clear();
        if(adaptiveUnitsAllowed_ && itm.adaptiveUnitsFactor > 1)
        {
            setup_unit_selection_->list().append("(Auto)");
            setup_unit_selection_->list().append(itm.adaptiveUnitsList);
            setup_unit_selection_->setVisible(true);
        }
        else
        {
            setup_unit_selection_->list().append(itm.units);
            setup_unit_selection_->setVisible(false);
        }
        setup_unit_selection_->refreshList();
    }
}

bool LH_MonitoringObject::dataValid()
{
    return setup_value_valid_->value();
}

QString LH_MonitoringObject::updateLinkPaths()
{
    bool ok;
    LH_MonitoringSource* source = selectedMonitoringApp(&ok);

    if(ok)
    {
        QString linkPath_Valid = source->getLinkPath("","","")+"/dataAvailable";
        setup_value_valid_->setSubscribePath(linkPath_Valid);
        setup_value_valid_->refresh();
    }

    if(!ok || !dataValid())
    {
        if(!isReset_)
        {
            isReset_ = true;
            setup_value_units_->setValue("");

            setup_value_str_->setSubscribePath("");
            setup_value_str_->setValue("N/A");

            setup_value_ptr_->setSubscribePath("");
            setup_value_ptr_->setValue(0);
        }
        return "";
    }
    else
    {
        isReset_ = false;
        QString linkPath = source->getLinkPath(setup_value_type_->valueText(), setup_value_group_->valueText(), setup_value_item_->valueText());
        bool ok;
        SensorItem itm = selectedSensor(&ok);
        setup_value_units_->setValue(ok? itm.units : "");

        setup_value_str_->setSubscribePath(linkPath+"/String");

        QString linkPath_ArrayPtr = source->getLinkPath(setup_value_type_->valueText(), setup_value_group_->valueText(), "__array_ptr");
        setup_value_ptr_->setSubscribePath(linkPath_ArrayPtr);

        return linkPath;
    }
}


LH_MonitoringSource* LH_MonitoringObject::selectedMonitoringApp(bool *ok)
{
    LH_MonitoringSource* source = getDataSources()->source(setup_monitoring_app_->valueText());
    if(ok) *ok = (source != NULL);
    return source;
}

SensorItem LH_MonitoringObject::selectedSensor(bool *ok, int *selectedIndex)
{
    LH_MonitoringSource* source = selectedMonitoringApp(ok);
    if(ok)
        return source->getItem(setup_value_type_->valueText(), setup_value_group_->valueText(), setup_value_item_->valueText(), ok, selectedIndex);
    else
    {
        *selectedIndex =-1;
        SensorItem dummy;
        return dummy;
    }
}

SensorGroup* LH_MonitoringObject::selectedSensorGroup(bool *ok)
{
    LH_MonitoringSource* source = selectedMonitoringApp(ok);
    if(ok)
        return source->getGroup(setup_value_type_->valueText(), setup_value_group_->valueText(), ok);
    else
        return NULL;
}

QVector<qreal> LH_MonitoringObject::getValuesVector(bool hasDead, qreal deadVal, bool &ok, QStringList *names)
{
    QVariantMap jvalues = Json::parse(setup_value_str_->value(), ok).toMap();
    QVector<qreal> currVals;
    if(ok)
    {
        if(names)
            names->clear();
        for(int i=0; i<jvalues.keys().count(); i++)
        {
            QVariantMap jvalue = jvalues[jvalues.keys()[i]].toMap();
            qreal currVal = jvalue["value"].toFloat();
            QString currName = jvalue["name"].toString();
            if (!hasDead || (deadVal != currVal) )
            {
                currVals.append(currVal);
                if(names)
                    names->append(currName);
            }
        }
    }
    return currVals;
}
