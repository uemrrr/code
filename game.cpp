#include "game.h"
#include"mainwindow.h"
#include <utility>
#include <stdlib.h>
#include <time.h>
#include<QDebug>
#include<QApplication>
#include<iostream>
#include<QTimer>
#include<QVector>
#include<QPoint>

#include <vector>
using namespace std;//不加这行需要在调用vector的函数时前面要加上 std::

Game::Game()
{
}

void Game::newgame(char type)
{
    memset(board,0,sizeof(board));
    chess.clear();
    if(type=='r'||type=='z')
    {
    memset(score,0,sizeof(score));
    }
    playerFlag=true;
    is_win=false;

}

void Game::updateMap(int x,int y)
{
  if(playerFirst==false&&gameType=='r')//人机模式且电脑先下时需要把白棋黑棋的数值换一下
  {
    if(playerFlag)
        board[x][y]=-1;
    else
        board[x][y]=1;
  }
  else{
      if(playerFlag)
          board[x][y]=1;
      else
          board[x][y]=-1;
  }

    // 换手
    playerFlag=!playerFlag;
//    qDebug()<<"map";
}

void Game::rechess()//悔棋
{
    int x,y;
    for(int i=0;i<2;i++)
    {
        x=chess.last().x();
        y=chess.last().y();
        board[x][y]=0;
        chess.pop_back();
    }
}

void Game::findbestAIplace(int &clickX,int &clickY)
{
    calculateScore();

    // 寻找并记录分数最大的点的评分和位置
    int maxScore = 0;
    vector<pair<int, int>> maxPoints;

    for (int row = 0; row < boardsize; row++)
        for (int col = 0; col < boardsize; col++)
        {
            // 坐标是空的才能下棋
            if (board[row][col] == 0)
            {
                if (score[row][col] > maxScore)
                {
                    maxPoints.clear(); //有新最大值时清空容器内之前存储的最大值位置，使用vector容器的clear()函数，所有元素置0
                    maxScore = score[row][col];
                    maxPoints.push_back(make_pair(row, col)); //用vector的push_back函数每次从末尾存入
                    //make_pair(x, y)等价于pair<int, char>(x, y)，写起来更方便
                }
                else if (score[row][col] == maxScore)     // 如果有多个最大的数，都存起来
                    maxPoints.push_back(make_pair(row, col));
            }
        }

   // 如果有多个最大点，随机在范围内落子，
    srand((unsigned)time(0));
    int index = rand() % maxPoints.size();  //随机下标，范围在容器容量内

    pair<int, int> pointPair = maxPoints.at(index); //访问，转存元素
    clickX = pointPair.first;// 记录落子点
    clickY = pointPair.second;

    updateMap(clickX,clickY);  //更新棋盘数组
}

void Game::calculateScore()
{
    memset(score,0,sizeof(score));//初始化

    // 统计自己和对手连成的子
    int personNum =0; // 对面连成子的个数
    int botNum = 0; // AI连成子的个数
    int emptyNum = 0; // 各方向空白位的个数

    // 计分，这里用扫描全局计算每个点的分再选出最大的（理论上调整该点在不同情况的赋值可以调整AI智能程度和攻守风格）
    for (int row = 0; row < boardsize; row++)
        for (int col = 0; col < boardsize; col++)
        {
            // 只算空白处
            if (board[row][col])continue;

                // 遍历四个大方位的正负两位
                for (int y = -1; y <= 1; y++)
                    for (int x = -1; x <= 1; x++)
                    {
                        // 重置
                        personNum = 0;
                        botNum = 0;
                        emptyNum = 0;

                        if (!(y == 0 && x == 0))
                        {
                            // 每个方向延伸5个子

                            // 对玩家白子评分（正反两个方向）
                            for (int i = 1; i <= 5; i++)
                            {
                                if (row + i * y > 0 && row + i * y < boardsize &&
                                    col + i * x > 0 && col + i * x < boardsize &&
                                    board[row + i * y][col + i * x] ==1) // 统计玩家下的子
                                {
                                    personNum++;
                                }
                                else if (row + i * y > 0 && row + i * y < boardsize &&
                                         col + i * x > 0 && col + i * x < boardsize &&
                                         board[row +i * y][col + i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }
                            for (int i = 1; i <= 5; i++)
                            {
                                 if (row - i * y > 0 && row - i * y < boardsize &&
                                    col - i * x > 0 && col - i * x < boardsize &&
                                    board[row - i * y][col - i * x] == 1)
                                {
                                    personNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < boardsize &&
                                         col - i * x > 0 && col - i * x < boardsize &&
                                         board[row - i * y][col - i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else
                                    break;
                            }
                            if (personNum == 1)                      // 杀二
                                score[row][col] += 10;
                            else if (personNum == 2)                 // 杀三
                            {
                                if (emptyNum == 1)
                                    score[row][col] += 30;
                                else if (emptyNum == 2)
                                    score[row][col] += 40;
                            }
                            else if (personNum == 3)                 // 杀四
                            {
                                // 量变空位不一样，优先级不一样
                                if (emptyNum == 1)
                                    score[row][col] += 60;
                                else if (emptyNum == 2)
                                    score[row][col] += 110;
                            }
                           else if (personNum == 4){       // 杀五
                                if (emptyNum == 1)
                                    score[row][col] += 130;
                                else if (emptyNum == 2)
                                    score[row][col] += 380;
                            }
                            else if (personNum == 5)                 // 杀六
                                score[row][col] += 10100;

                            // 清空，算AI棋子
                            emptyNum = 0;

                            // 对AI黑子评分
                            for (int i = 1; i <= 5; i++)
                            {
                                if (row + i * y > 0 && row + i * y < boardsize &&
                                    col + i * x > 0 && col + i * x < boardsize &&
                                    board[row + i * y][col + i * x] ==-1) // 统计玩家下的子
                                {
                                    botNum++;
                                }
                                else if (row + i * y > 0 && row + i * y < boardsize &&
                                         col + i * x > 0 && col + i * x < boardsize &&
                                         board[row +i * y][col + i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            for (int i = 1; i <= 5; i++)
                            {
                                if (row - i * y > 0 && row - i * y < boardsize &&
                                    col - i * x > 0 && col - i * x < boardsize &&
                                    board[row - i * y][col - i * x] == -1) // 统计AI下的子
                                {
                                    botNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < boardsize &&
                                         col - i * x > 0 && col - i * x < boardsize &&
                                         board[row - i * y][col - i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else
                                    break;
                            }

                            if (botNum == 0)                      // 普通下子
                                score[row][col] += 5;
                            else if (botNum == 1)                 // 活二
                                score[row][col] += 10;
                            else if (botNum == 2)
                            {
                                if (emptyNum == 1)                // 死三
                                    score[row][col] += 25;
                                else if (emptyNum == 2)
                                    score[row][col] += 50;  // 活三
                            }
                            else if (botNum == 3)
                            {
                                if (emptyNum == 1)                // 死四
                                    score[row][col] += 55;
                                else if (emptyNum == 2)
                                    score[row][col] += 100; // 活四
                            }
                            else if (botNum == 4)
                            {
                                if (emptyNum == 1)                // 死四
                                    score[row][col] += 105;
                                else if (emptyNum == 2)
                                    score[row][col] += 210;// 活四
                            }
                            else if (botNum >= 5)
                                score[row][col] += 10000;   // 活六
                        }
                    }
            }
        }



bool Game::isWin(int row, int col)
{
    //以当前点为中心，历遍8个方位，看各方位是否有包括当前点的同色连续六子
    int k=0;//记录连子数
    int dirctionX[8] = {0, 0, 1, 1, 1, -1, -1, -1};//用于函数判断的8个方位
    int dirctionY[8] = {1, -1, 0, 1, -1, 0, 1, -1};
    for (int h = 0; h < 8; h++) //遍历八个方位
    {
       for(int i=0;i<6;i++)
        {
        if(row + i*dirctionX[h]<boardsize && col+i*dirctionY[h]<boardsize)
          {
            if (board[row + i*dirctionX[h]][col+i*dirctionY[h]] ==board[row][col]) k++;
            else break;
                }
         }
     if(k==6) //长连为禁手，因为在这之前已经判断了禁手所以写k>=6也可以
        return true;
     else k=0;//初始化，为下一轮历遍做准备
    }
    return false;
}


//棋盘没有多余位置下棋时为死局，即全不为空白
bool Game::isHeQi()
{
    for (int i = 0; i < boardsize; i++)
        for (int j = 0; j <boardsize; j++)
        {
            if (board[i][j]==0)
                return false;
        }
    return true;
}


//禁手包括四四禁手和五五禁手
//禁手：黑棋一子落下同时形成下双活四、双五（冲五或活五）或长连（一个方位上有连续6个以上的棋子）
//判断所下的点是不是禁手点
bool Game::isJinShou(int row, int col)
{
    int dirctionX[8] = {0, 0, 1, 1, 1, -1, -1, -1};//用于函数判断的8个方位
    int dirctionY[8] = {1, -1, 0, 1, -1, 0, 1, -1};
    int num=0,k=0;
    for (int h = 0; h < 8; h++) //遍历八个方位
    {
        //包括下的这个点，朝当前方位延申至少六个点,用k计数这个方位有多少连续的黑子
       for(int i=0;i<6;i++)
        {
        if(row + i*dirctionX[h]<boardsize&&col +i*dirctionY[h]<boardsize)
          {
            if (board[row + i*dirctionX[h]][col +i*dirctionY[h]] == 1) k++;
            //只要k>=4即可，中间允许有空，也算禁手
            //该方位上有白子就跳出循环
            if (board[row + i*dirctionX[h]][col +i*dirctionY[h]] ==-1) break;
                }
        else break;
            }
    if(k>=4) num++;
    if(num>=2) return true;
    k=0;
    }
    return false;
}
