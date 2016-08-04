#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <vnmap.h>
#include <Config.h>
#include <c_arpa_data.h>
#include <C_radar_data.h>
#include <QTimer>
#include <QtNetwork>
#include <QUdpSocket>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CConfig         config;
private:
    Ui::MainWindow *ui;
    void DrawMap();
protected:
    void paintEvent(QPaintEvent *event);
    void DrawViewFrame(QPainter* p);
    void DrawSignal(QPainter *p);
    void resetScale();
    void setDisplayHeading(float heading_deg);
    void drawTarget(QPainter *p);
    void ShowPos();

    void mouseMoveEvent(QMouseEvent *);
private slots:
    void blinking();
    void playbackRadarData();
    void newConnection();
    void processFrame();


    void on_toolButton_zoomIn_clicked();

    void on_toolButton_zoomOut_clicked();

    void on_toolButton_zoomOut_2_toggled(bool checked);

    void on_toolButton_zoomIn_2_toggled(bool checked);

    void on_toolButton_standby_clicked();

    void on_toolButton_tx_clicked();

    void on_toolButton_zoomIn_4_clicked();

    void on_toolButton_zoomIn_5_clicked();

    void on_toolButton_zoomIn_3_clicked();

    void on_toolButton_zoomIn_3_toggled(bool checked);

    void on_toolButton_zoomIn_4_toggled(bool checked);

private:
    QTcpServer *controlServer;
};

#endif // MAINWINDOW_H
