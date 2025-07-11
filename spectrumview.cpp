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

    m_axisX->setTitleText("Frequency (Hz)");
    chartObj->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY->setTitleText("Amplitude");
    chartObj->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    setRenderHint(QPainter::Antialiasing);
    setRubberBand(QChartView::RectangleRubberBand);
}

void SpectrumView::ensureNonZeroRange(double& min, double& max, double padding) {

    if (min == max) {
        min -= padding;
        max += padding;
    }
}

void SpectrumView::setSpectrum(const QVector<double>& freq, const QVector<double>& amp) {
    m_series->clear();
    int n = qMin(freq.size(), amp.size());

    if (n <= 1) {
        m_axisX->setRange(0, 1);
        m_axisY->setRange(0, 1);
        return;
    }

    for (int i = 0; i < n; ++i)
        m_series->append(freq[i], amp[i]);

    double xmin = freq.first();
    double xmax = freq.last();
    ensureNonZeroRange(xmin, xmax, (xmax - xmin) * 0.05);
    m_axisX->setRange(xmin, xmax);

    auto [minIt, maxIt] = std::minmax_element(amp.constBegin(), amp.constEnd());
    double ymin = std::max(0.0, *minIt);
    double ymax = *maxIt;
    ensureNonZeroRange(ymin, ymax, (ymax - ymin) * 0.05);
    m_axisY->setRange(ymin, ymax);
}

void SpectrumView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton)
        chart()->zoomReset();
    QChartView::mouseReleaseEvent(event);
}

void SpectrumView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Plus) {
        chart()->zoomIn();
    } else if (event->key() == Qt::Key_Minus) {
        chart()->zoomOut();
    } else {
        QChartView::keyPressEvent(event);
    }
}
