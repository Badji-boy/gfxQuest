//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <vector>
#include <iostream>
using namespace std;
POINT mouse;

// ������ ������ ����  

enum class equip {
    sword,
    axe,
    helmet
};

typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//����� � ������� ������ 
} sprite;

sprite racket;//������� ������

struct {
    HWND hWnd;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBITMAP hBack;// ����� ��� �������� �����������

//c����� ����

struct Item {
    equip name;
    sprite sprite;
};

string getIDString(equip itemID)
{
    switch (itemID) {
    case equip::sword:
        return "sword";
        break;
    case equip::axe:
        return "axe";
        break;
    case equip::helmet:
        return "helmet";
        break;
    default:
        return "Unknown item\n";
        break;
    }
    return NULL;
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

int currentLocation = 0;

struct Location_ {
    HBITMAP hBack; 
    int LeftPort; 
    int RightPort;
    vector<Item> locationItems;
};

Location_ location[5];

struct player_
{

    vector<Item> hero_items;
    int count_horse;
    int life;

    void Init()
    {
        count_horse = 1;
        life = 1;
    }

};

player_ player;

void PickItem(int id) {

    player.hero_items.push_back(location[currentLocation].locationItems[id]);
    location[currentLocation].locationItems.erase(location[currentLocation].locationItems.begin() + id);
    
}

void DropItem(int ID)
{
    player.hero_items[ID].sprite.x = racket.x+100;
    player.hero_items[ID].sprite.y = racket.y;
    location[currentLocation].locationItems.push_back(player.hero_items[ID]);
        player.hero_items.erase(player.hero_items.begin() + ID);


}

HFONT hFont;
HFONT hTmp;

void PrintInventory()
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(0, 0, 0));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    if (!hFont)
    {
        hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
        hTmp = (HFONT)SelectObject(window.context, hFont);
    }

    TextOutA(window.context, 10, 10, "Inventory", 9);

    for (int i = 0; i < player.hero_items.size(); i++)
    {
        auto s = getIDString(player.hero_items[i].name);

        float w = 300;
        float h = 50;
        float x = 10;
        float y = 40 + h * i;

        if (mouse.x > x && mouse.x<x + w && mouse.y>y && mouse.y < y + h)
        {
            SetTextColor(window.context, RGB(250, 0, 0));

            if (GetAsyncKeyState(VK_LBUTTON))
            {
                DropItem(i);
            }

        }
        else
        {
            SetTextColor(window.context, RGB(0, 0, 250));
        }

        TextOutA(window.context, x, y, s.c_str(), s.size());

    }
    
}




void CheckCollisions() {
    int id = 0;
    for (auto& i : location[currentLocation].locationItems) 
    {
        if (racket.x < i.sprite.x + i.sprite.width &&
            racket.x + racket.width > i.sprite.x &&
            racket.y < i.sprite.y + i.sprite.height &&
            racket.y + racket.height > i.sprite.y) 
        {
            PickItem(id); 
        }

        id++;
    }
}



void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
   
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------
    location[0].hBack = (HBITMAP)LoadImageA(NULL, "loc0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    location[0].RightPort = 1;
    location[0].LeftPort = -1;
    Item helmetItem;
    helmetItem.name = equip::helmet;
    helmetItem.sprite.hBitmap = (HBITMAP)LoadImageA(NULL, "helmet.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    helmetItem.sprite.x = 100;
    helmetItem.sprite.y = 100;
    helmetItem.sprite.width = 50;
    helmetItem.sprite.height = 50;
    
    location[0].locationItems.push_back(helmetItem);

    Item axeItem;
    axeItem.name = equip::axe;
    axeItem.sprite.hBitmap = (HBITMAP)LoadImageA(NULL, "helmet.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    axeItem.sprite.x = 300;
    axeItem.sprite.y = 100;
    axeItem.sprite.width = 50;
    axeItem.sprite.height = 50;
    location[0].locationItems.push_back(axeItem);


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
    location[3].RightPort = -1;
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



void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;
    if (GetAsyncKeyState(VK_UP)) racket.y -= racket.speed;
    if (GetAsyncKeyState(VK_DOWN)) racket.y += racket.speed;

    if (racket.y < 0)
    {
        racket.y = 0;
    }
    if (racket.y > window.height - racket.height)
    {
        racket.y = window.height - racket.height;
    }

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

    if (racket.x > window.width - racket.width)
    {
        if (location[currentLocation].RightPort >= 0)
        {
            racket.x = 0;
            currentLocation = location[currentLocation].RightPort;
        }
        else
        {
            racket.x = window.width - racket.width;
        }
    }

}


void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, location[currentLocation].hBack);//������ ���
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ������� ������
    

}

void ShowItem()
{
    for (auto i : location[currentLocation].locationItems)
    {
        //if (!i.isPicked)
        {
            ShowBitmap(window.context, i.sprite.x, i.sprite.y, i.sprite.width, i.sprite.height, i.sprite.hBitmap);
        }
        
    }
   
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
     ShowCursor(FALSE);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//������ ���, ������� � �����
        ShowItem();
        PrintInventory();

        
        GetCursorPos(&mouse);
        ScreenToClient(window.hWnd, &mouse);
        int sz = 5;
        Ellipse(window.context, mouse.x - sz, mouse.y - sz, mouse.x + sz, mouse.y + sz);


        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)
        CheckCollisions();
        ProcessInput();//����� ����������

    }

}
