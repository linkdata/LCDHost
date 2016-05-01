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

#ifndef APPINSTANCE_H
#define APPINSTANCE_H

#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsLayoutItem>
#include <QMutex>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include <QTime>
#include <QVector>
#include <QFlags>
#include <QWidgetItem>

#include <QImage>
#include <QPixmap>

#ifdef QT_OPENGL_LIB
#include <QGLPixelBuffer>
#include <QGLFramebufferObject>
#endif

#include "AppObject.h"
#include "AppSetupItem.h"
#include "AppGraphicsScene.h"
#include "AppClass.h"

class QWidget;
class QDataStream;
class LibInstance;
class AppInstanceTree;
class AppInstance;

class AppInstanceLoading
{
public:
    AppInstanceLoading()
    {
        parent_ = NULL;
        id_ = x_ref_ = y_ref_ = w_ref_ = h_ref_ = NULL;
        z_ = w_mod_ = h_mod_ = 0.0;
        x_align_ =  y_align_ = x_ref_align_ = y_ref_align_ = 0;
        w_units_ = h_units_ = 0;
    }

    AppInstance *parent_; // pointer to parent AppInstance (real pointer, not loaded off storage)
    QList<AppInstance*> children_; // children (real pointers)
    AppInstance *id_; // id as loaded off storage (NULL when done loading)
    QPointF pos_; // final pos wanted as loaded off storage
    qreal z_; // final Z wanted as loaded off storage
    AppInstance *x_ref_, *y_ref_;
    qint32 x_align_,      y_align_;
    qint32 x_ref_align_,  y_ref_align_;
    AppInstance *w_ref_, *h_ref_;
    qreal  w_mod_,        h_mod_;
    qint32 w_units_,      h_units_;
};

class AppInstance : public AppObject, public QGraphicsItem, public QGraphicsLayoutItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_INTERFACES(QGraphicsLayoutItem)

public:
    AppInstance(); // Used by rootitem
    AppInstance( QString class_id ); // Used when loading from saves with unknown classes
    AppInstance(AppClass *app_class, const QString &name = QString() );
    AppInstance( AppInstance const& src );
    ~AppInstance();

    // two stage create/destroy
    virtual void init();
    virtual void ready() { AppObject::ready(); render(); }
    virtual void term();

    virtual QString path() const;

    // QGraphicsItem stuff
    QVariant itemChange( GraphicsItemChange change, const QVariant & value );
    AppGraphicsScene * scene () const { return static_cast<AppGraphicsScene*>( QGraphicsItem::scene() ); }
    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
    void keyPressEvent( QKeyEvent * event );
    int getGrabPoint( QPointF pos );
    void setCursorForMode( int e );
    void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
    void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );


    // QGraphicsLayoutItem stuff
    QSizeF sizeHint( Qt::SizeHint which, const QSizeF & constraint = QSizeF() ) const;
    void updateGeometry();

    AppLibrary *appLibrary() const;
    AppInstance *parentItem() const { return static_cast<AppInstance *>(QGraphicsItem::parentItem()); }
    void setParentItem( AppInstance *p );
    bool hasParentItem( AppInstance *p );

    int columnCount() const { return 3; }
    QVariant data( int column, int role );
    static QVariant headerData( int section, Qt::Orientation orientation, int role );

    QList<AppInstance *> childItemsByZ () const;
    void allChildren( QList<AppInstance*> & toList ) const;

    bool isLoading() const { return loading_ != NULL; }
    bool isModifiable() const { return modifiable_; }
    void setModifiable(bool b) { modifiable_ = b; }
    bool isSelectable() const { return selectable_; }
    void setSelectable(bool b) { selectable_ = b; }
    bool isAlertEnabled() const { return alerts_; }
    void setAlertEnabled(bool b) { alerts_ = b; }

    QPointF offset() const { return QPointF(0,0); }

    // Size stuff
    QSizeF size() const { return size_; }
    void setSize( QSizeF size );
    int width() const { return size_.width(); }
    int height() const { return size_.height(); }

    // QRectF sceneRect() const { return QRectF( pos(), size() ); }
    // QRectF clientRect() const { return clientrect_; }
    // void setClientRect( QRectF rect ) { clientrect_ = rect; }

    // Image
    QImage image() const { return image_; }
    QPixmap pixmap() const { return pixmap_; }
    QRectF boundingRect() const;

    QString classId() const { return class_id_; }
    void setClassId( QString id );
    QSize classSize() const { return class_size_; }
    void setClassSize(QSize s) { class_size_ = s; }

    void sceneGeometryChanged();
    // void setVisible( bool visible );
    // void refreshTree();


    // Positioning.
    //
    // What we can't change, inherited from the GraphicsItem:
    //   pos() is in parent coordinates, unless there is no parent (not an issue)
    //   Moving the parent within scene moves the item within scene
    //
    // Maintain origin() in parent coords, calculated from reference and rules, but
    // not including offset. Offset is expressed as difference between pos() and origin().
    //
    // We must restrict referenced items to those that share the same coordinate system,
    // which means siblings or parent only. Otherwise we go into scenePos() hell as soon
    // as something is moved.
    //
    // In order to support client area, we need to decouple our coordinate system from
    // the QGraphicsItem. Otherwise we get 'offset drift' when the client area changes.

    QPointF origin() { return origin_; } // origin in parent coords from x/y_ref_id_ and alignments
    void setScenePos( QPointF pos );

    // XW
    qreal xwCalculate();
    qreal yhCalculate();

    // X
    void xRefReset();
    void xRefSet( AppInstance * );
    AppInstance *xRef() { return x_ref_; }
    void xRefBlank() { xRefReset(); x_align_ = x_ref_align_ = 0; xwCalculate(); }
    int xRefAlign() { return x_ref_align_; }
    int xAlign() { return x_align_; }
    void xRefAlignSet(int n) { x_ref_align_ = n; xwCalculate(); }
    void xAlignSet(int n) { x_align_ = n; xwCalculate(); }
    int xOffset() { return (int) (pos().x() - origin().x()); }
    void xOffsetSet( int x ) { setPos( origin().x() + x, pos().y() ); xwCalculate(); }

    // Y
    void yRefReset();
    void yRefSet( AppInstance * );
    AppInstance *yRef() { return y_ref_; }
    void yRefBlank() { yRefReset(); y_align_ = y_ref_align_ = 0; yhCalculate(); }
    int yRefAlign() { return y_ref_align_; }
    int yAlign() { return y_align_; }
    void yRefAlignSet(int n) { y_ref_align_ = n; yhCalculate(); }
    void yAlignSet(int n) { y_align_ = n; yhCalculate(); }
    int yOffset() { return (int) (pos().y() - origin().y()); }
    void yOffsetSet( int y ) { setPos( pos().x(), origin().y() + y ); yhCalculate(); }

    static qreal valueFromMod( int method, int mod, qreal xy, qreal old_wh, qreal rxy, qreal rwh );
    static int modFromValue( int method, int old_mod, qreal xy, qreal wh, qreal rxy, qreal rwh );

    // W
    void wRefReset();
    void wRefBlank();
    void wRefSet( AppInstance * );
    AppInstance *wRef() { return w_ref_; }
    int wMod() { return (int) w_mod_; }
    int wUnits() { return w_units_; }
    void wModSet( int n );
    void wUnitsSet( int units );

    qreal wApplyMod( AppInstance *ref_inst, qreal old_w )
    {
        return valueFromMod( w_units_, w_mod_, scenePos().x(), old_w, ref_inst ? ref_inst->scenePos().x() : 0, old_w );
    }
    bool wSetModForWidth( qreal w )
    {
        int old_mod = w_mod_;
        w_mod_ = modFromValue( w_units_, w_mod_, scenePos().x(), w, w_ref_ ? w_ref_->scenePos().x() : 0, w_ref_ ? w_ref_->size().width() : plugin_size_.width() );
        return w_mod_ != old_mod;
    }

    // H
    void hRefReset();
    void hRefSet( AppInstance * );
    AppInstance *hRef() { return h_ref_; }
    void hRefBlank() { hRefReset(); h_mod_ = 0; h_units_ = 0; yhCalculate(); }
    int hMod() { return (int) h_mod_; }
    int hUnits() { return h_units_; }
    void hModSet( int n );
    void hUnitsSet( int units );

    qreal hApplyMod( AppInstance *ref_inst, qreal old_h )
    {
        return valueFromMod( h_units_, h_mod_, scenePos().y(), old_h, ref_inst ? ref_inst->scenePos().y() : 0, old_h );
    }
    bool hSetModForHeight( qreal h )
    {
        int old_mod = h_mod_;
        h_mod_ = modFromValue( h_units_, h_mod_, scenePos().y(), h, h_ref_ ? h_ref_->scenePos().y() : 0, h_ref_ ? h_ref_->size().height() : plugin_size_.height() );
        return h_mod_ != old_mod;
    }

    void refreshDetails();

    bool load( QXmlStreamReader & stream, AppInstance *parent = 0 );
    void save( QXmlStreamWriter & stream );

    bool load( QDataStream & stream, AppInstance *parent = 0 );
    void save( QDataStream & stream );

    void loadMap( QMap<AppInstance*,AppInstance*> &load_id_map );
    void loadTree( QMap<AppInstance*,AppInstance*> &load_id_map );
    void loadRefs( QMap<AppInstance*,AppInstance*> &load_id_map );

    void render();
    void eventRendered( QImage image, QSizeF pluginsize );

    int cleanUnusedSetupItems();
    bool event( QEvent * );

    static AppInstance *fromId( AppId id ) { return qobject_cast<AppInstance*>(id.appObject()); }
    static int xw_calc_count;
    static int yh_calc_count;

signals:
    void rendered();

protected:
    void timerEvent(QTimerEvent *event);
    void clear();

private:
    static const quint32 magicV1 = 0x19011337;
    static const quint32 magicV2 = 0x009AFB71;
    AppInstanceLoading *loading_; // NULL if not in load phase

    QString class_id_;
    QSize class_size_;
    QPointF origin_;
    QSizeF size_;
    QSizeF plugin_size_;     // wanted size reported by plugin

    AppInstance *x_ref_, *y_ref_;
    qint32 x_align_,      y_align_;
    qint32 x_ref_align_,  y_ref_align_;

    AppInstance *w_ref_, *h_ref_;
    qreal  w_mod_,        h_mod_;
    qint32 w_units_,      h_units_;

    bool xw_lock_, yh_lock_;
    QVector<AppInstance*> xw_kids_, yh_kids_;

    bool modifiable_; // allow user to modify the instance
    bool selectable_; // allow user to select instance in layout
    bool alerts_; // allow plugin alerts to override z order

    // Rendering targets
    QImage image_;       // image generated by plugin or a placeholder
    QPixmap pixmap_;
#ifdef QT_OPENGL_LIB
    bool hastexture_;
    GLuint texture_;
#endif

    bool placeholder_;   // true if the image is placeholder and not generated by plugin
    int selectiontimer_; // used to animate the selection border

    int dragmode_;
};

#endif // APPINSTANCE_H
