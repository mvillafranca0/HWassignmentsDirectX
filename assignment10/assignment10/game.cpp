#include "MyDirectX.h"

LPD3DXSPRITE sprite_obj;

//font
LPDIRECT3DTEXTURE9 font;
LPD3DXSPRITE sprite_handler;


struct TRANSFORM
{
    D3DXVECTOR3 position;
    float xrot, yrot, zrot;
    float scale;
};

const string APPTITLE = "Paddle 3D Game";
const int SCREENWIDTH = 640;
const int SCREENHEIGHT = 480;
int block_count = 0;

// One global variable is used to record 
// the number of hit blocks
int score = 0;

#define BALL_SPEED 2
#define PADDLE_SPEED 0.25f

#define LEFT -18             
#define RIGHT 18           
#define BOTTOM -12      
#define TOP 12
#define FRONT 0
#define BACK 60

#define BLOCK_WIDTH 3.9f
#define BLOCK_HEIGHT 2.4f
#define BLOCK_DEPTH 2.4f
#define BLOCK_START_Z 30  // location in z axis
#define BLOCKS_ACROSS 9  // 9 columns
#define BLOCKS_DOWN 10    // 10 rows
#define BLOCKS_DEEP 3       // 3 levels

#define NUM_BLOCKS BLOCKS_ACROSS*BLOCKS_DOWN* BLOCKS_DEEP


//game area
QUAD* back;
QUAD* leftwall;
QUAD* rightwall;
QUAD* bottom;
QUAD* top;

//sound effects
CSound* startgame;
CSound* launch;
CSound* missed;
CSound* hitwall;
CSound* hitblock;
CSound* hitpaddle;

struct OBJ
{
    D3DXVECTOR3 pos;  	// location
    D3DXVECTOR3 dir;   	// movement speed and direction
    int alive;		// whether the object active
};

MODEL* ball_model;	OBJ ball;
MODEL* paddle_model;	OBJ paddle;
MODEL* block_model[NUM_BLOCKS]; OBJ blocks[NUM_BLOCKS];

enum GAMESTATE
{
    PAUSE = 0,
    RUNNING = 1,
    GAMEOVER = 2
};
GAMESTATE state = PAUSE;

#define CAMERA_X 0.0f
#define CAMERA_Y 0.0f
#define CAMERA_Z -21.0f

float RandomBallSpeedX()
{
    //half normal speed for Z
    return (float)(1 + rand() % BALL_SPEED / 2) / 10;
}

float RandomBallSpeedY()
{
    //half normal speed for Z
    return (float)(1 + rand() % BALL_SPEED / 2) / 10;
}

float RandomBallSpeedZ()
{
    return (float)(2 + rand() % BALL_SPEED) / 10.0f;
}

int LoadBall()      // should be called in game_init
{
    ball_model = LoadModel("ball.x");
    if (ball_model == NULL) return 0;
    ball.pos = D3DXVECTOR3((float)(rand() % 20 - 10),
        (float)(rand() % 20 - 10), 0.0f);
    ball.dir = D3DXVECTOR3(RandomBallSpeedX(),
        RandomBallSpeedY(),
        RandomBallSpeedZ());
    return 1;
}

int LoadPaddle() // should be called in game_init
{
    paddle_model = LoadModel("paddle.x");
    if (paddle_model == NULL) return 0;
    paddle.pos = D3DXVECTOR3(0, 0, 3.0f);
    return 1;
}

int LoadSounds()
{
    startgame = LoadSound("startgame.wav");
    launch = LoadSound("launch.wav");
    missed = LoadSound("missed.wav");
    hitwall = LoadSound("hitwall.wav");
    hitblock = LoadSound("hitblock.wav");
    hitpaddle = LoadSound("hitpaddle.wav");

    if (hitwall == NULL || startgame == NULL
        || hitblock == NULL || missed == NULL
        || launch == NULL || hitpaddle == NULL)
        return 0;

    return 1;
}

int LoadFont()
{
    HRESULT result;

    //create the sprite handler
    result = D3DXCreateSprite(d3ddev, &sprite_handler);
    if (result != D3D_OK)  return 0;

    //load the font
    font = LoadTexture("smallfont.bmp", D3DCOLOR_XRGB(0, 0, 0));
    if (font == NULL)    return 0;

    return 1;
}

void MoveBall()
{
    const int BORDER = 2;
    if (state == PAUSE)
    {
        //ball follows paddle when game starts
        ball.pos.x = paddle.pos.x;
        ball.pos.y = paddle.pos.y;
        ball.pos.z = paddle.pos.z + 6;
    }
    else
    {
        //move the ball
        ball.pos.x += ball.dir.x;
        ball.pos.y += ball.dir.y;
        ball.pos.z += ball.dir.z;
        //left and right walls
        if (ball.pos.x < LEFT + BORDER
            || ball.pos.x > RIGHT - BORDER)
        {
            ball.dir.x *= -1;
            ball.pos.x += ball.dir.x;
            PlaySound(hitwall);
        }

        //floor and roof
        if (ball.pos.y < BOTTOM + BORDER
            || ball.pos.y > TOP - BORDER)
        {
            ball.dir.y *= -1;
            ball.pos.y += ball.dir.y;
            PlaySound(hitwall);
        }
        // back wall
        if (ball.pos.z > BACK)
        {
            ball.dir.z *= -1;
            ball.pos.z += ball.dir.z;
            PlaySound(hitwall);
        }

        //did the player miss the ball? must be if this occurs.
        if (ball.pos.z < paddle.pos.z)
        {
            PlaySound(missed);
            ball.dir.z = RandomBallSpeedZ();
            state = PAUSE;       // launch another ball
        }
    }
}

void MovePaddle()
{
    //update paddle x position
    if (Mouse_X() > 0)	     paddle.pos.x += PADDLE_SPEED;
    else if (Mouse_X() < 0)   paddle.pos.x -= PADDLE_SPEED;

    //update paddle y position
    if (Mouse_Y() > 0)           paddle.pos.y -= PADDLE_SPEED;
    else if (Mouse_Y() < 0)   paddle.pos.y += PADDLE_SPEED;

    if (paddle.pos.x < LEFT + 2)
        paddle.pos.x = LEFT + 2;
    else if (paddle.pos.x > RIGHT - 2)
        paddle.pos.x = RIGHT - 2;
    if (paddle.pos.y < BOTTOM + 1)
        paddle.pos.y = BOTTOM + 1;
    else if (paddle.pos.y > TOP - 1)
        paddle.pos.y = TOP - 1;
    //check for collision with ball
    D3DXMATRIX ballMatrix, paddleMatrix;
    D3DXMatrixTranslation(&paddleMatrix, paddle.pos.x, paddle.pos.y, paddle.pos.z);
    D3DXMatrixTranslation(&ballMatrix, ball.pos.x, ball.pos.y, ball.pos.z);
    if (Collided(ball_model, ballMatrix, paddle_model, paddleMatrix))
    {
        PlaySound(hitpaddle);
        ball.dir.z = -1 * ball.dir.z;
    } 
}

void LoadBlocks()
{
    int n = 0;
    for (int z = 0; z < BLOCKS_DEEP; z++)
        for (int y = 0; y < BLOCKS_DOWN; y++)
            for (int x = 0; x < BLOCKS_ACROSS; x++)
            {
                blocks[n].alive = 1;
                if (rand() % 3)    block_model[n] = LoadModel("greenblock.x");
                else if (rand() % 2)  block_model[n] = LoadModel("blueblock.x");
                else                     block_model[n] = LoadModel("redblock.x");

                blocks[n].pos = D3DXVECTOR3(
                    LEFT + BLOCK_WIDTH / 2 + x * BLOCK_WIDTH,
                    TOP - BLOCK_HEIGHT / 2 - y * BLOCK_HEIGHT,
                    BLOCK_START_Z + z * BLOCK_DEPTH);
                n++;
            }
}

void Translate(D3DXVECTOR3 vec)
{
    D3DXMATRIXA16 mat;
    D3DXMatrixTranslation(&mat, vec.x, vec.y, vec.z);
    d3ddev->SetTransform(D3DTS_WORLD, &mat);
}

void DrawBlocks()
{
    int x, y, z, n = 0;
    D3DXMATRIX ballMatrix, blockMatrix;

    //active blocks are rendered as solid
    d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    //draw the active blocks. 
    n = 0;     block_count = 0;
    for (z = 0; z < BLOCKS_DEEP; z++)
        for (y = 0; y < BLOCKS_DOWN; y++)
            for (x = 0; x < BLOCKS_ACROSS; x++)
            {
                //only render the live blocks
                if (blocks[n].alive)
                {
                    //draw the block
                    Translate(blocks[n].pos);
                    DrawModel(block_model[n]);
                    block_count++;
                    D3DXMatrixTranslation(&ballMatrix, ball.pos.x, ball.pos.y, ball.pos.z);
                    D3DXMatrixTranslation(&blockMatrix, blocks[n].pos.x, blocks[n].pos.y,
                        blocks[n].pos.z);
                    if (Collided(ball_model, ballMatrix, block_model[n], blockMatrix))
                    {
                        PlaySound(hitblock);
                        blocks[n].alive = 0; score++;
                        //check the collision position, front or back?
                        //game room deep 60, blocks at position 30
                        if (ball.pos.z < blocks[n].pos.z)           // hit front of block
                            ball.dir.z = -1 * ball.dir.z;                 // bounce back
                        else if (ball.pos.z >= blocks[n].pos.z)  // hit back 
                            ball.dir.z = (float)(5 + rand() % 9) / 10.0f;   // random bounce
                    }
                }
                n++;
            }

    // if all blocks are not active, game over
    if (block_count == 0)
        state = GAMEOVER;
}

void DrawChar(int x, int y, char c, LPDIRECT3DTEXTURE9 lpfont, int cols, int width, int height)
{
    sprite_handler->Begin(D3DXSPRITE_ALPHABLEND);
    //create vector to update sprite position
    D3DXVECTOR3 position((float)x, (float)y, 0);

    //ASCII code of ocrfont.bmp starts with 32 (space)
    int index = c - 32;

    //configure the rect
    RECT srcRect;
    srcRect.left = (index % cols) * width;
    srcRect.top = (index / cols) * height;
    srcRect.right = srcRect.left + width;
    srcRect.bottom = srcRect.top + height;
    //draw the sprite
    sprite_handler->Draw(lpfont, &srcRect, NULL,
        &position, D3DCOLOR_XRGB(255, 255, 255));

    sprite_handler->End();
}

void DrawText(int x, int y, char* text, LPDIRECT3DTEXTURE9 lpfont, int cols, int width, int height)
{
    for (unsigned int n = 0; n < strlen(text); n++)
    {
        DrawChar(x + n * 8, y, text[n], lpfont, cols, width, height);
    }
}

void DisplayStatus()
{
    static char s[80] = "";

    //ball position
    sprintf_s(s, "Ball Pos (%d,%d,%d)", (int)ball.pos.x, (int)ball.pos.y, (int)ball.pos.z);
    DrawText(1, SCREENHEIGHT - 15, s, font, 20, 8, 12);

    //ball direction
    sprintf_s(s, "Ball Dir (%1.2f,%1.2f,%1.2f)", ball.dir.x, ball.dir.y, ball.dir.z);
    DrawText(1, SCREENHEIGHT - 30, s, font, 20, 8, 12);

    //score
    sprintf_s(s, "Score (%d)", score);
    DrawText(SCREENWIDTH - strlen(s) * 8 - 1, 1, s, font, 20, 8, 12);
}

void CreateBoundary()
{
    // back wall
    back = CreateQuad("background.bmp");
    back->vertices[0].x = LEFT;
    back->vertices[0].y = TOP;
    back->vertices[0].z = BACK;
    
    back->vertices[1].x = RIGHT;
    back->vertices[1].y = TOP;
    back->vertices[1].z = BACK;
    
    back->vertices[2].x = LEFT;
    back->vertices[2].y = BOTTOM;
    back->vertices[2].z = BACK;
    
    back->vertices[3].x = RIGHT;
    back->vertices[3].y = BOTTOM;
    back->vertices[3].z = BACK;
    
    // left wall
    leftwall = CreateQuad("background.bmp");
    leftwall->vertices[0].x = LEFT;
    leftwall->vertices[0].y = TOP;
    leftwall->vertices[0].z = FRONT;

    leftwall->vertices[1].x = LEFT;
    leftwall->vertices[1].y = TOP;
    leftwall->vertices[1].z = BACK;

    leftwall->vertices[2].x = LEFT;
    leftwall->vertices[2].y = BOTTOM;
    leftwall->vertices[2].z = FRONT;

    leftwall->vertices[3].x = LEFT;
    leftwall->vertices[3].y = BOTTOM;
    leftwall->vertices[3].z = BACK;

    // right wall
    rightwall = CreateQuad("background.bmp");
    rightwall->vertices[0].x = RIGHT;
    rightwall->vertices[0].y = TOP;
    rightwall->vertices[0].z = BACK;

    rightwall->vertices[1].x = RIGHT;
    rightwall->vertices[1].y = TOP;
    rightwall->vertices[1].z = FRONT;

    rightwall->vertices[2].x = RIGHT;
    rightwall->vertices[2].y = BOTTOM;
    rightwall->vertices[2].z = BACK;

    rightwall->vertices[3].x = RIGHT;
    rightwall->vertices[3].y = BOTTOM;
    rightwall->vertices[3].z = FRONT;

    // top wall
    top = CreateQuad("background.bmp");
    top->vertices[0].x = LEFT;
    top->vertices[0].y = TOP;
    top->vertices[0].z = FRONT;

    top->vertices[1].x = RIGHT;
    top->vertices[1].y = TOP;
    top->vertices[1].z = FRONT;

    top->vertices[2].x = LEFT;
    top->vertices[2].y = TOP;
    top->vertices[2].z = BACK;

    top->vertices[3].x = RIGHT;
    top->vertices[3].y = TOP;
    top->vertices[3].z = BACK;

    // bottom wall
    bottom = CreateQuad("background.bmp");
    bottom->vertices[0].x = LEFT;
    bottom->vertices[0].y = BOTTOM;
    bottom->vertices[0].z = BACK;

    bottom->vertices[1].x = RIGHT;
    bottom->vertices[1].y = BOTTOM;
    bottom->vertices[1].z = BACK;

    bottom->vertices[2].x = LEFT;
    bottom->vertices[2].y = BOTTOM;
    bottom->vertices[2].z = FRONT;

    bottom->vertices[3].x = RIGHT;
    bottom->vertices[3].y = BOTTOM;
    bottom->vertices[3].z = FRONT;

    // use the similar method to create 
    // left wall, right wall, top wall and bottom wall
}


bool Game_Init(HWND hwnd)
{
    Direct3D_Init(hwnd, SCREENWIDTH, SCREENHEIGHT, false);
    DirectInput_Init(hwnd);
    DirectSound_Init(hwnd);

    float ratio = (float)SCREENWIDTH / (float)SCREENHEIGHT;
    SetPerspective(D3DX_PI / 3, ratio, 0.1f, 10000.0f);

    SetCamera(CAMERA_X, CAMERA_Y, CAMERA_Z, paddle.pos.x,
        paddle.pos.y, paddle.pos.z);

    d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
    d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
    d3ddev->SetRenderState(D3DRS_AMBIENT, WHITE);

    //play starting sound while loading
    if (!LoadSounds()) return false;

    PlaySound(startgame);
    CreateBoundary();

    LoadBall();
    LoadPaddle();
    LoadBlocks();
    LoadFont();

    return true;
}

void Game_Run(HWND hwnd)
{
    if (!d3ddev) return;

    DirectInput_Update();

    if (Mouse_Button(0) && state == PAUSE)
        state = RUNNING;	//mouse click starts the ball

    MoveBall();        MovePaddle();
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    if (d3ddev->BeginScene())
    {
        Translate(ball.pos);           DrawModel(ball_model);
        Translate(paddle.pos);       DrawModel(paddle_model);

        d3ddev->EndScene();
    }
    d3ddev->Present(NULL, NULL, NULL, NULL);

    if (d3ddev->BeginScene())
    {
        Translate(ball.pos);           DrawModel(ball_model);
        Translate(paddle.pos);       DrawModel(paddle_model);

        Translate(D3DXVECTOR3(0, 0, 0));
        DrawQuad(back);
        DrawQuad(leftwall);
        DrawQuad(rightwall);
        DrawQuad(bottom);
        DrawQuad(top);

        d3ddev->EndScene();
    }
    d3ddev->Present(NULL, NULL, NULL, NULL);

    if (d3ddev->BeginScene())
    {
        Translate(D3DXVECTOR3(0, 0, 0));
        //draw the blocks
        DrawBlocks();
        DisplayStatus();

        d3ddev->EndScene();
    }
    d3ddev->Present(NULL, NULL, NULL, NULL);
    /*
    if (d3ddev->BeginScene())
    {
        DisplayStatus();
        d3ddev->EndScene();
    }
    d3ddev->Present(NULL, NULL, NULL, NULL);
    */
    if (Key_Down(DIK_ESCAPE))
        gameover = true;
}

void Game_End()
{
    //delete sounds
    if (startgame != NULL) delete startgame;
    if (launch != NULL) delete launch;
    if (missed != NULL) delete missed;
    if (hitwall != NULL) delete hitwall;
    if (hitblock != NULL) delete hitblock;
    if (hitpaddle != NULL) delete hitpaddle;

    if (font != NULL)
        font->Release();

    if (sprite_handler != NULL)
        sprite_handler->Release();

    DeleteQuad(leftwall);
    DeleteQuad(rightwall);
    DeleteQuad(top);
    DeleteQuad(bottom);
    DeleteQuad(back);

    //delete the block models
    for (int n = 0; n < NUM_BLOCKS; n++)
        DeleteModel(block_model[n]);

    //delete models and quads
    DeleteModel(ball_model);
    DeleteModel(paddle_model);

    DirectSound_Shutdown();
    DirectInput_Shutdown();
    Direct3D_Shutdown();
}