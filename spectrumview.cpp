#include "spectrumview.h"
#include <QtCharts/QChart>
#include <algorithm>

SpectrumView::SpectrumView(QWidget* parent)
    : QChartView(new QChart(), parent)
    , m_series(new QSplineSeries(this))
    , m_axisX(new QValueAxis(this))
    , m_axisY(new QValueAxis(this))
{
    QChart* chartObj = chart();
    chartObj->addSeries(m_series);

    // Настройка оси X (частота)
    m_axisX->setTitleText("Frequency (Hz)");
    chartObj->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    // Настройка оси Y (амплитуда)
    m_axisY->setTitleText("Amplitude");
    chartObj->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    setRenderHint(QPainter::Antialiasing);     // Для сглаженной отрисовки линий
    setRubberBand(QChartView::RectangleRubberBand); // Возможность выделения области зума мышью
}

void SpectrumView::ensureNonZeroRange(double& min, double& max, double padding) {
    // Если диапазон нулевой (min == max), расширяем его на padding с обеих сторон,
    // чтобы избежать проблем при установке оси в QtCharts (там нулевой диапазон не разрешён)
    if (min == max) {
        min -= padding;
        max += padding;
    }
}

void SpectrumView::setSpectrum(const QVector<double>& freq, const QVector<double>& amp) {
    m_series->clear();
    int n = qMin(freq.size(), amp.size());

    // Если данных слишком мало, выставляем диапазоны по умолчанию и выходим
    if (n <= 1) {
        m_axisX->setRange(0, 1);
        m_axisY->setRange(0, 1);
        return;
    }

    // Заполняем серию точками спектра
    for (int i = 0; i < n; ++i)
        m_series->append(freq[i], amp[i]);

    // Определяем диапазон оси X по частотам
    double xmin = freq.first();
    double xmax = freq.last();
    ensureNonZeroRange(xmin, xmax, (xmax - xmin) * 0.05);
    m_axisX->setRange(xmin, xmax);

    // Определяем диапазон оси Y по амплитудам,
    // минимальная амплитуда не ниже 0, так как амплитуды неотрицательны
    auto [minIt, maxIt] = std::minmax_element(amp.constBegin(), amp.constEnd());
    double ymin = std::max(0.0, *minIt);
    double ymax = *maxIt;
    ensureNonZeroRange(ymin, ymax, (ymax - ymin) * 0.05);
    m_axisY->setRange(ymin, ymax);
}

void SpectrumView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton)
        chart()->zoomReset();  // Правая кнопка мыши — сбросить зум
    QChartView::mouseReleaseEvent(event);
}

void SpectrumView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Plus) {
        chart()->zoomIn();  // Клавиша "+" — приблизить
    } else if (event->key() == Qt::Key_Minus) {
        chart()->zoomOut(); // Клавиша "-" — отдалить
    } else {
        QChartView::keyPressEvent(event);
    }
}
