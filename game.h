#ifndef GAME_H
#define GAME_H

#include<QPoint>
#include <QVector>

const int boardsize=21;  //棋盘尺寸,20个格子，21条线，21*21个点可下棋

class Game
{

public:
    Game();

//    //用二维vector容器动态储存棋盘各点情况和分数，方便玩家自己调整棋盘大小
//    std::vector<std::vector<int>> mapVec;
//    // 存储当前游戏棋盘和棋子的情况,0=无子，1=黑子，-1=白子
//    std::vector<std::vector<int>> scoreVec; // 存储各个点的评分

    bool playerFlag;
    bool playerFirst;
    bool is_win;
    bool gameStatus; // 游戏状态,1=进行中,0=非进行中
    char endtype;//游戏结束情况，j=禁手，b=黑棋获胜，w=白棋获胜，h=和棋
    char gameType; // 游戏模式,p=双人对战，r=人机对战，z=机机对战,l=联机模式
    int board[21][21];  //存储当前游戏棋盘和棋子的情况,0=无子，1=黑子，-1=白子
    int score[21][21];  // 存储各个点的评分
     QVector<QPoint> chess; //按顺序存已下棋子位置，用QVecotr相较于Vector更安全

//    void boardclear();  //清理棋盘
//    void scoreclear();  //清空score数组

    void newgame(char type);  //开始游戏
    void updateMap(int x, int y); // 每次落子后更新游戏棋盘

    void rechess();//人悔棋
    void findbestAIplace(int &clickX, int &clickY); // 机器下棋
    void calculateScore(); // 计算各点评分，为AI下棋提供依据

    bool isJinShou(int row,int col); //判断是否禁手
    bool isWin(int row, int col); // 判断游戏是否胜利
    bool isHeQi(); // 判断是否下满
};

#endif // GAME_H
