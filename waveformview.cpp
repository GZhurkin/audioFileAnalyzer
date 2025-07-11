#include "waveformview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

WaveformView::WaveformView(QWidget* parent)
    : QWidget(parent),
    m_hScroll(new QScrollBar(Qt::Horizontal, this))
{
    connect(m_hScroll, &QScrollBar::valueChanged, this, [this]() {
        updateCachedPath();
        update();
    });
}

void WaveformView::setSamples(const QVector<double>& samples, quint32 sampleRate)
{
    m_samples = samples;
    m_sampleRate = sampleRate;
    m_markerSec = 0.0;
    m_zoom = 1.0;
    m_hScroll->setValue(0);

    updateScroll();
    updateCachedPath();
    update();
}

void WaveformView::setMarkerPosition(double seconds)
{
    double duration = m_sampleRate ? double(m_samples.size()) / m_sampleRate : 0.0;
    double newMarker = qBound(0.0, seconds, duration);

    if (!qFuzzyCompare(newMarker, m_markerSec)) {
        m_markerSec = newMarker;
        update();
    }
}

void WaveformView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (m_samples.isEmpty() || m_sampleRate == 0) {
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, "No audio loaded");
        return;
    }

    const int w = width();
    const int h = height() - m_hScroll->height();
    const int offset = m_hScroll->value();

    if (offset != m_cachedOffset || QSize(w, h) != m_cachedSize) {
        updateCachedPath();
        m_cachedOffset = offset;
        m_cachedSize = QSize(w, h);
    }

    p.setPen(QPen(Qt::green, 1));
    p.setBrush(QColor(0, 255, 0, 100));
    p.drawPath(m_cachedPath);

    double spp = double(m_samples.size()) / (m_zoom * w);
    double markerPx = (m_markerSec * m_sampleRate) / spp - offset;
    int mx = int(markerPx);

    if (mx >= 0 && mx <= w) {
        p.setPen(QPen(Qt::red, 2));
        p.drawLine(mx, 0, mx, h);
        p.setPen(Qt::white);
        p.drawText(mx + 4, h - 4, QString::number(m_markerSec, 'f', 2) + " s");
    }
}

void WaveformView::resizeEvent(QResizeEvent*)
{
    m_hScroll->setGeometry(0, height() - m_hScroll->height(), width(), m_hScroll->height());
    updateScroll();
    updateCachedPath();
}

void WaveformView::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        m_draggingMarker = true;
        updateMarkerFromPos(int(ev->position().x()));
    }
}

void WaveformView::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_draggingMarker)
        updateMarkerFromPos(int(ev->position().x()));
}

void WaveformView::mouseReleaseEvent(QMouseEvent*)
{
    m_draggingMarker = false;
}

void WaveformView::wheelEvent(QWheelEvent* ev)
{
    if (ev->modifiers() & Qt::ControlModifier) {
        double cursorX = ev->position().x();
        int w = width();
        if (w <= 0 || m_samples.isEmpty()) {
            ev->ignore();
            return;
        }

        double sampleCount = double(m_samples.size());
        double sppOld = sampleCount / (m_zoom * w);
        int oldOffset = m_hScroll->value();
        double sampleIndex = (oldOffset + cursorX) * sppOld;

        double delta = ev->angleDelta().y() > 0 ? 1.25 : 0.8;
        m_zoom = qBound(m_minZoom, m_zoom * delta, m_maxZoom);

        double sppNew = sampleCount / (m_zoom * w);
        double newOffset = sampleIndex / sppNew - cursorX;

        updateScroll();

        int clampedOffset = qBound(m_hScroll->minimum(), int(newOffset), m_hScroll->maximum());
        m_hScroll->setValue(clampedOffset);

        updateCachedPath();
        update();
        ev->accept();
    } else {
        QWidget::wheelEvent(ev);
    }
}

void WaveformView::updateScroll()
{
    if (m_samples.isEmpty() || m_sampleRate == 0) {
        m_hScroll->setRange(0, 0);
        return;
    }

    int w = width();
    if (w <= 0) return;

    double samplesPerPixel = double(m_samples.size()) / (m_zoom * w);
    int totalVisiblePx = int(double(m_samples.size()) / samplesPerPixel);

    int maxOffset = qMax(0, totalVisiblePx - w);

    int value = m_hScroll->value();
    value = qBound(0, value, maxOffset);

    m_hScroll->blockSignals(true);
    m_hScroll->setRange(0, maxOffset);
    m_hScroll->setPageStep(w);
    m_hScroll->setValue(value);
    m_hScroll->blockSignals(false);
}

void WaveformView::updateMarkerFromPos(int x)
{
    int w = width();
    if (w <= 0 || m_sampleRate == 0)
        return;

    int offset = m_hScroll->value();
    double spp = double(m_samples.size()) / (m_zoom * w);
    double posSec = ((offset + x) * spp) / m_sampleRate;

    setMarkerPosition(posSec);
    emit markerPositionChanged(m_markerSec);
}

void WaveformView::updateCachedPath()
{
    m_cachedPath = QPainterPath();

    const int viewWidth = width();
    const int h = height() - m_hScroll->height();
    const int offset = m_hScroll->value();

    if (viewWidth <= 0 || h <= 0 || m_samples.isEmpty())
        return;

    double spp = double(m_samples.size()) / (m_zoom * viewWidth);
    int totalPx = int(double(m_samples.size()) / spp);

    int endX = qMin(viewWidth, totalPx - offset);
    if (endX <= 0) return;

    QVector<double> maxVals(endX, -1.0);
    QVector<double> minVals(endX, 1.0);

    for (int x = 0; x < endX; ++x) {
        int startIdx = int((x + offset) * spp);
        int endIdx = int((x + offset + 1) * spp);
        startIdx = qBound(0, startIdx, m_samples.size() - 1);
        endIdx = qBound(startIdx + 1, endIdx, m_samples.size());

        for (int i = startIdx; i < endIdx; ++i) {
            double v = m_samples[i];
            maxVals[x] = qMax(maxVals[x], v);
            minVals[x] = qMin(minVals[x], v);
        }
    }

    m_cachedPath.moveTo(0, h / 2.0 - maxVals[0] * (h / 2.0));
    for (int x = 1; x < endX; ++x)
        m_cachedPath.lineTo(x, h / 2.0 - maxVals[x] * (h / 2.0));

    for (int x = endX - 1; x >= 0; --x)
        m_cachedPath.lineTo(x, h / 2.0 - minVals[x] * (h / 2.0));

    m_cachedPath.closeSubpath();
}
