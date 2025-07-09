// waveformview.cpp
#include "waveformview.h"

#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

WaveformView::WaveformView(QWidget* parent)
    : QWidget(parent)
    , m_hScroll(new QScrollBar(Qt::Horizontal, this))
{
    connect(m_hScroll, &QScrollBar::valueChanged, this, [this](int){ update(); });
}

void WaveformView::setSamples(const QVector<double>& samples, quint32 sampleRate)
{
    m_samples = samples;
    m_sampleRate = sampleRate;
    m_markerSec = 0.0;
    m_zoom = 1.0;
    m_hScroll->setValue(0);
    updateScroll();
    update();
}

void WaveformView::setMarkerPosition(double seconds)
{
    double duration = m_sampleRate ? double(m_samples.size()) / m_sampleRate : 0.0;
    m_markerSec = qBound(0.0, seconds, duration);
    updateScroll();
    update();
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

    int w = width();
    int h = height() - m_hScroll->height();
    int offset = m_hScroll->value();

    // samples per pixel
    double spp = (double(m_samples.size()) / m_zoom) / w;

    // waveform path
    QPainterPath path;
    path.moveTo(0, h / 2.0);
    for (int x = 0; x < w; ++x) {
        int idx = int((x + offset) * spp);
        idx = qBound(0, idx, m_samples.size() - 1);
        double v = m_samples[idx];
        double y = h / 2.0 - v * (h / 2.0);
        path.lineTo(x, y);
    }
    p.setPen(QPen(Qt::green, 1));
    p.drawPath(path);

    // vertical marker
    double markerPx = (m_markerSec * m_sampleRate) / spp - offset;
    int mx = int(markerPx);
    p.setPen(QPen(Qt::red, 2));
    p.drawLine(mx, 0, mx, h);
    p.setPen(Qt::white);
    p.drawText(mx + 4, h - 4, QString::number(m_markerSec, 'f', 2) + " s");
}

void WaveformView::resizeEvent(QResizeEvent*)
{
    m_hScroll->setGeometry(0, height() - m_hScroll->height(), width(), m_hScroll->height());
    updateScroll();
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
        // позиция курсора внутри виджета
        double cursorX = ev->position().x();
        int w = width();
        if (w <= 0 || m_samples.isEmpty())
            return;

        // старый samples-per-pixel
        double sampleCount = double(m_samples.size());
        double sppOld = sampleCount / (m_zoom * w);
        int oldOffset = m_hScroll->value();

        // вычисляем, какой sample попадёт под курсор
        double sampleIndex = (oldOffset + cursorX) * sppOld;

        // новый зум
        double delta = ev->angleDelta().y() > 0 ? 1.1 : 0.9;
        double newZoom = qBound(m_minZoom, m_zoom * delta, m_maxZoom);
        m_zoom = newZoom;

        // новый spp
        double sppNew = sampleCount / (m_zoom * w);

        // вычисляем новый оффсет, чтобы тот же sample остался под курсором
        double newOffset = sampleIndex / sppNew - cursorX;

        updateScroll();
        m_hScroll->setValue(int(newOffset));
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
    double totalPx = double(m_samples.size()) / m_zoom;
    int maxScroll = qMax(0, int(totalPx - w));
    m_hScroll->setRange(0, maxScroll);
    m_hScroll->setPageStep(w);
    m_hScroll->setValue(qBound(0, m_hScroll->value(), maxScroll));
}

void WaveformView::updateMarkerFromPos(int x)
{
    int w = width();
    if (w <= 0 || m_sampleRate == 0) return;

    int offset = m_hScroll->value();
    double spp = (double(m_samples.size()) / m_zoom) / w;
    double posSec = ((offset + x) * spp) / m_sampleRate;
    setMarkerPosition(posSec);
    emit markerPositionChanged(m_markerSec);
}
