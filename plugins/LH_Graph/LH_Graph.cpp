/**
  \file     LH_Graph.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#include <QtGlobal>
#include <QDebug>
#include <QLinearGradient>
#include <QPainter>
#include <QVarLengthArray>

#include "LH_Graph.h"
#include <math.h>

#define DEFAULT_SAMPLE_RATE 1


enum graph_orientation
{
    TimeHorizontal_NowRight_MaxTop = 0,
    TimeHorizontal_NowRight_MaxBottom = 1,
    TimeHorizontal_NowLeft_MaxTop = 2,
    TimeHorizontal_NowLeft_MaxBottom = 3,
    TimeVertical_NowTop_MaxLeft = 4,
    TimeVertical_NowTop_MaxRight = 5,
    TimeVertical_NowBottom_MaxLeft = 6,
    TimeVertical_NowBottom_MaxRight = 7
};

enum background_type
{
    Background_None = 0,
    Background_AreaFill = 1,
    Background_Image = 2
};

enum foreground_type
{
    Foreground_LineOnly = 0,
    Foreground_AreaFill = 1,
    Foreground_Image = 2
};

static inline uint PREMUL(uint x) {
    uint a = x >> 24;
    uint t = (x & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff) * a;
    x = (x + ((x >> 8) & 0xff) + 0x80);
    x &= 0xff00;
    x |= t | (a << 24);
    return x;
}

LH_Graph::LH_Graph(GraphDataMode dataMode, DataLineCollection* externalSource, LH_QtObject *parent)
    : LH_QtInstance(parent)
    , dataMaxY_(0)
    , dataMinY_(0)
    , dataDeltaY_(0)
    , graphMaxY_(0)
    , graphMinY_(0)
    , divisorY_(1)
    , userDefinableLimits_(false)
    , hasDeadValue_(false)
    , deadValue_(0)
    , dataMode_(dataMode)
    , lines_(0)
    , externalSource_(0)
    , lineData_(0)
    , graph_empty_(true)
    , setup_fg_type_(0)
    , setup_bg_type_(0)
    , setup_orientation_(0)
    , setup_line_selection_(0)
    , setup_pencolor_(0)
    , setup_fillcolor1_(0)
    , setup_fillcolor2_(0)
    , setup_line_configs_(0)
    , setup_bgcolor_(0)
    , setup_max_samples_(0)
    , setup_sample_rate_(0)
    , setup_description_(0)
    , setup_max_grow_(0)
    , setup_max_(0)
    , setup_min_(0)
    , setup_auto_scale_y_max_(0)
    , setup_auto_scale_y_min_(0)
    , setup_show_y_max_(0)
    , setup_show_y_min_(0)
    , setup_hide_when_empty_(0)
    , setup_show_real_limits_(0)
    , setup_y_labels_right_(0)
    , setup_label_font_(0)
    , setup_label_color_(0)
    , setup_label_shadow_(0)
    , setup_fg_image_(0)
    , setup_bg_image_(0)
    , setup_fg_alpha_(0)
{
    if (isDebug) qDebug() << "graph: init: begin";

    lines_ = new DataLineCollection(30);
    lineData_ = lines_;

    if (externalSource)
        setExternalSource(externalSource);

    QStringList fgTypes = QStringList();
    fgTypes.append("Line Only");
    fgTypes.append("Area Fill");
    fgTypes.append("Image");
    setup_fg_type_ = new LH_Qt_QStringList(this, "Graph Style", fgTypes, LH_FLAG_AUTORENDER);
    setup_fg_type_->setHelp( "<p>The graph's style.</p><p style='color:red'>IMPORTANT NOTE: Using a foreground image for a large graph, especially one with multiple lines can have a significant impact on CPU usage.</p>");

    QStringList bgTypes = QStringList();
    bgTypes.append("None");
    bgTypes.append("Area Fill");
    bgTypes.append("Image");
    setup_bg_type_ = new LH_Qt_QStringList(this, "Graph Background", bgTypes, LH_FLAG_AUTORENDER);
    setup_bg_type_->setHelp( "<p>The graph's background style.</p>");
    setup_bg_type_->setValue(1);

    QStringList orientations = QStringList();
    orientations.append("Time Horizontal (Now = Right), Max Value = Top");
    orientations.append("Time Horizontal (Now = Right), Max Value = Bottom");
    orientations.append("Time Horizontal (Now = Left), Max Value = Top");
    orientations.append("Time Horizontal (Now = Left), Max Value = Bottom");
    orientations.append("Time Vertical (Now = Top), Max Value = Left");
    orientations.append("Time Vertical (Now = Top), Max Value = Right");
    orientations.append("Time Vertical (Now = Bottom), Max Value = Left");
    orientations.append("Time Vertical (Now = Bottom), Max Value = Right");
    setup_orientation_ = new LH_Qt_QStringList(this, "Graph Orientation", orientations, LH_FLAG_AUTORENDER);
    setup_orientation_->setHelp( "<p>The layout of the graph.</p>");

    setup_bgcolor_ = new LH_Qt_QColor(this,"Background color",Qt::transparent,LH_FLAG_AUTORENDER);
    setup_bgcolor_->setHelp( "<p>The color for the background.</p>");

    setup_bg_image_ = new LH_Qt_QFileInfo(this, "Background Image", QFileInfo(""), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    setup_bgcolor_->setHelp( "<p>The image used for the background.</p>");

    setup_max_samples_ = new LH_Qt_int(this,"Max Samples",lines_->limit(),5,600,LH_FLAG_AUTORENDER);
    setup_max_samples_->setHelp( "<p>How many data points to store &amp; plot.</p>");

    setup_sample_rate_ = new LH_Qt_int(this,"Sample Rate",DEFAULT_SAMPLE_RATE,1,12,LH_FLAG_AUTORENDER);
    setup_sample_rate_->setHelp( "<p>How frequently to log data.</p>");

    setup_description_ = new LH_Qt_QString(this,"~","...",LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_description_->setHelp( "<p>Combining the \"Max Samples\" and the \"Sample Rate\" this field displays that total timespan of the graph.</p>");

    setup_line_selection_ = new LH_Qt_QStringList(this,"Selected Line",QStringList());
    setup_line_selection_->setHelp( "<p>Select a line here and configure it below. Seperate settings are stored for each line.</p>");

    setup_pencolor_ = new LH_Qt_QColor(this,"Line color",Qt::black,LH_FLAG_AUTORENDER);
    setup_pencolor_->setHelp( "<p>The colour used do draw the line.</p>");

    setup_fillcolor1_ = new LH_Qt_QColor(this,"Area color (start)",Qt::green,LH_FLAG_AUTORENDER);
    setup_fillcolor1_->setHelp( "<p>The color used to fill the area between the line and the axis, at the furthest point from the axis (this color is only truely visible when the graph is full).</p>");
    setup_fillcolor2_ = new LH_Qt_QColor(this,"Area color (end)",Qt::red,LH_FLAG_AUTORENDER);
    setup_fillcolor2_->setHelp( "<p>The color used to fill the area between the line and the axis, at the axis.</p>");

    setup_fg_image_ = new LH_Qt_QFileInfo(this, "Fill Image", QFileInfo(""), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    setup_fg_image_->setHelp( "<p>This image is used to create the foreground, filling the area between the line and the axis.</p>");
    setup_fg_alpha_ = new LH_Qt_int(this,"Fill Image Opacity", 255, 0, 255, LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    setup_fg_alpha_->setHelp( "<p>This value affects the opacity of the fill image.</p>");

    setup_max_ = new LH_Qt_float(this, "Graph Ymax",graphMaxY_,-99999999,99999999, LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    setup_max_->setHelp( "<p>The maximum value displayed on the graph.</p>"
                         "<p>This value can only be set when \"Ymax Can Grow\" is disabled (see below).</p>");

    setup_max_grow_ = new LH_Qt_bool(this,"Ymax Can Grow", true, LH_FLAG_AUTORENDER | LH_FLAG_READONLY | LH_FLAG_HIDDEN);
    setup_max_grow_->setHelp( "<p>When enabled the value for \"Graph Ymax\" will increase automatically to ensure no value goes off the top edge.</p>");

    setup_auto_scale_y_max_ = new LH_Qt_bool(this,"Auto Scale Ymax", false, LH_FLAG_AUTORENDER);
    setup_auto_scale_y_max_->setHelp( "<p>When enabled, the plotted area's highest point will shift with the visible data to create a \"zooming\" effect. The less variation in the data the tighter the zoom. (Best used with \"Auto Scale Ymin\".)</p>");

    setup_min_ = new LH_Qt_float(this, "Graph Ymin",graphMinY_,-99999999,99999999, LH_FLAG_AUTORENDER | LH_FLAG_READONLY | LH_FLAG_HIDDEN);
    setup_min_->setHelp( "<p>The minimum value displayed on the graph.</p>");

    setup_auto_scale_y_min_ = new LH_Qt_bool(this,"Auto Scale Ymin", false, LH_FLAG_AUTORENDER);
    setup_auto_scale_y_min_->setHelp( "<p>When enabled, the plotted area's lowest point will shift with the visible data to create a \"zooming\" effect. The less variation in the data the tighter the zoom. (Best used with \"Auto Scale Ymax\".)</p>");

    setup_show_y_max_ = new LH_Qt_bool(this,"Show Ymax", false, LH_FLAG_AUTORENDER);
    setup_show_y_max_->setHelp( "<p>Displays the maximum visible point on the graph's <i>Y-axis</i>, i.e. the highest value that could be plotted without changing the scale.</p>"
                         "<p>Selecting \"Show Real Y-Limits\" will change this to displaying the highest value currently visible on the graph's <i>line(s)</i>, i.e. the highest value actually \"seen\".</p>");

    setup_show_y_min_ = new LH_Qt_bool(this,"Show Ymin", false, LH_FLAG_AUTORENDER);
    setup_show_y_min_->setHelp( "<p>Displays the minimum visible point on the graph's <i>Y-axis</i>, i.e. the lowest value that could be plotted without changing the scale.</p>"
                         "<p>Selecting \"Show Real Y-Limits\" will change this to displaying the lowest value currently visible on the graph's <i>line(s)</i>, i.e. the lowest value actually \"seen\".</p>");

    setup_hide_when_empty_ = new LH_Qt_bool(this,"Hide When Empty", false, LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
    setup_hide_when_empty_->setHelp( "<p>When enabled, this will override the \"Show Ymax\" and \"Show Ymin\" values to hide the axis labels when the graph shows only a straight line at 0 (i.e. no valid data has been received).</p>");

    setup_show_real_limits_ = new LH_Qt_bool(this,"Show Real Y-Limits", false, LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
    setup_show_real_limits_->setHelp( "<p>By default the labels shown on a graph correspond to the upper & lower values visible on the Y-axis, i.e. the highest & lowest values that could be plotted without changing the scale. This is most useful when a graph auto scales.</p>"
                                      "<p>However, some users may wish instead to show the highest & lowest points currently visible on the graph's line(s). This is particularly useful on graphs that are not set to auto scale.");

    setup_y_labels_right_ = new LH_Qt_bool(this,"Y Labels on Right", false, LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
    setup_y_labels_right_->setHelp( "<p>Position the labels on the right side instead of the left (or bottom instead of top for graphs which swap the X & Y axis).</p>");

    setup_line_configs_ = new LH_Qt_QTextEdit(this, "Line Configs","",LH_FLAG_HIDDEN);
    setup_line_configs_->setHelp( "<p>This text field stores the configuration data for each individual line and should not be edited manually.</p>");
    setup_line_configs_->setOrder(1);

    setup_label_font_ = new LH_Qt_QFont(this, "Axis Label Font", QFont("Arial",8),LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
    setup_label_font_->setHelp( "<p>The font used for Axis labels.</p>");

    setup_label_color_ = new LH_Qt_QColor(this,"Axis Label Color","white",LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
    setup_label_color_->setHelp( "<p>The color used for Axis labels.</p>");

    setup_label_shadow_ = new LH_Qt_QColor(this,"Axis Label Glow",QColor(0,0,0,92),LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
    setup_label_shadow_->setHelp( "<p>The color used for for the \"Glow/Shadow\" effect around Axis labels (designed to improve legibility).</p>"
                                  "<p>Set the transparency to 0 to remove the effect.</p>");

    if(dataMode_==gdmInternallyManaged)
        addLine("Default");
    else
        updateLinesList(true);

    connect( setup_fg_type_, SIGNAL(changed()), this, SLOT(changeType()) );
    connect( setup_bg_type_, SIGNAL(changed()), this, SLOT(changeType()) );
    connect( setup_max_samples_, SIGNAL(changed()), this, SLOT(changeMaxSamples()) );
    connect( setup_sample_rate_, SIGNAL(changed()), this, SLOT(changeSampleRate()) );
    connect( setup_line_selection_, SIGNAL(changed()), this, SLOT(changeSelectedLine()) );
    connect( setup_pencolor_, SIGNAL(changed()), this, SLOT(updateSelectedLine()) );
    connect( setup_fillcolor1_, SIGNAL(changed()), this, SLOT(updateSelectedLine()) );
    connect( setup_fillcolor2_, SIGNAL(changed()), this, SLOT(updateSelectedLine()) );
    connect( setup_fg_image_, SIGNAL(changed()), this, SLOT(updateFGImage()) );
    connect( setup_bg_image_, SIGNAL(changed()), this, SLOT(updateBGImage()) );
    connect( setup_fg_alpha_, SIGNAL(changed()), this, SLOT(updateSelectedLine()) );
    connect( setup_show_y_max_, SIGNAL(changed()), this, SLOT(updateLabelSelection()) );
    connect( setup_show_y_min_, SIGNAL(changed()), this, SLOT(updateLabelSelection()) );
    connect( setup_show_real_limits_, SIGNAL(changed()), this, SLOT(updateLabelSelection()) );
    connect( setup_max_grow_, SIGNAL(changed()), this, SLOT(updateLimitControls()) );

    connect( this, SIGNAL(initialized()), this, SLOT(updateDescText()) );
    connect( this, SIGNAL(initialized()), this, SLOT(updateLabelSelection()) );

    if (isDebug) qDebug() << "Line Count (Init): " << lineData_->count() << " lines;";

    if (isDebug) qDebug() << "graph: init: done";

    return;
}

LH_Graph::~LH_Graph() {
    lineData_ = 0;
    if (lines_) {
        delete lines_;
        lines_ = 0;
    }
}

const char* LH_Graph::userInit() {
    if( const char *err = LH_QtInstance::userInit() ) return err;
    return 0;
}

void LH_Graph::setExternalSource(DataLineCollection* externalSource)
{
    Q_ASSERT_X(dataMode_!=gdmInternallyManaged || externalSource == NULL, "LH_Graph::setExternalSource", "Cannot set the external source if the data is managed internally.");
    externalSource_ = externalSource;

    switch(dataMode_)
    {
    case gdmExternallyManaged:
        lineData_ = externalSource;
        break;
    case gdmHybrid:
        Q_ASSERT(lines_);
        if (lines_) {
            lines_->clear();
            if(externalSource_)
                lines_->copyFrom((*externalSource_).averageOver(DEFAULT_SAMPLE_RATE * 1000));
        }
        lineData_ = lines_;
        break;
    case gdmInternallyManaged:
        lineData_ = lines_;
        break;
    }
}


qreal LH_Graph::max_val()
{
    if (setup_min_->value() < setup_max_->value())
        return setup_max_->value();
    else
        return setup_min_->value()+1;
}

qreal LH_Graph::min_val()
{
    return setup_min_->value();
}

bool LH_Graph::canGrow()
{
    return setup_max_grow_->value();
}
void LH_Graph::setCanGrow(bool val)
{
    setup_max_grow_->setValue(val);
    updateLimitControls();
}

void LH_Graph::findDataBounds(DataLineCollection* lineData)
{
    qreal _dataMaxY;
    qreal _dataMinY;
    qreal _dataDeltaY;

    qreal _graphMaxY;
    qreal _graphMinY;

    qreal _min;
    qreal _max;

    bool _graph_empty;

    if (isDebug) qDebug() << "graph: find bounds: begin";

    _min = min_val();
    _max = max_val();

    // apply fixes for cases where somehow graph limits have become NaN
    if(graphMinY_ != graphMinY_)
        _graphMinY = _min;
    else
        _graphMinY = graphMinY_;
    if(graphMaxY_ != graphMaxY_)
        _graphMaxY = _max;
    else
        _graphMaxY = graphMaxY_;

    // set the bounds such that min is suitably high (so it can be correctly
    // brought down) and max is suitably low (so it can be correctly raised)
    _dataMinY = _max;
    _dataMaxY = _min;

    // examine data points for each line and shift the data boundries accordingly
    for(int lineID=0;lineID<linesCount(); lineID++)
    {
        qreal valueMin = _max;
        qreal valueMax = _min;
        bool isConstant = true;
        qreal constantValue = 0;
        for(int i=0;i<(*lineData)[lineID].length() && i<(*lineData).limit();i++)
        {
            qreal y = (*lineData).at(lineID).at(i).value;
            if(i==0)
                constantValue = y;
            else
                isConstant = isConstant && (constantValue == y);
            if(valueMin>y) valueMin = y;
            if(valueMax<y) valueMax = y;
        }
        bool doDraw = !(hasDeadValue_ && isConstant && (deadValue_ == constantValue));
        if(doDraw)
        {
            if(_dataMinY>valueMin) _dataMinY = valueMin;
            if(_dataMaxY<valueMax) _dataMaxY = valueMax;
        }
    }

    // if the set maximum value is allowed to move itself, apply any necessary movement
    if(canGrow() && _dataMaxY > _max)
        _max = _dataMaxY * 1.1;

    // calculate the visible range drawn
    _dataDeltaY = (setup_auto_scale_y_max_->value()? _dataMaxY : _max) -
                  (setup_auto_scale_y_min_->value()? _dataMinY : _min);

    //set whether the graph is empty
    _graph_empty = (_dataMaxY == _dataMinY) && (_dataMinY == 0);

    // if the range is 0 and is left as such the graph will experience weird behaviour
    // so to fix this a small alteration is made in this case.
    if(_dataDeltaY == 0)
        _dataDeltaY = 0.01 * (_max-_min);

    //set the graph's max
    if (!setup_auto_scale_y_max_->value())
        //unzoomed
        _graphMaxY = _max;
    else if (_dataMaxY > _graphMaxY || _dataMaxY + _dataDeltaY * 0.667 < _graphMaxY)
    {
        //auto-zoom
        _graphMaxY = _dataMaxY + _dataDeltaY * 0.333;
        if(canGrow() && _graphMaxY > _max)
            _max = (_graphMaxY * 1.1);
        if(_graphMaxY > _max)
            _graphMaxY = _max;
    }

    //set the graph's min
    if (!setup_auto_scale_y_min_->value())
         //unzoomed
        _graphMinY = _min;
    else if (_dataMinY < _graphMinY || _dataMinY - _dataDeltaY * 0.667 > _graphMinY)
    {
        //auto-zoom
        _graphMinY = _dataMinY - _dataDeltaY * 0.333;
        if(_graphMinY < _min)
            _graphMinY = _min;
    }

    // if a graph min = graph max has slipped through, fix it with a hammer here.
    if (_graphMaxY == _graphMinY) _graphMaxY += _dataDeltaY;

    // update values
    dataMaxY_ = _dataMaxY;
    dataMinY_ = _dataMinY;
    dataDeltaY_ = _dataDeltaY;
    graphMaxY_ = _graphMaxY;
    graphMinY_ = _graphMinY;
    graph_empty_ = _graph_empty;

    setMin(_min);
    setMax(_max);

    if (isDebug) qDebug() << "graph: find bounds: done";
}

int LH_Graph::lastVisibleLine()
{
    if(!lineData_)
        return -1;
#ifdef MONITORING_CLASS
    for( int i=lineData_->count()-1; i>=0; i-- )
        if( !lineData_->at(i).group && !lineData_->at(i).hidden )
            return i;
    return 0;
#else
    return linesCount() - 1;
#endif
}

void LH_Graph::drawSingle(int lineID)
{
    if (lineID < 0 || lineID >= linesCount()) return;

    if (isDebug) qDebug() << "Line Count (Draw Single): " << lineData_->count() << " lines;";
    if (isDebug) qDebug() << "graph: draw line: begin " << lineID;
    QColor penColor = QColor();
    QColor fillColor1 = QColor();
    QColor fillColor2 = QColor();
    QString fgImgPath = "";
    int fgImgAlpha = 255;

    //get the colours required for this line & it's fill area
    loadColors(lineID, penColor, fillColor1, fillColor2, fgImgPath, fgImgAlpha, true);

    QPainter painter;

    QVector<QPointF> points((*lineData_).limit()+2);

    int w = image()->width();
    int h = image()->height();

    //assemble the array of points for the graph (based on values & orientation)
    bool isConstant = true;
    qreal constantValue = 0;
    qreal point_position = 0;

    int desired_duration = (setup_sample_rate_->value() * 1000);
    DataLineCollection avgData;
    switch(dataMode_)
    {
    case gdmInternallyManaged:
    case gdmHybrid:
        avgData = (*lineData_);
        break;
    case gdmExternallyManaged:
        avgData = (*lineData_).averageOver(desired_duration);
        break;
    }

    //empty the graph when drawing line 0
    if(lineID==0)
    {
        if (img_size_.width() != w || img_size_.height()!= h)
            reload_images();

        if(QImage *img = initImage(w, h))
        {
            switch(setup_bg_type_->value())
            {
            case Background_Image:
                if(!bgImg_.isNull() && setup_bg_image_->value().isFile())
                {
                    *img = bgImg_;
                    break;
                }
            case Background_None:
                img->fill(PREMUL(QColor(0,0,0,0).rgba()));
                break;
            case Background_AreaFill:
                img->fill(PREMUL( setup_bgcolor_->value().rgba()));
                break;
            }

            findDataBounds(&avgData);
        }
    }

    qreal axis_max = (qMin(setup_max_samples_->value(), avgData.limit())-1) * desired_duration;

    int point_count = 0;
    for(int i=0; i<avgData[lineID].length() && i<(setup_max_samples_->value()); i++)
    {       
        qreal point_value = ((avgData.at(lineID).at(i).value) - graphMinY_);
        qreal point_duration = (qreal)avgData.at(lineID).at(i).duration;

        if(point_count!=0)
            point_position += point_duration;

        qreal x = 0; qreal y=0;
        switch(setup_orientation_->value())
        {
        case TimeHorizontal_NowRight_MaxTop:
            x = w * (1 - point_position / axis_max);
            y = h - (point_value * h / (graphMaxY_ - graphMinY_));
            break;
        case TimeHorizontal_NowRight_MaxBottom:
            x = w * (1 - point_position / axis_max);
            y = (point_value * h / (graphMaxY_ - graphMinY_));
            break;
        case TimeHorizontal_NowLeft_MaxTop:
            x = w * (point_position / axis_max);
            y = h - (point_value * h / (graphMaxY_ - graphMinY_));
            break;
        case TimeHorizontal_NowLeft_MaxBottom:
            x = w * (point_position / axis_max);
            y = (point_value * h / (graphMaxY_ - graphMinY_));
            break;
        case TimeVertical_NowTop_MaxLeft:
            x = w - (point_value * w / (graphMaxY_ - graphMinY_));
            y = h * (point_position / axis_max);
            break;
        case TimeVertical_NowTop_MaxRight:
            x = (point_value * w / (graphMaxY_ - graphMinY_));
            y = h * (point_position / axis_max);
            break;
        case TimeVertical_NowBottom_MaxLeft:
            x = w - (point_value * w / (graphMaxY_ - graphMinY_));
            y = h * (1 - point_position / axis_max);
            break;
        case TimeVertical_NowBottom_MaxRight:
            x = (point_value * w / (graphMaxY_ - graphMinY_));
            y = h * (1 - point_position / axis_max);
            break;
        }
        if(point_count==0)
            constantValue = avgData.at(lineID).at(i).value;
        else
            isConstant = isConstant && (constantValue == avgData.at(lineID).at(i).value);
        points[point_count] = QPointF(x, y);
        point_count++;
    }


    //apply point corrections & prep gradient
    QLinearGradient gradient;
    if (point_count > 0) {
        qreal x = points[point_count-1].x();
        qreal y = points[point_count-1].y();
        gradient.setColorAt(0,fillColor2);
        gradient.setColorAt(1,fillColor1);
        switch(setup_orientation_->value())
        {
        case TimeHorizontal_NowRight_MaxTop:
            points[point_count++] =  QPointF(x, h+10);
            points[point_count++] =  QPointF(w, h+10);
            gradient.setStart( QPointF(0, 0) );
            gradient.setFinalStop( QPointF(0,h) );
            break;
        case TimeHorizontal_NowRight_MaxBottom:
            points[point_count++] =  QPointF(x, -10);
            points[point_count++] =  QPointF(w, -10);
            gradient.setStart( QPointF(0, h) );
            gradient.setFinalStop( QPointF(0,0) );
            break;
        case TimeHorizontal_NowLeft_MaxTop:
            points[point_count++] =  QPointF(x, h+10);
            points[point_count++] =  QPointF(0, h+10);
            gradient.setStart( QPointF(0, 0) );
            gradient.setFinalStop( QPointF(0,h) );
            break;
        case TimeHorizontal_NowLeft_MaxBottom:
            points[point_count++] =  QPointF(x, -10);
            points[point_count++] =  QPointF(0, -10);
            gradient.setStart( QPointF(0, h) );
            gradient.setFinalStop( QPointF(0,0) );
            break;
        case TimeVertical_NowTop_MaxLeft:
            points[point_count++] =  QPointF(w+10, y);
            points[point_count++] =  QPointF(w+10, 0);
            gradient.setStart( QPointF( 0,0 ) );
            gradient.setFinalStop( QPointF(w, 0) );
            break;
        case TimeVertical_NowTop_MaxRight:
            points[point_count++] =  QPointF(-10, y);
            points[point_count++] =  QPointF(-10, 0);
            gradient.setStart( QPointF(w, 0) );
            gradient.setFinalStop( QPointF(0,0) );
            break;
        case TimeVertical_NowBottom_MaxLeft:
            points[point_count++] =  QPointF(w+10, y);
            points[point_count++] =  QPointF(w+10, h);
            gradient.setStart( QPointF( 0,0 ) );
            gradient.setFinalStop( QPointF(w, 0) );
            break;
        case TimeVertical_NowBottom_MaxRight:
            points[point_count++] =  QPointF(-10, y);
            points[point_count++] =  QPointF(-10, h);
            gradient.setStart( QPointF(w, 0) );
            gradient.setFinalStop( QPointF(0,0) );
            break;
        }
    }

    if( painter.begin(image()) )
    {
        painter.setRenderHint( QPainter::Antialiasing, true );
        painter.setPen(penColor);

        //draw the line and fill the area if required
        bool doDraw = !(hasDeadValue_ && isConstant && (deadValue_ == constantValue));
        if (doDraw)
        {
            switch(setup_fg_type_->value())
            {
            case Foreground_LineOnly:
                painter.drawPolyline(points.constData(), point_count-2);
                break;
            case Foreground_AreaFill:
                painter.setBrush(QBrush(gradient));
                painter.drawPolygon(points.constData(), point_count);
                break;
            case Foreground_Image:
                {
                    QRectF graph_area = QRectF( 0,0, w, h );

                    //build mask
                    uchar blank_data[4] = {0,0,0,0};
                    QImage maskImg = QImage(blank_data,1,1,QImage::Format_ARGB32).scaled(w,h);
                    QPainter maskPaint;
                    if( maskPaint.begin( &maskImg ) )
                    {
                        maskPaint.setRenderHint( QPainter::Antialiasing, true );
                        QColor maskCol = QColor(0,0,0,fgImgAlpha);
                        maskPaint.setPen(maskCol);
                        maskPaint.setBrush(QBrush(maskCol));
                        maskPaint.drawPolygon(points.constData(), point_count);
                        maskPaint.end();
                    }

                    //apply mask
                    if(!fgImgs_.contains(lineID) && QFileInfo(fgImgPath).isFile())
                         fgImgs_.insert(lineID,QImage(fgImgPath).scaled(w,h));
                    if(fgImgs_.contains(lineID))
                    {
                        QImage tempImg = fgImgs_.value(lineID);
                        QPainter tempPaint;
                        if( tempPaint.begin( &tempImg ) )
                        {
                            tempPaint.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                            tempPaint.drawImage(graph_area, maskImg);
                            tempPaint.end();
                        }
                        painter.drawImage(graph_area, tempImg);
                    }
                    painter.drawPolyline(points.constData(), (*lineData_).at(lineID).length());
                }
                break;
            }
        }

        // when completing the last line, add any labels if required
        if(lineID == lastVisibleLine())
        {
            painter.setFont( setup_label_font_->value() );
            int flags = Qt::TextSingleLine|Qt::TextIncludeTrailingSpaces;

            QString maxLabel = getLabelText((setup_show_real_limits_->value()? dataMaxY_: graphMaxY_));
            QString minLabel = getLabelText((setup_show_real_limits_->value()? dataMinY_: graphMinY_));

            bool emptyHide = setup_hide_when_empty_->value() && graph_empty_;
            switch(setup_orientation_->value())
            {
            case TimeHorizontal_NowRight_MaxTop:
            case TimeHorizontal_NowLeft_MaxTop:
                flags = flags | (setup_y_labels_right_->value() ? Qt::AlignRight : Qt::AlignLeft);
                if(setup_show_y_max_->value() && !emptyHide) addText(painter, image()->rect(), flags|Qt::AlignTop,    maxLabel);
                if(setup_show_y_min_->value() && !emptyHide) addText(painter, image()->rect(), flags|Qt::AlignBottom, minLabel);
                break;
            case TimeHorizontal_NowRight_MaxBottom:
            case TimeHorizontal_NowLeft_MaxBottom:
                flags = flags | (setup_y_labels_right_->value() ? Qt::AlignRight : Qt::AlignLeft);
                if(setup_show_y_max_->value() && !emptyHide) addText(painter, image()->rect(), flags|Qt::AlignBottom, maxLabel);
                if(setup_show_y_min_->value() && !emptyHide) addText(painter, image()->rect(), flags|Qt::AlignTop,    minLabel);
                break;
            case TimeVertical_NowTop_MaxLeft:
            case TimeVertical_NowBottom_MaxLeft:
                flags = flags | (setup_y_labels_right_->value() ? Qt::AlignTop : Qt::AlignBottom);
                if(setup_show_y_max_->value() && !emptyHide) addText(painter, image()->rect(), flags|Qt::AlignLeft,   maxLabel);
                if(setup_show_y_min_->value() && !emptyHide) addText(painter, image()->rect(), flags|Qt::AlignRight,  minLabel);
                break;
            case TimeVertical_NowTop_MaxRight:
            case TimeVertical_NowBottom_MaxRight:
                flags = flags | (setup_y_labels_right_->value() ? Qt::AlignTop : Qt::AlignBottom);
                if(setup_show_y_max_->value()) addText(painter, image()->rect(), flags|Qt::AlignRight,  maxLabel);
                if(setup_show_y_min_->value()) addText(painter, image()->rect(), flags|Qt::AlignLeft,   minLabel);
                break;
            }
        }      

        painter.end();
    }
    if (isDebug) qDebug() << "graph: draw line: done " << lineID;
}

void LH_Graph::drawAll()
{
    addMissingConfigs();
    for (int i = 0; i < linesCount(); ++i)
    {
#ifdef LH_MONITORING_LIBRARY
        if( lineData_->at(i).group )
            continue;
        if( lineData_->at(i).hidden )
            continue;
#endif
        drawSingle(i);
    }
}

QString LH_Graph::getLabelText(qreal val)
{
    int prec = 0;
    QString units;
    val = adaptToUnits(val, &units, &prec);

    QString str = QString::number(val,'f',prec);
    while( str.contains(".") && ( str.endsWith("0") || str.endsWith(".") ) ) str.remove(str.length()-1,1);
    return str + units;
}

void LH_Graph::addText(QPainter& painter, QRect rect, int flags, QString text)
{
    if (isDebug) qDebug() << "graph: add label: begin ";
    painter.setPen( setup_label_shadow_->value() );
    addText(painter, rect, flags, text, 0 , 0);
    addText(painter, rect, flags, text, 1 , 0);
    addText(painter, rect, flags, text, 2 , 0);
    addText(painter, rect, flags, text, 0 , 1);
    addText(painter, rect, flags, text, 2 , 1);
    addText(painter, rect, flags, text, 0 , 2);
    addText(painter, rect, flags, text, 1 , 2);
    addText(painter, rect, flags, text, 2 , 2);

    painter.setPen( setup_label_color_->value() );
    addText(painter, rect, flags, text, 1 , 1);
    if (isDebug) qDebug() << "graph: add label: done ";
}

void LH_Graph::addText(QPainter& painter, QRect rect, int flags, QString text, int Xmod, int Ymod)
{
    QRect modRect = rect;
    modRect.setTopLeft(QPoint(modRect.left()+Xmod,modRect.top()+Ymod));
    modRect.setBottomRight(QPoint(modRect.right()+Xmod-2,modRect.bottom()+Ymod));
    painter.drawText( modRect, flags, text );
}

void LH_Graph::addLine(QString name)
{
    if (isDebug) qDebug() << "Add Line: " << name;

    setup_line_selection_->list().append(name);
    cacheCount_.append(0);
    cacheVal_.append(0);
    lines_->add(name);

    updateLinesList();
}

void LH_Graph::updateLinesList(bool fullResync)
{
    if(fullResync)
    {
        setup_line_selection_->list().clear();
        cacheCount_.clear();
        cacheVal_.clear();

        if (lines_) {
            for(int i=0; i<lines_->count(); i++)
            {
                setup_line_selection_->list().append(lines_->at(i).name);
                cacheCount_.append(0);
                cacheVal_.append(0);
            }
        }
    }

    setup_line_selection_->refreshList();

    addMissingConfigs();

    setup_line_selection_->setFlag(LH_FLAG_HIDDEN, setup_line_selection_->list().count()==1);
    setup_line_selection_->setValue(0);
}

void LH_Graph::addMissingConfigs()
{
    QStringList configs = setup_line_configs_->value().split('~',QString::SkipEmptyParts);
    if (configs.length()<lineConfigsCount())
    {
        QString configString = buildColorConfig();
        while(configs.length()<lineConfigsCount())
            configs.append(configString);
        setup_line_configs_->setValue(configs.join("~"));
    }
}

int LH_Graph::lineConfigsCount()
{
    return setup_line_selection_->list().count();
}


void LH_Graph::clearLines()
{
    setup_line_selection_->list().clear();
    setup_line_selection_->refreshList();
    cacheCount_.clear();
    cacheVal_.clear();

    if (lines_) {
        lines_->clear();
        if (dataMode_ == gdmHybrid)
        {
            int desired_duration = (setup_sample_rate_->value() * 1000);
            lines_->clear();
            lines_->copyFrom((*externalSource_).averageOver(desired_duration));
        }
    }

    setup_line_selection_->setFlag(LH_FLAG_HIDDEN, false);
}

bool LH_Graph::setMin( qreal r )
{
    if( min_val() == r ) return false;
    setup_min_->setValue(r);
    graphMinY_ = r;
    if(graphMaxY_==r) setMax(r+1);
    return true;
}

bool LH_Graph::setMax( qreal r, GraphBoundGrowState gbg )
{
    if(!userDefinableLimits_ && gbg != BoundGrowthDefault) setCanGrow( gbg == BoundGrowthCanGrow );
    return setMax(r);
}

bool LH_Graph::setMax( qreal r )
{
    if(graphMinY_==r) r++;
    if( max_val() == r ) return false;
    setup_max_->setValue(r);
    graphMaxY_ = r;
    return true;
}

void LH_Graph::setYUnit( QString str, qreal divisor )
{
    unitText_ = str;
    if (divisor!=0) divisorY_ = divisor;
}

QImage *LH_Graph::render_qimage( int w, int h )
{
    if(QImage *img = initImage(w, h))
    {
        if(setup_bgcolor_)
            img->fill(PREMUL(setup_bgcolor_->value().rgba()));
        return img;
    }
    return 0;
}

void LH_Graph::addValue(qreal value, int lineID)
{
    Q_ASSERT_X(dataMode_!=gdmExternallyManaged, "LH_Graph::addValue", "Cannot add a value to the internal cache if the data is managed externally.");

    if (!lines_ || lineID < 0 || lineID >= linesCount()) {
        qCritical("LH_Graph::addValue(%f, %d): lineData_ == %p (count() == %d), objectName() == \"%s\"",
                  value, lineID,
                  (const void*) lineData_, linesCount(),
                  qPrintable(objectName()));
        return;
    }

    if (isDebug) qDebug() << "Line Count (add value): " << lineData_->count() << " lines;";
    if (isDebug) qDebug() << "graph: add value: begin " << lineID;
    cacheCount_[lineID] ++;
    cacheVal_[lineID] += (qreal)value;
    if(cacheCount_[lineID] >= setup_sample_rate_->value())
    {
        lines_->operator[](lineID).addValue(cacheVal_[lineID] / cacheCount_[lineID], setup_sample_rate_->value() * 1000);
        cacheCount_[lineID] = 0;
        cacheVal_[lineID] = 0;
    }
    if (isDebug) qDebug() << "graph: add value: end ";
}

void LH_Graph::loadColors(int lineID, QColor& penColor, QColor& fillColor1, QColor& fillColor2, QString& fgImgPath, int& fgImgAlpha, bool compensateForHidden)
{
    Q_ASSERT_X(lineID!=-1, "LH_Graph::loadColors", "Illegal line id!");

    if (isDebug) qDebug() << "graph: load colours: begin " << lineID;
    QStringList configs = setup_line_configs_->value().split('~',QString::SkipEmptyParts);

    if( lineID < 0 ) lineID = 0;

#ifdef LH_MONITORING_LIBRARY
    if (compensateForHidden && lineData_)
    {
        int realLineID = lineID;
        lineID = 0;
        for(int i = 0; i<realLineID; i++)
            if(!lineData_->at(i).hidden)
                lineID++;
    }
#else
    Q_UNUSED(compensateForHidden);
#endif

    if( lineID >= configs.length() ) lineID = configs.length()-1;

    QString configString = configs.at(lineID);
    QStringList config = configString.split(',');

    penColor.setRed(QString(config.at(0)).toInt());
    penColor.setGreen(QString(config.at(1)).toInt());
    penColor.setBlue(QString(config.at(2)).toInt());
    penColor.setAlpha(QString(config.at(3)).toInt());

    fillColor1.setRed(QString(config.at(4)).toInt());
    fillColor1.setGreen(QString(config.at(5)).toInt());
    fillColor1.setBlue(QString(config.at(6)).toInt());
    fillColor1.setAlpha(QString(config.at(7)).toInt());

    fillColor2.setRed(QString(config.at(8)).toInt());
    fillColor2.setGreen(QString(config.at(9)).toInt());
    fillColor2.setBlue(QString(config.at(10)).toInt());
    fillColor2.setAlpha(QString(config.at(11)).toInt());

    if(config.length()>12)fgImgPath = config.at(12);
    if(config.length()>13)fgImgAlpha = config.at(13).toInt();

    if (isDebug) qDebug() << "graph: load colours: end ";
}

QString LH_Graph::buildColorConfig()
{
    QColor penColor = setup_pencolor_->value();
    QColor fillColor1 = setup_fillcolor1_->value();
    QColor fillColor2 = setup_fillcolor2_->value();

    QStringList config = QStringList();

    config.append(QString::number(penColor.red()));
    config.append(QString::number(penColor.green()));
    config.append(QString::number(penColor.blue()));
    config.append(QString::number(penColor.alpha()));

    config.append(QString::number(fillColor1.red()));
    config.append(QString::number(fillColor1.green()));
    config.append(QString::number(fillColor1.blue()));
    config.append(QString::number(fillColor1.alpha()));

    config.append(QString::number(fillColor2.red()));
    config.append(QString::number(fillColor2.green()));
    config.append(QString::number(fillColor2.blue()));
    config.append(QString::number(fillColor2.alpha()));

    config.append(setup_fg_image_->value().absoluteFilePath());
    config.append(QString::number(setup_fg_alpha_->value()));

    return config.join(",");
}

void LH_Graph::changeMaxSamples()
{
    Q_ASSERT(lines_);
    if (lines_)
        lines_->setLimit( setup_max_samples_->value() );
    updateDescText();
}

void LH_Graph::changeSampleRate()
{
    updateDescText();
}

void LH_Graph::updateDescText()
{
    int sTotal = setup_max_samples_->value() * setup_sample_rate_->value();
    int s = sTotal % 60;
    int m = ((sTotal - s)/60) % 60;
    int h = ((sTotal - s)/60 - m) / 60;

    QString strH = QString("%1").arg(h);
    if (strH.length()==1) strH = "0" + strH;
    QString strM = QString("%1").arg(m);
    if (strM.length()==1) strM = "0" + strM;
    QString strS = QString("%1").arg(s);
    if (strS.length()==1) strS = "0" + strS;

    QString str1 = QString(" timespan (%1 samples @ ").arg(setup_max_samples_->value());
    QString str2 = QString("%1s intervals)").arg(setup_sample_rate_->value());

    setup_description_->setValue( strH + ":" +strM + ":" + strS + str1 + str2 );
}

void LH_Graph::changeType()
{
    setup_fillcolor1_->setFlag(LH_FLAG_HIDDEN, (setup_fg_type_->value()!=1));
    setup_fillcolor2_->setFlag(LH_FLAG_HIDDEN, (setup_fg_type_->value()!=1));
    setup_fg_image_->setFlag(LH_FLAG_HIDDEN, (setup_fg_type_->value()!=2));
    setup_fg_alpha_->setFlag(LH_FLAG_HIDDEN, (setup_fg_type_->value()!=2));

    setup_bgcolor_->setFlag(LH_FLAG_HIDDEN, (setup_bg_type_->value()!=1));
    setup_bg_image_->setFlag(LH_FLAG_HIDDEN, (setup_bg_type_->value()!=2));
}

void LH_Graph::changeSelectedLine()
{
    QColor penColor = QColor();
    QColor fillColor1 = QColor();
    QColor fillColor2 = QColor();
    QString fgImgPath = "";
    int fgImgAlpha = 255;

    if (setup_line_selection_->value() >= lineConfigsCount()) setup_line_selection_->setValue(lineConfigsCount()-1);
    if (setup_line_selection_->value() < 0) setup_line_selection_->setValue(0);

    if (setup_line_selection_->value() >= 0)
    {
        loadColors(setup_line_selection_->value(), penColor, fillColor1, fillColor2, fgImgPath, fgImgAlpha, false);

        setup_pencolor_->setValue(penColor);
        setup_fillcolor1_->setValue(fillColor1);
        setup_fillcolor2_->setValue(fillColor2);
        setup_fg_image_->setValue(QFileInfo(fgImgPath));
        setup_fg_alpha_->setValue(fgImgAlpha);
    }
}

void LH_Graph::updateSelectedLine()
{
    QStringList configs = setup_line_configs_->value().split('~',QString::SkipEmptyParts);

    int lineID = setup_line_selection_->value();
    if( lineID >= 0 && lineID < configs.length())
    {
        configs.replace(lineID, buildColorConfig());
        setup_line_configs_->setValue(configs.join("~"));
    }
}

void LH_Graph::updateLabelSelection()
{
    setup_hide_when_empty_->setFlag(LH_FLAG_HIDDEN, !(setup_show_y_max_->value() | setup_show_y_min_->value()));
    setup_show_real_limits_->setFlag(LH_FLAG_HIDDEN, !(setup_show_y_max_->value() | setup_show_y_min_->value()));
    setup_y_labels_right_->setFlag(LH_FLAG_HIDDEN, !(setup_show_y_max_->value() | setup_show_y_min_->value()));
    setup_label_font_->setFlag(LH_FLAG_HIDDEN, !(setup_show_y_max_->value() | setup_show_y_min_->value()));
    setup_label_color_->setFlag(LH_FLAG_HIDDEN, !(setup_show_y_max_->value() | setup_show_y_min_->value()));
    setup_label_shadow_->setFlag(LH_FLAG_HIDDEN, !(setup_show_y_max_->value() | setup_show_y_min_->value()));
}

void LH_Graph::clear(qreal newMin, qreal newMax, bool newGrow)
{
    if(lines_ && dataMode_!=gdmExternallyManaged)
        for(int lineID=0;lineID<linesCount(); lineID++)
            lines_->operator[](lineID).clear();
    graphMinY_ = graphMaxY_ = 0.0;
    setup_min_->setValue(newMin);
    setup_max_->setValue(newMax);
    setCanGrow(newGrow);
}

bool LH_Graph::userDefinableLimits()
{
    return userDefinableLimits_;
}

bool LH_Graph::setUserDefinableLimits(bool v)
{
    if(userDefinableLimits_!=v)
    {
        userDefinableLimits_ = v;
        updateLimitControls();
        return true;
    }
    else
        return false;
}

void LH_Graph::updateLimitControls()
{
    setup_max_grow_->setFlag(LH_FLAG_HIDDEN, !userDefinableLimits_);
    setup_max_grow_->setFlag(LH_FLAG_READONLY, !userDefinableLimits_);
    setup_max_->setFlag(LH_FLAG_HIDDEN, !userDefinableLimits_);
    setup_max_->setFlag(LH_FLAG_READONLY, !userDefinableLimits_ || canGrow());
    setup_min_->setFlag(LH_FLAG_HIDDEN, !userDefinableLimits_);
    setup_min_->setFlag(LH_FLAG_READONLY, !userDefinableLimits_);
}

void LH_Graph::updateFGImage()
{
    updateSelectedLine();
    reload_images();
}

void LH_Graph::updateBGImage()
{
    reload_images();
}

void LH_Graph::reload_images()
{
    if(!hasImage())
        return;

    int w = image()->width();
    int h = image()->height();

    img_size_.setHeight(h);
    img_size_.setWidth(w);

    if(setup_bg_image_->value().isFile())
        bgImg_ = QImage(setup_bg_image_->value().absoluteFilePath()).scaled(w,h);

    for(int lineID=0;lineID<linesCount(); lineID++)
    {
        QColor penColor = QColor();
        QColor fillColor1 = QColor();
        QColor fillColor2 = QColor();
        QString fgImgPath = "";
        int fgImgAlpha = 255;

        loadColors(lineID, penColor, fillColor1, fillColor2, fgImgPath, fgImgAlpha, true);

        fgImgs_.remove(lineID);
        if(QFileInfo(fgImgPath).isFile())
            fgImgs_.insert(lineID, QImage(fgImgPath).scaled(w,h));
    }
}
