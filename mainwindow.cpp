#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QPainter>
#include<QPaintEvent>
#include<QPen>
#include<QBrush>
#include<QBitmap>
#include<QPixmap>
#include<QLabel>
#include<qtimer.h>
#include<QPushButton>
#include<qmath.h>
#include<QDebug>
#include<QMouseEvent>
#include<QMessageBox>
#include <QSound>
#include<QMediaPlayer>
#include<QMenuBar>
#include<QMenu>
#include<QIcon>


#define CHESS_PLAY  "://sound/chessone.wav"  //落子音的路径
#define WIN_SOUND  "://sound/win.wav"   //获胜提示音路径
#define LOSE_SOUND "://sound/lose.wav"   //失败提示音路径

const int boardMargin = 35; // 棋盘外边距属性
const int r = 15; // 棋子半径
const int markSize = 6; // 落子标记边长
const int blockSize = 40; // 格子的大小
const int mousearea= 20; // 鼠标点击的模糊距离上限
const int AIthinktime = 600; // AI下棋的思考时间

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置棋盘大小
    setFixedSize(boardMargin * 2 + blockSize * boardsize+300, boardMargin * 2 + blockSize * boardsize-50);
    //设置窗口题目
    setWindowTitle("六子棋");

    //设置菜单栏
    QMenuBar *menubar=menuBar();
    //菜单中的游戏设置
    QMenu *menujs=menubar->addMenu("设置");
    QAction *actionon=menujs->addAction("开启禁手");
    QAction *actionoff=menujs->addAction("关闭禁手");
    connect(actionon,&QAction::triggered,[=](){
               on_jinshou=true;
               actionon->setEnabled(false);
               actionoff->setEnabled(true);
    });
    connect(actionoff,&QAction::triggered,[=](){
               on_jinshou=false;
               actionoff->setEnabled(false);
               actionon->setEnabled(true);
            });

    QAction *actiony=menujs->addAction("开启悔棋");
    QAction *actionn=menujs->addAction("关闭悔棋");
    connect(actiony,&QAction::triggered,[=](){
               on_rechess=true;
               actiony->setEnabled(false);
               actionn->setEnabled(true);
    });
    connect(actionn,&QAction::triggered,[=](){
               on_rechess=false;
               actionn->setEnabled(false);
               actiony->setEnabled(true);
               ui->pushButton_5->setEnabled(false);
            });


    //退出游戏
    QAction *actioncl=menubar->addAction("退出游戏");
    connect(actioncl,&QAction::triggered,this,&QMainWindow::close);


    //设置“帮助”菜单
    QMenu *menuhe=menubar->addMenu("帮助");
    //菜单中的关于游戏
    QAction *actionin=menuhe->addAction("关于游戏");
    QString strin=  "游戏简介：\n"
                    "    六子棋是一种双人对弈搏杀的一种游戏，规则与五子棋类似。"
                    "本游戏支持双人对战、人机对战、机机对战三种模式。";
    connect(actionin,&QAction::triggered,[=](){
                QMessageBox::about(this,"游戏简介",strin);
            });
    //规则说明
    QAction *actionru=menuhe->addAction("规则说明");
    QString strru=         "规则说明:\n"
                           "先有六个棋子连成一条线的一方为胜者。\n"
                           "游戏默认允许悔棋，采用禁手模式，包括四四禁手、五五禁手等情况，先手出现禁手判后手获胜，可以在设置中关闭悔棋和禁手。\n"
                           "如果处于联机对决，但在联机中不可对此做任何改动\n"
                           "tips:禁手：黑棋一子落下同时形成下双活四、双五（冲五或活五）或长连（有6个以上的棋子连成了一条线）\n";
   //弹出游戏规则内容
    connect(actionru,&QAction::triggered,[=](){
                QMessageBox::about(this,"规则说明",strru);
            });

    //默认开启禁手,菜单中开启禁手不能用
    on_jinshou=true;
    actionon->setEnabled(false);

    //默认开启悔棋,菜单中开启悔棋不能用
    on_rechess=true;
    actiony->setEnabled(false);
    ui->pushButton_5->setEnabled(false);//仅双人模式可用
    // 开始游戏
     game=new Game;
     time1=nullptr;

}

MainWindow::~MainWindow()
{
    delete ui;
    if(game)   //撤销游戏指针
    {
        delete game;
        game = nullptr;
    }
    if(time1)
    {
        delete time1;
        time1=nullptr;
    }
}

void MainWindow::init(char type)
{
    if(game)
    {
        delete game;
    }
    game=new Game;

    game->gameType=type;
    game->gameStatus=1;//游戏进行中

    if(time1)
    {
        delete time1;
        time1=nullptr;
    }

    clickPosX=-1;
    clickPosY=-1;//初始化

   game->newgame(type);

    update();
}

void MainWindow::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    painter.drawPixmap(0,0,this->width(),this->height(),QPixmap("://pic/01.jpg"));
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    //画棋盘,x向右递增，y向下递增
    for(int i=0;i<boardsize;i++)
    {
        painter.drawLine(boardMargin,boardMargin+i*blockSize,boardMargin+(boardsize-1)*blockSize,boardMargin+i*blockSize);//横线
        painter.drawLine(boardMargin+i*blockSize,boardMargin,boardMargin+i*blockSize,boardMargin+(boardsize-1)*blockSize);//竖线
    }
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);//笔刷实现实线

    // 绘制落子标记(防止鼠标出框越界)
    if(clickPosX>=0&&clickPosX<boardsize&&
       clickPosY>=0&&clickPosY<boardsize&&
       game->gameStatus==1&&game->board[clickPosX][clickPosY]==0)
    {
        if(game->playerFlag&&!(game->playerFirst==false&&game_type=='r'))//先手为ture
        {
            //黑棋落子标记
            brush.setColor(Qt::black);
            painter.setBrush(brush);
            painter.drawRect(boardMargin+blockSize*clickPosX-markSize/2,boardMargin+blockSize*clickPosY-markSize/2,markSize,markSize);
        }
        else if((game->playerFlag==false&&game_type!='r'&&game->playerFirst)||
                (game_type=='r'&&game->playerFirst==false))
        {
            //白棋落子标记
            brush.setColor(Qt::white);
            painter.setBrush(brush);
            painter.drawRect(boardMargin+blockSize*clickPosX-markSize/2,boardMargin+blockSize*clickPosY-markSize/2,markSize,markSize);
        }
    }
    //绘制棋子
    for(int i=0;i<boardsize;i++)
    {
        for(int j=0;j<boardsize;j++)
        {

            if(game->board[i][j]==1&&game->gameStatus==1) //游戏进行中才允许绘制棋子
           {
              brush.setColor(Qt::black);
              painter.setBrush(brush);
              painter.drawEllipse(QPoint(boardMargin+ blockSize * i ,boardMargin+ blockSize * j),r,r);
           }
            else if(game->board[i][j]==-1&&game->gameStatus==1)
            {
                brush.setColor(Qt::white);
                painter.setBrush(brush);
                painter.drawEllipse(QPoint(boardMargin + blockSize * i ,boardMargin+ blockSize * j),r,r);
            }
        }
    }
    update();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //左键按住拖动，只有拖动页面条时才可以拖动,不写也行
    if(event->buttons()&Qt::LeftButton){
    move(this->frameGeometry().topLeft());
    }

    // 通过鼠标的hover确定鼠标的位置即落子标记的位置
    int x = event->x();
    int y = event->y();

    // 鼠标要在棋盘内
    if (x >= boardMargin + blockSize/ 2 &&
            x <boardMargin+(boardsize-1)*blockSize&&
            y >= boardMargin+ blockSize / 2 &&
            y <boardMargin+(boardsize-1)*blockSize&&
            game->gameStatus==1)
    {
        // 获取最近的左上角的点
        int row = x / blockSize;
        int col = y / blockSize;
        //获取最近的点的坐标
        int leftTopPosX = boardMargin+ blockSize * row;
        int leftTopPosY = boardMargin+blockSize * col;

        clickPosX = -1; // 初始化最终的值
        clickPosY = -1;
        int len = 0; // 下的地方为整数,用int直接取整

        // 算离附近格点的范围，选在允许的鼠标模糊范围之内
        //离左上格点距离
        len = sqrt((x - leftTopPosX) * (x - leftTopPosX) + (y - leftTopPosY) * (y - leftTopPosY));
        if (len < mousearea)
        {
            clickPosX = row;
            clickPosY = col;
        }
        //离右上格点距离
        len = sqrt((x - leftTopPosX - blockSize) * (x - leftTopPosX - blockSize) + (y - leftTopPosY) * (y - leftTopPosY));
        if (len < mousearea)
        {
            clickPosX = row;
            clickPosY = col + 1;
        }
        //离左下格点距离
        len = sqrt((x - leftTopPosX) * (x - leftTopPosX) + (y - leftTopPosY - blockSize) * (y - leftTopPosY - blockSize));
        if (len < mousearea)
        {
            clickPosX = row + 1;
            clickPosY = col;
        }
        //离右下格点距离
        len = sqrt((x - leftTopPosX - blockSize) * (x - leftTopPosX - blockSize) + (y - leftTopPosY - blockSize) * (y - leftTopPosY - blockSize));
        if (len < mousearea)
        {
            clickPosX = row + 1;
            clickPosY = col + 1;
        }
    }
    update();
//    qDebug()<<"move";
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
      if (clickPosX != -1 && clickPosY != -1&&game->gameStatus==1)//要有效点击且游戏进行中
      {
          //鼠标点击释放，如果人下棋就染色下棋。不是就转AI
          if((game_type=='p'&&game->board[clickPosX][clickPosY] == 0)||game->playerFlag==true)
          {
              game->chess.push_back(QPoint(clickPosX,clickPosY));//存入已下的点
              game->updateMap(clickPosX,clickPosY); //重绘
              QSound::play(CHESS_PLAY);
              isOver();//每下一子需要判断是否游戏结束
              update();
          }
           if(game_type=='r'&&!game->playerFlag)  //ai操作
              QTimer::singleShot(AIthinktime, this, SLOT(autochessAI()));
      }

      update();
//      qDebug()<<"release";
}

void MainWindow::AIfirst(){

    //当AI先走时需要先随机开一棋，调整随机数，使尽量落在棋盘中间区域
   srand((unsigned)time(nullptr));
   clickPosX=boardsize/2+rand()%4;
   clickPosY=boardsize/2+rand()%4;
   game->board[clickPosX][clickPosY]=1;
   QSound::play(CHESS_PLAY);
}

void MainWindow::autochessAI()//将下棋声音判断胜负AI下一棋合并为一个函数，方便循环
{
    QSound::play(CHESS_PLAY);
    game->findbestAIplace(clickPosX,clickPosY);
    isOver();
    update();
}

//实现AI对决
void MainWindow::chessAIvsAI()
{
    time1=new QTimer(this);   //设置定时器，记录AI思考时间
    connect(time1,SIGNAL(timeout()),this,SLOT(autochessAI()));   //连接槽函数,循环进行AI下棋
    time1->start(AIthinktime);   //设置AI思考时间
}

void MainWindow::reset()
{
    game->gameStatus=0;
    for(int i=0;i<boardsize;i++)
        for(int j=0;j<boardsize;j++)
            game->board[i][j]=0;  //数组初始化置0

    ui->pushButton->setText("双人对决");
    ui->pushButton_2->setText("人机对决");
    ui->pushButton_3->setText("AI对决");
}

void MainWindow::isOver()
{
    // 判断输赢，每下一子就遍历判断
    if(clickPosX>=0&&clickPosX<boardsize&&
       clickPosY>=0&&clickPosY<boardsize&&
       game->board[clickPosX][clickPosY]!=0)
    {
        if(game->board[clickPosX][clickPosY]==1&&game->isJinShou(clickPosX,clickPosY)&&game->gameStatus==1)
        {
            game->endtype='j';game->is_win=true;
        }
        //没有出现禁手判断是否有一方获胜
        else if(game->isWin(clickPosX,clickPosY)==true&&game->gameStatus==1)
        {
            if(time1)
            {
                time1->stop();
                time1=nullptr;   //关闭定时器
            }
            if(game->board[clickPosX][clickPosY]==1)
            {
               game->endtype='b';game->is_win=true;
            }
            else
            {
               game->endtype='w';game->is_win=true;
            }
        }
        else if(game->isHeQi()&&game->gameStatus==1)
        {
            if(time1)
            {
                time1->stop();
                time1=nullptr;   //关闭定时器
            }
          game->endtype='h';game->is_win=true;
        }
    }
    if(game->is_win){
        messagebox(game->endtype);}
}

void MainWindow::messagebox(char type)
{
    QString str;
    if(type=='j'){
        str=" 禁手，游戏结束！白棋获胜！";
    }
    else if(type=='b'){
         str=" 黑棋获胜！";
    }
     else if(type=='w'){
       str=" 白棋获胜！";
    }
    else if(type=='h'){
       str="已无空位下棋！和棋!";
    }
    QMessageBox::StandardButton standarbutton=QMessageBox::information(this,"游戏结束！",str+"\n 点击Ok返回主界面",QMessageBox::Ok);
    if(standarbutton==QMessageBox::Ok)
    {
       reset();
    }
}

void MainWindow::on_pushButton_clicked()//双人
{
    //如果计时器还开着，关闭计时器
    if(time1)
    {
        time1->stop();
        delete time1;
        time1=nullptr;
    }

    //激活鼠标追踪
     ui->centralwidget->setMouseTracking(true);
     this->setMouseTracking(true);  //激活整个窗体的鼠标追踪
     ui->pushButton->setMouseTracking(true); //进入某个按钮鼠标追踪失效，重新激活

     game_type='p';//双人模式
     init(game_type);  //初始化

     if(on_rechess==true){
     ui->pushButton_5->setEnabled(true);}//仅双人模式且开启禁手才能用可用

     ui->pushButton->setText("重新开始");
     ui->pushButton_2->setText("人机对决");
     ui->pushButton_3->setText("AI对决");

     update();
}

void MainWindow::on_pushButton_2_clicked()//人机
{
    //如果计时器还开着，关闭计时器
    if(time1)
    {
        time1->stop();
        delete time1;
        time1=nullptr;
    }

    //激活鼠标追踪
    ui->centralwidget->setMouseTracking(true);
    this->setMouseTracking(true);  //激活整个窗体的鼠标追踪
    ui->pushButton_2->setMouseTracking(true); //进入某个按钮鼠标追踪失效，重新激活

    game_type='r';//人机模式
    init(game_type);

    int ret = QMessageBox::question(this, "先手?", "是否选择玩家为先手（执黑棋）",
                  QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape);

        if (ret == QMessageBox::Yes) {
           game->playerFirst=true;
        }
        else {
            game->playerFirst=false;
        }
    if(game->playerFirst==false)
        AIfirst();

    ui->pushButton_5->setEnabled(false);//仅双人模式可用
    ui->pushButton_2->setText("重新开始");
    ui->pushButton->setText("双人对决");
    ui->pushButton_3->setText("AI对决");

    update();
}

void MainWindow::on_pushButton_3_clicked()//AIvsAI
{
    //AIvsAI不允许人下棋,关闭鼠标追踪
    this->setMouseTracking(false);

    ui->pushButton_5->setEnabled(false);//仅双人模式可用
    ui->pushButton_3->setText("重新开始");
    ui->pushButton->setText("双人对决");
    ui->pushButton_2->setText("人机对决");

    game_type='z';
    init(game_type);

    AIfirst();
    game->playerFlag=!game->playerFlag;//换手
    chessAIvsAI();

    update();
}



void MainWindow::on_pushButton_5_clicked()
{
    game->rechess();

}

void MainWindow::on_pushButton_4_clicked()//联机
{
    if(time1)
    {
        time1->stop();
        delete time1;
        time1=nullptr;
    }

    netgame *netgame2=new netgame;
    netgame *netgame3=new netgame;
    this->close();
    netgame2->show();
    netgame3->show();
}

void netgame::on_pushButton_3_clicked()//联机返回
{
    MainWindow *win=new MainWindow;
    win->show();
    this->close();
}
