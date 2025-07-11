#include "spectrogramview.h"
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>
#include <cmath>

SpectrogramView::SpectrogramView(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(150);
}

void SpectrogramView::addSpectrumSlice(const QVector<double>& freqBins, const QVector<double>& magnitudes)
{
    QMutexLocker locker(&m_mutex);

    if (freqBins.size() != magnitudes.size())
        return;

    if (m_freqBinCount == 0)
        m_freqBinCount = freqBins.size();

    if (freqBins.size() != m_freqBinCount)
        return;

    m_spectrogramData.append(magnitudes);

    if (m_spectrogramData.size() > m_maxTimeSlices)
        m_spectrogramData.pop_front();

    updateImage();
    update();
}

void SpectrogramView::setSpectrogramData(const QVector<QVector<double>>& data)
{
    QMutexLocker locker(&m_mutex);

    m_spectrogramData = data;
    m_freqBinCount = data.isEmpty() ? 0 : (data[0].size());

    updateImage();
    update();
}

void SpectrogramView::clear()
{
    QMutexLocker locker(&m_mutex);

    m_spectrogramData.clear();
    m_image = QImage();
    update();
}

void SpectrogramView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    QMutexLocker locker(&m_mutex);

    if (m_image.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "No spectrogram data");
        return;
    }

    painter.drawImage(rect(), m_image);
}

void SpectrogramView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateImage();
}

void SpectrogramView::updateImage()
{
    if (m_spectrogramData.isEmpty() || m_freqBinCount == 0)
        return;

    const int width = m_spectrogramData.size();
    const int height = m_freqBinCount;

    QImage img(width, height, QImage::Format_RGB32);

    for (int x = 0; x < width; ++x) {
        const QVector<double>& magnitudes = m_spectrogramData[x];
        for (int y = 0; y < height; ++y) {
            int imgY = height - 1 - y;
            QColor col = magnitudeToColor(magnitudes[y]);
            img.setPixelColor(x, imgY, col);
        }
    }

    m_image = img.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QColor SpectrogramView::magnitudeToColor(double magnitude) const
{
    constexpr double maxMagnitude = 1.0;
    double norm = std::clamp(magnitude / maxMagnitude, 0.0, 1.0);

    int intensity = static_cast<int>(norm * 255);

    return QColor(intensity, intensity, 0);
}
