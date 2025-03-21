//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <vector>
#include <iostream>
using namespace std;


// секция данных игры  

enum class equip {
    sword,
    axe,
    helmet
};

typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//хэндл к спрайту шарика 
} sprite;

sprite racket;//ракетка игрока

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

//cекция кода

struct Item {
    equip name;
    sprite sprite;
    bool isPicked;
};

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
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

    vector<equip> hero_items;
    int count_horse;
    int life;

    void Init()
    {
        count_horse = 1;
        life = 1;
    }

};

player_ player;

void PickItem(Item& item) {
    if (!item.isPicked) {
        player.hero_items.push_back(item.name); 
        item.isPicked = true; 
        
    }
}

void CheckCollisions() {
    for (auto& i : location[currentLocation].locationItems) {
        if (!i.isPicked) {
            
            if (racket.x < i.sprite.x + i.sprite.width &&
                racket.x + racket.width > i.sprite.x &&
                racket.y < i.sprite.y + i.sprite.height &&
                racket.y + racket.height > i.sprite.y) {
                PickItem(i); 
            }
        }
    }
}



void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
   
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
    helmetItem.isPicked = false;
    location[0].locationItems.push_back(helmetItem);


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
    racket.speed = 30;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    
    

}



void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
    //PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
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
    ShowBitmap(window.context, 0, 0, window.width, window.height, location[currentLocation].hBack);//задний фон
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока
    

}

void ShowItem()
{
    for (auto i : location[currentLocation].locationItems)
    {
        if (!i.isPicked)
        {
            ShowBitmap(window.context, i.sprite.x, i.sprite.y, i.sprite.width, i.sprite.height, i.sprite.hBitmap, true);
        }
        
    }
   
}


void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    // mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    // ShowCursor(NULL);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowItem();
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
        CheckCollisions();
        ProcessInput();//опрос клавиатуры

    }

}
