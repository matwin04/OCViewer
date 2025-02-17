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
    void selectInputDevice();
    void selectOutputDevice();

private:
    Ui::MainWindow *ui;
    cv::VideoCapture cap;
    cv::Mat frame;
    QTimer *videoTimer, *audioTimer;

    // SDL Audio Input & Output
    SDL_AudioDeviceID inputDevice;
    SDL_AudioDeviceID outputDevice;
    std::vector<std::string> availableInputDevices;
    std::vector<std::string> availableOutputDevices;

    QPushButton *inputDeviceButton;
    QPushButton *outputDeviceButton;

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
