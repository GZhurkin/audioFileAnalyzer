#pragma once
#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QPainterPath>
#include <QScrollBar>
#include <QVector>
#include <QWidget>

class WaveformView : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformView(QWidget *parent = nullptr);

public slots:

    void setSamples(const QVector<double> &samples, quint32 sampleRate);

    void setMarkerPosition(double seconds);

signals:

    void markerPositionChanged(double seconds);

protected:
    void paintEvent(QPaintEvent *ev) override;

    void resizeEvent(QResizeEvent *ev) override;

    void mousePressEvent(QMouseEvent *ev) override;

    void mouseMoveEvent(QMouseEvent *ev) override;

    void mouseReleaseEvent(QMouseEvent *ev) override;

    void wheelEvent(QWheelEvent *ev) override;

private:
    QVector<double> m_samples;
    quint32 m_sampleRate = 0;
    double m_markerSec = 0.0;

    double m_zoom = 1.0;
    const double m_minZoom = 0.5;
    const double m_maxZoom = 100.0;

    QScrollBar *m_hScroll = nullptr;
    bool m_draggingMarker = false;

    QPainterPath m_cachedPath;
    int m_cachedOffset = -1;
    QSize m_cachedSize;

    void updateScroll();

    void updateMarkerFromPos(int x);

    void updateCachedPath();
};

#endif // WAVEFORMVIEW_H
