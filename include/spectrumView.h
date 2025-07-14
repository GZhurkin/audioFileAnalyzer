#pragma once
#ifndef SPECTRUMVIEW_H
#define SPECTRUMVIEW_H

#include <QLinearGradient>
#include <QMutex>
#include <QPoint>
#include <QRubberBand>
#include <QVector>
#include <QWidget>

class SpectrumView : public QWidget
{
    Q_OBJECT
public:
    explicit SpectrumView(QWidget *parent = nullptr);
    ~SpectrumView() override = default;

    void setFrequencyRange(double minFreq, double maxFreq);
    void setDecibelRange(double minDB, double maxDB);

public slots:
    void setSpectrumData(const QVector<double> &frequencies, const QVector<double> &magnitudes);
    void clear();
    void zoomReset();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    struct SpectrumPoint
    {
        double frequency;
        double magnitude;
    };

    QVector<SpectrumPoint> m_spectrumData;
    QMutex m_mutex;

    double m_minFrequency = 20.0;
    double m_maxFrequency = 20000.0;
    double m_minDB = -100.0;
    double m_maxDB = 100.0;
    double m_zoomFactor = 1.0;
    double m_panOffset = 0.0;

    QLinearGradient m_spectrumGradient;
    QColor m_lineColor = QColor(138, 43, 226);

    QPoint m_zoomStart;
    QPoint m_zoomEnd;
    QRubberBand *m_rubberBand;
    bool m_isPanning = false;
    QPoint m_lastPanPoint;

    void drawGrid(QPainter &painter);
    void drawSpectrum(QPainter &painter);
    void drawLabels(QPainter &painter);
    void applyZoom(const QRect &zoomRect);
    QPointF dataToPoint(double freq, double mag) const;
    QPointF pointToData(const QPoint &point) const;

    void updateGradient();
    double getVisibleMinFreq() const;
    double getVisibleMaxFreq() const;
};

#endif
