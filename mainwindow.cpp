#include "mainwindow.h"
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_model(new AudioModel(this))             // Объект для работы с аудиофайлом
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_waveform(new WaveformView(this))        // Осциллограмма
    , m_spectrogram(new SpectrogramView(this))  // Спектрограмма
{
    this->setWindowTitle("Audio File Analyzer");
    this->setMinimumSize(640, 480);

    m_player->setAudioOutput(m_audioOutput);

    // Инициализация панели инструментов
    //-----------------------------------------
    auto *tb = addToolBar("Controls");
    QAction *openAct = tb->addAction("Open");
    QAction *playAct = tb->addAction("Play");
    QAction *pauseAct = tb->addAction("Pause");
    QAction *stopAct = tb->addAction("Stop");
    //-----------------------------------------

    // Подключение к слотам для обработки нажатий на кнопки
    //---------------------------------------------------------------------
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(playAct, &QAction::triggered, m_player, &QMediaPlayer::play);
    connect(pauseAct, &QAction::triggered, m_player, &QMediaPlayer::pause);
    connect(stopAct, &QAction::triggered, m_player, &QMediaPlayer::stop);
    //---------------------------------------------------------------------

    m_progressSlider = new QSlider(Qt::Horizontal, this); // Инициализация ползунка

    // Виджеты для отображения осциллограммы, (спектограммы), спектра и ползунка
    //--------------------------------------
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    layout->addWidget(m_spectrogram, 0);
    layout->addWidget(m_waveform, 0);
    layout->addWidget(m_progressSlider, 0);

    setCentralWidget(central);
    //--------------------------------------

    // Подключение слотов
    //------------------------------------------------------------------------------------------
    connect(m_model, &AudioModel::metadataReady, this, &MainWindow::onMetadataReady);         // Вывод метаданных
    connect(m_model, &AudioModel::waveformReady, this, &MainWindow::onWaveformReady);         // Вывод осциллограммы
    connect(m_model, &AudioModel::spectrogramReady, this, &MainWindow::onSpectrogramReady);   // Вывод спектрограммы
    connect(m_model, &AudioModel::errorOccurred, this, &MainWindow::onError);                 // Сообщение об ошибке
    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);  // Перемещение маркера при проигрывании аудиофайла

    connect(m_waveform, &WaveformView::markerPositionChanged, this, [this](double seconds) {  // Перемещение ползунка при перемещении маркера
        qint64 posMs = static_cast<qint64>(seconds * 1000);
        if (m_player->position() != posMs) {
            m_player->setPosition(posMs);
        }
        if (m_player->duration() > 0) {
            int sliderVal = static_cast<int>((posMs * 100) / m_player->duration());
            if (m_progressSlider->value() != sliderVal)
                m_progressSlider->setValue(sliderVal);
        }
    });

    connect(m_progressSlider, &QSlider::sliderMoved, this, [this](int value) {                // Перемещение маркера при перемещении ползунка
        if (m_player->duration() > 0) {
            qint64 newPos = (value * m_player->duration()) / 100;
            if (m_player->position() != newPos) {
                m_player->setPosition(newPos);
            }
            double seconds = newPos / 1000.0;
            m_waveform->setMarkerPosition(seconds);
        }
    });
    //------------------------------------------------------------------------------------------
}

MainWindow::~MainWindow(){}

void MainWindow::onOpenFile()
{
    const QString file = QFileDialog::getOpenFileName(this, "Select WAV", {}, "WAV Files (*.wav)");
    if (file.isEmpty())
        return;

    m_waveform->setSamples({}, 0);
    m_spectrogram->setSpectrogramData({});

    AudioModel::Meta meta; // Создание структуры для хранения метаданных
    QString err;
    if (!m_model->loadWav(file, meta, err)) { // Загрузка данных из аудиофайла
        return;
    }

    m_progressSlider->setRange(0, meta.durationSeconds);
    m_progressSlider->setValue(0);

    m_player->setSource(QUrl::fromLocalFile(file)); // Установка медиа-источника для плеера
}

void MainWindow::onMetadataReady(const AudioModel::Meta &m) // Вывод метаданных
{
    setWindowTitle(QString("Dur: %1 s  SR: %2 Hz  BR: %3 bps  Ch: %4  Bits: %5")
                       .arg(m.durationSeconds, 0, 'f', 2)
                       .arg(m.sampleRate)
                       .arg(m.bitRate)
                       .arg(m.channels)
                       .arg(m.bitsPerSample));
}

void MainWindow::onWaveformReady(const QVector<double> &samples, quint32 sampleRate) // Вывод осциллограммы
{
    m_waveform->setSamples(samples, sampleRate);
}

void MainWindow::onSpectrogramReady(const QVector<QVector<double>> &frames) // Вывод спектрограммы
{
    m_spectrogram->setSpectrogramData(frames);
}

void MainWindow::onError(const QString &err) // Вывод сообщения об ошибке
{
    QMessageBox::critical(this, "Error", err);
}

void MainWindow::onPositionChanged(qint64 pos) // Перемещение ползунка при проигрывании аудиофайла
{
    double seconds = pos / 1000.0;

    m_waveform->setMarkerPosition(seconds);

    if (m_player->duration() > 0) {
        int sliderVal = static_cast<int>((pos * 100) / m_player->duration());
        if (m_progressSlider->value() != sliderVal)
            m_progressSlider->setValue(sliderVal);
    }
}
