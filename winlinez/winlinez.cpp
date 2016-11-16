// winlinez.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "winlinez.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <vector>

#define MAX_LOADSTRING 100
#define BLOCK_WIDTH 36
#define ROWS 9
#define BOARD_WIDTH BLOCK_WIDTH*ROWS
#define	BANNER_HEIGHT 50
#define SCORE_CHAR_NUM 5
#define X_OFFSET 0
#define Y_OFFSET 225
#define X_CYLINDER 420
#define CYLINDER_WIDTH 44
#define Y_CYLINDER 100
#define CYLINDER_HEIGHT 2
#define X_KING 1
#define Y_KING 0
#define KING_WIDTH 72
#define KING_HEIGHT 100
#define X_PLAYER 1
#define Y_PLAYER 100
#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 65
#define CHARACTER_HEIGHT 100
#define CHARACTER_WIDTH 72
#define KING_DX -7
#define PLAYER_DX -16
#define BOTTOM_DX -7
#define X_BOTTOM 1
#define Y_BOTTOM 201
#define BOTTOM_HEIGHT 9
#define BOTTOM_WIDTH 58
#define NEXT_NUM 3
#define INIT_NUM 5
#define TYPE_NUM 8
#define ANIMATION_INTERVAL 100
#define ANIMATION_TIMER 1
#define ANIMATION_NUM 6
#define NEW_INTERVAL 100
#define MOVE_INTERVAL 100

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
int gBoardLeft, gBoardTop, gBoardRight, gBoardBottom;
int gNextLeft, gNextTop;
int gScoreWidth;
int gScoreHeight;
int gPaddingWidth;
int g_aBoard[ROWS + 2][ROWS + 2];
int g_aNext[NEXT_NUM];
HDC g_hdcMem;
bool g_bInit, g_bChoose;
int g_rSrc, g_cSrc, g_rDst, g_cDst;
int g_emptyBlockNum;
int g_iKingScore, g_iPlayerScore;

class CPos
{
public:
	int row, col;
	CPos(int r, int c) : row(r), col(c) {}
};

class CNode
{
public:
	int row, col;
	std::vector<CPos> path;
	CNode() {}
	CNode(int r, int c, std::vector<CPos> &p) :row(r), col(c), path(p)
	{		
		path.push_back(CPos(r, c));
	}
};


// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINLINEZ, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINLINEZ));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINLINEZ));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINLINEZ);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0, 640, 480, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void DrawBlock(HDC hdc, int index, int row, int col)
{
	BitBlt(hdc, gBoardLeft + BLOCK_WIDTH * (col - 1), gBoardTop + BLOCK_WIDTH * (row - 1), BLOCK_WIDTH, BLOCK_WIDTH,
		g_hdcMem, X_OFFSET + BLOCK_WIDTH * index, Y_OFFSET, SRCCOPY);
}

void DrawAnimation(HDC hdc, int index, int state, int row, int col)
{
	BitBlt(hdc, gBoardLeft + BLOCK_WIDTH * (col - 1), gBoardTop + BLOCK_WIDTH * (row - 1), BLOCK_WIDTH, BLOCK_WIDTH,
		g_hdcMem, X_OFFSET + BLOCK_WIDTH * index, Y_OFFSET + BLOCK_WIDTH * state, SRCCOPY);
}

void NewBall(HDC hdc, int index, int row, int col)
{
	for (int i = 6; i < 8; ++i)
	{
		BitBlt(hdc, gBoardLeft + BLOCK_WIDTH * (col - 1), gBoardTop + BLOCK_WIDTH * (row - 1), BLOCK_WIDTH, BLOCK_WIDTH,
			g_hdcMem, X_OFFSET + BLOCK_WIDTH * index, Y_OFFSET + BLOCK_WIDTH * i, SRCCOPY);
		Sleep(NEW_INTERVAL);
	}
	DrawBlock(hdc, index, row, col);
}

void DrawBoard(HDC hdc)
{
	for (int i = 1; i <= ROWS; ++i)
		for (int j = 1; j <= ROWS; ++j)
			DrawBlock(hdc, g_aBoard[i][j], i, j);
}

void DrawNext(HDC hdc)
{
	for (int i = 0; i < NEXT_NUM; ++i)
		BitBlt(hdc, gNextLeft + BLOCK_WIDTH * i, gNextTop, BLOCK_WIDTH, BLOCK_WIDTH,
			g_hdcMem, X_OFFSET + BLOCK_WIDTH * g_aNext[i], Y_OFFSET + BLOCK_WIDTH * 6, SRCCOPY);
}

void DrawScore(HDC hdc)
{
	TCHAR szBuffer[SCORE_CHAR_NUM+1];
	int xOffset = (gPaddingWidth - gScoreWidth) / 2;
	int yOffset = (BANNER_HEIGHT - gScoreHeight) / 2;
	SetTextColor(hdc, RGB(255, 0, 0));
	SetBkColor(hdc, RGB(0, 0, 0));
	TextOut(hdc, xOffset, yOffset, szBuffer, wsprintf(szBuffer, TEXT("%d"), g_iKingScore));
	TextOut(hdc, gBoardRight + xOffset, yOffset, szBuffer, wsprintf(szBuffer, TEXT("%d"), g_iPlayerScore));
}

void DrawKing(HDC hdc, int score)
{
	int xOffset = (gPaddingWidth - CYLINDER_WIDTH) / 2;
	int yOffset = gBoardBottom - BOTTOM_HEIGHT - CYLINDER_HEIGHT * score;
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Rectangle(hdc, xOffset + KING_DX, gBoardTop, xOffset + KING_DX + CHARACTER_WIDTH, gBoardBottom);
	for (int i = 0; i < score; ++i)
		BitBlt(hdc, xOffset, yOffset + CYLINDER_HEIGHT * i, CYLINDER_WIDTH, CYLINDER_HEIGHT, g_hdcMem, X_CYLINDER, Y_CYLINDER, SRCCOPY);
	if (score >= 70)
		BitBlt(hdc, xOffset + KING_DX, yOffset - CHARACTER_HEIGHT, CHARACTER_WIDTH, CHARACTER_HEIGHT, g_hdcMem, X_KING, Y_KING, SRCCOPY);
	else
		BitBlt(hdc, xOffset + KING_DX, yOffset - CHARACTER_HEIGHT, CHARACTER_WIDTH, CHARACTER_HEIGHT, g_hdcMem, X_KING + (KING_WIDTH + 1) * (6 - score / 10), Y_KING, SRCCOPY);
	BitBlt(hdc, xOffset + BOTTOM_DX, gBoardBottom - BOTTOM_HEIGHT, BOTTOM_WIDTH, BOTTOM_HEIGHT, g_hdcMem, X_BOTTOM, Y_BOTTOM, SRCCOPY);
}

void DrawPlayer(HDC hdc, int score)
{
	int xOffset = gBoardRight + (gPaddingWidth - CYLINDER_WIDTH) / 2;
	int yOffset = gBoardBottom - BOTTOM_HEIGHT - CYLINDER_HEIGHT * score;
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Rectangle(hdc, xOffset + PLAYER_DX, gBoardTop, xOffset + PLAYER_DX + CHARACTER_WIDTH, gBoardBottom);
	for (int i = 0; i < score; ++i)
		BitBlt(hdc, xOffset, yOffset + CYLINDER_HEIGHT * i, CYLINDER_WIDTH, CYLINDER_HEIGHT, g_hdcMem, X_CYLINDER, Y_CYLINDER, SRCCOPY);
	BitBlt(hdc, xOffset + PLAYER_DX, yOffset - CHARACTER_HEIGHT, CHARACTER_WIDTH, CHARACTER_HEIGHT, g_hdcMem, X_PLAYER, Y_PLAYER, SRCCOPY);
	if (score >= 50)
		BitBlt(hdc, xOffset + PLAYER_DX, yOffset - CHARACTER_HEIGHT + 6, PLAYER_WIDTH, PLAYER_HEIGHT, g_hdcMem, X_PLAYER + CHARACTER_WIDTH + 1 + (PLAYER_WIDTH + 1) * (score / 10 - 5), Y_PLAYER, SRCCOPY);
	BitBlt(hdc, xOffset + BOTTOM_DX, gBoardBottom - BOTTOM_HEIGHT, BOTTOM_WIDTH, BOTTOM_HEIGHT, g_hdcMem, X_BOTTOM, Y_BOTTOM, SRCCOPY);
}

bool EliminateBlock(HDC hdc, int row, int col)
{
	static int dr[] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static int dc[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	static int cntSame[8];
	int current = g_aBoard[row][col];
	int r, c, sum = 0;
	memset(cntSame, 0, sizeof(cntSame));
	for (int i = 0; i < 8; ++i)
	{
		r = row;
		c = col;
		while (g_aBoard[r += dr[i]][c += dc[i]] == g_aBoard[row][col])
			++cntSame[i];
	}
	for (int i = 8; i < 10; ++i)
	{
		for (int j = 0; j < 4; ++j)
			if (cntSame[j] + cntSame[j + 4] + 1 >= 5)
			{
				DrawAnimation(hdc, g_aBoard[row][col], i, row, col);
				r = row;
				c = col;
				for (int k = 0; k < cntSame[j]; ++k)
				{
					r += dr[j];
					c += dc[j];
					DrawAnimation(hdc, g_aBoard[r][c], i, r, c);
				}
				r = row;
				c = col;
				for (int k = 0; k < cntSame[j + 4]; ++k)
				{
					r += dr[j + 4];
					c += dc[j + 4];
					DrawAnimation(hdc, g_aBoard[r][c], i, r, c);
				}
			}
		Sleep(ANIMATION_INTERVAL);
	}
	for (int j = 0; j < 4; ++j)
		if (cntSame[j] + cntSame[j + 4] + 1 >= 5)
		{
			sum += cntSame[j] + cntSame[j + 4];
			DrawBlock(hdc, 0, row, col);
			g_aBoard[row][col] = 0;
			r = row;
			c = col;
			for (int k = 0; k < cntSame[j]; ++k)
			{
				r += dr[j];
				c += dc[j];
				DrawBlock(hdc, 0, r, c);
				g_aBoard[r][c] = 0;
			}
			r = row;
			c = col;
			for (int k = 0; k < cntSame[j + 4]; ++k)
			{
				r += dr[j + 4];
				c += dc[j + 4];
				DrawBlock(hdc, 0, r, c);
				g_aBoard[r][c] = 0;
			}
		}
	if (sum)
	{
		g_iPlayerScore += (sum + 1) * 2;
		g_emptyBlockNum += sum + 1;
		DrawScore(hdc);
		DrawKing(hdc, g_iPlayerScore < 100 ? 100 : 100 * 100 / g_iPlayerScore);
		DrawPlayer(hdc, g_iPlayerScore < 100 ? g_iPlayerScore : 100);
		return true;
	}
	else
		return false;
}

bool NextRound(HDC hdc)
{
	int n;
	if (g_emptyBlockNum <= 0)
		return false;
	n = rand() % g_emptyBlockNum;
	for (int i = 1; i <= ROWS; ++i)
		for (int j = 1; j <= ROWS; ++j)
			if (g_aBoard[i][j] == 0 && n-- == 0)
			{
				int t = g_aNext[0];
				for (int k = 0; k < NEXT_NUM - 1; ++k)
					g_aNext[k] = g_aNext[k + 1];
				g_aNext[NEXT_NUM - 1] = (rand() % (TYPE_NUM - 1)) + 1;
				DrawNext(hdc);
				NewBall(hdc, t, i, j);
				g_aBoard[i][j] = t;
				--g_emptyBlockNum;
				EliminateBlock(hdc, i, j);
				if (g_emptyBlockNum)
					return true;
				else
					return false;
			}
	return false;
}

void Init(HDC hdc)
{
	int cnt = 0;
	int row, col, index;
	g_bChoose = false;
	g_rSrc = 1;
	g_cSrc = 1;
	g_iKingScore = 100;
	g_iPlayerScore = 0;
	DrawScore(hdc);
	DrawKing(hdc, 100);
	DrawPlayer(hdc, 0);
	memset(g_aBoard, 0, sizeof(g_aBoard));
	for (int i = 0; i < ROWS + 2; ++i)
		g_aBoard[i][0] = g_aBoard[i][ROWS + 1] = g_aBoard[0][i] = g_aBoard[ROWS + 1][i] = -1;
	DrawBoard(hdc);
	srand(time(NULL));
	for (int i = 0; i < NEXT_NUM; ++i)
		g_aNext[i] = (rand() % (TYPE_NUM - 1)) + 1;
	DrawNext(hdc);
	while (cnt < INIT_NUM)
	{
		row = (rand() % ROWS) + 1;
		col = (rand() % ROWS) + 1;
		if (g_aBoard[row][col] == 0)
		{
			++cnt;
			index = (rand() % (TYPE_NUM - 1)) + 1;
			g_aBoard[row][col] = index;
			NewBall(hdc, index, row, col);
		}
	}
	g_emptyBlockNum = ROWS * ROWS - INIT_NUM;
}

bool FindWay(HDC hdc, int rSrc, int cSrc, int rDst, int cDst)
{
	using std::queue;
	using std::vector;
	queue<CNode> q;
	bool bVisited[ROWS + 2][ROWS + 2];
	CNode t;
	memset(bVisited, false, sizeof(bVisited));
	q.push(CNode(rSrc, cSrc, vector<CPos>()));
	bVisited[rSrc][cSrc] = true;
	while (!q.empty())
	{
		t = q.front();
		q.pop();
		if (t.row == rDst && t.col == cDst)
		{
			for (size_t i = 1; i < t.path.size(); ++i)
			{
				DrawBlock(hdc, 0, t.path[i - 1].row, t.path[i - 1].col);
				DrawBlock(hdc, g_aBoard[rSrc][cSrc], t.path[i].row, t.path[i].col);
				Sleep(MOVE_INTERVAL);
			}
			g_aBoard[rDst][cDst] = g_aBoard[rSrc][cSrc];
			g_aBoard[rSrc][cSrc] = 0;
			return true;
		}
		if (g_aBoard[t.row + 1][t.col] == 0 && !bVisited[t.row + 1][t.col])
		{
			q.push(CNode(t.row + 1, t.col, t.path));
			bVisited[t.row + 1][t.col] = true;
		}
		if (g_aBoard[t.row - 1][t.col] == 0 && !bVisited[t.row - 1][t.col])
		{
			q.push(CNode(t.row - 1, t.col, t.path));
			bVisited[t.row - 1][t.col] = true;
		}
		if (g_aBoard[t.row][t.col + 1] == 0 && !bVisited[t.row][t.col + 1])
		{
			q.push(CNode(t.row, t.col + 1, t.path));
			bVisited[t.row][t.col + 1] = true;
		}
		if (g_aBoard[t.row][t.col - 1] == 0 && !bVisited[t.row][t.col - 1])
		{
			q.push(CNode(t.row, t.col - 1, t.path));
			bVisited[t.row][t.col - 1] = true;
		}
	}
	return false;
}


//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT clientRect;
	static HBITMAP hBitmap;
	static int iState;
	int x, y, cxChar, cyChar;
	TEXTMETRIC tm;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_RESTART:
			hdc = GetDC(hWnd);
			g_hdcMem = CreateCompatibleDC(hdc);
			SelectObject(g_hdcMem, hBitmap);
			Init(hdc);
			g_bInit = false;
			DeleteDC(g_hdcMem);
			ReleaseDC(hWnd, hdc);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
		GetClientRect(hWnd, &clientRect);
		gBoardLeft = clientRect.left + (gPaddingWidth = (clientRect.right - clientRect.left - BOARD_WIDTH) / 2);
		gBoardTop = clientRect.top + BANNER_HEIGHT + (clientRect.bottom - clientRect.top - BANNER_HEIGHT - BOARD_WIDTH) / 2;
		gBoardRight = gBoardLeft + BLOCK_WIDTH * ROWS;
		gBoardBottom = gBoardTop + BLOCK_WIDTH * ROWS;
		gNextLeft = gBoardLeft + (ROWS - NEXT_NUM) / 2 * BLOCK_WIDTH;
		gNextTop = clientRect.top + (BANNER_HEIGHT - BLOCK_WIDTH) / 2;
		hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SPIRIT));
		g_bInit = true;
		SetTimer(hWnd, ANIMATION_TIMER, ANIMATION_INTERVAL, NULL);
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		gScoreWidth = cxChar * SCORE_CHAR_NUM;
		gScoreHeight = cyChar;
		ReleaseDC(hWnd, hdc);
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &clientRect);
		gBoardLeft = clientRect.left + (gPaddingWidth = (clientRect.right - clientRect.left - BOARD_WIDTH) / 2);
		gBoardTop = clientRect.top + BANNER_HEIGHT + (clientRect.bottom - clientRect.top - BANNER_HEIGHT - BOARD_WIDTH) / 2;
		gBoardRight = gBoardLeft + BLOCK_WIDTH * ROWS;
		gBoardBottom = gBoardTop + BLOCK_WIDTH * ROWS;
		gNextLeft = gBoardLeft + (ROWS - NEXT_NUM) / 2 * BLOCK_WIDTH;
		gNextTop = clientRect.top + (BANNER_HEIGHT - BLOCK_WIDTH) / 2;
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		g_hdcMem = CreateCompatibleDC(hdc);
		SelectObject(g_hdcMem, hBitmap);
		if (g_bInit)
		{
			Init(hdc);
			g_bInit = false;
		}
		else
		{
			DrawScore(hdc);
			DrawKing(hdc, g_iPlayerScore < 100 ? 100 : 100 * 100 / g_iPlayerScore);
			DrawPlayer(hdc, g_iPlayerScore < 100 ? g_iPlayerScore : 100);
			DrawNext(hdc);
			DrawBoard(hdc);
		}
		DeleteDC(g_hdcMem);
		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		hdc = GetDC(hWnd);
		g_hdcMem = CreateCompatibleDC(hdc);
		SelectObject(g_hdcMem, hBitmap);
		if (x < gBoardLeft || x >= gBoardRight || y < gBoardTop || y >= gBoardBottom)
		{
			g_bChoose = false;
			DrawBlock(hdc, g_aBoard[g_rSrc][g_cSrc], g_rSrc, g_cSrc);
		}
		else
		{
			x -= gBoardLeft;
			y -= gBoardTop;
			if (g_bChoose)
			{ 
				g_cDst = x / BLOCK_WIDTH + 1;
				g_rDst = y / BLOCK_WIDTH + 1;
				if (g_aBoard[g_rDst][g_cDst])
				{
					DrawBlock(hdc, g_aBoard[g_rSrc][g_cSrc], g_rSrc, g_cSrc);
					g_cSrc = g_cDst;
					g_rSrc = g_rDst;
					iState = 0;
				}
				else
				{
					if (FindWay(hdc, g_rSrc, g_cSrc, g_rDst, g_cDst))
					{
						if (!EliminateBlock(hdc, g_rDst, g_cDst))
							for (int i = 0; i < NEXT_NUM; ++i)
								if (!NextRound(hdc))
								{
									KillTimer(hWnd, ANIMATION_TIMER);
									MessageBox(hWnd, TEXT("GAME OVER!"), TEXT("INFO"), MB_OK);
									SetTimer(hWnd, ANIMATION_TIMER, ANIMATION_INTERVAL, NULL);
									Init(hdc);
									g_bInit = false;
									break;	//注意这里是跳出for循环
								}
						g_bChoose = false;
					}
					else
						MessageBeep(MB_APPLMODAL);
				}
				
			}
			else
			{
				g_cSrc = x / BLOCK_WIDTH + 1;
				g_rSrc = y / BLOCK_WIDTH + 1;
				if (g_aBoard[g_rSrc][g_cSrc])
				{
					g_bChoose = true;
					iState = 0;
				}
			}
		}
		DeleteDC(g_hdcMem);
		ReleaseDC(hWnd, hdc);
		break;
	case WM_TIMER:
		if (g_bChoose && wParam == ANIMATION_TIMER)
		{
			hdc = GetDC(hWnd);
			g_hdcMem = CreateCompatibleDC(hdc);
			SelectObject(g_hdcMem, hBitmap);
			DrawAnimation(hdc, g_aBoard[g_rSrc][g_cSrc], iState++, g_rSrc, g_cSrc);
			if (iState >= ANIMATION_NUM)
				iState = 0;
			DeleteDC(g_hdcMem);
			ReleaseDC(hWnd, hdc);

		}
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		KillTimer(hWnd, ANIMATION_TIMER);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
