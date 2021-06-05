#include "netgame.h"
#include "ui_netgame.h"

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
#include<QLabel>

#define CHESS_PLAY  "://sound/chessone.wav"  //落子音的路径
#define WIN_SOUND  "://sound/win.wav"   //获胜提示音路径
#define LOSE_SOUND "://sound/lose.wav"   //失败提示音路径

const int boardMargin = 35; // 棋盘外边距属性
const int r = 15; // 棋子半径
const int markSize = 6; // 落子标记边长
const int blockSize = 40; // 格子的大小
const int mousearea= 20; // 鼠标点击的模糊距离上限

//通过tcp构建sever和client，通过readall和write传递棋面信息
//通过改变msg[5]传递信息,用recv读
//recv[0]:color of chess:b=black,w=white;
//        c=connect successfully,
//        g=newgame order,y=yes to new game,n=no to new game
//       d=disconnect,s=sb win
//recv[1],recv[2]:clickPosX
//recv[3],recv[4]:clickPosY

netgame::netgame(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::netgame)
{
    ui->setupUi(this);
    setWindowTitle("六子棋-联机");

    ui->pushButton->setEnabled(false);

    ui->label_2->setText("连接断开");

    game=new Game;

    // 设置棋盘大小
    setFixedSize(boardMargin * 2 + blockSize * boardsize+300, boardMargin * 2 + blockSize * boardsize-50);

    //设置菜单栏
    QMenuBar *menubar=menuBar();
    //菜单中的游戏设置

    //退出游戏
    QAction *actioncl=menubar->addAction("退出游戏");
    connect(actioncl,&QAction::triggered,this,[=]{
        this->close();
        char msg[1]={'d'};
        socket->write(msg,1);
    });


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


}

netgame::~netgame()
{
    delete ui;
    if(game)   //撤销游戏指针
    {
        delete game;
        game = nullptr;
    }
}

void netgame::initGame()
{
    //激活鼠标追踪
    ui->centralwidget->setMouseTracking(true);
    this->setMouseTracking(true);  //激活整个窗体的鼠标追踪
    ui->pushButton->setMouseTracking(true); //进入某个按钮鼠标追踪失效，重新激活

    if(game)
    {
        delete game;
    }
    game=new Game;

    game->gameType='l';
    game->gameStatus=1;//游戏进行中
    game->is_win=false;

    clickPosX=-1;
    clickPosY=-1;//初始化

    game->newgame(game->gameType);
    ui->pushButton->setText("重新开始");

    update();
}

void netgame::paintEvent(QPaintEvent *)
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
        if(is_server)//先手为ture
        {
            //黑棋落子标记
            brush.setColor(Qt::black);
            painter.setBrush(brush);
            painter.drawRect(boardMargin+blockSize*clickPosX-markSize/2,boardMargin+blockSize*clickPosY-markSize/2,markSize,markSize);
        }
        else
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

void netgame::mouseMoveEvent(QMouseEvent *event)
{
    //左键按住拖动，只有拖动页面条时才可以拖动,不写也行
    if(event->buttons()&Qt::LeftButton){
    move(this->frameGeometry().topLeft());
    }

    // 通过鼠标的hover确定落子的标记
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
}

void netgame::mouseReleaseEvent(QMouseEvent *)
{
    //只有有效点击才能下棋，即clickPosX和clickPosY！=初始值-1
  if (clickPosX != -1 && clickPosY != -1
      &&game->gameStatus==1&&game->board[clickPosX][clickPosY] == 0)//要有效点击且游戏进行中
  {
      game->updateMap(clickPosX,clickPosY); //染色
      QSound::play(CHESS_PLAY);
      setmessage(clickPosX,clickPosY);
      update();
      isOver();//每下一子需要判断是否游戏结束
      playerchange();
  }
    update();

}

void netgame::setmessage(int x,int y)
{
    char msg[5];

    if(game->board[x][y]==1)//存下的棋的颜色
       msg[0]='b';
    else msg[0]='w';

    msg[1]=x/10+'0';//存下棋点的行的十位数
    msg[2]=x%10+'0';//存下棋点的行的个位数
    msg[3]=y/10+'0';//列的十位数
    msg[4]=y%10+'0';//列的个位数

    socket->write(msg,5);
}

void netgame::reset()
{
    ui->pushButton->setText("开始游戏");
    game->gameStatus=0;
    for(int i=0;i<boardsize;i++)
        for(int j=0;j<boardsize;j++)
            game->board[i][j]=0;  //数组初始化置0
}

void netgame::isOver()
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
          game->endtype='h';game->is_win=true;
        }
    }
    if(game->is_win){
        messagebox(game->endtype);
        char msg[1]={'s'};
        socket->write(msg,1);}
}

void netgame::messagebox(char type)
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

void netgame::slot_NewConnection()
{
    if(socket)
        return;
    socket = server->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(slot_ReadyRead()));

    char msg[1]={'c'};
    socket->write(msg,1);
}

void netgame::slot_ReadyRead()
{
    QByteArray recv = socket->readAll();//传递信息

//    switch(recv[0]){
//    case 'c': ui->label_2->setText("连接成功");break;
//    }

    if(recv[0]=='c')
    {
        ui->label_2->setText("连接成功");
    }
    else if(recv[0]=='g')
    {
        QMessageBox::StandardButton type=QMessageBox::information(this,"新游戏","对方提出开启新游戏请求，是否同意?",
                                                                  QMessageBox::No|QMessageBox::Yes);
        if(type==QMessageBox::Yes)
        {
           initGame();
            char msg[1]={'y'};
            socket->write(msg,1);
        }
        else
        {
            char msg[1]={'n'};
            socket->write(msg,1);
        }
    }
    //确定重开
    else if(recv[0]=='y')
    {
        initGame();
        playerchange();
    }
    //拒绝重开
    else if(recv[0]=='n')
    {
        //do nothing
    }
    else if(recv[0]=='s')
    {
        isOver();
        messagebox(game->endtype);
    }
    else if(recv[0]=='d')
    {
        ui->label_2->setText("连接断开");
        QMessageBox::StandardButton type=QMessageBox::information(this,"联机断开","对方退出了游戏，是否退出游戏？",
                                                                  QMessageBox::No|QMessageBox::Yes);
        if(type==QMessageBox::Yes)
        {
            this->close();
        }
    }
    //黑子或白子，//接受对面信息，模拟netplayer下棋
    else
    {
        int row=(recv[1]-'0')*10+recv[2]-'0';
        int col=(recv[3]-'0')*10+recv[4]-'0';

        if(recv[0]=='b')
        game->board[row][col]=1;
        else {
            if(recv[0]=='w')
            game->board[row][col]= -1;
        }

        game->playerFlag=!game->playerFlag;
        playerchange();
        update();
    }
}

void netgame::playerchange()
{
    if(game->playerFlag)
        ui->label->setPixmap(QPixmap("://pic/bl.png"));
    else
        ui->label->setPixmap(QPixmap("://pic/w.png"));

    if(is_server!=game->playerFlag)
    {
        this->setMouseTracking(false); //没轮到不能下棋，关闭整个窗体的鼠标追踪
    }
    else{
         this->setMouseTracking(true);  //轮到时激活整个窗体的鼠标追踪
      }
}

void netgame::on_pushButton_clicked()//新游戏请求
{

    QMessageBox::StandardButton type=QMessageBox::information(this,"新游戏?","确定开启新游戏吗？",
                                                              QMessageBox::No|QMessageBox::Yes);
        if(type==QMessageBox::Yes)
        {
            char msg[1]={'g'};
            socket->write(msg,1);
    }

}

void netgame::on_pushButton_2_clicked()
{
    //选择成为sever or client，sever为先手
   QMessageBox::StandardButton type=QMessageBox::information(this,"server or client","server?",
                                                             QMessageBox::No|QMessageBox::Yes);
   if(type==QMessageBox::Yes)
   {
       is_server=true;
       server=new QTcpServer(this);
       socket=NULL;
       server->listen(QHostAddress::Any, 9999);
       connect(server,SIGNAL(newConnection()),this,SLOT(slot_NewConnection()));
       ui->label_3->setPixmap(QPixmap("://pic/bl.png"));
       ui->label_3->setScaledContents(true);
        setWindowTitle("六子棋-联机-sever");

   }
   else if(type==QMessageBox::No)
   {
       is_server=false;
       this->setMouseTracking(false);
       socket=new QTcpSocket(this);
       socket->connectToHost(QHostAddress("127.0.0.1"), 9999);
       connect(socket, SIGNAL(readyRead()), this, SLOT(slot_ReadyRead()));
       ui->label_3->setPixmap(QPixmap("://pic/w.png"));
       ui->label_3->setScaledContents(true);
        setWindowTitle("六子棋-联机-client");
   }

   //让label显示图片，并让图片适配label大小
   ui->label->setPixmap(QPixmap("://pic/bl.png"));
   ui->label->setScaledContents(true);
   ui->label_2->setText("连接成功");
   ui->pushButton->setEnabled(true);
}


void netgame::on_pushButton_4_clicked()
{
    this->close();
    char msg[1]={'d'};
    socket->write(msg,1);
    slot_ReadyRead();
}
