#ifndef NETGAME_H
#define NETGAME_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

#include "game.h"

namespace Ui {
class netgame;
}

class netgame : public QMainWindow
{
    Q_OBJECT

public:
    explicit netgame(QWidget *parent = nullptr);
    ~netgame();
    Game *game;
    bool is_server;//server 是黑方
    int clickPosX, clickPosY; //鼠标点击的位置

    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void initGame();
    void isOver();  //判断游戏是否结束
    void messagebox(char type);//不同游戏结束情况弹出相应的消息
    void reset();//游戏结束重置数据

private slots:
    void setmessage(int x,int y);
    void slot_NewConnection();
    void slot_ReadyRead();
    void playerchange();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::netgame *ui;
    QTcpServer* server;
    QTcpSocket* socket;

};

#endif // NETGAME_H
