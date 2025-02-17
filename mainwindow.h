#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QProgressBar>
#include <QPushButton>
#include <opencv2/opencv.hpp>
#include <SDL2/SDL.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void changeResolution(int index);

    void selectAudioDevice();

private:
    Ui::MainWindow *ui;
    cv::VideoCapture cap;
    cv::Mat frame;
    QTimer *videoTimer, *audioTimer;

    // SDL Audio
    SDL_AudioDeviceID audioDevice;
    std::vector<std::string> availableDevices;
    QPushButton *deviceSelectButton;

    // VU Meter
    QProgressBar *vuMeter;

    struct Resolution {
        int width;
        int height;
    };
    std::vector<Resolution> resolutions;

    static void audioCallback(void* userdata, Uint8* stream, int len);
};

#endif // MAINWINDOW_H
