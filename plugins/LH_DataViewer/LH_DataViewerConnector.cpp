/**
  \file     LH_DataViewerConnector.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andy Bridges
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

#include "LH_DataViewerConnector.h"
#include <QDebug>
#include <QPainter>
#include <QStringList>
#include <cmath>
#include <QtXml>


#include <sstream>

LH_PLUGIN_CLASS(LH_DataViewerConnector)

#define DEBUG_MESSAGES

lh_class *LH_DataViewerConnector::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Dynamic/DataViewer",
        "DataViewerConnector",
        "Data Connector",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };
#if 0
    if( classInfo.width == -1 )
    {
        QFont arial10("Arial",10);
        QFontMetrics fm( arial10 );
        classInfo.height = fm.height();
        classInfo.width = classInfo.height * 4;
    }
#endif
    return &classInfo;
}

LH_DataViewerConnector::LH_DataViewerConnector()
{
    dataExpiry_ = 0;
    repolled_ = false;
    watchPath_ = "";
    processName_ = "";
    processVersion_ = "";
    needsClearing_ = false;
}

const char *LH_DataViewerConnector::userInit(){
    LH_QtInstance::userInit();
    hide();

    setup_feedback_ = new LH_Qt_QString(this, "Feedback", "", LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    QStringList langs = listLanguages();
    qDebug() << "languages: " << langs.count() << ": " << langs.join(",");
    setup_language_ = new LH_Qt_QStringList(this,"Language",langs, LH_FLAG_AUTORENDER | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_language_->setHelp("To add a new language copy the \"[List:<name>]\" blocks from the data map into a new file called lists.XX.txt (where XX is the language code, e.g. Spanish lists would go in a file called lists.ES.txt, French in lists.FR.txt) and translate the text values into the desired language.");
    setup_language_->setFlag(LH_FLAG_HIDDEN, langs.length()<=1);
    connect( setup_language_, SIGNAL(changed()), this, SLOT(languageFileChanged()) );

    setup_map_file_ = new LH_Qt_QFileInfo(this,"Data Map",QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE );
    setup_map_file_->setHelp("The data map file contains information needed to understand and parse the data source.<br><br>"
                             "(Unlike text files, XML data sources do not require a map file, although they do support them if advanced data parsing or data-expiriation is required.)");
    connect( setup_map_file_, SIGNAL(changed()), this, SLOT(mapFileChanged()) );

    setup_data_file_ = new LH_Qt_QFileInfo(this,"Data Source",QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE );
    setup_data_file_ ->setHelp("The data source. Only XML data sources can be understood without a data map. If the datamap specifies \"MEM\" as the type then no source file is required.");
    connect( setup_data_file_, SIGNAL(changed()), this, SLOT(sourceFileChanged()) );

    sourceWatcher_ = new QFileSystemWatcher(this);
    connect( sourceWatcher_, SIGNAL(fileChanged(const QString)), this, SLOT(sourceFileUpdated(const QString)) );

    rootNode = new dataNode();
    sharedData = new sharedCollection();

    connect( this, SIGNAL(initialized()), this, SLOT(sourceFileUpdated()) );

    return NULL;
}

int LH_DataViewerConnector::polling()
{
    if(sourceType_ == SOURCETYPE_MEM)
    {
        if(readMemoryValues())
            return 200;
        else
            return 1000;
    }
    else
        return 0;
}

QString LH_DataViewerConnector::get_dir_layout()
{
    return QString::fromUtf8( state()->dir_layout );
}

QStringList LH_DataViewerConnector::listLanguages()
{
    QStringList languages = QStringList();
    languages.append("Default");

    QDir layoutDir = QDir(get_dir_layout());
    QStringList filters;
    filters << "lists.*.txt";
    layoutDir.setNameFilters(filters);
    QFileInfoList langFiles = layoutDir.entryInfoList(QDir::Files);

    QRegExp rxList("lists\\.(.*)\\.txt");
    foreach(QFileInfo f, langFiles)
    {
        QString fileName = f.fileName();
        fileName = fileName.replace(rxList,"\\1");
        languages.append(fileName);
    }
    return languages;
}

LH_DataViewerConnector::~LH_DataViewerConnector()
{
    delete rootNode;
    rootNode = 0;
    delete sharedData;
    sharedData = 0;
    return ;
}

void LH_DataViewerConnector::populateValues(dataNode* node, QStringList sourceLines)
{
    foreach(QString key, node->keys())
    {
        foreach(dataNode* childNode, node->child(key))
        {
            if(childNode->hasData())
                childNode->setValue( getTextValue(sourceLines, childNode->definition()) );
            if(childNode->hasChildren())
                populateValues(childNode, sourceLines);
        }
    }
}

QString LH_DataViewerConnector::getTextValue(QStringList lines, itemDefinition def)
{
    if (def.y<lines.count())
    {
        QString dataLine = lines[def.y];
        QStringList dataItems;
        if (isDelimited_)
            dataItems = dataLine.split(delimiter_);
        else
            dataItems = splitByWidth(dataLine, columnWidth_);

        if (def.x<dataItems.count())
            return formatData(dataItems[def.x].trimmed(), def.formatting);
        else
            return "";
    }
    else
        return "";
}

QString LH_DataViewerConnector::formatData(QString data, QString formatting)
{
    QString result = data;
    QRegExp rxList("list:(.*),(.*)");
    if (formatting!="")
    {
        if (formatting.startsWith("rx:"))
        {
            QRegExp rx = QRegExp(formatting.mid(3));
            if(rx.captureCount()!=0)
                result = result.replace(rx,"\\1").trimmed();
            else
                result = result.remove(rx).trimmed();
        } else
        if (rxList.indexIn(formatting)!=-1)
        {
            QString segmentName = rxList.cap(1).toLower();
            int listIndex    = rxList.cap(2).toInt();
            qreal levelVal   = result.toFloat();

            if(!lists_.contains(segmentName))
                qWarning()<<"List " << segmentName << " not found!";
            else
            {
                thresholdList* list = lists_.value(segmentName);
                //qDebug()<<"List " << segmentName << " : " << list->levels.count();
                for(int i=0; i<list->levels.count(); i++)
                {
                    //qDebug() << levelVal << " vs " <<  list->levels.at(i).levelBase << " (" << list->levels.at(i).levelNames.at(listIndex).trimmed() << ")";
                    if(levelVal < list->levels.at(i).levelBase && !list->levels.at(i).adaptiveBase) break;
                    if(listIndex<list->levels.at(i).levelNames.count())
                        result = list->levels.at(i).levelNames.at(listIndex).trimmed();
                    else
                        result = "ERROR: Invalid list column!";
                }
            }
        } else
        if (formatting == "seconds>time")
        {
            int s = result.toInt() ;
            int d = floor( (double)s / (24 *60 *60));
            QTime tm = QTime::fromString("00:00:00.000", "HH:mm:ss.zzz");
            result = QString("%1 day%2 %3").arg(d).arg(d!=1? "s" : "").arg(tm.addSecs(s%(24 *60 *60)).toString("HH:mm:ss"));
        }
    }
    return result;
}

QStringList LH_DataViewerConnector::splitByWidth(QString str, int w)
{
    QStringList list;
    while (str!="")
    {
        if(str.length()>w)
        {
            list.append(str.left(w));
            str = str.right(str.length()-w);
        } else {
            list.append(str);
            str = "";
            break;
        }
    }
    return list;
}

void LH_DataViewerConnector::sourceFileChanged()
{
    if(watchPath_!="") sourceWatcher_->removePath(watchPath_);
    if( !setup_data_file_->value().isFile() )
    {
        watchPath_ = "";
        setup_feedback_->setValue("No such source file.");
    }
    else
    {
        setup_feedback_->setValue("Connected to Data");
        watchPath_ = setup_data_file_->value().absoluteFilePath();
#ifdef DEBUG_MESSAGES
        qDebug() << "Source File Changed: watching source file: " << watchPath_;
#endif

        if( !setup_map_file_->value().isFile() && watchPath_.trimmed().toLower().endsWith(".xml"))
        {
            sourceType_ = SOURCETYPE_XML;
            isDelimited_ = false;
            isSingleWrite_ = true;
            dataExpiry_ = 0;
        }

        sourceWatcher_->addPath(watchPath_);
        sourceFileUpdated(watchPath_);
        setup_map_file_->setHidden(sourceType_ == SOURCETYPE_MEM);
    }
}

MemoryDataType LH_DataViewerConnector::ToMemType(QString s)
{
    if(s.contains(QRegExp("^\\s*(4\\s?Byte|Int(eger)?)\\s*$",Qt::CaseInsensitive)))
        return MEMTYPE_4BYTE;
    if(s.contains(QRegExp("^\\s*(8\\s?Byte|Long(\\s?Long))\\s*$",Qt::CaseInsensitive)))
        return MEMTYPE_8BYTE;
    if(s.contains(QRegExp("^\\s*(Float)\\s*$",Qt::CaseInsensitive)))
        return MEMTYPE_FLOAT;
    if(s.contains(QRegExp("^\\s*(Double)\\s*$",Qt::CaseInsensitive)))
        return MEMTYPE_DOUBLE;
    if(s.contains(QRegExp("^\\s*(Text([-_]?\\s*U(ni(code)?)?)?)\\s*$",Qt::CaseInsensitive)))
        return MEMTYPE_TEXT_UNICODE;
    if(s.contains(QRegExp("^\\s*(Text([-_]?\\s*A(sc(ii)?)?)?)\\s*$",Qt::CaseInsensitive)))
        return MEMTYPE_TEXT_ASCII;
    qWarning() << QString("LH_DataViewer: \"%1\" is not recognised as a data type").arg(s);
    return MEMTYPE_NONE;
}

void LH_DataViewerConnector::mapFileChanged()
{
    setup_map_file_->value().refresh();
    if( !setup_map_file_->value().isFile() )
    {
        setup_feedback_->setValue("No such map file.");
        return;
    }
    else
    {
        setup_feedback_->setValue("Connected to Data");
        QFile file( setup_map_file_->value().filePath() );

        if( file.open( QIODevice::ReadOnly) )
        {
            dataNode *currentNode = rootNode;
            QTextStream stream(&file);
            QRegExp rx = QRegExp(";.*$");
            QString fileContent = stream.readAll();

            QStringList items = fileContent.split('\r',QString::SkipEmptyParts);

            itemDefinitions_.clear();
            lists_.clear();
            QString segmentName="";
            QString segment = "";
            parsingList.clear();

            sourceType_ = SOURCETYPE_TXT;
            isDelimited_ = true;
            delimiter_ = ':';
            columnWidth_ = 0;
            isSingleWrite_ = true;
            completeCount_ = 1;
            dataExpiry_ = 0;
            updateLength_ = 0;

            foreach (QString item, items)
            {
                QString removedPart = item.right(item.length() - item.remove(rx).trimmed().length() - 1);
                item = item.remove(rx).trimmed();
                if (item!="")
                {
                    if(item.toLower()=="[[end]]")
                        break;
                    else
                    if(item.startsWith("["))
                    {
                        segment = item.toLower();
                        if(segment.startsWith("[list:"))
                        {
                            segmentName = QString(segment).replace(QRegExp("\\[list:(.*)\\]") , "\\1" );
                            if (!lists_.contains(segmentName)) lists_.insert(segmentName, new thresholdList() );
                        }
                        if(segment.startsWith("[definition:"))
                        {
                            segmentName = QString(segment).replace(QRegExp("\\[definition:(.*)\\]") , "\\1" );
                            currentNode = currentNode->addChild(segmentName);
                        }
                        if(segment.startsWith("[/definition:"))
                        {
                            segmentName = QString(segment).replace(QRegExp("\\[definition:(.*)\\]") , "\\1" );
                            currentNode = currentNode->parentNode();
                            if(currentNode->name()=="")
                                segment="";
                            else
                                segment = QString("[definition:%1]").arg(currentNode->name());
                        }
                    } else
                    if(segment=="[format]")
                    {
                        QString property = item.section('=',0,0).trimmed().toLower();
                        QString value = item.section('=',1,-1).trimmed();
                        if(property=="type")
                            sourceType_ = (value.toLower()=="xml"? SOURCETYPE_XML : (value.toLower()=="mem"? SOURCETYPE_MEM : (value.toLower()=="ini"? SOURCETYPE_INI : SOURCETYPE_TXT))); else
                        if(property=="delimited")                        
                            isDelimited_ = (value.toLower()=="true" || value.toLower()=="yes" || value.toLower()=="1"); else
                        if(property=="delimiter")
                        {
                            value += removedPart;
                            if (value.at(0) == '\\') {
                                if (value.at(1) == 't') delimiter_= '\t'; else
                                if (value.at(1) == ';') delimiter_= ';'; else
                                if (value.at(1) == '\\') delimiter_= '\\'; else
                                    delimiter_ = value.at(1).toLatin1();
                            } else
                                delimiter_ = value.at(0).toLatin1();
                        } else
                        if(property=="width")
                            columnWidth_ = value.toInt(); else
                        if(property=="singlewrite")
                            isSingleWrite_ = (value.toLower()=="true" || value.toLower()=="yes" || value.toLower()=="1"); else
                        if(property=="completeat")
                            completeCount_ = value.toInt();
                        if(property=="dataexpiry")
                            dataExpiry_ = value.toInt();
                        if(property=="language")
                        {
                            setup_language_->list()[0] = QString("%1 (Default)").arg(value);
                            setup_language_->refreshList();
                        }
                        if(property=="updatelength")
                            updateLength_ = value.toInt();
                    } else
                    if(segment.startsWith("[list:"))
                    {
                        thresholdItem newLevel;
                        //qDebug() << "Add list entry : " << item;
                        newLevel.levelNames = item.split('\t',QString::SkipEmptyParts);
                        newLevel.levelBase = newLevel.levelNames.at(0).trimmed().toFloat();
                        lists_.value(segmentName)->levels.append(newLevel);
                    } else
                    if(segment=="[process]")
                    {
                        QString property = item.section('=',0,0).trimmed().toLower();
                        QString value = item.section('=',1,-1).trimmed();
                        if(property=="name")
                            processName_ = value;
                        if(property=="version")
                            processVersion_ = value;
                    } else
                    if(segment.startsWith("[definition:"))
                    {
                        QStringList parts = item.split('\t',QString::SkipEmptyParts);
                        if((parts.count()>=2 && sourceType_ == SOURCETYPE_TXT) || (parts.count()>=3 && sourceType_ == SOURCETYPE_MEM))
                        {
                            QRegExp rx("^\"(.*)\"$");
                            if (rx.indexIn(parts.at(1).trimmed()) != -1)
                                currentNode->addChild( parts.at(0).trimmed(), rx.cap(1) );
                            else
                            {
                                if (sourceType_ == SOURCETYPE_TXT)
                                    currentNode->addChild( itemDefinition(
                                        parts.at(0).trimmed(),
                                        parts.at(1).trimmed(),
                                        parts.at(1).trimmed().section(',',0,0).toInt(),
                                        parts.at(1).trimmed().section(',',1,1).toInt(),
                                        (parts.count()<3? "" : parts.at(2)),
                                        true,"",QList<uint>(),MEMTYPE_NONE
                                    ));
                                if (sourceType_ == SOURCETYPE_MEM)
                                {
                                    QStringList memAddress = parts.at(1).trimmed().split('>',QString::SkipEmptyParts);
                                    QList<uint> offsets;
                                    if(memAddress.count()>1)
                                    {
                                        for(int i = 0; i<memAddress.count()-1; i++)
                                        {
                                            uint val;
                                            sscanf(memAddress.at(i+1).toLatin1().data(), "%x", &val);
                                            offsets.append(val);
                                        }
                                    }

                                    currentNode->addChild( itemDefinition(
                                        parts.at(0).trimmed(),
                                        "",
                                        0,
                                        0,
                                        (parts.count()<4? "" : parts.at(3)),
                                        true,
                                        memAddress.at(0),
                                        offsets,
                                        ToMemType(parts.at(2).trimmed())
                                    ));
                                }
                            }
                        }
                    } else
                    if(segment.startsWith("[/definition:"))
                    {
                        qWarning() << "Block cannot being with \"[/definition:\" (check the INI file - it is badly written)";
                        Q_ASSERT(!segment.startsWith("[/definition:"));
                    } else
                    if(segment=="[definitions]")
                    {
                        QStringList parts = item.split('\t',QString::SkipEmptyParts);
                        if(parts.count()>=2)
                        {
                            itemDefinition def;
                            def.name = parts.at(0).trimmed();
                            def.address = parts.at(1).trimmed();
                            if(sourceType_ == SOURCETYPE_TXT)
                            {
                                def.x = def.address.section(',',0,0).toInt();
                                def.y = def.address.section(',',1,1).toInt();
                            }
                            if(parts.count()>=3)
                                def.formatting = parts.at(2);
                            else
                                def.formatting = "";

                            itemDefinitions_.append(def);
                        }
                    } else
                    if(segment=="[parsing]") {
                        //rules for reinterpreting parsed data loaded here
                        QStringList parts = item.split('\t',QString::SkipEmptyParts);
                        if(parts.count()>=1)
                            parsingList.append(parts);
                    } else
                    {
                        qWarning() << QString("Line in unknown segment (%1) unparsed: ").arg(segment) << item;
                    }
                }
            }
            languageFileChanged();
            if(sourceType_ == SOURCETYPE_MEM)
                callback(lh_cb_polling, NULL);
        } else {
            setup_feedback_->setValue("Unable to open file.");
            return;
        }
    }
}

void LH_DataViewerConnector::sourceFileUpdated(const QString &path)
{
    Q_UNUSED(path);

    QDomDocument xmlDoc;
    if( watchPath_ != "" )
    {
        setup_feedback_->setValue("Connected to Data");

        QFile file( watchPath_ );
        QString fileContent = "";
        if( file.open( QIODevice::ReadOnly) )
        {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream.setAutoDetectUnicode(true);
            fileContent = stream.readAll();
            file.close();
        }
        switch(sourceType_){
        case SOURCETYPE_TXT:
        case SOURCETYPE_INI:
            {
                QStringList sourceLines = fileContent.split('\r',QString::SkipEmptyParts);

                if (isSingleWrite_ || sourceLines.count()>=completeCount_)
                {
                    // clear memory
                    sharedData->clear(itemDefinitions_.count());

                    // set expiry
                    if(dataExpiry_!=0)
                        sharedData->expiresAt =  QFileInfo(file).lastModified().addSecs(dataExpiry_).toString("yyyyMMddHHmmss.zzz");
                    else
                        sharedData->expiresAt = "N/A";

                    // set last updated time
                    sharedData->lastUpdated = QDateTime::currentDateTime().toString("yyyyMMddHHmmss.zzz");

                    // remove unwanted lines (for situations where new data is appended to old data)
                    if(updateLength_!=0)
                        while(sourceLines.count()>abs(updateLength_))
                        {
                            if(updateLength_>0)
                                sourceLines.removeFirst();
                            else
                                sourceLines.removeLast();
                        }

                    // load defined data
                    switch(sourceType_){
                    case SOURCETYPE_TXT:
                        //fill any index-based fields
                        for (int i=0; i<itemDefinitions_.count(); i++)
                            sharedData->setItem(i, itemDefinitions_.at(i).name, getTextValue(sourceLines, itemDefinitions_.at(i)));
                        //fill any node-based fields
                        populateValues(rootNode, sourceLines);
                        break;
                    case SOURCETYPE_INI:
                        updateNodes(sourceLines);
                        break;
                    default:
                        break;
                    }

                    // declare valid
                    sharedData->valid = true;
                }
            }
            break;
        case SOURCETYPE_XML:
            {
                xmlDoc = QDomDocument("data");
                if(!xmlDoc.setContent(fileContent))
                {
                    qWarning() << "LH_DataViewer: Could not load source document";
                    return;
                }

                // clear memory
                sharedData->clear(itemDefinitions_.count());

                // set expiry
                if(dataExpiry_!=0)
                    sharedData->expiresAt =  QFileInfo(file).lastModified().addSecs(dataExpiry_).toString("yyyyMMddHHmmss.zzz");
                else
                    sharedData->expiresAt = "N/A";

                //set last updated time
                sharedData->lastUpdated = QDateTime::currentDateTime().toString("yyyyMMddHHmmss.zzz");

                // load defined data
                updateNodes(xmlDoc.firstChild());

                // declare valid
                sharedData->valid = true;

            }
            break;
        case SOURCETYPE_MEM:
            //this type doesn't have a data file.
            break;
        default:
            {
                setup_feedback_->setValue("Data file type not yet supported");
            }
        }
    }
}

void LH_DataViewerConnector::updateNodes(QStringList sourceLines)
{
    dataNode* currentNode = rootNode;
    currentNode->resetCursors();
    QRegExp rx = QRegExp(";.*$");
    for(int i=0; i<sourceLines.count(); i++)
    {
        //QString removedPart = line.right(line.length() - line.indexOf(rx)).trimmed();
        QString line = sourceLines[i].trimmed();
        line = line.remove(rx).trimmed();
        if(line=="")
            continue;
        QStringList parts = line.split(delimiter_,QString::SkipEmptyParts);

        QString nodeName = parts.at(0).trimmed();
        parts.removeFirst();
        QString nodeValue = parts.join(QString(delimiter_)).trimmed();

        if(line.startsWith("[/"))
           currentNode = currentNode->parentNode();
        else
        if(line.startsWith("["))
           currentNode = currentNode->openChild(line.replace(QRegExp("\\[(.*)\\]") , "\\1" ));
        else
           currentNode->openChild( nodeName,  nodeValue );
    }

    for(int i=0; i<parsingList.count(); i++)
    {
        parseAddress(rootNode, parsingList[i][0].trimmed().split('.',QString::SkipEmptyParts), parsingList[i], QHash<QString,int>() );
    }
}

void LH_DataViewerConnector::updateNodes(QDomNode n, dataNode* currentNode)
{
    bool isRoot = false;
    if(!currentNode)
    {
        currentNode = rootNode;
        currentNode->resetCursors();
        isRoot = true;
    }

    QDomNamedNodeMap attrs = n.toElement().attributes();
    for (int i = 0; i < attrs.size(); i++)
    {
        QDomAttr attr = attrs.item(i).toAttr();
        currentNode->attributes.insert(attr.name(), attr.value());
    }

    for(int i = 0; i<n.childNodes().size(); i++)
    {
        QDomNode child = n.childNodes().at(i);
        if(child.nodeType()==QDomNode::TextNode)
            currentNode->setValue(child.nodeValue());
        else
            updateNodes(child, currentNode->openChild(child.nodeName()));
    }

    //Apply parsing rules
    if(isRoot)
        for(int i=0; i<parsingList.count(); i++)
            parseAddress(rootNode, parsingList[i][0].trimmed().split('.',QString::SkipEmptyParts), parsingList[i], QHash<QString,int>() );
}

void LH_DataViewerConnector::parseAddress(dataNode* currentNode, QStringList addresses, QStringList parseData, QHash<QString,int> indexes)
{
    if(addresses.count()>1)
    {
        QStringList myAddresses = QStringList(addresses);
        //continue drilling down the node heirarchy
        QString nodeName = myAddresses[0];
        myAddresses.removeFirst();
        int nodeIndex = -1;
        QRegExp rx("^(.*)\\[([0-9]+)\\]$");
        if(rx.indexIn(nodeName) != -1)
        {
            nodeName = rx.cap(1);
            nodeIndex = rx.cap(2).toInt();
        }

        //You can only create one new layer of nodes here
        if(!currentNode->contains(nodeName))
        {
            qWarning() << QString("LH_DataViewer: You can only create one new layer of nodes via the Parsing section at a time - the current datamap is trying to create more than that (\"%1\" does not exist but new nodes are trying to be added beneath it). Check the map file's format definition and the data file.").arg(nodeName);
            return; //Q_ASSERT(currentNode->contains(nodeName));
        }

        QList<dataNode*> nodesList = currentNode->child(nodeName);
        if(nodeIndex==-1 && nodesList.count()==1) nodeIndex = 0;
        if(nodeIndex==-1)
        {
            for (nodeIndex=0; nodeIndex<nodesList.count(); nodeIndex++)
            {
                QHash<QString,int> myIndexes = QHash<QString,int>(indexes);
                myIndexes.insert(nodesList[nodeIndex]->address(), nodeIndex);
                parseAddress(nodesList[nodeIndex], myAddresses, parseData, myIndexes);
            }
        } else
        if(nodeIndex < nodesList.count() && nodeIndex >= 0)
        {
            QHash<QString,int> myIndexes = QHash<QString,int>(indexes);
            myIndexes.insert(nodesList[nodeIndex]->address(), nodeIndex);
            parseAddress(nodesList[nodeIndex], myAddresses, parseData, myIndexes);
        }
    } else {
        //apply parsing
        QString formatCode = (parseData.count()<3? "" : parseData[2].trimmed());

        //now parse the format code as a template using stored indexes
        QRegExp rx = QRegExp("\\{([a-zA-Z0-9.[\\]]*)\\}");
        QStringList matches;
        QString tempFormatCode = formatCode;
        while (rx.indexIn(tempFormatCode) != -1)
        {
            if(!matches.contains(rx.cap(1)))
                matches.append(rx.cap(1));
            tempFormatCode.replace(rx.cap(0), "");
        }
        foreach(QString match, matches)
        {
            dataNode* matchNode = findNode(match, indexes);
            if(matchNode == 0)
                continue;
            else
                formatCode.replace(QString("{%1}").arg(match), matchNode->value());
        }

        if(parseData.count()<2)
            currentNode->openChild(addresses[0]);
        else
        {
            dataNode* valueNode = findNode(parseData[1].trimmed(), indexes);
            QString nodeValue = formatData(valueNode->value(), formatCode);
            //create the new child node
            if(valueNode->address() == currentNode->address(addresses[0]))
                valueNode->setValue(nodeValue);
            else
                currentNode->openChild(addresses[0], nodeValue);
        }
    }
}

dataNode* LH_DataViewerConnector::findNode(QString address, QHash<QString,int> indexes)
{
    QStringList path = address.split(".");
    dataNode* curNode = rootNode;
    QRegExp rx("^(.*)\\[([0-9]+)\\]$");
    while (path.length()!=0)
    {
        QString nodeName = path.first();
        path.removeFirst();

        int nodeIndex = -1;
        if(rx.indexIn(nodeName) != -1)
        {
            nodeName = rx.cap(1);
            nodeIndex = rx.cap(2).toInt();
        }

        Q_ASSERT(curNode->contains(nodeName));

        QList<dataNode*> nodesList = curNode->child(nodeName);
        if(nodeIndex==-1 && nodesList.count()==1) nodeIndex = 0;
        if(nodeIndex==-1)
            if(indexes.contains(curNode->address(nodeName)))
                nodeIndex = indexes[curNode->address(nodeName)];

        Q_ASSERT(nodeIndex>=0);
        Q_ASSERT(nodeIndex<curNode->child(nodeName).count());

        curNode = curNode->child(nodeName)[nodeIndex];
    }
    return curNode;
}

void LH_DataViewerConnector::languageFileChanged()
{
    QString fileName;
    if(setup_language_->value()>0 && setup_language_->value() < setup_language_->list().count())
        fileName = QString("%1lists.%2.txt").arg(get_dir_layout()).arg(setup_language_->list().at(setup_language_->value()));
    else
        fileName = setup_map_file_->value().absoluteFilePath();

    if(QFileInfo(fileName).exists())
    {
        QFile file( fileName );
        if( file.open( QIODevice::ReadOnly) )
        {
            QTextStream stream(&file);
            QRegExp rxEnd = QRegExp("\\s*;.*$");
            QRegExp rxPre = QRegExp("^[\\n\\r ]*");
            QString fileContent = stream.readAll();

            QStringList items = fileContent.split('\r',QString::SkipEmptyParts);

            QString segmentName="";
            QString segment = "";
            foreach (QString item, items)
            {
                item = item.remove(rxEnd);
                item = item.remove(rxPre);
                if (item!="")
                {
                    if(item.startsWith("["))
                    {
                        segment = item.toLower();
                        if(segment.startsWith("[list:"))
                        {
                            segmentName = QString(segment).replace(QRegExp("\\[list:(.*)\\]") , "\\1" );
                            if (lists_.contains(segmentName))
                            {
                                lists_.remove(segmentName);
                                lists_.insert(segmentName, new thresholdList() );
                            } else
                                segment = ""; //list does not exist
                        }
                    } else
                    if(segment.startsWith("[list:"))
                    {
                        thresholdItem newLevel;
                        newLevel.levelNames = item.split('\t');
                        if(newLevel.levelNames.at(0)=="" && lists_.value(segmentName)->levels.count() == 0)
                            newLevel.adaptiveBase = true;
                        else
                        {
                            newLevel.adaptiveBase = false;
                            newLevel.levelBase = newLevel.levelNames.at(0).trimmed().toFloat();
                            if (lists_.value(segmentName)->levels.count() == 1)
                                if (lists_.value(segmentName)->levels.at(0).adaptiveBase)
                                    lists_.value(segmentName)->levels[0].levelBase = newLevel.levelBase - 1;
                        }
                        lists_.value(segmentName)->levels.append(newLevel);
                    }
                }
            }

#ifdef DEBUG_MESSAGES
            qDebug() << "Language File Changed - reload data";
#endif
            sourceFileUpdated(setup_data_file_->value().absoluteFilePath());
        }
    }
}

bool LH_DataViewerConnector::readMemoryValues()
{
    QString feedbackMessage;
    if(rootNode->openProcess(processName_, processVersion_, feedbackMessage))
    {
        rootNode->refreshProcessValues();
        sharedData->lastUpdated = QDateTime::currentDateTime().toString("yyyyMMddHHmmss.zzz");
        sharedData->expiresAt = QDateTime::currentDateTime().addSecs(1).toString("yyyyMMddHHmmss.zzz");
        sharedData->valid = true;
        needsClearing_ = true;
        setup_feedback_->setValue( "Connected to Data" );
        return true;
    }
    else
    {
        if(feedbackMessage!="")
            setup_feedback_->setValue( feedbackMessage );
        if(needsClearing_)
        {
            rootNode->clearValues();
            sharedData->expiresAt = QDateTime::currentDateTime().addSecs(5).toString("yyyyMMddHHmmss.zzz");
            sharedData->valid = true;
            needsClearing_ = false;
        }
        return false;
    }
}
