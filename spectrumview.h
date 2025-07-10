#pragma once
#ifndef SPECTRUMVIEW_H
#define SPECTRUMVIEW_H

#include <QChartView>
#include <QSplineSeries>
#include <QValueAxis>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QVector>

class SpectrumView : public QChartView
{
    Q_OBJECT

public:
    explicit SpectrumView(QWidget* parent = nullptr);
    ~SpectrumView() override = default;

public slots:
    void setSpectrum(const QVector<double>& freq, const QVector<double>& amp);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QSplineSeries* m_series;
    QValueAxis*    m_axisX;
    QValueAxis*    m_axisY;

    void ensureNonZeroRange(double& min, double& max, double padding = 1.0);
};

#endif // SPECTRUMVIEW_H
