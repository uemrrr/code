#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QPushButton>
#include<QWidget>
#include<QMouseEvent>
#include<QEvent>

#include"game.h"
#include"netgame.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Game *game; //游戏指针
    QTimer *time1;  //定时器
    QPushButton pushButton; //双人
    QPushButton pushButton2;  //人机
    QPushButton pushButton3;  //AIvsAI

    char game_type; // 游戏模式,p=双人对战，b=人机对战，z=机机对战
    int clickPosX, clickPosY; //鼠标点击的位置
    bool on_jinshou;
    bool on_rechess;

    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void init(char type);
    void isOver();  //判断游戏是否结束
    void messagebox(char type);
    void reset();//游戏结束重置数据

    QTimer* wtimer;
    QTimer* btimer;
    int whiteTimes;
    int blackTimes;

//槽函数
private slots:
    void AIfirst();
    void autochessAI();
    void chessAIvsAI();


//    void btimerUpdate();
//    void wtimerUpdate();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    netgame *netgame2;

};

#endif // MAINWINDOW_H








