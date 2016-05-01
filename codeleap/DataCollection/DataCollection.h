/**
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  @legalese Copyright (c) 2010-2011 Andy Bridges

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

#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H

#include <QList>
#include <QStringList>
#include <QDebug>
#include <qmath.h>

typedef struct {qreal value; int duration;} DataPoint;

class DataPointCollection : public QList<DataPoint> {
public:
    DataPointCollection() : QList<DataPoint>() {}
};

class DataLine : public DataPointCollection
{
    friend class DataLineCollection;

    int limit_;
    int totalDuration_;
    int popDuration_;

    void setPopDuration(int popDuration) { popDuration_ = popDuration;}

    void copyFrom(DataLine src)
    {
        limit_ = src.limit_;
        totalDuration_ = src.totalDuration_;
        popDuration_ = src.popDuration_;
        name = src.name;

        //qDebug() << src.count();
        for(int i = 0; i<src.count(); i++)
            append( src[i] );
    }

#ifndef MONITORING_CLASS
    DataLine() : DataPointCollection()
    {
        name = "Unnamed";
        limit_ = 0;
        totalDuration_ = 0;
        popDuration_ = 0;
    }
#endif

public:

#ifdef MONITORING_CLASS
    QString units;
    QStringList adaptiveUnitsList;
    qreal adaptiveUnitsFactor;
    bool aggregate;
    bool group;
    bool hidden;
    DataLine() : DataPointCollection()
    {
        name = "Unnamed";
        limit_ = DATA_CACHE_MAX_POINTS;
        totalDuration_ = 0;
        popDuration_ = 0;
    }

    double adaptToUnits(qreal val, bool autoAdapt, QString sourceUnits, QString desiredUnits, QString* units = 0, qreal *logDivisor = 0)
    {
        int sourcePos = adaptiveUnitsList.indexOf(sourceUnits);
        if(units) *units = sourceUnits;

        if(logDivisor) *logDivisor = 0;
        if(!autoAdapt)
        {
            int desiredPos = adaptiveUnitsList.indexOf(desiredUnits);
            if(units) *units = desiredUnits;
            if(desiredPos > sourcePos)
                for(int i = sourcePos; i < desiredPos; i++)
                {
                    val /= adaptiveUnitsFactor;
                    if(logDivisor) *logDivisor -= log10(adaptiveUnitsFactor);
                }
            if(desiredPos < sourcePos)
                for(int i = sourcePos; i > desiredPos; i--)
                {
                    val *= adaptiveUnitsFactor;
                    if(logDivisor) *logDivisor += log10(adaptiveUnitsFactor);
                }
        }
        else
        {
            for(int i = sourcePos-1; i >= 0 && val < 1; i--)
            {
                val *= adaptiveUnitsFactor;
                if(logDivisor) *logDivisor += log10(adaptiveUnitsFactor);
                if(units) *units = adaptiveUnitsList[i];
            }
            while (sourcePos + 1 < adaptiveUnitsList.count() && val >= adaptiveUnitsFactor) {
                ++sourcePos;
                val /= adaptiveUnitsFactor;
                if(logDivisor) *logDivisor -= log10(adaptiveUnitsFactor);
                if(units) *units = adaptiveUnitsList[sourcePos];
            }
        }
        return val;
    }

#endif

    QString name;

    DataLine(QString name, int limit) : DataPointCollection()
    {
        this->name = name;
        limit_ = limit;
        totalDuration_ = 0;
        popDuration_ = 0;
    }

    int limit() { return limit_; }

    qreal averageDuration()
    {
        if(length()==0)
            return 0;
        else
            return ((qreal)totalDuration_)/(length());
    }

    qreal predictedTotalDuration()
    {
        return averageDuration() * limit_;
    }

    void setLimit(int limit)
    {
        limit_ = limit;
        while(limit_<length())
            removeLast();
    }

    void clear()
    {
        DataPointCollection::clear();
        totalDuration_ = 0;
    }

    void addValue(qreal value, int duration)
    {
        Q_ASSERT_X(limit_!=0, "DataLineCollection::addValue", "This instance of DataLineCollection has a limit of zero!" );
        if (length()>=limit_)
        {
            int popped_duration = 0;
            bool popped_once = false;
            //if(popDuration_ != 0) qDebug() << "popDuration_: " << popDuration_;
            while( (!popped_once) || (popped_duration + last().duration) <= popDuration_ )
            {
                popped_once = true;
                popped_duration += last().duration;
                totalDuration_ -= last().duration;
                pop_back();
                //if(popDuration_ != 0) qDebug() << "pop";
                if(length()==0)
                    break;
            }
            //if(popDuration_ != 0) qDebug() << "popped: " << popped_duration;
        }
        DataPoint _tmp = {value, duration};
        push_front( _tmp );
        totalDuration_ += duration;
    }
};

class DataLineCollection : private QList<DataLine>
{
    int limit_;

private:
    void setNames(QStringList names)
    {
        this->clear();
        for(int i = 0; i < names.count(); i++)
            add(names[i]);
    }

public:
    DataLineCollection() : QList<DataLine>() {
#ifdef DATA_CACHE_MAX_POINTS
        limit_ = DATA_CACHE_MAX_POINTS;
#else
        limit_ = 0;
#endif
    }
    DataLineCollection(int limit) : QList<DataLine>() {
        limit_ = limit;
    }

#ifdef MONITORING_CLASS
    DataLine &operator[](QString name)
    {
        for(int i = 0; i<this->count(); i++)
            if(QList<DataLine>::operator [](i).name == name)
                return QList<DataLine>::operator [](i);
        Q_ASSERT_X(false,"DataLineCollection","List out of bounds");
        return QList<DataLine>::operator [](-1);
    }

    bool contains(QString name)
    {
        for(int i = 0; i<this->count(); i++)
            if(QList<DataLine>::operator [](i).name == name)
                return true;
        return false;
    }

    int indexOf(QString name)
    {
        for(int i = 0; i<this->count(); i++)
            if(QList<DataLine>::operator [](i).name == name)
                return i;
        return -1;
    }

#endif

    //const DataLine &operator[](int i) const;
    DataLine &operator[](int i)
    {
        Q_ASSERT_X(i >= 0 && i < this->count(), "DataLineCollection::operator[]", QString("index (%1) out of range (0-%2)").arg(i).arg(this->count()-1).toLocal8Bit());
        return  QList<DataLine>::operator [](i);
    }
    const DataLine &operator[](int i) const
    {
        return  QList<DataLine>::operator [](i);
    }
    const DataLine at(int i) const
    {
        return QList<DataLine>::at(i);
    }

    void clear()
    {
        QList<DataLine>::clear();
    }

    int limit() { return limit_; }

    int pointLength()
    {
        return (length()==0? 0 : (*this)[0].length());
    }

    int count() const
    {
        return QList<DataLine>::count();
    }

    void add(QString name)
    {
        QList<DataLine>::append( DataLine(name, limit_) );
    }

    void append(DataLine dl)
    {
        QList<DataLine>::append( dl );
    }

    void setCount(int c, QString nameTemplate = "line %1")
    {
        this->clear();
        for(int i = 0; i < c; i++)
            add(nameTemplate.arg(i));
    }

    QStringList names()
    {
        QStringList names;
        for(int i = 0; i < count(); i++)
            names.append(this->at(i).name);
        return names;
    }

    void setLimit(int limit)
    {
        limit_ = limit;
        for(int i =0; i<length(); i++ )
            (*this)[i].setLimit(limit);
    }

    DataLineCollection averageOver(int desiredDuration)
    {
        DataLineCollection dlc;
        dlc.setLimit(limit_);
        dlc.setNames(this->names());
        //qDebug() << "averageOver " << desiredDuration << " ms";
        for(int lineIndex = 0; lineIndex < dlc.count(); lineIndex++)
        {
            QList<DataLine>::operator [](lineIndex).setPopDuration(desiredDuration);

            DataLine dl = this->at(lineIndex);

            qreal value = 0;
            int duration = 0;
            int count = 0;
            for(int i = dl.length() - 1; i >=0; i--)
            {
                value += dl.at(i).value;
                duration += dl.at(i).duration;
                count ++;

                if(duration == desiredDuration || (i > 0 && duration + dl.at(i-1).duration > desiredDuration))
                {
                    dlc[lineIndex].addValue(value / count, desiredDuration);
                    value = 0;
                    duration = 0;
                    count = 0;
                }
            }
        }
        return dlc;
    }

    void copyFrom(DataLineCollection src)
    {
        limit_ = src.limit();
        //setNames(src.names());
        for(int lineIndex = 0; lineIndex < src.count(); lineIndex++)
        {
            DataLine dl;
            dl.copyFrom(src[lineIndex]);
            append( dl );
        }
    }



    class const_iterator
    {
    private:
        int index_;
        const DataLineCollection *dlc_;
    public:
        enum etype{START,END};
        const_iterator(const DataLineCollection *dlc, etype t)
        {
            dlc_ = dlc;
            index_ = (t==START? 0 : dlc_->count());
        }
        const DataLine &operator*() const { return (*dlc_)[index_]; }
        bool operator!=(const const_iterator& it) { return (index_!=it.index_); }
        const_iterator& operator++() { index_++; return *this; }
    };
    const_iterator begin() const { return const_iterator(this, const_iterator::START); }
    const_iterator end() const { return const_iterator(this, const_iterator::END); }
}
;

#endif // DATACOLLECTION_H
