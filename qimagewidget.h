#ifndef QIMAGEWIDGET_H
#define QIMAGEWIDGET_H

#include <QWidget>
#include <QImage>

#include <QWheelEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QRectF>
#include <QPointF>
#include <QVector>

class QImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QImageWidget(QWidget *parent = nullptr);

    void setImage(const QImage &value);

    QRectF makeInscribedRect(const QRectF &_bound,const QRectF &_source);

    void setHotmap(const QImage &value);

    void clearHotmap();

    QImage getHotmap() const;

    void setRadius(int newRadius);

    void setLabel(uint8_t _label);

    void setColors(const QMap<int, QColor> &newColors);

signals:
    void fileDropped(const QString &filename);

public slots:

protected:
    void paintEvent(QPaintEvent *_event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void dropEvent(QDropEvent *_event);
    void dragEnterEvent(QDragEnterEvent *_event);

private:

    QImage image, hotmap, overlay;
    QRectF inscribedrect;
    int radius;
    uint8_t label;
    qreal scale;
    QPoint translation;
    QPoint translationstart;
    QPoint translationend;
    bool allowtranslation, leftbuttondown;
    QMap<int,QColor> colors;
};

#endif // QIMAGEWIDGET_H
