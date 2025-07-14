#pragma once
#ifndef SPECTROGRAMVIEW_H
#define SPECTROGRAMVIEW_H

#include <QImage>
#include <QMutex>
#include <QVector>
#include <QWidget>

class SpectrogramView : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrogramView(QWidget *parent = nullptr);
    ~SpectrogramView() override = default;

public slots:

    void addSpectrumSlice(const QVector<double> &freqBins, const QVector<double> &magnitudes);

    void clear();

    void setSpectrogramData(const QVector<QVector<double>> &data);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QImage m_image;
    int m_maxTimeSlices = 500;

    int m_freqBinCount = 0;

    QVector<QVector<double>> m_spectrogramData;
    QMutex m_mutex;

    void updateImage();

    QColor magnitudeToColor(double magnitude) const;
};

#endif
