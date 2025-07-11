#include "waveformview.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

// Конструктор. Создаём горизонтальный скроллбар и подключаем обновление отображения
WaveformView::WaveformView(QWidget* parent)
    : QWidget(parent),
    m_hScroll(new QScrollBar(Qt::Horizontal, this))
{
    connect(m_hScroll, &QScrollBar::valueChanged, this, [this]() {
        updateCachedPath(); // Перерисовываем waveform при изменении скролла
        update();           // Триггерим paintEvent
    });
}

// Установка новых сэмплов и частоты дискретизации
void WaveformView::setSamples(const QVector<double>& samples, quint32 sampleRate)
{
    m_samples = samples;
    m_sampleRate = sampleRate;
    m_markerSec = 0.0;
    m_zoom = 1.0;
    m_hScroll->setValue(0);

    updateScroll();       // Пересчитываем границы скролла
    updateCachedPath();   // Кэшируем новую форму сигнала
    update();             // Перерисовываем виджет
}

// Устанавливаем маркер в заданное время (в секундах)
void WaveformView::setMarkerPosition(double seconds)
{
    double duration = m_sampleRate ? double(m_samples.size()) / m_sampleRate : 0.0;
    double newMarker = qBound(0.0, seconds, duration);

    // Обновляем только если позиция действительно изменилась
    if (!qFuzzyCompare(newMarker, m_markerSec)) {
        m_markerSec = newMarker;
        update(); // Перерисовываем маркер
    }
}

// Основная отрисовка виджета
void WaveformView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black); // Фон

    if (m_samples.isEmpty() || m_sampleRate == 0) {
        // Если данные не загружены — пишем сообщение
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, "No audio loaded");
        return;
    }

    const int w = width();
    const int h = height() - m_hScroll->height(); // Учитываем высоту скроллбара
    const int offset = m_hScroll->value();

    // Обновляем кэшированную форму сигнала, если размер или скролл изменился
    if (offset != m_cachedOffset || QSize(w, h) != m_cachedSize) {
        updateCachedPath();
        m_cachedOffset = offset;
        m_cachedSize = QSize(w, h);
    }

    // Отрисовка осциллограммы
    p.setPen(QPen(Qt::green, 1));
    p.setBrush(QColor(0, 255, 0, 100));
    p.drawPath(m_cachedPath);

    // Вычисляем и отрисовываем маркер текущей позиции
    double spp = (double(m_samples.size()) / m_zoom) / w; // сэмплов на пиксель
    double markerPx = (m_markerSec * m_sampleRate) / spp - offset;
    int mx = int(markerPx);

    // Рисуем маркер только если он попадает в область видимости
    if (mx >= 0 && mx <= w) {
        p.setPen(QPen(Qt::red, 2));
        p.drawLine(mx, 0, mx, h); // Вертикальная линия
        p.setPen(Qt::white);
        p.drawText(mx + 4, h - 4, QString::number(m_markerSec, 'f', 2) + " s");
    }
}

// Обработка изменения размера окна
void WaveformView::resizeEvent(QResizeEvent*)
{
    // Перемещаем скроллбар вниз
    m_hScroll->setGeometry(0, height() - m_hScroll->height(), width(), m_hScroll->height());
    updateScroll();      // Пересчитываем границы скролла
    updateCachedPath();  // Перестраиваем форму сигнала
}

// Пользователь нажал мышкой — начинаем перетаскивание маркера
void WaveformView::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        m_draggingMarker = true;
        updateMarkerFromPos(int(ev->position().x()));
    }
}

// При перемещении мыши — обновляем позицию маркера
void WaveformView::mouseMoveEvent(QMouseEvent* ev)
{
    if (m_draggingMarker)
        updateMarkerFromPos(int(ev->position().x()));
}

// Завершение перетаскивания маркера
void WaveformView::mouseReleaseEvent(QMouseEvent*)
{
    m_draggingMarker = false;
}

// Обработка колесика мыши — приближение/отдаление с удержанием Ctrl
void WaveformView::wheelEvent(QWheelEvent* ev)
{
    if (ev->modifiers() & Qt::ControlModifier) {
        double cursorX = ev->position().x();
        int w = width();
        if (w <= 0 || m_samples.isEmpty())
            return;

        // Считаем индекс сэмпла под курсором (до зума)
        double sampleCount = double(m_samples.size());
        double sppOld = sampleCount / (m_zoom * w);
        int oldOffset = m_hScroll->value();
        double sampleIndex = (oldOffset + cursorX) * sppOld;

        // Изменяем зум (с запасом)
        double delta = ev->angleDelta().y() > 0 ? 1.25 : 0.8;
        m_zoom = qBound(m_minZoom, m_zoom * delta, m_maxZoom);

        // Новая позиция скролла — чтобы зум был вокруг курсора
        double sppNew = sampleCount / (m_zoom * w);
        double newOffset = sampleIndex / sppNew - cursorX;

        updateScroll(); // Обновляем скроллбар
        m_hScroll->setValue(int(newOffset)); // Применяем
        updateCachedPath();
        update();

        ev->accept();
    } else {
        QWidget::wheelEvent(ev); // обычная прокрутка
    }
}

// Перерасчёт границ скроллбара при изменении масштаба или размеров
void WaveformView::updateScroll()
{
    if (m_samples.isEmpty() || m_sampleRate == 0) {
        m_hScroll->setRange(0, 0);
        return;
    }

    int w = width();
    double totalPx = double(m_samples.size()) / m_zoom; // сколько пикселей занимает сигнал при текущем зуме
    int maxScroll = qMax(0, int(totalPx - w));          // ограничиваем по правой границе

    m_hScroll->setRange(0, maxScroll);
    m_hScroll->setPageStep(w);
    m_hScroll->setValue(qBound(0, m_hScroll->value(), maxScroll));
}

// Вычисляем позицию маркера в секундах по клику или перемещению мыши
void WaveformView::updateMarkerFromPos(int x)
{
    int w = width();
    if (w <= 0 || m_sampleRate == 0)
        return;

    int offset = m_hScroll->value();
    double spp = (double(m_samples.size()) / m_zoom) / w;
    double posSec = ((offset + x) * spp) / m_sampleRate;

    setMarkerPosition(posSec); // Применяем
    emit markerPositionChanged(m_markerSec); // Сообщаем внешнему миру
}

// Генерация кэшированного QPainterPath для ускоренной отрисовки формы сигнала
void WaveformView::updateCachedPath()
{
    m_cachedPath = QPainterPath();

    const int viewWidth = width();
    const int h = height() - m_hScroll->height();
    const int offset = m_hScroll->value();

    if (viewWidth <= 0 || h <= 0 || m_samples.isEmpty())
        return;

    double spp = (double(m_samples.size()) / m_zoom) / viewWidth;

    // Ограничиваем отрисовку по количеству реальных сэмплов (чтобы не было "хвоста")
    int totalPx = int(m_samples.size() / spp);
    int endX = qMin(viewWidth, totalPx - offset);

    QVector<double> maxVals(endX, -1.0);
    QVector<double> minVals(endX, 1.0);

    // Находим максимум и минимум по каждому x
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

    if (endX == 0) return;

    // Создаём контур осциллограммы от max к min
    m_cachedPath.moveTo(0, h / 2.0 - maxVals[0] * (h / 2.0));
    for (int x = 1; x < endX; ++x)
        m_cachedPath.lineTo(x, h / 2.0 - maxVals[x] * (h / 2.0));
    for (int x = endX - 1; x >= 0; --x)
        m_cachedPath.lineTo(x, h / 2.0 - minVals[x] * (h / 2.0));
    m_cachedPath.closeSubpath(); // замыкаем контур для заливки
}
