//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <vector>
#include <iostream>
using namespace std;


// ������ ������ ����  
typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//����� � ������� ������ 
} sprite;

sprite racket;//������� ������
sprite ball;//�����

struct {
    int score, balls;//���������� ��������� ����� � ���������� "������"
    bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;

struct {
    HWND hWnd;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBITMAP hBack;// ����� ��� �������� �����������

//c����� ����
int currentLocation = 0;

struct Location_
{

    HBITMAP hBack;
    int LeftPort;
    int RightPort;

};

Location_ location[5];


void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------
    location[0].hBack = (HBITMAP)LoadImageA(NULL, "loc0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    location[0].RightPort = 1;
    location[0].LeftPort = -1;


    location[1].hBack = (HBITMAP)LoadImageA(NULL, "loc1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    location[1].RightPort = 2;
    location[1].LeftPort = 0;


    location[2].hBack = (HBITMAP)LoadImageA(NULL, "loc2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    location[2].RightPort = 3;
    location[2].LeftPort = 1;

    location[3].hBack = (HBITMAP)LoadImageA(NULL, "loc3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    location[3].RightPort = 4;
    location[3].LeftPort = 2;

    location[4].hBack = (HBITMAP)LoadImageA(NULL, "loc4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    location[3].RightPort = 5;
    location[3].LeftPort = 3;


    racket.width = 50;
    racket.height = 50;
    racket.speed = 30;//�������� ����������� �������
    racket.x = window.width / 2.;//������� ���������� ����
    racket.y = window.height - racket.height;//���� ���� ���� ������ - �� ������ �������

}

void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
{
    //PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
}

void ShowScore()
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//����� ��� ������
    _itoa_s(game.score, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;
    if (GetAsyncKeyState(VK_UP)) racket.y -= racket.speed;
    if (GetAsyncKeyState(VK_DOWN)) racket.y += racket.speed;


    if (racket.x < 0)
    {
        if (location[currentLocation].LeftPort >= 0)
        {
            racket.x = window.width - racket.width;
            currentLocation = location[currentLocation].LeftPort;
        }
        else
        {
            racket.x = 0;
        }
    }

    if (racket.x > window.width)
    {
        if (location[currentLocation].RightPort >= 0)
        {
            racket.x = 0;
            currentLocation = location[currentLocation].RightPort;
        }
        else
        {
            racket.x = window.width;
        }
    }

}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

    if (hOldbm) // ���� �� ���� ������, ���������� ������
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, location[currentLocation].hBack);//������ ���
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ������� ������


}


void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

    // mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    // ShowCursor(NULL);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//������ ���, ������� � �����
        ShowScore();//������ ���� � �����
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)

        ProcessInput();//����� ����������

    }

}
