//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <vector>
#include <iostream>
using namespace std;
POINT mouse;

// секция данных игры  

enum class equip
{
    sword,
    axe,
    helmet,
    key
};

//enum class objects
//{
//    door1
//};

typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;
} sprite;

sprite racket;//ракетка игрока

sprite healing;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

//cекция кода

struct Item {
    equip name;
    sprite itemSprite;
};

//struct Object
//{
//    objects title;
//    sprite objectsSprite;
//};

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
    case equip::key:
        return "key";
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

struct portal_ {
    sprite spr;
    int destination;
};

struct Location_ {
    HBITMAP hBack; 
    vector<portal_> portal;
    vector<Item> locationItems;
    vector<sprite> locationObjects;
};

Location_ location[5];

struct player_
{

    vector<Item> hero_items;

    HBITMAP hHealthFull, hHealthEmpty;
    int health_width = 40;
    int max_lives = 5;  
    int current_lives = 3;  
};

player_ player;



void PickItem(int id) {

    player.hero_items.push_back(location[currentLocation].locationItems[id]);
    location[currentLocation].locationItems.erase(location[currentLocation].locationItems.begin() + id);
    
}

void DropItem(int ID)
{
    player.hero_items[ID].itemSprite.x = racket.x+100;
    player.hero_items[ID].itemSprite.y = racket.y;
    location[currentLocation].locationItems.push_back(player.hero_items[ID]);
        player.hero_items.erase(player.hero_items.begin() + ID);


}

void printString(const char* text, int x, int y)
{
    TextOutA(window.context, x, y, text, strlen(text));
}

HFONT hFont;
HFONT hFont2;
HFONT hTmp;

void PrintInventory()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(0, 0, 0));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    if (!hFont2)
    {
        hFont2 = CreateFont(50, 0, 0, 0, FW_LIGHT, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    }


    if (!hFont)
    {
        hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    }

    SelectObject(window.context, hFont);
    TextOutA(window.context, 10, 10, "Inventory", 9);

    for (int i = 0; i < player.hero_items.size(); i++)
    {
        auto s = getIDString(player.hero_items[i].name);

        float w = 300;
        float h = 50;
        float x = 10;
        float y = 60 + h * i *2;

        if (mouse.x > x && mouse.x<x + w && mouse.y>y && mouse.y < y + h)
        {
            SetTextColor(window.context, RGB(0, 0, 0));
            SelectObject(window.context, hFont2);
            printString("Press lkm to drop", x + w, y);

            SetTextColor(window.context, RGB(255, 0, 0));
            if (GetAsyncKeyState(VK_LBUTTON))
            {
                DropItem(i);
            }

        }
        else
        {
            SetTextColor(window.context, RGB(0, 0, 250));
        }

        SelectObject(window.context, hFont);
        TextOutA(window.context, x, y , s.c_str(), s.size());

    }
    
}

bool CheckCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2)
{
    return x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2;
}



void ItemCheckCollisions() 
{
    bool pickKey = GetAsyncKeyState('P');
    int id = 0;

    SetTextColor(window.context, RGB(0, 0, 0));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    if (!hFont)
    {
        hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
        hTmp = (HFONT)SelectObject(window.context, hFont);
    }

    
    for (auto& i : location[currentLocation].locationItems) 
    {
        if (CheckCollision (racket.x, racket.y, racket.width, racket.height, i.itemSprite.x, i.itemSprite.y, i.itemSprite.width, i.itemSprite.height))
        {

            printString("Press P to pick", racket.x + racket.width, racket.y + 40);
            if (pickKey)
            {
                PickItem(id);
            }
           
        }

        id++;
    }
}

void ObjectCheckCollisions()
{
    int id = 0;
    for (auto& i : location[currentLocation].locationObjects)
    {
        if (CheckCollision(racket.x, racket.y, racket.width, racket.height, i.x, i.y, i.width, i.height))
        {
            
            if (i.hBitmap == healing.hBitmap)
            {
                player.current_lives++;
                location[currentLocation].locationObjects.erase(location[currentLocation].locationObjects.begin() + id);
            }

        }
        id++;

    }

}



void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
   
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
   
    healing.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player.hHealthFull = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player.hHealthEmpty = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------
    location[0].hBack = (HBITMAP)LoadImageA(NULL, "loc0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    
    Item helmetItem;
    helmetItem.name = equip::helmet;
    helmetItem.itemSprite.hBitmap = (HBITMAP)LoadImageA(NULL, "helmet.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    helmetItem.itemSprite.x = 100;
    helmetItem.itemSprite.y = 100;
    helmetItem.itemSprite.width = 50;
    helmetItem.itemSprite.height = 50;
    location[0].locationItems.push_back(helmetItem);
    portal_ door1;
    door1.destination = 2;
    door1.spr.hBitmap = (HBITMAP)LoadImageA(NULL, "door1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    door1.spr.width = 50;
    door1.spr.height = 50;
    door1.spr.x = 500;
    door1.spr.y = 500;
    location[1].portal.push_back(door1);

    portal_ port0;
    port0.destination = 1;
    port0.spr.width = 50;
    port0.spr.height = 50;
    port0.spr.x = window.width - door1.spr.width;
    port0.spr.y = 500;
    location[0].portal.push_back(port0);

    Item axeItem;
    axeItem.name = equip::axe;
    axeItem.itemSprite.hBitmap = (HBITMAP)LoadImageA(NULL, "axe.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    axeItem.itemSprite.x = 300;
    axeItem.itemSprite.y = 100;
    axeItem.itemSprite.width = 50;
    axeItem.itemSprite.height = 50;
    location[0].locationItems.push_back(axeItem);

    healing.width = 40;
    healing.height = 40;
    healing.x = 700;
    healing.y = 400;
    location[0].locationObjects.push_back(healing);


    location[1].hBack = (HBITMAP)LoadImageA(NULL, "loc1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    

    Item keyItem;
    keyItem.name = equip::key;
    keyItem.itemSprite.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    keyItem.itemSprite.x = 600;
    keyItem.itemSprite.y = 200;
    keyItem.itemSprite.width = 50;
    keyItem.itemSprite.height = 50;
    location[1].locationItems.push_back(keyItem);
    


    location[2].hBack = (HBITMAP)LoadImageA(NULL, "loc2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    

    location[3].hBack = (HBITMAP)LoadImageA(NULL, "loc3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
   

    location[4].hBack = (HBITMAP)LoadImageA(NULL, "loc4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    


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
    

    for (auto& i : location[currentLocation].portal)
    {
        if(i.spr.hBitmap)
        {
            ShowBitmap(window.context, i.spr.x, i.spr.y, i.spr.width, i.spr.height, i.spr.hBitmap);
        }

        if (CheckCollision(racket.x, racket.y, racket.width, racket.height, i.spr.x, i.spr.y, i.spr.width, i.spr.height))
        {
            currentLocation = i.destination;
            racket.x = 0;
        }
    }
}


void DrawHealth() {
    int margin = 10;
    int startX = window.width - 50;
    int startY = 10;

    for (int i = 0; i < player.max_lives; i++) {
        
        HBITMAP currentHealth = (i < player.current_lives) ? player.hHealthFull : player.hHealthEmpty;

        
        ShowBitmap(
            window.context,
            startX - (i * (player.health_width + margin)), 
            startY,
            player.health_width,
            player.health_width,
            currentHealth
        );
    }
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, location[currentLocation].hBack);//задний фон
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока
    

}

void ShowItem()
{
    for (const auto& i : location[currentLocation].locationItems)
    {
        
            ShowBitmap(window.context, i.itemSprite.x, i.itemSprite.y, i.itemSprite.width, i.itemSprite.height, i.itemSprite.hBitmap);
        
    }
   
}

void ShowObjects()
{
    for (const auto& i : location[currentLocation].locationObjects)
    {
        
            ShowBitmap(window.context, i.x, i.y, i.width, i.height, i.hBitmap);
            
        
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
     ShowCursor(FALSE);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowItem();
        ShowObjects();
        PrintInventory();
        DrawHealth();

        
        GetCursorPos(&mouse);
        ScreenToClient(window.hWnd, &mouse);
        int sz = 5;
        Ellipse(window.context, mouse.x - sz, mouse.y - sz, mouse.x + sz, mouse.y + sz);

        ItemCheckCollisions();
        ObjectCheckCollisions();
        ProcessInput();//опрос клавиатуры

        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

    }

}
