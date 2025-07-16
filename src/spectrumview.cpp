#include "spectrumview.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QWheelEvent>
#include <cmath>

SpectrumView::SpectrumView(QWidget *parent)
    : QWidget(parent)
    , m_rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    // Инициализация градиента
    updateGradient();
}

void SpectrumView::updateGradient()
{
    m_spectrumGradient = QLinearGradient(0, 0, 0, height());
    m_spectrumGradient.setColorAt(0.0, QColor(75, 0, 130, 200));   // Индиго
    m_spectrumGradient.setColorAt(0.5, QColor(138, 43, 226, 150)); // Фиолетовый
    m_spectrumGradient.setColorAt(1.0, QColor(147, 112, 219, 50)); // Светло-фиолетовый
}

double SpectrumView::getVisibleMinFreq() const
{
    double visibleMinFreq = m_minFrequency + m_panOffset;
    double visibleMaxFreq = m_maxFrequency + m_panOffset;
    double freqRange = visibleMaxFreq - visibleMinFreq;
    return visibleMinFreq + (freqRange - freqRange / m_zoomFactor) / 2;
}

double SpectrumView::getVisibleMaxFreq() const
{
    double visibleMinFreq = m_minFrequency + m_panOffset;
    double visibleMaxFreq = m_maxFrequency + m_panOffset;
    double freqRange = visibleMaxFreq - visibleMinFreq;
    return visibleMaxFreq - (freqRange - freqRange / m_zoomFactor) / 2;
}

void SpectrumView::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFrequency = minFreq;
    m_maxFrequency = maxFreq;
    update();
}

void SpectrumView::setDecibelRange(double minDB, double maxDB)
{
    m_minDB = minDB;
    m_maxDB = maxDB;
    update();
}

void SpectrumView::setSpectrumData(const QVector<double> &frequencies,
                                   const QVector<double> &magnitudes)
{
    if (frequencies.size() != magnitudes.size())
        return;

    QMutexLocker locker(&m_mutex);
    m_spectrumData.clear();
    m_spectrumData.reserve(frequencies.size());

    for (int i = 0; i < frequencies.size(); ++i) {
        // Ограничиваем значения амплитуд
        double mag = qBound(m_minDB, magnitudes[i], m_maxDB);
        m_spectrumData.append({frequencies[i], mag});
    }

    update();
}

void SpectrumView::clear()
{
    QMutexLocker locker(&m_mutex);
    m_spectrumData.clear();
    update();
}

void SpectrumView::zoomReset()
{
    m_zoomFactor = 1.0;
    m_panOffset = 0.0;
    update();
}

void SpectrumView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter.fillRect(rect(), QColor(30, 30, 40));

    drawGrid(painter);

    drawSpectrum(painter);

    drawLabels(painter);
}

void SpectrumView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGradient(); // Обновляем градиент при изменении размера
    update();
}

void SpectrumView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_zoomStart = event->pos();
        m_rubberBand->setGeometry(QRect(m_zoomStart, QSize()));
        m_rubberBand->show();
    } else if (event->button() == Qt::RightButton) {
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void SpectrumView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_rubberBand->isVisible()) {
        m_zoomEnd = event->pos();
        m_rubberBand->setGeometry(QRect(m_zoomStart, m_zoomEnd).normalized());
    } else if (m_isPanning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();

        double freqRange = m_maxFrequency - m_minFrequency;
        double panPercent = static_cast<double>(delta.x()) / width();
        m_panOffset -= panPercent * freqRange / m_zoomFactor;

        // Улучшенное ограничение панорамирования
        double maxPan = freqRange * (m_zoomFactor - 1.0) / 2.0;
        m_panOffset = qBound(-maxPan, m_panOffset, maxPan);
        update();
    }
}

void SpectrumView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_rubberBand->isVisible()) {
        QRect zoomRect = m_rubberBand->geometry();
        m_rubberBand->hide();

        if (zoomRect.width() > 10 && zoomRect.height() > 10) {
            applyZoom(zoomRect);
        }
    } else if (event->button() == Qt::RightButton && m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void SpectrumView::wheelEvent(QWheelEvent *event)
{
    double zoomFactor = 1.0 + (event->angleDelta().y() > 0 ? 0.1 : -0.1); // Более плавное масштабирование
    m_zoomFactor = qBound(1.0, m_zoomFactor * zoomFactor, 100.0);

    // Автоматическая коррекция панорамирования при увеличении
    double maxPan = (m_maxFrequency - m_minFrequency) * (m_zoomFactor - 1.0) / 2.0;
    m_panOffset = qBound(-maxPan, m_panOffset, maxPan);

    update();
}

void SpectrumView::drawGrid(QPainter &painter)
{
    painter.save();

    QPen gridPen(QColor(60, 60, 70));
    gridPen.setWidth(1);
    painter.setPen(gridPen);

    // Получаем видимый диапазон частот
    double visibleMinFreq = getVisibleMinFreq();
    double visibleMaxFreq = getVisibleMaxFreq();

    for (int db = static_cast<int>(m_minDB); db <= static_cast<int>(m_maxDB); db += 20) {
        double normDB = (db - m_minDB) / (m_maxDB - m_minDB);
        int y = height() - static_cast<int>(normDB * height());
        painter.drawLine(0, y, width(), y);

        painter.setPen(Qt::white);
        painter.drawText(5, y - 5, QString::number(db) + " dB");
        painter.setPen(gridPen);
    }

    // Vertical grid lines (frequency) - динамический диапазон
    double logMin = log10(qMax(1.0, visibleMinFreq)); // Защита от log(0)
    double logMax = log10(visibleMaxFreq);
    double logRange = logMax - logMin;

    // Основные декады
    for (int decade = static_cast<int>(pow(10, floor(logMin)));
         decade <= static_cast<int>(pow(10, ceil(logMax)));
         decade *= 10) {
        for (int multiplier = 1; multiplier <= 10; multiplier++) {
            double freq = decade * multiplier;
            if (freq < visibleMinFreq || freq > visibleMaxFreq)
                continue;

            double logFreq = log10(freq);
            int x = static_cast<int>((logFreq - logMin) / logRange * width());

            if (multiplier == 1) {
                painter.setPen(QPen(QColor(80, 80, 90), 1));
            } else {
                painter.setPen(gridPen);
            }

            painter.drawLine(x, 0, x, height());

            if (multiplier == 1 || multiplier == 2 || multiplier == 5 || multiplier == 10) {
                painter.setPen(Qt::white);
                QString label;
                if (freq >= 1000) {
                    label = QString::number(freq / 1000, 'f', freq < 10000 ? 1 : 0) + "k";
                } else {
                    label = QString::number(freq, 'f', 0);
                }
                painter.drawText(x + 2, height() - 5, label);
                painter.setPen(gridPen);
            }
        }
    }

    painter.restore();
}

void SpectrumView::drawSpectrum(QPainter &painter)
{
    if (m_spectrumData.isEmpty())
        return;

    QMutexLocker locker(&m_mutex);

    // Получаем видимый диапазон частот
    double visibleMinFreq = getVisibleMinFreq();
    double visibleMaxFreq = getVisibleMaxFreq();

    // Логарифмические преобразования
    double logMin = log10(qMax(1.0, visibleMinFreq)); // Защита от log(0)
    double logMax = log10(visibleMaxFreq);
    double logRange = logMax - logMin;
    double dbRange = m_maxDB - m_minDB;

    QPainterPath path;
    bool firstPoint = true;

    // Находим минимальное и максимальное значение для нормализации
    double minMag = m_maxDB;
    double maxMag = m_minDB;
    const int count = m_spectrumData.size();
    for (int i = 0; i < count; ++i) {
        const SpectrumPoint& point = m_spectrumData[i];
        if (point.frequency >= visibleMinFreq && point.frequency <= visibleMaxFreq) {
            if (point.magnitude < minMag)
                minMag = point.magnitude;
            if (point.magnitude > maxMag)
                maxMag = point.magnitude;
        }
    }

    const int dataCount = m_spectrumData.size();
    for (int i = 0; i < dataCount; ++i) {
        const SpectrumPoint& point = m_spectrumData[i];
        double freq = point.frequency;
        double mag = point.magnitude;

        // Пропускаем точки вне видимого диапазона
        if (freq < visibleMinFreq || freq > visibleMaxFreq)
            continue;

        double normalizedFreq = (log10(freq) - logMin) / logRange;
        double normalizedMag = (mag - m_minDB) / dbRange;

        // Гарантируем, что значения в пределах [0, 1]
        normalizedFreq = qBound(0.0, normalizedFreq, 1.0);
        normalizedMag = qBound(0.0, normalizedMag, 1.0);

        int x = static_cast<int>(normalizedFreq * width());
        int y = height() - static_cast<int>(normalizedMag * height());

        if (firstPoint) {
            path.moveTo(x, y);
            firstPoint = false;
        } else {
            path.lineTo(x, y);
        }
    }

    QPainterPath filledPath = path;
    filledPath.lineTo(width(), height());
    filledPath.lineTo(0, height());
    filledPath.closeSubpath();

    painter.fillPath(filledPath, m_spectrumGradient);

    painter.setPen(QPen(m_lineColor, 2));
    painter.drawPath(path);
}

void SpectrumView::drawLabels(QPainter &painter)
{
    painter.save();
    painter.setPen(Qt::white);

    if (m_zoomFactor > 1.01) {
        painter.drawText(10, 20, QString("Zoom: x%1").arg(m_zoomFactor, 0, 'f', 1));
    }

    double visibleMinFreq = getVisibleMinFreq();
    double visibleMaxFreq = getVisibleMaxFreq();

    QString freqRangeStr
        = QString("%1 Hz - %2 Hz").arg(visibleMinFreq, 0, 'f', 0).arg(visibleMaxFreq, 0, 'f', 0);

    painter.drawText(width() - 200, 20, freqRangeStr);

    if (underMouse()) {
        QPoint pos = mapFromGlobal(QCursor::pos());
        QPointF dataPoint = pointToData(pos);

        QString info
            = QString("%1 Hz, %2 dB").arg(dataPoint.x(), 0, 'f', 1).arg(dataPoint.y(), 0, 'f', 1);

        painter.drawText(pos + QPoint(15, -10), info);
        painter.drawEllipse(pos, 3, 3);
    }

    painter.restore();
}

void SpectrumView::applyZoom(const QRect &zoomRect)
{
    // Получаем текущий видимый диапазон
    double visibleMinFreq = getVisibleMinFreq();
    double visibleMaxFreq = getVisibleMaxFreq();

    // Логарифмические преобразования
    double logMin = log10(qMax(1.0, visibleMinFreq)); // Защита от log(0)
    double logMax = log10(visibleMaxFreq);
    double logRange = logMax - logMin;

    double startFreq = pow(10, logMin + (zoomRect.left() * logRange) / width());
    double endFreq = pow(10, logMin + (zoomRect.right() * logRange) / width());

    double newCenter = (startFreq + endFreq) / 2;
    double newWidth = endFreq - startFreq;

    m_zoomFactor = (m_maxFrequency - m_minFrequency) / newWidth;
    m_panOffset = newCenter - (m_minFrequency + m_maxFrequency) / 2;

    update();
}

QPointF SpectrumView::dataToPoint(double freq, double mag) const
{
    // Получаем текущий видимый диапазон
    double visibleMinFreq = getVisibleMinFreq();
    double visibleMaxFreq = getVisibleMaxFreq();

    // Логарифмические преобразования
    double logMin = log10(qMax(1.0, visibleMinFreq)); // Защита от log(0)
    double logMax = log10(visibleMaxFreq);
    double logRange = logMax - logMin;

    double x = (log10(freq) - logMin) / logRange * width();
    double y = height() - ((mag - m_minDB) / (m_maxDB - m_minDB) * height());

    return QPointF(x, y);
}

QPointF SpectrumView::pointToData(const QPoint &point) const
{
    // Получаем текущий видимый диапазон
    double visibleMinFreq = getVisibleMinFreq();
    double visibleMaxFreq = getVisibleMaxFreq();

    // Логарифмические преобразования
    double logMin = log10(qMax(1.0, visibleMinFreq)); // Защита от log(0)
    double logMax = log10(visibleMaxFreq);
    double logRange = logMax - logMin;

    double freq = pow(10, logMin + (point.x() * logRange) / width());
    double mag = m_minDB + ((height() - point.y()) * (m_maxDB - m_minDB)) / height();

    return QPointF(freq, mag);
}
