#include "gridwidget.h"
#include <QPainter>
#include <QPen>

GridWidget::GridWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
}

void GridWidget::setSamples(const QVector<double>& samples, quint32 sampleRate)
{
    m_samples = samples;
    m_sampleRate = sampleRate;
    update(); // перерисовать виджет
}

void GridWidget::setMarkerPosition(double seconds)
{
    if (m_markerPosition != seconds) {
        m_markerPosition = seconds;
        update();
        emit markerPositionChanged(seconds);
    }
}

void GridWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Черный фон
    painter.fillRect(rect(), Qt::black);

    // Сетка
    QPen gridPen(QColor(80, 80, 80));
    gridPen.setStyle(Qt::DotLine);
    painter.setPen(gridPen);

    const int gridSize = 25;

    for (int x = 0; x < width(); x += gridSize)
        painter.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += gridSize)
        painter.drawLine(0, y, width(), y);

    // Рисуем форму волны, если есть сэмплы
    if (!m_samples.isEmpty()) {
        QPen wavePen(Qt::green);
        painter.setPen(wavePen);

        int w = width();
        int h = height();
        int sampleCount = m_samples.size();

        // Рисуем линию формы волны по горизонтали
        for (int x = 0; x < w - 1; ++x) {
            int sampleIndex1 = (x * sampleCount) / w;
            int sampleIndex2 = ((x + 1) * sampleCount) / w;

            double y1 = h / 2 * (1.0 - m_samples[sampleIndex1]);
            double y2 = h / 2 * (1.0 - m_samples[sampleIndex2]);

            painter.drawLine(x, static_cast<int>(y1), x + 1, static_cast<int>(y2));
        }
    }

    // Рисуем маркер позиции, если установлен
    if (m_markerPosition > 0 && m_sampleRate > 0) {
        double totalDuration = static_cast<double>(m_samples.size()) / m_sampleRate;
        int x = static_cast<int>((m_markerPosition / totalDuration) * width());

        QPen markerPen(Qt::red);
        markerPen.setWidth(2);
        painter.setPen(markerPen);

        painter.drawLine(x, 0, x, height());
    }
}
