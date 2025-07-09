#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include "AudioModel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onOpenFile();
    void onMetadataReady(const AudioModel::Meta& m);
    void onError(const QString&);

private:
    AudioModel*    m_model;
    QPushButton*   m_btnOpen;
    QLabel*        m_lblDuration;
    QLabel*        m_lblSampleRate;
    QLabel*        m_lblBitRate;
    QLabel*        m_lblChannels;
};

#endif // MAINWINDOW_H
