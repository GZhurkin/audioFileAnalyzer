// spectrogramview.cpp
#include "spectrogramview.h"
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>
#include <cmath>

SpectrogramView::SpectrogramView(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(150); // Минимальная высота виджета
}

// Добавление нового среза спектра
void SpectrogramView::addSpectrumSlice(const QVector<double> &freqBins,
                                       const QVector<double> &magnitudes)
{
    QMutexLocker locker(&m_mutex); // Защита от конкурентного доступа

    // Проверка согласованности данных
    if (freqBins.size() != magnitudes.size())
        return;

    // Инициализация при первом вызове
    if (m_freqBinCount == 0)
        m_freqBinCount = freqBins.size();

    if (freqBins.size() != m_freqBinCount)
        return;

    // Добавление данных с ограничением истории
    m_spectrogramData.append(magnitudes);
    if (m_spectrogramData.size() > m_maxTimeSlices)
        m_spectrogramData.pop_front(); // Удаление устаревших данных

    updateImage();
    update(); // Запрос перерисовки
}

// Установка новых данных спектрограммы
void SpectrogramView::setSpectrogramData(const QVector<QVector<double>> &data)
{
    QMutexLocker locker(&m_mutex);

    m_spectrogramData = data;
    m_freqBinCount = data.isEmpty() ? 0 : data[0].size();

    updateImage();
    update();
}

void SpectrogramView::clear() // Очистка данных спектрограммы
{
    QMutexLocker locker(&m_mutex);

    m_spectrogramData.clear();
    m_image = QImage(); // Сброс изображения
    update();
}

// Отрисовка виджета
void SpectrogramView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black); // Черный фон

    QMutexLocker locker(&m_mutex);

    // Отображение заглушки при отсутствии данных
    if (m_image.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "No spectrogram data");
        return;
    }

    // Отрисовка спектрограммы
    painter.drawImage(rect(), m_image);
}

// Обработка изменения размера виджета
void SpectrogramView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateImage(); // Обновление изображения под новый размер
}

// Генерация изображения спектрограммы
void SpectrogramView::updateImage()
{
    if (m_spectrogramData.isEmpty() || m_freqBinCount == 0)
        return;

    const int width = m_spectrogramData.size(); // Временные отсчеты (ось X)
    const int height = m_freqBinCount;          // Частотные бины (ось Y)

    QImage img(width, height, QImage::Format_RGB32);

    // Преобразование данных в пиксели
    for (int x = 0; x < width; ++x) {
        const QVector<double> &magnitudes = m_spectrogramData[x];
        for (int y = 0; y < height; ++y) {
            int imgY = height - 1 - y; // Инвертирование Y (низкие частоты внизу)
            QColor col = magnitudeToColor(magnitudes[y]);
            img.setPixelColor(x, imgY, col);
        }
    }

    // Масштабирование под текущий размер виджета
    m_image = img.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

// Преобразование величины амплитуды в цвет
QColor SpectrogramView::magnitudeToColor(double magnitude) const
{
    constexpr double maxMagnitude = 1.0;
    double norm = std::clamp(magnitude / maxMagnitude, 0.0, 1.0);

    // Градации желтого: от черного (0) до желтого (1)
    int intensity = static_cast<int>(norm * 255);
    return QColor(intensity, intensity, 0);
}
