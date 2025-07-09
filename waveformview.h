// waveformview.h
#pragma once
#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QWidget>
#include <QVector>
#include <QScrollBar>

class WaveformView : public QWidget {
    Q_OBJECT
public:
    explicit WaveformView(QWidget* parent = nullptr);

public slots:
    // samples — нормированные амплитуды [-1..1]
    void setSamples(const QVector<double>& samples, quint32 sampleRate);
    // seconds — текущее время воспроизведения
    void setMarkerPosition(double seconds);

signals:
    // Сигнал при перетаскивании маркера мышью
    void markerPositionChanged(double seconds);

protected:
    void paintEvent(QPaintEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    // Горизонтальный зум (Ctrl + колесо)
    void wheelEvent(QWheelEvent* ev) override;

private:
    QVector<double> m_samples;
    quint32 m_sampleRate = 0;
    double m_markerSec = 0.0;

    // текущий масштаб (1.0 = 100%)
    double m_zoom = 1.0;
    const double m_minZoom = 0.5;
    const double m_maxZoom = 10.0;

    QScrollBar* m_hScroll = nullptr;
    bool m_draggingMarker = false;

    void updateScroll();
    void updateMarkerFromPos(int x);
};

#endif // WAVEFORMVIEW_H
