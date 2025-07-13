#include "spectrumview.h"
#include <QChart>
#include <algorithm>

SpectrumView::SpectrumView(QWidget* parent)
    : QChartView(new QChart(), parent)
    , m_series(new QSplineSeries())
    , m_axisX(new QValueAxis())
    , m_axisY(new QValueAxis())
{
    auto* c = chart();
    c->addSeries(m_series);

    m_axisX->setTitleText("Frequency (Hz)");
    c->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY->setTitleText("Amplitude");
    c->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    setRenderHint(QPainter::Antialiasing);
    setRubberBand(QChartView::RectangleRubberBand); // 1.2.2.2
}

void SpectrumView::ensureNonZeroRange(double& min, double& max, double padding)
{
    if (min == max) {
        min -= padding;
        max += padding;
    }
}

void SpectrumView::setSpectrum(const QVector<double>& freq, const QVector<double>& amp)
{
    m_series->clear();
    int n = qMin(freq.size(), amp.size());
    if (n <= 1) {
        m_axisX->setRange(0, 1);
        m_axisY->setRange(0, 1);
        return;
    }

    // Предполагаем отсортированный freq
    for (int i = 0; i < n; ++i)
        m_series->append(freq[i], amp[i]);

    // X‑диапазон
    double xmin = freq.first(), xmax = freq.last();
    ensureNonZeroRange(xmin, xmax, (xmax - xmin)*0.05);
    m_axisX->setRange(xmin, xmax);

    // Y‑диапазон
    auto [itMin, itMax] = std::minmax_element(amp.constBegin(), amp.constEnd());
    double ymin = *itMin, ymax = *itMax;
    if (ymin < 0) ymin = 0;  // амплитуды ≥0
    ensureNonZeroRange(ymin, ymax, (ymax - ymin)*0.05);
    m_axisY->setRange(ymin, ymax);
}

void SpectrumView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
        chart()->zoomReset();  // 1.2.2.2 сброс правым кликом
    QChartView::mouseReleaseEvent(event);
}

void SpectrumView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Plus)
        chart()->zoomIn();
    else if (event->key() == Qt::Key_Minus)
        chart()->zoomOut();
    else
        QChartView::keyPressEvent(event);
}
