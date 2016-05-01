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

#ifndef LH_GRAPH_H
#define LH_GRAPH_H

#include "LH_QtInstance.h"
#include "LH_Qt_QColor.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_int.h"
#include "LH_Qt_float.h"
#include "LH_Qt_QTextEdit.h"
#include "LH_Qt_QFont.h"
#include "LH_Qt_QFileInfo.h"
#include "QList"
#include "QHash"
#include "qmath.h"

#include "DataCollection.h"


#ifdef LH_GRAPH_LIBRARY
# define LH_GRAPH_EXPORT Q_DECL_EXPORT
#else
# define LH_GRAPH_EXPORT Q_DECL_IMPORT
#endif

enum GraphDataMode
{
    /* Internally Managed Data means the graph uses it's own internal data store.   *
     * Data points are added to the store (usually via a decendant class) using the *
     * "addValue" routine.                                                          */
    gdmInternallyManaged,

    /* Externally Managed Data means the graph makes no use of it's internal store. *
     * Instead, some 3rd party (possibly a decendant, but not necessarily) is going *
     * to maintain it's own DataLineCollection object which the graph will make use *
     * of.                                                                          */
    gdmExternallyManaged,

    /* "Hybrid" management exists where a 3rd party will be maintaining it's own    *
     * record of data values (in a DataLineCollection object), which will be copied *
     * into the graph object when the data is initialised. However, once this has   *
     * been done, the graph will maintain it's own collection and changes to the    *
     * 3rd party collection will be ignored.                                         */
    gdmHybrid
};

enum GraphBoundGrowState
{
    BoundGrowthDefault = 0,
    BoundGrowthFixed = 1,
    BoundGrowthCanGrow = 2
};

class LH_GRAPH_EXPORT LH_Graph : public LH_QtInstance {
    Q_OBJECT

    const static bool isDebug = false;

    qreal dataMaxY_;
    qreal dataMinY_;
    qreal dataDeltaY_;
    qreal graphMaxY_;
    qreal graphMinY_;
    qreal divisorY_;
    bool userDefinableLimits_;
    bool hasDeadValue_;
    qreal deadValue_;
    GraphDataMode dataMode_;

    QString unitText_;
    QSize img_size_;
    QImage bgImg_;
    QHash<int,QImage> fgImgs_;
    QVector<int> cacheCount_;
    QVector<qreal> cacheVal_;

    DataLineCollection* lines_;
    DataLineCollection* externalSource_;
    DataLineCollection* lineData_;

    void reload_images();
    void findDataBounds(DataLineCollection* lineData);
    int lastVisibleLine();
    void addMissingConfigs();

protected:
    bool graph_empty_;

    LH_Qt_QStringList *setup_fg_type_;
    LH_Qt_QStringList *setup_bg_type_;
    LH_Qt_QStringList *setup_orientation_;
    LH_Qt_QStringList *setup_line_selection_;
    LH_Qt_QColor *setup_pencolor_;
    LH_Qt_QColor *setup_fillcolor1_;
    LH_Qt_QColor *setup_fillcolor2_;
    LH_Qt_QTextEdit *setup_line_configs_;
    LH_Qt_QColor *setup_bgcolor_;
    LH_Qt_int *setup_max_samples_;
    LH_Qt_int *setup_sample_rate_;
    LH_Qt_QString *setup_description_;

    LH_Qt_bool *setup_max_grow_;
    LH_Qt_float *setup_max_;
    LH_Qt_float *setup_min_;

    LH_Qt_bool *setup_auto_scale_y_max_;
    LH_Qt_bool *setup_auto_scale_y_min_;
    LH_Qt_bool *setup_show_y_max_;
    LH_Qt_bool *setup_show_y_min_;
    LH_Qt_bool *setup_hide_when_empty_;
    LH_Qt_bool *setup_show_real_limits_;
    LH_Qt_bool *setup_y_labels_right_;
    LH_Qt_QFont *setup_label_font_;
    LH_Qt_QColor *setup_label_color_;
    LH_Qt_QColor *setup_label_shadow_;

    LH_Qt_QFileInfo *setup_fg_image_;
    LH_Qt_QFileInfo *setup_bg_image_;
    LH_Qt_int *setup_fg_alpha_;

    // void __ctor( qreal defaultMin, qreal defaultMax, GraphDataMode dataMode, DataLineCollection* externalSource );

    void setExternalSource(DataLineCollection* externalSource);
    DataLineCollection* externalSource() const { return externalSource_;}
    DataLineCollection* lineData() const { return lineData_; }
    qreal dataDeltaY() const { return dataDeltaY_; }
    bool setMax( qreal r ); // return true if rendering needed

public:
    explicit LH_Graph(GraphDataMode dataMode = gdmInternallyManaged, DataLineCollection* externalSource = 0, LH_QtObject *parent = 0);
    virtual ~LH_Graph();

    const char* userInit();
/*
    LH_Graph( GraphDataMode dataMode, DataLineCollection* externalSource, LH_QtObject *parent = 0)
        : lines_(30)
        , lineData_(&lines_)
    { __ctor(0, 0, dataMode, externalSource); }

    LH_Graph( qreal defaultMin, qreal defaultMax, GraphDataMode dataMode, DataLineCollection* externalSource )
        : lines_(30)
        , lineData_(&lines_)
    { __ctor(defaultMin, defaultMax, dataMode, externalSource); }
*/
    QImage *render_qimage( int w, int h );

    qreal max_val();
    qreal min_val();
    void setCanGrow(bool);
    bool canGrow();

    bool setMin( qreal r ); // return true if rendering needed
    bool setMax( qreal r, GraphBoundGrowState b ); // return true if rendering needed

    void addLine(QString name);
    void updateLinesList(bool fullResync = false);
    int linesCount() const { return lineData_ ? lineData_->count() : 0; }
    int lineConfigsCount();
    void clearLines();
    void setLines(QStringList names) {clearLines(); for(int i=0; i<names.length(); i++) addLine(names[i]);}


    void setYUnit( QString str, qreal divisor = 1);

    void drawSingle(int lineID = 0);
    void drawAll();

    void addValue(qreal value, int lineID = 0);
    void addValues(QVector<qreal> values ) { for( int i=0; i<values.size(); ++i ) addValue( values.at(i), i ); }

    void loadColors(int lineID, QColor& penColor, QColor& fillColor1, QColor& fillColor2, QString& fgImgPath, int& fgImgAlpha, bool compensateForHidden);
    QString buildColorConfig();

    //void findDataBounds();
    void addText(QPainter& painter, QRect rect, int flags, QString text);
    void addText(QPainter& painter, QRect rect, int flags, QString text, int Xmod, int Ymod);
    QString getLabelText(qreal val);

    void clear(qreal newMin=0, qreal newMax=1, bool newGrow = true);

    bool userDefinableLimits();
    bool setUserDefinableLimits(bool v);

    GraphDataMode dataMode() { return dataMode_; }

    void setDeadValue(qreal v){
        hasDeadValue_ = true;
        deadValue_ = v;
    }

    virtual qreal adaptToUnits(qreal val, QString *units = 0, int *prec = 0)
    {
        if(units)
            *units = unitText_;
        if(prec)
        {
            *prec = 1 - int(log10(dataDeltaY_/ divisorY_));
            if (*prec<0)
                *prec = 0;
        }
        return val / divisorY_;
    }

public slots:
    void changeMaxSamples();
    void changeSampleRate();
    void changeType();
    void changeSelectedLine();
    void updateSelectedLine();
    void updateLabelSelection();
    void updateLimitControls();
    void updateDescText();
    void updateFGImage();
    void updateBGImage();
};

#endif // LH_GRAPH_H
