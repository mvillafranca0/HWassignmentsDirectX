#include "MyDirectX.h"

const string APPTITLE = "tank_game";
const int SCREENWIDTH = 640;
const int SCREENHEIGHT = 480;
const int cellwidth = 24;
const int cellheight = 24;
const int cellrows = SCREENHEIGHT / cellheight;
const int cellcolumns = SCREENWIDTH / cellwidth;
const int BULLET_NUMBERS = 100;
const int ENEMY_NUMBERS = 5;
int cell[cellrows][cellcolumns];
int last_bullet = -1;
int valid_bullet[BULLET_NUMBERS];
int last_explode = -1;
int valid_explode[BULLET_NUMBERS];
int start_bullet;
int start_enemy_bullet = GetTickCount();
int valid_enemy[ENEMY_NUMBERS];
int valid_die = 0;
int score = 0;

LPDIRECT3DSURFACE9 back_img = NULL;
LPDIRECT3DTEXTURE9 tank_img = NULL;
LPDIRECT3DTEXTURE9 enemy_image = NULL;
LPDIRECT3DTEXTURE9 stone_image = NULL;
LPDIRECT3DTEXTURE9 bullet_image = NULL;
LPDIRECT3DTEXTURE9 explode_image = NULL;
LPDIRECT3DTEXTURE9 die_image = NULL;
LPD3DXSPRITE sprite_obj;
CSound* sound_explode = NULL;
CSound* sound_fire = NULL;

struct SPRITE	// define structure SPRITE at the beginning of MyGame.cpp
{
    int x, y, movex, movey;
    int frame, columns;
    int width, height;
    int startframe, endframe;
    int starttime, delay;
    int direction;
    SPRITE()
    {
        x = y = 0; 			movex = movey = 0;
        frame = 0; columns = 1; 	width = height = 0;
        startframe = endframe = 0;	starttime = delay = 0;
        direction = 1;
    }
};

int Collision(SPRITE tank, SPRITE stonecell)
{
    RECT rect1;
    rect1.left = tank.x + 1;
    rect1.top = tank.y + 1;
    rect1.right = tank.x + tank.width - 1;
    rect1.bottom = tank.y + tank.height - 1;

    RECT rect2;
    rect2.left = stonecell.x + 1;
    rect2.top = stonecell.y + 1;
    rect2.right = stonecell.x + stonecell.width - 1;
    rect2.bottom = stonecell.y + stonecell.height - 1;

    RECT dest;
    return IntersectRect(&dest, &rect1, &rect2);  // if rect1 and rect2 has intersection 
}

SPRITE tank;
SPRITE enemy[ENEMY_NUMBERS];
SPRITE stonecell[cellrows][cellcolumns];
SPRITE bullet[BULLET_NUMBERS];
SPRITE explode[BULLET_NUMBERS];
SPRITE die;

bool LoadTank()
{
    D3DXIMAGE_INFO info;
    HRESULT result;

    //load the tank sprite
    tank_img = LoadTexture("tank.bmp", D3DCOLOR_XRGB(255, 0, 255));
    if (tank_img == NULL)
        return false;
    //set the tank's properties
    result = D3DXGetImageInfoFromFile("tank.bmp", &info);
    if (result != D3D_OK)
        return false;

    tank.width = info.Width;
    tank.height = info.Height / 4;
    tank.x = (SCREENWIDTH - tank.width) / 2;
    tank.y = SCREENHEIGHT - tank.height;
    tank.movex = 0; tank.movey = -1;
    tank.endframe = 0;

    return true;
}

void UpdateTank()
{
    //check for right arrow
    if (Key_Down(DIK_D))
    {
        tank.movex = 1;
        tank.movey = 0;
        tank.frame = 1;
        tank.x += tank.movex; // only move when key pressed
        tank.y += tank.movey;
    }
    //check for up arrow
    else if (Key_Down(DIK_W))
    {
        tank.movex = 0;
        tank.movey = -1;
        tank.frame = 0;
        tank.x += tank.movex;
        tank.y += tank.movey;
    }
    //check for down arrow
    else if (Key_Down(DIK_S))
    {
        tank.movex = 0;
        tank.movey = 1;
        tank.frame = 2;
        tank.x += tank.movex;
        tank.y += tank.movey;
    }
    //check for left arrow
    else if (Key_Down(DIK_A))
    {
        tank.movex = -1;
        tank.movey = 0;
        tank.frame = 3;
        tank.x += tank.movex;
        tank.y += tank.movey;
    }
    if (tank.x < 0)
        tank.x = 0;
    if (tank.x > SCREENWIDTH - tank.width)
        tank.x = SCREENWIDTH - tank.width;
    if (tank.y < 0)
        tank.y = 0;
    if (tank.y > SCREENHEIGHT - tank.height)
        tank.y = SCREENHEIGHT - tank.height;

    for (int i = 0; i < cellrows; i++)
        for (int j = 0; j < cellcolumns; j++)
        {
            // if hit stone, tank backward to its previous position
            if (cell[i][j] == 1 && Collision(tank, stonecell[i][j]))
            {
                //**** something may be wrong here
                tank.x = tank.x - tank.movex;
                tank.y = tank.y - tank.movey;
            }
        }
}

bool LoadEnemy()
{
    D3DXIMAGE_INFO info;
    HRESULT result;

    enemy_image = LoadTexture("enemy.bmp",
        D3DCOLOR_XRGB(255, 0, 255));
    if (enemy_image == NULL)    return false;

    result = D3DXGetImageInfoFromFile("enemy.bmp", &info);
    if (result != D3D_OK)   return false;

    for (int i = 0; i < ENEMY_NUMBERS; i++)
    {
        enemy[i].width = info.Width;
        enemy[i].height = info.Height / 4;
        enemy[i].x = rand() % SCREENWIDTH;
        enemy[i].y = rand() % SCREENHEIGHT;
        enemy[i].endframe = 3;
        switch (rand() % 4)
        {
            // use a random initial direction for enemy tank
        case 0: 	enemy[i].movex = 0;
            enemy[i].movey = -1;
            enemy[i].frame = 0;
            break;
        case 1: 	enemy[i].movex = -1;
            enemy[i].movey = 0;
            enemy[i].frame = 1;
            break;
        case 2: 	enemy[i].movex = 0;
            enemy[i].movey = 1;
            enemy[i].frame = 2;
            break;
        case 3: 	enemy[i].movex = 1;
            enemy[i].movey = 0;
            enemy[i].frame = 3;
            break;
        }
    }
    // initialize all enemy status to be valid
    for (int i = 0; i < ENEMY_NUMBERS; i++)
        valid_enemy[i] = 1;

    for (int i = 0; i < cellrows; i++)
        for (int j = 0; j < cellcolumns; j++)
            for (int k = 0; k < ENEMY_NUMBERS; k++)
            {
                // there should be no stone cells on tank's initial location
                if (Collision(stonecell[i][j], enemy[k]))
                    cell[i][j] = 0;
                if (Collision(stonecell[i][j], tank))
                    cell[i][j] = 0;
            }

    return true;
}

void UpdateEnemy()
{
    for (int i = 0; i < ENEMY_NUMBERS; i++)
    {
        // update enemy tanks positions
        enemy[i].x += enemy[i].movex;   enemy[i].y += enemy[i].movey;
        if (enemy[i].x < 0)
        {
            enemy[i].movex = 1;	enemy[i].frame = 3;
        }
        if (enemy[i].x > SCREENWIDTH - enemy[i].width)
        {
            enemy[i].movex = -1;	enemy[i].frame = 1;
        }
        if (enemy[i].y < 0)
        {
            enemy[i].movey = 1;	enemy[i].frame = 2;
        }
        if (enemy[i].y > SCREENHEIGHT - enemy[i].height)
        {
            enemy[i].movey = -1;	enemy[i].frame = 0;
        }
        for (int j = 0; j < cellrows; j++)
            for (int k = 0; k < cellcolumns; k++)
            {
                // if hit stone, take another direction
                if (cell[j][k] == 1 && Collision(enemy[i], stonecell[j][k]))
                {
                    enemy[i].x -= enemy[i].movex;   enemy[i].y -= enemy[i].movey;
                    switch (rand() % 4)
                    {
                    case 0: enemy[i].movex = 0;  enemy[i].movey = -1;
                        enemy[i].frame = 0;  break;
                    case 1: enemy[i].movex = -1; enemy[i].movey = 0;
                        enemy[i].frame = 1;  break;
                    case 2: enemy[i].movex = 0;  enemy[i].movey = 1;
                        enemy[i].frame = 2; break;
                    case 3: enemy[i].movex = 1;  enemy[i].movey = 0;
                        enemy[i].frame = 3; break;
                    }
                }
            }
    }
    for (int i = 0; i < ENEMY_NUMBERS; i++)
    {
        bool firebullet = false;
        if (valid_enemy[i])
        {
            // if valid enemy and my tank be in same column cell 
            if (tank.x - tank.width < enemy[i].x && enemy[i].x < tank.x + tank.width)
            {
                // if enemy's direction point to tank
                if (enemy[i].movey == -1 && enemy[i].y > tank.y) firebullet = true;
                if (enemy[i].movey == 1 && enemy[i].y < tank.y)   firebullet = true;
            }
            // if valid enemy and my tank be in same row cell 
            if (tank.y - tank.height < enemy[i].y && enemy[i].y < tank.y + tank.height)
            {
                // if enemy's direction point to tank
                if (enemy[i].movex == -1 && enemy[i].x > tank.x) firebullet = true;
                if (enemy[i].movex == 1 && enemy[i].x < tank.x)   firebullet = true;
            }
            if (firebullet && GetTickCount() - start_enemy_bullet > 500)
            {
                start_enemy_bullet = GetTickCount();
                last_bullet++;   // fire a new bullet
                if (last_bullet >= BULLET_NUMBERS)
                    last_bullet = 0;   // use a circular array
                valid_bullet[last_bullet] = 1;
                bullet[last_bullet].x = enemy[i].x + enemy[i].width / 2;
                bullet[last_bullet].y = enemy[i].y + enemy[i].height / 2;
                bullet[last_bullet].movex = enemy[i].movex * 5;
                bullet[last_bullet].movey = enemy[i].movey * 5;
                bullet[last_bullet].x += enemy[i].movex * enemy[i].width;
                bullet[last_bullet].y += enemy[i].movey * enemy[i].height;
                PlaySound(sound_fire);
            }
        }
    }
}


bool LoadStones()
{
    D3DXIMAGE_INFO info;
    HRESULT result;

    //load the stone image
    stone_image = LoadTexture("stone.bmp", D3DCOLOR_XRGB(255, 0, 255));
    if (stone_image == NULL)
        return false;
    //set the stone's properties
    for (int i = 0; i < cellrows; i++)
        for (int j = 0; j < cellcolumns; j++)
        {
            cell[i][j] = 0;
            stonecell[i][j].width = cellwidth;
            stonecell[i][j].height = cellheight;
            stonecell[i][j].x = j * cellwidth;
            stonecell[i][j].y = i * cellheight;
            stonecell[i][j].movex = 0;
            stonecell[i][j].movey = 0;
        }

    // randomly build 100 stone cells
    for (int i = 0; i < 100; i++)
        cell[rand() % cellrows][rand() % cellcolumns] = 1;

    return true;
}

bool LoadBullets()
{
    D3DXIMAGE_INFO info;
    HRESULT result;

    //load the bullet sprite
    bullet_image = LoadTexture("bullet.bmp",
        D3DCOLOR_XRGB(255, 0, 255));
    if (bullet_image == NULL)      return false;

    //set the bullet's properties
    result = D3DXGetImageInfoFromFile("bullet.bmp", &info);
    if (result != D3D_OK)        return false;;
    for (int i = 0; i < BULLET_NUMBERS; i++)
    {
        bullet[i].width = info.Width;
        bullet[i].height = info.Height;
    }
    // initialize all bullet status to be invalid
    for (int i = 0; i < BULLET_NUMBERS; i++)
        valid_bullet[i] = 0;

    return true;
}

void UpdateBullets()
{
    for (int i = 0; i < BULLET_NUMBERS; i++)
    {
        // update bullets positions
        bullet[i].x += bullet[i].movex;
        bullet[i].y += bullet[i].movey;

        // if bullet move out of screen, that bullet turns to be invalid
        if (bullet[i].x < 0 || bullet[i].x > SCREENWIDTH - bullet[i].width)
            valid_bullet[i] = 0;

        if (bullet[i].y < 0 || bullet[i].y > SCREENHEIGHT - bullet[i].height)
            valid_bullet[i] = 0;
    }

    if (Key_Down(DIK_SPACE) && GetTickCount() - start_bullet > 500)
    {
        start_bullet = GetTickCount();
        last_bullet++;   // fire a new bullet
        if (last_bullet >= BULLET_NUMBERS)
            last_bullet = 0;   // use a circular array
        valid_bullet[last_bullet] = 1;
        bullet[last_bullet].x = tank.x + tank.width / 2;
        bullet[last_bullet].y = tank.y + tank.height / 2;
        bullet[last_bullet].movex = tank.movex * 5;
        bullet[last_bullet].movey = tank.movey * 5;
        bullet[last_bullet].x += tank.movex * tank.width;
        bullet[last_bullet].y += tank.movey * tank.height;
        PlaySound(sound_fire);
    }

    for (int i = 0; i < cellrows; i++)
        for (int j = 0; j < cellcolumns; j++)
            for (int k = 0; k < BULLET_NUMBERS; k++)
            {
                if (cell[i][j] == 1 && valid_bullet[k] &&
                    Collision(stonecell[i][j], bullet[k]))
                {
                    // bullet hits stone and both turns invalid
                    cell[i][j] = 0;
                    valid_bullet[k] = 0;
                    last_explode++;
                    if (last_explode >= BULLET_NUMBERS)
                        last_explode = 0;   // use a circular array

                    valid_explode[last_explode] = 1;					explode[last_explode].x = stonecell[i][j].x;
                    explode[last_explode].y = stonecell[i][j].y;
                    PlaySound(sound_explode);
                }
            }

    for (int i = 0; i < ENEMY_NUMBERS; i++)
        for (int j = 0; j < BULLET_NUMBERS; j++)
        {
            // if valid bullet and valid enemy tank hit together
            if (valid_enemy[i] && valid_bullet[j] &&
                Collision(enemy[i], bullet[j]))
            {
                score++;
                valid_enemy[i] = 0;    // enemy exploded and turns invalid
                valid_bullet[j] = 0;	// bullet hits enemy and turns invalid
                last_explode++;
                if (last_explode >= BULLET_NUMBERS)
                    last_explode = 0;   // use a circular array
                valid_explode[last_explode] = 1;
                explode[last_explode].x = enemy[i].x;
                explode[last_explode].y = enemy[i].y;
                PlaySound(sound_explode);
            }
        }
    for (int i = 0; i < BULLET_NUMBERS; i++)
    {
        // if valid bullet and tank hit together
        if (valid_bullet[i] && Collision(bullet[i], tank) && !valid_die)
        {
            valid_bullet[i] = 0;	valid_die = 1;
            die.x = tank.x; 		die.y = tank.y;
            PlaySound(sound_explode);
        }
    }
}


bool LoadExplosions()
{
    D3DXIMAGE_INFO info;
    HRESULT result;

    //load the explode sprite
    explode_image = LoadTexture("explosion2.bmp",
        D3DCOLOR_XRGB(255, 0, 255));

    if (explode_image == NULL)    return false;
    //set the explode's properties
    result = D3DXGetImageInfoFromFile("explosion2.bmp", &info);
    if (result != D3D_OK)      return false;

    for (int i = 0; i < BULLET_NUMBERS; i++)
    {
        explode[i].width = info.Width / 8;
        explode[i].height = info.Height;
        explode[i].endframe = 7;
        explode[i].columns = 8;
    }

    // initialize all explode status to be invalid
    for (int i = 0; i < BULLET_NUMBERS; i++)
        valid_explode[i] = 0;

    //load the my own tank explode sprite
    die_image = LoadTexture("explosion1.bmp",
        D3DCOLOR_XRGB(255, 0, 255));
    if (explode_image == NULL)   return false;

    result = D3DXGetImageInfoFromFile("explosion1.bmp", &info);
    if (result != D3D_OK) 	return NULL;

    die.width = info.Width / 9;
    die.height = info.Height;
    die.endframe = 8;
    die.columns = 9;

    return true;
}

void UpdateExplosions(/*HWND hwnd*/)
{
    for (int i = 0; i < BULLET_NUMBERS; i++)
    {
        // update explision frames
        if (valid_explode[i])
        {
            explode[i].frame++;
            if (explode[i].frame > explode[i].endframe)
                valid_explode[i] = 0;
        }
    }

    if (valid_die)
    {
        die.frame++;
        if (die.frame > die.endframe)
        {
            MessageBox(0, "Game Over", "Game Over", MB_OK);
            gameover = true;
        }
    }

    for (int i = 0; i < BULLET_NUMBERS; i++)
    {
        if (valid_explode[i]) // update explision frames
        {
            explode[i].frame++;
            if (explode[i].frame > explode[i].endframe)
            {
                valid_explode[i] = 0;
                if (score == ENEMY_NUMBERS)
                {
                    MessageBox(0, "You Win", "You Win", MB_OK);
                    gameover = true;
                }
            }
        }
    }
}

int frame = 0;
int starttime = 0;

bool Game_Init(HWND hwnd)
{
    Direct3D_Init(hwnd, SCREENWIDTH, SCREENHEIGHT, false);
    DirectInput_Init(hwnd);
    DirectSound_Init(hwnd);

    back_img = LoadSurface("background.bmp");
    if (!back_img) {
        MessageBox(hwnd, "Error loading back_img", "Error", 0);
        return false;
    }

    tank_img = LoadTexture("tank.bmp", D3DCOLOR_XRGB(255, 0, 255));
    if (!tank_img) {
        MessageBox(hwnd, "Error loading tank_img", "Error", 0);
        return false;
    }

    stone_image = LoadTexture("stone.bmp", D3DCOLOR_XRGB(255, 0, 255));
    if (!stone_image) {
        MessageBox(hwnd, "Error loading stone_image", "Error", 0);
        return false;
    }

    sound_explode = LoadSound("EXPLODE.wav");
    if (sound_explode == NULL) {
        MessageBox(hwnd, "Error loading sound_explode", "Error", 0);
        return false;
    }

    sound_fire = LoadSound("gunfire.wav");
    if (sound_fire == NULL) {
        MessageBox(hwnd, "Error loading sound_fire", "Error", 0);
        return false;
    }

    LoadTank();
    LoadStones();
    if (!LoadEnemy())   return 0;
    if (!LoadBullets())  return 0;
    if (!LoadExplosions())   return 0;

    return true;
}

void Game_Run(HWND hwnd)
{
    if (!d3ddev) return;
    DirectInput_Update();

    UpdateTank();
    UpdateBullets();
    UpdateExplosions();
    UpdateEnemy();

    //start rendering
    if (d3ddev->BeginScene())
    {
        //draw the background
        DrawSurface(backbuffer, 0, 0, back_img);

        //start sprite handler
        sprite_obj->Begin(D3DXSPRITE_ALPHABLEND);
        Sprite_Draw_Frame(tank_img, tank.x, tank.y, tank.frame, tank.width, tank.height, tank.columns);

        for (int i = 0; i < cellrows; i++)
            for (int j = 0; j < cellcolumns; j++)
                if (cell[i][j] == 1)
                    Sprite_Draw_Frame(stone_image,
                        stonecell[i][j].x, stonecell[i][j].y,
                        stonecell[i][j].frame, stonecell[i][j].width,
                        stonecell[i][j].height, stonecell[i][j].columns);

        for (int i = 0; i < BULLET_NUMBERS; i++)
            if (valid_bullet[i])
                Sprite_Draw_Frame(bullet_image, bullet[i].x, bullet[i].y,
                    bullet[i].frame, bullet[i].width, bullet[i].height,
                    bullet[i].columns);

        for (int i = 0; i < BULLET_NUMBERS; i++)
            if (valid_explode[i])
                Sprite_Draw_Frame(explode_image, explode[i].x,
                    explode[i].y, explode[i].frame, explode[i].width,
                    explode[i].height, explode[i].columns);

        for (int i = 0; i < ENEMY_NUMBERS; i++)
            if (valid_enemy[i])				
                Sprite_Draw_Frame(enemy_image,
                enemy[i].x, enemy[i].y, enemy[i].frame,
                enemy[i].width, enemy[i].height,
                enemy[i].columns);

        if(valid_die)
            Sprite_Draw_Frame(die_image, die.x, die.y,
                die.frame, die.width, die.height, die.columns);

        //stop drawing
        sprite_obj->End();

        //stop rendering
        d3ddev->EndScene();
    }
    d3ddev->Present(NULL, NULL, NULL, NULL);
    if (Key_Down(DIK_ESCAPE)) 	//escape key exits
        gameover = true;
}

void Game_End()
{
    if (back_img) back_img->Release();
    if (tank_img) tank_img->Release();
    if (stone_image) stone_image->Release();
    if (bullet_image) bullet_image->Release();
    if (explode_image) explode_image->Release();
    if (enemy_image) enemy_image->Release();
    if (die_image) die_image->Release();
    if (sound_explode != NULL)	delete sound_explode;
    if (sound_fire != NULL)	delete sound_fire;

    DirectInput_Shutdown();
    Direct3D_Shutdown();
    DirectSound_Shutdown();
}