#include "qimagewidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QJsonObject>

#include <cmath>

#include <QDebug>

bool isNear(const QPointF &_lp, const QPointF &_rp)
{
    return std::sqrt((_lp.x()-_rp.x())*(_lp.x()-_rp.x()) + (_lp.y()-_rp.y())*(_lp.y()-_rp.y())) < 0.05;
}

QImageWidget::QImageWidget(QWidget *parent) : QWidget(parent),
    radius(3),
    label(100),
    scale(1),    
    allowtranslation(false),
    leftbuttondown(false)
{
    setAcceptDrops(true);
}

void prepare_overlay(QImage &rgba, const QImage &gs, const QMap<int,QColor> &colors)
{
    QColor color;
    for(int y = 0; y < gs.height(); ++y) {
        uchar *alpha = rgba.scanLine(y);
        const uchar *labels = gs.scanLine(y);
        for(int x = 0; x < gs.width(); ++x) {
            if(labels[x] != 0) {
                color = colors.value(labels[x],Qt::transparent);
                alpha[4*x] = color.red();
                alpha[4*x+1] = color.green();
                alpha[4*x+2] = color.blue();
            }
            alpha[4*x+3] = labels[x] != 0 ? 100 : 0;
        }
    }
}


void QImageWidget::paintEvent(QPaintEvent *_event)
{
    Q_UNUSED(_event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(translation);
    painter.scale(scale,scale);

    painter.fillRect(rect(),Qt::Dense6Pattern);
    if(!image.isNull()) {
        painter.drawImage(inscribedrect,image);
        prepare_overlay(overlay,hotmap,colors);
        painter.drawImage(inscribedrect,overlay);
    } else {
        QFont _font = painter.font();
        _font.setPointSizeF(_font.pointSizeF()*2);
        painter.setFont(_font);
        painter.setPen(Qt::darkGray);
        painter.drawText(rect(),Qt::AlignCenter,tr("drop dicom file right here"));
    }
}

void QImageWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    if(!image.isNull())
        inscribedrect = makeInscribedRect(rect(),image.rect());
    else
        inscribedrect = QRectF();
}

bool insidecircle(int x, int y, int cx, int cy, int r)
{
    float dx = cx - x - 0.5f;
    float dy = cy - y - 0.5f;
    if(std::sqrt(dx*dx + dy*dy) < r - 0.1f)
        return true;
    return false;
}

void setHotmapPixelValue(QImage &greyscale, const QPoint &point, int radius, uchar value)
{   
    for(int y = point.y() - radius; y < point.y() + radius; ++y) {
        uchar *row = greyscale.scanLine(y);
        for(int x = point.x() - radius; x < point.x() + radius; ++x) {
            if(greyscale.valid(x,y)) {
                if(insidecircle(x,y,point.x(),point.y(),radius)) {
                    row[x] = value;
                }
            }
        }
    }
}

void QImageWidget::mousePressEvent(QMouseEvent *event)
{
    if(inscribedrect.isValid() && !hotmap.isNull() && (event->button() == Qt::LeftButton)) {
        leftbuttondown = true;
        QPointF _point((event->x()/scale - inscribedrect.x() - translation.x()/scale)/inscribedrect.width(),
                       (event->y()/scale - inscribedrect.y() - translation.y()/scale)/inscribedrect.height());
        //qDebug("Absolute (%d; %d) >> relative to image's rect (%.3f; %.3f)",event->x(),event->y(),_point.x(),_point.y());
        setHotmapPixelValue(hotmap, QPoint(_point.x()*hotmap.width(),_point.y()*hotmap.height()),radius, label);
        update();
    } else if(event->button() == Qt::MiddleButton) {
        allowtranslation = true;
        translationstart = event->pos();
    }
}

void QImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(leftbuttondown) {
        if(inscribedrect.isValid() && !hotmap.isNull()) {
            QPointF _point((event->x()/scale - inscribedrect.x() - translation.x()/scale)/inscribedrect.width(),
                           (event->y()/scale - inscribedrect.y() - translation.y()/scale)/inscribedrect.height());
            setHotmapPixelValue(hotmap, QPoint(qRound(_point.x()*hotmap.width()),qRound(_point.y()*hotmap.height())),radius, label);
            update();
        }
    } else if(allowtranslation) {
        translation = translationend + event->pos() - translationstart;
        update();
    }
}

void QImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if(leftbuttondown) {
        leftbuttondown = false;
    }
    if(allowtranslation) {
        translationend = translation;
        allowtranslation = false;
        update();
    }
}

void QImageWidget::wheelEvent(QWheelEvent *event)
{
    const qreal _oldscale = scale;
    QPoint numDegrees = event->angleDelta() / 120;
    //qDebug("wheel: %d, %d",numDegrees.x(),numDegrees.y());
    scale += 0.15f * scale * numDegrees.y();
    if(scale < 1.0f)
        scale = 1.0f;
    else if(scale > 20.0f)
        scale = 20.0f;
    if(numDegrees.y() != 0) {
        // Math is awesome!
        translation =  event->pos()*(1.0f - scale/_oldscale) + translation * scale/_oldscale;
        translationstart = translation;
        translationend = translation;
    }
    update();
}

void QImageWidget::dropEvent(QDropEvent *_event)
{
    QString _filename = QUrl(_event->mimeData()->text()).path();
#ifdef Q_OS_WIN
    _filename = _filename.section('/',1);
#endif
    emit fileDropped(QFileInfo(_filename).path());
}

void QImageWidget::dragEnterEvent(QDragEnterEvent *_event)
{
    if(_event->mimeData()->hasText())
        _event->acceptProposedAction();
}

void QImageWidget::setColors(const QMap<int, QColor> &newColors)
{
    colors = newColors;
}

void QImageWidget::setRadius(int newRadius)
{
    radius = newRadius;
}

void QImageWidget::setLabel(uint8_t _label)
{
    label = _label;
}

void QImageWidget::setImage(const QImage &value)
{
    // Drop scale and translation
    translation.setX(0);
    translation.setY(0);
    translationstart.setX(0);
    translationend.setY(0);
    translationend.setX(0);
    translationend.setY(0);
    scale = 1;
    // Update image
    image = value;
    if(!image.isNull())
        inscribedrect = makeInscribedRect(rect(),image.rect());
    else
        inscribedrect = QRectF();
    update();
}

QRectF QImageWidget::makeInscribedRect(const QRectF &_bound, const QRectF &_source)
{
    static float padding_factor = 1.0f;
    QRectF _output;
    if(_bound.width()/_bound.height() > _source.width()/_source.height()) {
        _output.setHeight(_bound.height() / padding_factor);
        _output.setWidth((_bound.height()/ padding_factor) * _source.width()/_source.height());
        _output.moveLeft((_bound.width() - _output.width())/2.0);
        _output.moveTop((_bound.height() - _output.height())/2.0);
    } else {
        _output.setWidth(_bound.width() / padding_factor);
        _output.setHeight((_bound.width() / padding_factor) * _source.height()/_source.width());
        _output.moveTop((_bound.height() - _output.height())/2.0);
        _output.moveLeft((_bound.width() - _output.width())/2.0);
    }
    return _output;
}

void QImageWidget::setHotmap(const QImage &value)
{
    hotmap = value.copy();
    overlay = QImage(hotmap.width(),hotmap.height(),QImage::Format_RGBA8888);
    overlay.fill(0);
    update();
}

void QImageWidget::clearHotmap()
{
    if(!image.isNull()) {
        hotmap = QImage(image.width(),image.height(),QImage::Format_Grayscale8);
        hotmap.fill(0);
        overlay = QImage(hotmap.width(),hotmap.height(),QImage::Format_RGBA8888);
        overlay.fill(0);
        update();
    }
}

QImage QImageWidget::getHotmap() const
{
    return hotmap;
}

