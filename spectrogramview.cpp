#include "spectrogramview.h"
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>
#include <cmath>

SpectrogramView::SpectrogramView(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(150); // Минимальная высота для удобного просмотра спектрограммы
}

void SpectrogramView::addSpectrumSlice(const QVector<double>& freqBins, const QVector<double>& magnitudes)
{
    QMutexLocker locker(&m_mutex);

    // Проверяем, что частотные полосы и амплитуды соответствуют по размеру
    if (freqBins.size() != magnitudes.size())
        return;

    // Устанавливаем количество частотных полос при первой вставке
    if (m_freqBinCount == 0)
        m_freqBinCount = freqBins.size();

    // Все срезы должны иметь одинаковое число частотных полос
    if (freqBins.size() != m_freqBinCount)
        return;

    m_spectrogramData.append(magnitudes);

    // Ограничиваем количество срезов, чтобы не расходовать слишком много памяти
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

void SpectrogramView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    QMutexLocker locker(&m_mutex);

    if (m_image.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "No spectrogram data");
        return;
    }

    // Рисуем изображение спектрограммы масштабированным под размер виджета
    painter.drawImage(rect(), m_image);
}

void SpectrogramView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateImage(); // При изменении размера виджета пересчитываем изображение с масштабированием
}

void SpectrogramView::updateImage()
{
    if (m_spectrogramData.isEmpty() || m_freqBinCount == 0)
        return;

    const int width = m_spectrogramData.size(); // количество временных срезов — ширина изображения
    const int height = m_freqBinCount;           // количество частотных полос — высота изображения

    QImage img(width, height, QImage::Format_RGB32);

    // Заполняем изображение, цветом кодируя амплитуду
    for (int x = 0; x < width; ++x) {
        const QVector<double>& magnitudes = m_spectrogramData[x];
        for (int y = 0; y < height; ++y) {
            // Отображаем частоты снизу вверх — инвертируем y
            int imgY = height - 1 - y;
            QColor col = magnitudeToColor(magnitudes[y]);
            img.setPixelColor(x, imgY, col);
        }
    }

    // Масштабируем изображение под размер виджета, без сохранения пропорций, с плавной интерполяцией
    m_image = img.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QColor SpectrogramView::magnitudeToColor(double magnitude) const
{
    // Нормализация амплитуды к диапазону [0,1] с ограничением
    constexpr double maxMagnitude = 1.0;
    double norm = std::clamp(magnitude / maxMagnitude, 0.0, 1.0);

    int intensity = static_cast<int>(norm * 255);

    // Возвращаем цвет: желтый оттенок, где интенсивность задаёт яркость
    return QColor(intensity, intensity, 0);
}
