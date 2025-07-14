#include "mainwindow.h"
#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidgetAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_model(new AudioModel(this)) // Объект для работы с аудиофайлом
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_waveform(new WaveformView(this))       // Осциллограмма
    , m_spectrogram(new SpectrogramView(this)) // Спектрограмма
    , m_spectrum(new SpectrumView(this))       // ДОБАВИЛ
    , m_metadatalabel(new QLabel(this))        // Метаданные
{
    // Настройка главного окна
    this->setWindowTitle("Audio File Analyzer");
    this->setMinimumSize(1280, 720);
    this->setStyleSheet("QMainWindow {"
                        "   background-color: #808080;" // Основной фон
                        "   color: #FFFCF2;"            // Светлый текст
                        "}"
                        "QWidget#centralWidget {" // Центральный виджет
                        "   background-color: #999999;"
                        "}");

    m_player->setAudioOutput(m_audioOutput);

    // Инициализация панели инструментов
    auto *tb = addToolBar("Controls");
    QAction *openAct = tb->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon),
                                     "Open"); // Иконка папки для открытия файлов

    // Разделитель перед элементами громкости
    tb->addSeparator();

    // Регулятор громкости с выпадающим меню
    QToolButton *volumeButton = new QToolButton(this);
    volumeButton->setIcon(QIcon::fromTheme("audio-volume-high"));
    volumeButton->setPopupMode(QToolButton::InstantPopup);
    volumeButton->setToolTip("Volume");

    // Виджет для меню громкости
    QWidget *volumeMenuWidget = new QWidget(this);
    QHBoxLayout *volumeLayout = new QHBoxLayout(volumeMenuWidget);
    volumeLayout->setContentsMargins(5, 5, 5, 5);

    // Слайдер громкости
    QSlider *volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(static_cast<int>(m_audioOutput->volume() * 100));

    // Лейбл для отображения значения (0-100)
    QLabel *volumeValueLabel = new QLabel(QString::number(volumeSlider->value()), this);
    volumeValueLabel->setFixedWidth(30);
    volumeValueLabel->setAlignment(Qt::AlignCenter);

    volumeLayout->addWidget(volumeSlider);
    volumeLayout->addWidget(volumeValueLabel);

    QWidgetAction *volumeAction = new QWidgetAction(this);
    volumeAction->setDefaultWidget(volumeMenuWidget);

    QMenu *volumeMenu = new QMenu(this);
    volumeMenu->addAction(volumeAction);
    volumeButton->setMenu(volumeMenu);

    // Добавление кнопки в тулбар
    tb->addWidget(volumeButton);

    // Обработка изменения громкости
    connect(volumeSlider,
            &QSlider::valueChanged,
            this,
            [this, volumeButton, volumeValueLabel](int value) {
                // Установление громкости
                float volume = value / 100.0f;
                m_audioOutput->setVolume(volume);

                // Обновление текста
                volumeValueLabel->setText(QString::number(value));

                // Изменениe иконки
                if (value == 0) {
                    volumeButton->setIcon(QIcon::fromTheme("audio-volume-muted"));
                } else if (value < 33) {
                    volumeButton->setIcon(QIcon::fromTheme("audio-volume-low"));
                } else if (value < 66) {
                    volumeButton->setIcon(QIcon::fromTheme("audio-volume-medium"));
                } else {
                    volumeButton->setIcon(QIcon::fromTheme("audio-volume-high"));
                }
            });

    // Подключение к слотам для обработки нажатий на кнопки
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);

    // Инициализация ползунка
    m_progressSlider = new QSlider(Qt::Horizontal, this);

    // Инициализация таймера
    m_timeLabel = new QLabel("00:00 / 00:00", this);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setFixedHeight(20);
    m_timeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_timeLabel->setStyleSheet("color: #FFFCF2;"
                               "font-size: 10pt;"
                               "margin: 0px;"
                               "padding: 0px;");

    // Настройка метаданных
    m_metadatalabel->setAlignment(Qt::AlignLeft);
    m_metadatalabel->setFixedHeight(22);
    m_metadatalabel->setStyleSheet("background-color: #403D39;"
                                   "color: #FFFCF2;"
                                   "padding: 2px 4px;"
                                   "font-size: 9pt;");
    m_metadatalabel->setText("No data");

    // Кнопки управления
    QToolButton *playBtn = new QToolButton(this);
    playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playBtn->setIconSize(QSize(30, 30));

    QToolButton *pauseBtn = new QToolButton(this);
    pauseBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    pauseBtn->setIconSize(QSize(30, 30));

    QToolButton *stopBtn = new QToolButton(this);
    stopBtn->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopBtn->setIconSize(QSize(30, 30));

    // Вертикальный контейнер для нижней панели
    QWidget *bottomPanel = new QWidget(this);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomPanel);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);

    QWidget *sliderRow = new QWidget(this);
    QHBoxLayout *sliderLayout = new QHBoxLayout(sliderRow);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->setSpacing(4);

    m_progressSlider->setFixedHeight(20);
    sliderLayout->addWidget(m_progressSlider, 1);
    sliderLayout->addWidget(m_timeLabel);

    bottomLayout->addWidget(sliderRow);

    QWidget *infoRow = new QWidget(this);
    QGridLayout *infoLayout = new QGridLayout(infoRow);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(0);

    m_metadatalabel->setFixedHeight(20);
    infoLayout->addWidget(m_metadatalabel, 0, 0, Qt::AlignLeft);

    // Выравнивание внопок по центру
    QWidget *centerButtons = new QWidget(this);
    QHBoxLayout *centerLayout = new QHBoxLayout(centerButtons);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(4);                 // Устанавление промежутков между кнопками
    centerLayout->setAlignment(Qt::AlignCenter); // Центрируем блок кнопок

    centerLayout->addWidget(playBtn); // Добавление кнопок в горизонтальный блок
    centerLayout->addWidget(pauseBtn);
    centerLayout->addWidget(stopBtn);

    infoLayout->addWidget(centerButtons, 0, 1, Qt::AlignHCenter);

    QWidget *rightSpacer = new QWidget(
        this); //Заглушка справа, чтобы сохранить симметрию и растяжку виджетов
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    infoLayout->addWidget(rightSpacer, 0, 2);

    infoLayout->setColumnStretch(0, 1);
    infoLayout->setColumnStretch(1, 0);
    infoLayout->setColumnStretch(2, 1);

    // Добавляем строку в нижний контейнер
    bottomLayout->addWidget(infoRow);

    // Основная компоновка
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    layout->addWidget(m_spectrogram, 1);
    layout->addWidget(m_spectrum, 2);
    layout->addWidget(m_waveform, 1);
    layout->addWidget(bottomPanel, 0); // Добавление объединенной нижней панели

    m_spectrum->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_spectrum->setFrequencyRange(20, 20000);
    m_spectrum->setDecibelRange(-100, 100);
    setCentralWidget(centralWidget);

    // Стилизация кнопок
    QString btnStyle = "QToolButton {"
                       "   border: none;"
                       "   background: transparent;"
                       "   padding: 0px;"
                       "   margin: 0px;"
                       "}"
                       "QToolButton:hover {"
                       "   background: #504945;"
                       "   border-radius: 2px;"
                       "}";
    playBtn->setStyleSheet(btnStyle);
    pauseBtn->setStyleSheet(btnStyle);
    stopBtn->setStyleSheet(btnStyle);

    // Подключение сигналов
    connect(playBtn, &QToolButton::clicked, m_player, &QMediaPlayer::play);
    connect(pauseBtn, &QToolButton::clicked, m_player, &QMediaPlayer::pause);
    connect(stopBtn, &QToolButton::clicked, m_player, &QMediaPlayer::stop);

    // Подключение слотов
    connect(m_model,
            &AudioModel::metadataReady,
            this,
            &MainWindow::onMetadataReady); // Вывод метаданных
    connect(m_model,
            &AudioModel::waveformReady,
            this,
            &MainWindow::onWaveformReady); // Вывод осциллограммы
    connect(m_model,
            &AudioModel::spectrogramReady,
            this,
            &MainWindow::onSpectrogramReady);                                 // Вывод спектрограммы
    connect(m_model, &AudioModel::errorOccurred, this, &MainWindow::onError); // Сообщение об ошибке
    connect(m_player,
            &QMediaPlayer::positionChanged,
            this,
            &MainWindow::onPositionChanged); // Перемещение маркера при проигрывании аудиофайла
    connect(m_model, &AudioModel::spectrumReady, this, &MainWindow::onSpectrumReady);

    connect(m_waveform,
            &WaveformView::markerPositionChanged,
            this,
            [this](double seconds) { // Перемещение ползунка при перемещении маркера
                qint64 posMs = static_cast<qint64>(seconds * 1000);
                if (m_player->position() != posMs) {
                    m_player->setPosition(posMs);
                }
                if (m_player->duration() > 0) {
                    int sliderVal = static_cast<int>((posMs * 100) / m_player->duration());
                    if (m_progressSlider->value() != sliderVal) {
                        m_progressSlider->setValue(sliderVal);
                    }
                }
            });

    connect(m_progressSlider,
            &QSlider::sliderMoved,
            this,
            [this](int value) { // Перемещение маркера при перемещении ползунка
                if (m_player->duration() > 0) {
                    qint64 newPos = (value * m_player->duration()) / 100;
                    if (m_player->position() != newPos) {
                        m_player->setPosition(newPos);
                    }
                    double seconds = newPos / 1000.0;
                    m_waveform->setMarkerPosition(seconds);
                }
            });
}

MainWindow::~MainWindow() {}

void MainWindow::onOpenFile()
{
    const QString file = QFileDialog::getOpenFileName(this, "Select WAV", {}, "WAV Files (*.wav)");
    if (file.isEmpty())
        return;

    m_metadatalabel->setText("Loading: "
                             + QFileInfo(file).fileName()); // Обновление статуса метаданных
    m_waveform->setSamples({}, 0);
    m_spectrogram->setSpectrogramData({});
    m_spectrum->setSpectrumData({}, {});

    // Сбросить сохраненные сэмплы
    m_samples.clear();
    m_sampleRate = 0;

    AudioModel::Meta meta; // Создание структуры для хранения метаданных
    QString err;
    if (!m_model->loadWav(file, meta, err)) { // Загрузка данных из аудиофайла
        return;
    }

    m_progressSlider->setRange(0, 100);
    m_progressSlider->setValue(0);
    m_timeLabel->setText("00:00 / 00:00");
    m_player->setSource(QUrl::fromLocalFile(file)); // Установка медиа-источника для плеера
}

// Вывод метаданных
void MainWindow::onMetadataReady(const AudioModel::Meta &m)
{
    m_metadatalabel->setText(QString("%1 s | %2 Hz | %3 kbps | %4 ch | %5 bit")
                                 .arg(m.durationSeconds, 0, 'f', 1)
                                 .arg(m.sampleRate)
                                 .arg(m.bitRate / 1000)
                                 .arg(m.channels)
                                 .arg(m.bitsPerSample));
}

// Вывод осциллограммы
void MainWindow::onWaveformReady(const QVector<double> &samples, quint32 sampleRate)
{
    //НАЧАЛО ДОБАВЛЕННОГО КОДА
    m_samples = samples;       // Сохраняем сэмплы
    m_sampleRate = sampleRate; // Сохраняем частоту дискретизации
    //КОНЕЦ ДОБАВЛЕННОГО КОДА
    m_waveform->setSamples(samples, sampleRate);
}
// Вывод спектрограммы
void MainWindow::onSpectrogramReady(const QVector<QVector<double>> &frames)
{
    m_spectrogram->setSpectrogramData(frames);
}

//ДОБАВИЛ ФУНКЦИЮ
void MainWindow::onSpectrumReady(const QVector<double> &frequencies,
                                 const QVector<double> &magnitudes)
{
    m_spectrum->setFrequencyRange(20, 20000); // 20Hz - 20kHz
    m_spectrum->setDecibelRange(-100, 100);   // -1000dB to 0dB ДОБАВИЛ ПОСЛДЕДНИЙ РАЗ
    m_spectrum->setSpectrumData(frequencies, magnitudes);
}

// Вывод сообщения об ошибке
void MainWindow::onError(const QString &err)
{
    QMessageBox::critical(this, "Error", err);
}
// Перемещение ползунка при проигрывании аудиофайла
void MainWindow::onPositionChanged(qint64 pos)
{
    double seconds = pos / 1000.0;
    m_waveform->setMarkerPosition(seconds);

    if (m_player->duration() > 0) {
        int sliderVal = static_cast<int>((pos * 100) / m_player->duration());
        if (m_progressSlider->value() != sliderVal) {
            m_progressSlider->setValue(sliderVal);
        }

        QString currentTime1 = QString("%1:%2")
                                   .arg(pos / 60000, 2, 10, QLatin1Char('0'))
                                   .arg((pos % 60000) / 1000, 2, 10, QLatin1Char('0'));

        QString totalTime = QString("%1:%2")
                                .arg(m_player->duration() / 60000, 2, 10, QLatin1Char('0'))
                                .arg((m_player->duration() % 60000) / 1000, 2, 10, QLatin1Char('0'));

        m_timeLabel->setText(currentTime1 + " / " + totalTime);

        // Обновление спектра в реальном времени
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - m_lastSpectrumUpdate < SPECTRUM_UPDATE_INTERVAL_MS) {
            return;
        }
        m_lastSpectrumUpdate = currentTime;

        if (m_samples.isEmpty() || m_sampleRate == 0) {
            return;
        }

        const int fftSize = 2048;
        double posSeconds = pos / 1000.0;
        quint64 startSample = static_cast<quint64>(posSeconds * m_sampleRate);

        // Проверка выхода за границы
        if (startSample >= static_cast<quint64>(m_samples.size())) {
            return;
        }

        // Берем сэмплы для текущей позиции
        QVector<double> frame(fftSize, 0.0);
        int count = qMin(fftSize, m_samples.size() - static_cast<int>(startSample));
        for (int i = 0; i < count; ++i) {
            frame[i] = m_samples[static_cast<int>(startSample) + i];
        }

        // Рассчитываем спектр для текущего фрагмента
        m_model->calculateSpectrum(frame, m_sampleRate);
    }
}
