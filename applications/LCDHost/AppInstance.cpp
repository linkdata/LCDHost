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


#include <QtDebug>
#include <QWidget>
#include <QDataStream>
#include <QPainter>
#include <QBuffer>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QStackedWidget>
#include <QApplication>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppDevice.h"
#include "AppGraphicsScene.h"
#include "EventDestroyInstance.h"
#include "EventCreateInstance.h"
#include "EventInstanceRender.h"
#include "EventInstanceRendered.h"
#include "EventRequestRender.h"
#include "EventRequestVisibility.h"
#include "AppState.h"
#include "AppClass.h"
#include "AppRawInput.h"
#include "AppInstance.h"
#include "AppClass.h"
#include "AppInstanceTree.h"
#include "LibInstance.h"

int AppInstance::xw_calc_count = 0;
int AppInstance::yh_calc_count = 0;

AppInstance::AppInstance() : AppObject( AppLibrary::orphanage() ), QGraphicsItem(), QGraphicsLayoutItem()
{
    loading_ = NULL;
    clear();
    return;
}

AppInstance::AppInstance( QString classId ) : AppObject( AppLibrary::orphanage() ), QGraphicsItem(), QGraphicsLayoutItem()
{
    Q_ASSERT( AppLibrary::getClass(classId) == NULL );
    loading_ = NULL;
    clear();
    class_id_ = classId;
}

AppInstance::AppInstance(AppClass *app_class, const QString &name) :
    AppObject(app_class->parent()),
    QGraphicsItem(),
    QGraphicsLayoutItem()
{
    loading_ = NULL;
    clear();
    class_id_ = app_class->id();
    class_size_ = app_class->size();
    setObjectName(name.isEmpty() ? app_class->generateName() : name);
    return;
}

AppInstance::AppInstance( AppInstance const& src ) : AppObject( src.appLibrary() ), QGraphicsItem(), QGraphicsLayoutItem()
{
    AppClass *app_class = AppLibrary::getClass(src.classId());
    loading_ = NULL;
    clear();

    class_id_ = src.classId();

    if( app_class ) setObjectName( app_class->generateName() );
    else setObjectName( QString::number((qptrdiff)this,0,36) );

    return;
}

AppInstance::~AppInstance()
{
#ifndef QT_NO_DEBUG
    if( id().hasAppObject() )
        qWarning() << id() << "has not been term()-inated";
#endif
    Q_ASSERT( !id().hasAppObject() );
    return;
}

void AppInstance::init()
{
    AppObject::init();
    Q_ASSERT( id().hasAppObject() );
    if( appLibrary() )
        appLibrary()->id().postLibEvent( new EventCreateInstance( class_id_, id(), objectName() ) );
    return;
}

void AppInstance::term()
{
    xRefBlank();
    yRefBlank();
    wRefBlank();
    hRefBlank();

    foreach( AppInstance *kid, xw_kids_ )
    {
        if( kid->xRef() == this ) kid->xRefBlank();
        if( kid->wRef() == this ) kid->wRefBlank();
    }
    Q_ASSERT( xw_kids_.isEmpty() );

    foreach( AppInstance *kid, yh_kids_ )
    {
        if( kid->yRef() == this ) kid->yRefBlank();
        if( kid->hRef() == this ) kid->hRefBlank();
    }
    Q_ASSERT( yh_kids_.isEmpty() );

    if( loading_ )
    {
        delete loading_;
        loading_ = NULL;
    }

#ifdef QT_OPENGL_LIB
    if( hastexture_ && mainWindow && mainWindow->glWidget() )
    {
        mainWindow->glWidget()->deleteTexture( texture_ );
        texture_ = 0;
        hastexture_ = false;
    }
#endif

    foreach( AppInstance *kid, childItemsByZ() )
        kid->term();

    AppObject::term();
}

QString AppInstance::path() const
{
    if( AppInstance * p = qobject_cast<AppInstance *>(parentItem()) )
        return p->path().append('/').append( objectName() );

    return QString("/layout/").append( AppState::instance()->layout_file() );
}

void AppInstance::clear()
{
    dragmode_ = 0;
#ifdef QT_OPENGL_LIB
    hastexture_ = false;
    texture_ = 0;
#endif
    setAcceptHoverEvents( true );
    setFlag( QGraphicsItem::ItemIsSelectable, true );
    setFlag( QGraphicsItem::ItemIsMovable, true );
    setFlag( QGraphicsItem::ItemSendsGeometryChanges, true );

    if( loading_ )
    {
        delete loading_;
        loading_ = NULL;
    }
    origin_ = QPointF(0,0);
    setPos(0,0);
    setZValue(-1);
    class_id_ = QString();
    class_size_ = QSize();
    setup_items_.clear();
    modifiable_ = true;
    selectable_ = true;
    alerts_ = true;
    size_ = QSizeF();
    // clientrect_ = QRectF();
    x_align_ = y_align_ = LH_ALIGN_LEFT;
    x_ref_align_ = y_ref_align_ = LH_ALIGN_LEFT;
    x_ref_ = y_ref_ = NULL;
    w_ref_ = h_ref_ = NULL;
    w_mod_ = h_mod_ = 0;
    w_units_ = h_units_ = 0;

    xw_lock_ = yh_lock_ = false;

    pixmap_ = QPixmap();
    selectiontimer_ = 0;

    return;
}

void AppInstance::setClassId( QString classid )
{
    AppClass *app_class;

    if( classid == classId() ) return;
    if( appLibrary() )
        appLibrary()->id().postLibEvent( new EventDestroyInstance(id()) );
    class_id_ = classid;
    app_class = AppLibrary::getClass(classid);
    if( app_class )
    {
        class_size_ = app_class->size();
        setParent( app_class->parent() );
        appLibrary()->id().postLibEvent( new EventCreateInstance( app_class->id(), id(), objectName() ) );
    }

    return;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
////  QGraphicsItem stuff
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QRectF AppInstance::boundingRect() const
{
    if( flags() & ItemIsSelectable )
    {
        qreal pw = 1.0;
        return QRectF(offset(), size()).adjusted(-pw/2, -pw/2, pw/2, pw/2);
    }
    return QRectF(offset(), size());
}

QVariant AppInstance::itemChange( GraphicsItemChange change, const QVariant & value )
{
    if( change == QGraphicsItem::ItemPositionHasChanged )
    {
        // notify all siblings
        xw_lock_ = true;
        foreach( AppInstance *kid, xw_kids_ )
        {
            if( kid->parentItem() == parentItem() )
                kid->moveBy( kid->xwCalculate(), 0 );
        }
        xw_lock_ = false;
        yh_lock_ = true;
        foreach( AppInstance *kid, yh_kids_ )
        {
            if( kid->parentItem() == parentItem() )
                kid->moveBy( 0, kid->yhCalculate() );
        }
        yh_lock_ = false;
    }

    if( change == QGraphicsItem::ItemPositionChange )
    {
        // round to nearest pixel
        if( !modifiable_ ) return pos();
        QPointF newPos = value.toPointF();
        newPos.rx() = qRound( newPos.x() );
        newPos.ry() = qRound( newPos.y() );
        return newPos;
    }

    if( change == QGraphicsItem::ItemVisibleHasChanged )
    {
        if( mainWindow )
            mainWindow->tree()->refreshInstance( this );
        if( isVisible() ) render();
    }

    return QGraphicsItem::itemChange(change, value);
}

void AppInstance::keyPressEvent( QKeyEvent * event )
{
    if( !modifiable_ ) return;

    switch( event->key() )
    {
    case Qt::Key_Delete:
        QCoreApplication::postEvent( mainWindow, new EventDestroyInstance( id() ) );
        return;
    case Qt::Key_Left:
        moveBy( -1, 0 );
        xwCalculate();
        layoutModified();
        refreshDetails();
        return;
    case Qt::Key_Right:
        moveBy( +1, 0 );
        xwCalculate();
        layoutModified();
        refreshDetails();
        return;
    case Qt::Key_Up:
        moveBy( 0, -1 );
        yhCalculate();
        layoutModified();
        refreshDetails();
        return;
    case Qt::Key_Down:
        moveBy( 0, +1 );
        yhCalculate();
        layoutModified();
        refreshDetails();
        return;
    default:
        break;
    }
    return;
}

void AppInstance::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *w )
{
    Q_UNUSED(w);

    if( mainWindow )
    {
        mainWindow->pps ++;

        switch( mainWindow->renderMethod() )
        {
        case 0:
            painter->drawImage( offset(), image(), QRectF(offset(),size()) );
            break;
        case 1:
            painter->drawPixmap( offset(), pixmap(), QRectF(offset(),size()) );
            break;
#ifdef QT_OPENGL_LIB
        case 2:
            if( hastexture_ )
                mainWindow->glWidget()->drawTexture( offset(), texture_ );
            else
            {
                painter->drawImage( offset(), image(), QRectF(offset(),size()) );
            }
#endif
            break;
        }
    }

    if( scene()->markoutline() )
    {
        QPen oldPen = painter->pen();
        QBrush oldBrush = painter->brush();
        QRectF adj( offset(), size() );
        painter->setPen( QColor( 200, 200, 200, 128 ) );
        painter->setBrush( Qt::transparent );
        painter->drawRect( adj );

        if( pixmap().isNull() )
        {
            painter->drawLine( adj.topLeft(), adj.bottomRight() );
            painter->drawLine( adj.bottomLeft(), adj.topRight() );
        }

        if( option->state & QStyle::State_Selected )
        {
            QPen pen;
            QVector<qreal> v;

            v << 3 << 2;

            pen.setWidth( 1 );
            pen.setColor( Qt::white );
            painter->setPen( pen );
            painter->setBrush( Qt::transparent );
            painter->drawRect( QRectF(offset(),size()) );

            pen.setColor( Qt::black );
            pen.setDashOffset( ((qreal)QTime::currentTime().msec())/200.0 );
            pen.setDashPattern( v );
            painter->setPen( pen );
            painter->setBrush( Qt::transparent );
            painter->drawRect( QRectF(offset(),size()) );

            if( !selectiontimer_ ) selectiontimer_ = startTimer(200);
        }

        painter->setPen( oldPen );
        painter->setBrush( oldBrush );
    }

    return;
}

#define edgeTop    0x01
#define edgeBottom 0x02
#define edgeLeft   0x04
#define edgeRight  0x08

int AppInstance::getGrabPoint( QPointF pos )
{
    int e = 0;

    if( pos.x() <=  1 ) e |= edgeLeft;
    if( pos.y() <=  1 ) e |= edgeTop;
    if( pos.x() >= width() - 1 ) e |= edgeRight;
    if( pos.y() >= height() - 1 ) e |= edgeBottom;
    return e;
}

void AppInstance::setCursorForMode( int e )
{
    if( !modifiable_ )
    {
        setCursor( Qt::ForbiddenCursor );
        return;
    }

    switch( e )
    {
    case edgeLeft | edgeTop :
    case edgeBottom | edgeRight :
        setCursor(Qt::SizeFDiagCursor);
        break;
    case edgeRight :
    case edgeLeft :
        setCursor(Qt::SizeHorCursor);
        break;
    case edgeLeft | edgeBottom :
    case edgeRight | edgeTop :
        setCursor(Qt::SizeBDiagCursor);
        break;
    case edgeBottom :
    case edgeTop :
        setCursor(Qt::SizeVerCursor);
        break;
    default:
        setCursor(Qt::OpenHandCursor);
        break;
    }
    update();
    return;
}

void AppInstance::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
{
    if( isSelected() )
        setCursorForMode( getGrabPoint( event->pos() ) );

    return;
}

void AppInstance::hoverLeaveEvent ( QGraphicsSceneHoverEvent * )
{
    unsetCursor();
    return;
}

void AppInstance::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    if( !modifiable_ )
    {
        QGraphicsItem::mousePressEvent( event );
        return;
    }
    dragmode_ = getGrabPoint( event->pos() );
    if( dragmode_ == 0 )
    {
        setCursor(Qt::ClosedHandCursor);
        QGraphicsItem::mousePressEvent( event );
        return;
    }
    setCursorForMode( dragmode_ );
    return;
}

void AppInstance::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsItem::mouseReleaseEvent( event );
    dragmode_ = 0;
    unsetCursor();
    return;
}

void AppInstance::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
    int delta;
    QPointF oldPos;
    QSizeF oldSize;

    if( !modifiable_ ) return;
    if( !dragmode_ )
    {
        QGraphicsItem::mouseMoveEvent( event );
        foreach( QGraphicsItem *item, scene()->selectedItems() )
        {
            AppInstance *app_inst = static_cast<AppInstance*>(item);
            app_inst->moveBy( app_inst->xwCalculate(), app_inst->yhCalculate() );
        }
        layoutModified();
        refreshDetails();
        return;
    }

    oldPos = scenePos();
    oldSize = size();

    if( dragmode_ & edgeRight )
    {
        if( w_ref_ == NULL ) wUnitsSet(LH_METHOD_ABSOLUTE);
        if( wSetModForWidth( event->scenePos().x() - scenePos().x() ) )
            moveBy( xwCalculate(), 0 );
    }

    if( dragmode_ & edgeBottom )
    {
        if( h_ref_ == NULL ) hUnitsSet(LH_METHOD_ABSOLUTE);
        if( hSetModForHeight( event->scenePos().y() - scenePos().y() ) )
            yhCalculate();
    }

    if( dragmode_ & edgeTop )
    {
        delta = (int) ( scenePos().y() - event->scenePos().y() );
        int new_h = (int) height() + delta;
        if( h_ref_ == NULL ) hUnitsSet(LH_METHOD_ABSOLUTE);
        moveBy( 0, -delta );
        hSetModForHeight( new_h );
        yhCalculate();
    }

    if( dragmode_ & edgeLeft )
    {
        delta = (int) ( scenePos().x() - event->scenePos().x() );
        int new_w = (int) width() + delta;
        if( w_ref_ == NULL ) wUnitsSet(LH_METHOD_ABSOLUTE);
        moveBy( -delta, 0 );
        wSetModForWidth( new_w );
        xwCalculate();
    }

    if( oldPos.x() != scenePos().x() ) xwCalculate();
    if( oldPos.y() != scenePos().y() ) yhCalculate();

    if( oldSize.width() != width() && oldSize.height() != height() )
    {
        xwCalculate();
        yhCalculate();
    }
    else
    {
        if( oldSize.width() != width() ) xwCalculate();
        if( oldSize.height() != height() ) yhCalculate();
    }

    if( oldPos != scenePos() || oldSize != size() )
    {
        layoutModified();
        refreshDetails();
    }

    return;
}

void AppInstance::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event )
{
    Q_UNUSED( event );
    setupFocusUI();
    return;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// QGraphicsLayoutItem stuff
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QSizeF AppInstance::sizeHint( Qt::SizeHint which, const QSizeF & constraint ) const
{
    Q_UNUSED(constraint);

    switch( which )
    {
    case Qt::NSizeHints: // wtf is this?
        break;
    case Qt::MinimumSize: //	0	is used to specify the minimum size of a graphics layout item.
        return QSizeF(0,0);
    case Qt::PreferredSize: //	1	is used to specify the preferred size of a graphics layout item.
        return plugin_size_;
    case Qt::MaximumSize: //	2	is used to specify the maximum size of a graphics layout item.
        return AppDevice::current().size();
    case Qt::MinimumDescent: //	3	is used to specify the minimum descent of a text string in a graphics layout item.
        break;
    }
    return QSizeF(0,0);
}

void AppInstance::updateGeometry()
{
    QGraphicsLayoutItem::updateGeometry();
    render();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// Templates for standard functions
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// XW
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

qreal AppInstance::xwCalculate()
{
    bool modified = false;
    qreal x, w;
    qreal wx;
    qreal dx = 0;

    if( xw_lock_ ) return 0.0;
    xw_lock_ = true;

    xw_calc_count ++;

    if( parentItem() == NULL )
    {
        w = width();
        wx = 0;
    }
    else if( w_ref_ == NULL )
    {
        if( plugin_size_.width() >= 0 ) w = plugin_size_.width();
        else w = width();
        wx = 0;
    }
    else
    {
        if( w_ref_ == parentItem() )
        {
            w = parentItem()->width(); // clientrect_.width();
            wx = 0;//parentItem()->clientrect_.x();
        }
        else
        {
            w = w_ref_->width();
            wx = w_ref_->x();
        }
    }

    if( x_ref_ )
    {
        if( x_ref_ == parentItem() )
        {
            // parent
            x = 0;//x_ref_->clientrect_.x();
            switch( x_ref_align_ )
            {
            case LH_ALIGN_LEFT:
                break;
            case LH_ALIGN_CENTER:
                x += x_ref_->width()/2;//x_ref_->clientrect_.width() / 2;
                break;
            case LH_ALIGN_RIGHT:
                x += x_ref_->width();//x_ref_->clientrect_.width();
                break;
            }
        }
        else
        {
            // sibling
            x = x_ref_->pos().x();
            switch( x_ref_align_ )
            {
            case LH_ALIGN_LEFT:
                break;
            case LH_ALIGN_CENTER:
                x += x_ref_->width() / 2;
                break;
            case LH_ALIGN_RIGHT:
                x += x_ref_->width();
                break;
            }
        }
    }
    else
    {
        x = 0;
    }

    switch( x_align_ )
    {
    case LH_ALIGN_LEFT:
        w = valueFromMod( w_units_, w_mod_, pos().x() - origin_.x() + x, w, wx, w );
        break;
    case LH_ALIGN_CENTER:
        w = valueFromMod( w_units_, w_mod_, pos().x(), w, wx, w );
        x -= w / 2;
        if( w != width() ) dx += (width() - w)/2;
        break;
    case LH_ALIGN_RIGHT:
        w = valueFromMod( w_units_, w_mod_, pos().x(), w, wx, w );
        x -= w;
        if( w != width() ) dx += width() - w;
        break;
    }

    if( dx ) moveBy( dx, 0 );
    dx = 0;

    if( x != origin_.x() )
    {
        dx = x - origin_.x();
        modified = true;
        origin_.setX( x );
    }

    if( w != width() )
    {
        modified = true;
        prepareGeometryChange();
        size_.setWidth( w );
        render();
    }

    if( modified )
    {
        foreach( AppInstance *kid, xw_kids_ )
        {
            if( !kid->xw_lock_ )
            {
                kid->moveBy( kid->xwCalculate(), 0 );
            }
        }
    }

    xw_lock_ = false;
    return dx;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// YH
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

qreal AppInstance::yhCalculate()
{
    bool modified = false;
    qreal y, h;
    qreal hy;
    qreal dy = 0;

    if( yh_lock_ ) return 0.0;
    yh_lock_ = true;

    yh_calc_count ++;


    if( parentItem() == NULL )
    {
        h = height();
        hy = 0;
    }
    else if( h_ref_ == NULL )
    {
        if( plugin_size_.height() >= 0 ) h = plugin_size_.height();
        else h = height();
        hy = 0;
    }
    else
    {
        if( h_ref_ == parentItem() )
        {
            h = h_ref_->height();//parentItem()->clientrect_.height();
            hy = 0; //parentItem()->clientrect_.y();
        }
        else
        {
            h = h_ref_->height();
            hy = h_ref_->y();
        }
    }

    if( y_ref_ )
    {
        if( y_ref_ == parentItem() )
        {
            // parent
            y = 0;//y_ref_->clientrect_.y();
            switch( y_ref_align_ )
            {
            case LH_ALIGN_LEFT:
                break;
            case LH_ALIGN_CENTER:
                y += y_ref_->height()/2;//y_ref_->clientrect_.height() / 2;
                break;
            case LH_ALIGN_RIGHT:
                y += y_ref_->height();//y_ref_->clientrect_.height();
                break;
            }
        }
        else
        {
            // sibling
            y = y_ref_->pos().y();
            switch( y_ref_align_ )
            {
            case LH_ALIGN_TOP:
                break;
            case LH_ALIGN_CENTER:
                y += y_ref_->height() / 2;
                break;
            case LH_ALIGN_BOTTOM:
                y += y_ref_->height();
                break;
            }
        }
    }
    else
    {
        y = 0;
    }

    switch( y_align_ )
    {
    case LH_ALIGN_TOP:
        h = valueFromMod( h_units_, h_mod_, pos().y() - origin_.y() + y, h, hy, h );
        break;
    case LH_ALIGN_CENTER:
        h = valueFromMod( h_units_, h_mod_, pos().y(), h, hy, h );
        y -= h / 2;
        if( h != height() ) dy = (height() - h)/2;
        break;
    case LH_ALIGN_BOTTOM:
        h = valueFromMod( h_units_, h_mod_, pos().y(), h, hy, h );
        y -= h;
        if( h != height() ) dy = (height() - h);
        break;
    }

    if( dy ) moveBy( 0, dy );
    dy = 0;

    if( y != origin_.y() )
    {
        dy = y - origin_.y();
        modified = true;
        origin_.setY( y );
    }

    if( h != height() )
    {
        modified = true;
        prepareGeometryChange();
        // clientrect_.setHeight( clientrect_.height() + (h - size_.height()) );
        size_.setHeight( h );
        render();
    }

    if( modified )
    {
        foreach( AppInstance *kid, yh_kids_ )
        {
            if( !kid->yh_lock_ )
            {
                kid->moveBy( 0, kid->yhCalculate() );
            }
        }
    }

    yh_lock_ = false;
    return dy;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// X
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void AppInstance::xRefReset()
{
    if( x_ref_ == NULL ) return;
    if( w_ref_ != x_ref_ )
    {
        int idx = x_ref_->xw_kids_.indexOf(this);
        Q_ASSERT( idx >= 0 );
        if( idx >= 0 ) x_ref_->xw_kids_.remove(idx);
        Q_ASSERT( x_ref_->xw_kids_.contains(this) == false );
    }
    x_ref_ = NULL;
    return;
}

void AppInstance::xRefSet( AppInstance *p )
{
    if( p == x_ref_ ) return;
    if( p && p->hasParentItem( parentItem() ) )
    {
        xRefReset();
        x_ref_ = p;
        if( w_ref_ != x_ref_ )
        {
            x_ref_->xw_kids_.append(this);
            Q_ASSERT( x_ref_->xw_kids_.count(this) == 1 );
        }
        xwCalculate();
    }
    else xRefBlank();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// Y
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void AppInstance::yRefReset()
{
    if( y_ref_ == NULL ) return;
    if( h_ref_ != y_ref_ )
    {
        int idx = y_ref_->yh_kids_.indexOf(this);
        Q_ASSERT( idx >= 0 );
        if( idx >= 0 ) y_ref_->yh_kids_.remove(idx);
        Q_ASSERT( y_ref_->yh_kids_.contains(this) == false );
    }
    y_ref_ = NULL;
    return;
}

void AppInstance::yRefSet( AppInstance *p )
{
    if( p == y_ref_ ) return;
    if( p && p->hasParentItem( parentItem() ) )
    {
        yRefReset();
        y_ref_ = p;
        if( h_ref_ != y_ref_ )
        {
            y_ref_->yh_kids_.append(this);
            Q_ASSERT( y_ref_->yh_kids_.count(this) == 1 );
        }
        yhCalculate();
    }
    else yRefBlank();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// Value <-> Mod calculators
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Calculate the width/height given scene coords and method/mod
qreal AppInstance::valueFromMod( int method, int mod, qreal xy, qreal old_wh, qreal rxy, qreal rwh )
{
    qreal wh = old_wh;

    switch( method )
    {
    case LH_METHOD_ABSOLUTE: // absolute
        if( mod>0 ) wh = mod;
        break;
    case LH_METHOD_LEFT: // LH_METHOD_TOP:
        wh = qRound( rxy - xy + mod );
        break;
    case LH_METHOD_RIGHT: // LH_METHOD_BOTTOM:
        wh = qRound( rxy + rwh - xy + mod );
        break;
    case LH_METHOD_PERMILLE: // permille
        if( rwh > 0 ) wh = qRound( ( rwh * mod ) / 1000 );
        break;
    case LH_METHOD_ADJUST: // pixels
        wh = qRound( rwh + mod );
        break;
    default:
        Q_ASSERT( !"Unknown method" );
        break;
    }

    if( wh < 0 ) wh = 0;
    if( wh > 800 ) wh = 800;

    return wh;
}

int AppInstance::modFromValue( int method, int old_mod, qreal xy, qreal wh, qreal rxy, qreal rwh  )
{
    int mod = old_mod;

    if( wh < 0 ) wh = 0;
    if( wh > 800 ) wh = 800;
    wh = qRound( wh );

    switch( method )
    {
    case LH_METHOD_ABSOLUTE: // absolute
        mod = wh;
        break;
    case LH_METHOD_RIGHT: // LH_METHOD_BOTTOM:
        mod = wh - rxy - rwh + xy;
        break;
    case LH_METHOD_LEFT: // LH_METHOD_TOP:
        mod = wh - rxy + xy;
        break;
    case LH_METHOD_PERMILLE: // permille
        mod = ( wh * 1000 ) / rwh;
        break;
    case LH_METHOD_ADJUST: // pixels
        mod = wh - rwh;
        break;
    default:
        Q_ASSERT( !"Unknown method" );
        break;
    }

    Q_ASSERT( valueFromMod(method,mod,xy,wh,rxy,rwh) == wh );

    return mod;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// W
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void AppInstance::wRefReset()
{
    if( w_ref_ == NULL ) return;
    if( w_ref_ != x_ref_ )
    {
        int idx = w_ref_->xw_kids_.indexOf(this);
        Q_ASSERT( idx >= 0 );
        if( idx >= 0 ) w_ref_->xw_kids_.remove(idx);
        Q_ASSERT( w_ref_->xw_kids_.contains(this) == false );
    }
    w_ref_ = NULL;
    return;
}

void AppInstance::wRefBlank()
{
    wRefReset();
    w_mod_ = 0;
    w_units_ = 0;
    xwCalculate();
}

void AppInstance::wRefSet( AppInstance *p )
{
    if( p == w_ref_ ) return;
    if( p && p->hasParentItem( parentItem() ) )
    {
        wRefReset();
        w_ref_ = p;
        if( w_ref_ != x_ref_ )
        {
            w_ref_->xw_kids_.append(this);
            Q_ASSERT( w_ref_->xw_kids_.count(this) == 1 );
        }
        xwCalculate();
    }
    else wRefBlank();
}

void AppInstance::wModSet( int n )
{
    if( w_mod_ == n ) return;
    w_mod_ = n;
    xwCalculate();
}

void AppInstance::wUnitsSet( int units )
{
    qreal old_w;
    if( units < 0 || units == w_units_ ) return;
    old_w = width();
    w_units_ = units;
    wSetModForWidth( old_w );
    xwCalculate();
    return;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
//// H
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void AppInstance::hRefReset()
{
    if( h_ref_ == NULL ) return;
    if( h_ref_ != y_ref_ )
    {
        int idx = h_ref_->yh_kids_.indexOf(this);
        Q_ASSERT( idx >= 0 );
        if( idx >= 0 ) h_ref_->yh_kids_.remove(idx);
        Q_ASSERT( h_ref_->yh_kids_.contains(this) == false );
    }
    h_ref_ = NULL;
    return;
}

void AppInstance::hRefSet( AppInstance *p )
{
    if( p == h_ref_ ) return;
    if( p && p->hasParentItem( parentItem() ) )
    {
        hRefReset();
        h_ref_ = p;
        if( h_ref_ != y_ref_ )
        {
            h_ref_->yh_kids_.append(this);
            Q_ASSERT( h_ref_->yh_kids_.count(this) == 1 );
        }
        yhCalculate();
    }
    else hRefBlank();
}

void AppInstance::hModSet( int n )
{
    if( h_mod_ == n ) return;
    h_mod_ = n;
    yhCalculate();
}

void AppInstance::hUnitsSet( int units )
{
    qreal old_h;
    if( units < 0 || units == h_units_ ) return;
    old_h = height();
    h_units_ = units;
    hSetModForHeight( old_h );
    yhCalculate();
    return;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////
////
////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void AppInstance::setSize( QSizeF size )
{
    if( size != size_ )
    {
        prepareGeometryChange();
        size_ = size;
    }
    return;
}

void AppInstance::render()
{
    if( !isLoading() && isVisible() )
    {
        id().postLibEvent(
                new EventInstanceRender( id(), w_ref_ ? size().width() : -1,w_units_,w_mod_,
                                         h_ref_ ? size().height() : -1,h_units_,h_mod_
                                         )
                );
    }
}

void AppInstance::refreshDetails()
{
    if( mainWindow && isSelected() ) mainWindow->refreshInstanceDetails();
}

AppLibrary *AppInstance::appLibrary() const
{
    Q_ASSERT( parent()->inherits("AppLibrary") );
    return static_cast<AppLibrary*>(parent());
}

bool AppInstance::hasParentItem( AppInstance *app_inst )
{
    if( app_inst == NULL ) return false;
    if( this == app_inst ) return true;
    if( parentItem() == NULL ) return false;
    return parentItem()->hasParentItem(app_inst);
}

void AppInstance::setParentItem( AppInstance* app_inst )
{
    if( app_inst == this || app_inst == parentItem() ) return;
    if( app_inst && app_inst->hasParentItem(this) ) return;

    QPointF abspos = scenePos();

    // If our referenced items can't match to our new parent, ditch them
    bool x_keep = xRef() ? false : true;
    bool y_keep = yRef() ? false : true;
    bool w_keep = wRef() ? false : true;
    bool h_keep = hRef() ? false : true;
    if( app_inst )
    {
        if( xRef() == app_inst ) x_keep = true;
        if( yRef() == app_inst ) y_keep = true;
        if( wRef() == app_inst ) w_keep = true;
        if( hRef() == app_inst ) h_keep = true;
        foreach( AppInstance *kid, app_inst->childItemsByZ() )
        {
            if( xRef() == kid ) x_keep = true;
            if( yRef() == kid ) y_keep = true;
            if( wRef() == kid ) w_keep = true;
            if( hRef() == kid ) h_keep = true;
        }
    }
    if( !x_keep ) xRefBlank();
    if( !y_keep ) yRefBlank();
    if( !w_keep ) wRefBlank();
    if( !h_keep ) hRefBlank();

    // Remove all siblings from referencing kids
    foreach( AppInstance *kid, xw_kids_ )
    {
        if( kid->xRef() == this && kid->parentItem() == parentItem() ) { kid->xRefBlank(); }
        if( kid->wRef() == this && kid->parentItem() == parentItem() ) { kid->wRefBlank(); }
    }

    foreach( AppInstance *kid, yh_kids_ )
    {
        if( kid->yRef() == this && kid->parentItem() == parentItem() ) { kid->yRefBlank(); }
        if( kid->hRef() == this && kid->parentItem() == parentItem() ) { kid->hRefBlank(); }
    }

    QGraphicsItem::setParentItem(app_inst);
    setScenePos( abspos );

    return;
}

void AppInstance::setScenePos( QPointF pos )
{
    setPos( mapToParent( mapFromScene( pos ) ) );
}

static bool AppInstanceLessThan( AppInstance *a1, AppInstance *a2 )
{
    return a1->zValue() < a2->zValue();
}

QList<AppInstance *> AppInstance::childItemsByZ() const
{
    QList<QGraphicsItem *> orig_list = childItems();
    QList<QGraphicsItem *>::const_iterator i;
    QList<AppInstance *> list;
    for( i=orig_list.constBegin(); i!=orig_list.constEnd(); ++i )
        list.append( static_cast<AppInstance*>(*i) );
    qStableSort(list.begin(), list.end(), AppInstanceLessThan );
    return list;
}

void AppInstance::allChildren( QList<AppInstance*> & toList ) const
{
    foreach( QGraphicsItem *c, childItems() )
    {
        toList.append( static_cast<AppInstance*>(c) );
        static_cast<AppInstance*>(c)->allChildren(toList);
    }
    return;
}

// Return display data for the treeview
QVariant AppInstance::data( int column, int role )
{
    if( role == Qt::TextAlignmentRole )
    {
        if( column == 0 ) return Qt::AlignLeft;
        return Qt::AlignCenter;
    }
    if( role == Qt::ForegroundRole )
    {
        if( isVisible() ) return QColor(Qt::black);
        return QColor(Qt::gray);
    }
    if( role == Qt::CheckStateRole )
    {
        switch( column )
        {
        case 1: return selectable_;
        case 2: return modifiable_;
        }
    }
    if( role == Qt::DisplayRole )
    {
        switch( column )
        {
        case 0: return objectName();
        case 1: return QString();
        case 2: return QString();
        }
    }
    if( role == Qt::ToolTipRole )
    {
        return QString("[%1] %2").arg( id().toInt() ).arg( classId() );
    }
    return QVariant();
}

QVariant AppInstance::headerData( int section, Qt::Orientation orientation, int role )
{
    if( orientation == Qt::Horizontal)
    {
        if( role == Qt::TextAlignmentRole )
        {
            return Qt::AlignLeft;
        }
        if( role == Qt::DisplayRole )
        {
            switch( section )
            {
            case 0: return QString("Instance name"); break;
            case 1: return QString("Selectable"); break;
            case 2: return QString("Modifiable"); break;
            }
            return QString("Column %1").arg(section);
        }
        else if( role == Qt::ToolTipRole )
        {
            switch( section )
            {
            case 0: return QString("Name of the instance"); break;
            case 1: return QString("Instance selectable in layout"); break;
            case 2: return QString("Instance is modifiable"); break;
            }
            return QString("Column %1").arg(section);
        }
    }
    return QVariant();
}


void AppInstance::sceneGeometryChanged()
{
    render();
}

void AppInstance::timerEvent(QTimerEvent *event)
{
    if( event->timerId() == selectiontimer_ )
    {
        if( isSelected() ) update();
        else killTimer(selectiontimer_), selectiontimer_ = 0;
    }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////
//// Configuration
////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////
//// Support
////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


bool AppInstance::load( QXmlStreamReader & stream, AppInstance *parent )
{
    QString class_id;
    int startline = stream.lineNumber();
    AppClass *app_class;
    AppInstance *app_inst;

    if( stream.name() != "instance" )
    {
        qDebug() << "expected an instance, got" << stream.name() << "line" << stream.lineNumber();
        return false;
    }

    loading_ = new AppInstanceLoading();
    loading_->parent_ = parent;

    while( !stream.atEnd() )
    {
        stream.readNext();
        if( stream.isStartElement() )
        {
            if( stream.name() == "identity" )
            {
                setObjectName( stream.attributes().value("name").toString() );
                loading_->id_ = (AppInstance*) (qptrdiff) stream.attributes().value("id").toString().toLongLong(0,36);
                if( lh_log_load )
                    qDebug() << QString("<span style=\"background-color: #f0f0f0;\">%1</span> Loading <tt>%2</tt>")
                        .arg(startline,5,10,QChar('0'))
                        .arg( objectName() );
            }

            if( stream.name() == "state" )
            {
                modifiable_ = stream.attributes().value("modifiable").toString().toInt();
                selectable_ = stream.attributes().value("selectable").toString().toInt();
                alerts_ = stream.attributes().value("alerts").toString().toInt();
            }

            if( stream.name() == "origin" )
            {
                origin_.setX( stream.attributes().value("x").toString().toDouble() );
                origin_.setY( stream.attributes().value("y").toString().toDouble() );
            }

            if( stream.name() == "size" )
            {
                size_.setWidth( stream.attributes().value("w").toString().toDouble() );
                size_.setHeight( stream.attributes().value("h").toString().toDouble() );
            }

            if( stream.name() == "plugin_size" )
            {
                plugin_size_.setWidth( stream.attributes().value("w").toString().toDouble() );
                plugin_size_.setHeight( stream.attributes().value("h").toString().toDouble() );
            }

            if( stream.name() == "pos" )
            {
                loading_->pos_.setX( stream.attributes().value("x").toString().toDouble() );
                loading_->pos_.setY( stream.attributes().value("y").toString().toDouble() );
                loading_->z_ = stream.attributes().value("z").toString().toDouble();
            }

            if( stream.name() == "x_ref" )
            {
                loading_->x_ref_ = (AppInstance*) (qptrdiff) stream.attributes().value("id").toString().toLongLong(0,36);
                loading_->x_align_ = stream.attributes().value("align").toString().toInt();
                loading_->x_ref_align_ = stream.attributes().value("ref_align").toString().toInt();
            }

            if( stream.name() == "y_ref" )
            {
                loading_->y_ref_ = (AppInstance*) (qptrdiff) stream.attributes().value("id").toString().toLongLong(0,36);
                loading_->y_align_ = stream.attributes().value("align").toString().toInt();
                loading_->y_ref_align_ = stream.attributes().value("ref_align").toString().toInt();
            }

            if( stream.name() == "w_ref" )
            {
                loading_->w_ref_ = (AppInstance*) (qptrdiff) stream.attributes().value("id").toString().toLongLong(0,36);
                loading_->w_mod_ = stream.attributes().value("mod").toString().toDouble();
                loading_->w_units_ = stream.attributes().value("units").toString().toInt();
            }

            if( stream.name() == "h_ref" )
            {
                loading_->h_ref_ = (AppInstance*) (qptrdiff) stream.attributes().value("id").toString().toLongLong(0,36);
                loading_->h_mod_ = stream.attributes().value("mod").toString().toDouble();
                loading_->h_units_ = stream.attributes().value("units").toString().toInt();
            }

            if( stream.name() == "setupitem" )
            {
                AppSetupItem *asi = AppSetupItem::fromXmlStream(this,stream);
                if( asi ) setup_items_.append( asi );
            }

            if( stream.name() == "instance" )
            {
                class_id = stream.attributes().value("class").toString();

                if( lh_log_load )
                    qDebug() << QString("<span style=\"background-color: #f0f0f0;\">%1</span> New instance of type <tt>%2</tt>")
                        .arg(stream.lineNumber(),5,10,QChar('0'))
                        .arg( class_id );

                app_class = AppLibrary::getClass( class_id );
                if( app_class ) app_inst = new AppInstance( app_class );
                else app_inst = new AppInstance( class_id );

                app_inst->init();

                if( !app_inst->load(stream, this) ) return false;

                loading_->children_.append(app_inst);

                if( lh_log_load )
                    qDebug() << QString("<span style=\"background-color: #f0f0f0;\">%1</span> Loaded <tt>%2</tt>")
                        .arg(stream.lineNumber(),5,10,QChar('0'))
                        .arg( app_inst->objectName() );
            }
        }

        if( stream.isEndElement() )
        {
            if( stream.name() == "instance" )
            {
                stream.readNext();
                return true;
            }
        }
    }

    qDebug() << "AppInstance::load(): xml stream ended prematurely" << objectName();
    return false;
}

void AppInstance::save( QXmlStreamWriter & stream )
{
    stream.writeStartElement( "instance" );
    if( !classId().isEmpty() ) stream.writeAttribute( "class", classId() );

    stream.writeEmptyElement("identity");
    stream.writeAttribute( "name", objectName() );
    stream.writeAttribute( "id", QString::number((qptrdiff)(void*)this,36) );

    stream.writeEmptyElement("state");
    stream.writeAttribute( "modifiable", QString::number(modifiable_) );
    stream.writeAttribute( "selectable", QString::number(selectable_) );
    stream.writeAttribute( "alerts", QString::number(alerts_) );

    stream.writeEmptyElement( "origin" );
    stream.writeAttribute( "x", QString::number(origin_.x()) );
    stream.writeAttribute( "y", QString::number(origin_.y()) );

    stream.writeEmptyElement( "size" );
    stream.writeAttribute( "w", QString::number(width()) );
    stream.writeAttribute( "h", QString::number(height()) );

    stream.writeEmptyElement( "plugin_size" );
    stream.writeAttribute( "w", QString::number(plugin_size_.width()) );
    stream.writeAttribute( "h", QString::number(plugin_size_.height()) );

    stream.writeEmptyElement( "pos" );
    stream.writeAttribute( "x", QString::number(pos().x()) );
    stream.writeAttribute( "y", QString::number(pos().y()) );
    stream.writeAttribute( "z", QString::number(zValue()) );

    stream.writeEmptyElement( "x_ref" );
    stream.writeAttribute( "id", QString::number((qptrdiff)(void*)x_ref_,36) );
    stream.writeAttribute( "align", QString::number(x_align_) );
    stream.writeAttribute( "ref_align", QString::number(x_ref_align_) );

    stream.writeEmptyElement( "y_ref" );
    stream.writeAttribute( "id", QString::number((qptrdiff)(void*)y_ref_,36) );
    stream.writeAttribute( "align", QString::number(y_align_) );
    stream.writeAttribute( "ref_align", QString::number(y_ref_align_) );

    stream.writeEmptyElement( "w_ref" );
    stream.writeAttribute( "id", QString::number((qptrdiff)(void*)w_ref_,36) );
    stream.writeAttribute( "mod", QString::number(w_mod_) );
    stream.writeAttribute( "units", QString::number(w_units_) );

    stream.writeEmptyElement( "h_ref" );
    stream.writeAttribute( "id", QString::number((qptrdiff)(void*)h_ref_,36) );
    stream.writeAttribute( "mod", QString::number(h_mod_) );
    stream.writeAttribute( "units", QString::number(h_units_) );

    foreach( AppSetupItem *asi, setup_items_ )
        if (asi) asi->save(stream);

    foreach( AppInstance *app_inst, childItemsByZ() )
        app_inst->save(stream);

    stream.writeEndElement();
    return;
}


void AppInstance::loadMap( QMap<AppInstance*,AppInstance*> &load_id_map )
{
    load_id_map.insert( loading_->id_, this );
    foreach( AppInstance *app_inst, loading_->children_ )
        app_inst->loadMap( load_id_map );
    return;
}

void AppInstance::loadTree( QMap<AppInstance*,AppInstance*> &load_id_map )
{
    if( mainWindow && mainWindow->tree() && loading_->parent_ )
        mainWindow->tree()->insertAppInstance( this, loading_->parent_ );

    Q_ASSERT( loading_->x_ref_ ? load_id_map.value( loading_->x_ref_, NULL ) != NULL : true );
    Q_ASSERT( loading_->y_ref_ ? load_id_map.value( loading_->y_ref_, NULL ) != NULL : true );
    Q_ASSERT( loading_->w_ref_ ? load_id_map.value( loading_->w_ref_, NULL ) != NULL : true );
    Q_ASSERT( loading_->h_ref_ ? load_id_map.value( loading_->h_ref_, NULL ) != NULL : true );

    setPos( loading_->pos_ );
    setZValue( loading_->z_ );

    foreach( AppInstance *app_inst, loading_->children_ )
        app_inst->loadTree( load_id_map );
    
    return;
}

void AppInstance::loadRefs( QMap<AppInstance*,AppInstance*> &load_id_map )
{
    AppClass *app_class;
    Q_ASSERT( isLoading() );

    x_align_ = loading_->x_align_;
    x_ref_align_ = loading_->x_ref_align_;
    xRefSet( load_id_map.value( loading_->x_ref_, NULL ) );
    if( x_ref_ == NULL ) xRefSet( parentItem() );

    y_align_ = loading_->y_align_;
    y_ref_align_ = loading_->y_ref_align_;
    yRefSet( load_id_map.value( loading_->y_ref_, NULL ) );
    if( y_ref_ == NULL ) yRefSet( parentItem() );

    w_mod_ = loading_->w_mod_;
    w_units_ = loading_->w_units_;
    wRefSet( load_id_map.value( loading_->w_ref_, NULL ) );

    h_mod_ = loading_->h_mod_;
    h_units_ = loading_->h_units_;
    hRefSet( load_id_map.value( loading_->h_ref_, NULL ) );

    app_class =  AppLibrary::getClass( classId() );
    if( app_class ) class_size_ = app_class->size();

    setupComplete();
    update();

    foreach( AppInstance *app_inst, loading_->children_ ) app_inst->loadRefs( load_id_map );

    delete loading_;
    loading_ = NULL;

    render();
    return;
}

void AppInstance::eventRendered(QImage image, QSizeF pluginsize)
{
    plugin_size_ = pluginsize;
    image_ = image;

    if( mainWindow )
    {
        switch( mainWindow->renderMethod() )
        {
        case 0:
            if( !pixmap_.isNull() ) pixmap_ = QPixmap();
            break;
        case 1:
            pixmap_ = QPixmap::fromImage( image );
            break;
#ifdef QT_OPENGL_LIB
        case 2:
            if( !pixmap_.isNull() ) pixmap_ = QPixmap();
            if( mainWindow->glWidget() )
            {
                mainWindow->glWidget()->makeCurrent();
                if( hastexture_ ) mainWindow->glWidget()->deleteTexture( texture_ );
                texture_ = mainWindow->glWidget()->bindTexture(
                        image,
                        GL_TEXTURE_2D,
                        GL_RGBA,
                        QGLContext::InvertedYBindOption |
                        QGLContext::LinearFilteringBindOption );
                hastexture_ = true;
            }
#endif
            break;
        }
    }

    xwCalculate();
    yhCalculate();

    update();
    emit rendered();
    return;
}

int AppInstance::cleanUnusedSetupItems()
{
    int cleaned = 0;
    foreach( AppSetupItem *asi, setup_items_ )
    {
        if( asi && !asi->isVerified() )
        {
            setup_items_.removeOne( asi );
            delete asi;
            ++ cleaned;
        }
    }
    foreach( AppInstance* app_inst, childItemsByZ() )
        cleaned += app_inst->cleanUnusedSetupItems();
    return cleaned;
}

bool AppInstance::event( QEvent *event )
{
    if( AppObject::event(event) ) return true;
    Q_ASSERT( id().isValid() );

    if( event->type() == EventRequestRender::type() )
    {
        render();
        return true;
    }

    if( event->type() == EventRequestVisibility::type() )
    {
        EventRequestVisibility *e = static_cast<EventRequestVisibility*>(event);
        setVisible(e->visible);
        return true;
    }

    if( event->type() == EventInstanceRendered::type() )
    {
        EventInstanceRendered *e = static_cast<EventInstanceRendered*>(event);
        eventRendered( e->image, e->pluginsize );
        return true;
    }

    qWarning() << "AppInstance::event() unhandled user event" << EventBase::name(event->type()) << "for" << objectName();
    return false;
}
