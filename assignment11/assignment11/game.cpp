#include "MyDirectX.h"

LPD3DXSPRITE sprite_obj;

struct TRANSFORM
{
    D3DXVECTOR3 position;
    float xrot, yrot, zrot;
    float scale;
};

const string APPTITLE = "Shooting Game";
const int SCREENWIDTH = 640;
const int SCREENHEIGHT = 480;

D3DMATERIAL9 WHITE_MTRL = { WHITE, WHITE, WHITE, BLACK, 2.0f };

QUAD* ground;
MODEL* carModel, * heliModel, * ballModel;

const int car_number = 10;
bool validCar[car_number];
TRANSFORM cars[car_number], heli, ball;
D3DXVECTOR3 ballDirection(0, 0, 0);
float carScale = 0.5, heliScale = 0.2, ballScale = 0.2;
int score = 0;

float FindModelHeight(MODEL* model)
{
    void* pVertices = NULL;

    D3DXVECTOR3 minBounds, maxBounds;
    model->mesh->LockVertexBuffer(D3DLOCK_READONLY,
        (LPVOID*)&pVertices);
    D3DXComputeBoundingBox((D3DXVECTOR3*)pVertices,
        model->mesh->GetNumVertices(),
        D3DXGetFVFVertexSize(model->mesh->GetFVF()),
        &minBounds, &maxBounds);
    model->mesh->UnlockVertexBuffer();

    return maxBounds.y - minBounds.y;
}


void MoveHeli()
{
    float speed = 0, yrot = 0;
    D3DXMATRIX R;
    D3DXVECTOR3 CameraPosition, HeliDirection, InitHeliDirection(0.0f, 0.0f, -1.0f);

    if (Key_Down(DIK_UP))    speed = 0.1;
    if (Key_Down(DIK_DOWN))    speed = -0.1;
    if (Key_Down(DIK_LEFT))    yrot = -0.01;
    if (Key_Down(DIK_RIGHT))     yrot = 0.01;
    heli.yrot = heli.yrot + yrot;

    D3DXMatrixRotationY(&R, heli.yrot); //set up the camera view matrix
    D3DXVec3TransformCoord(&HeliDirection, &InitHeliDirection, &R);
    heli.position = heli.position + HeliDirection * speed;
    CameraPosition = heli.position - HeliDirection * 4;
    CameraPosition.y = 3;
    SetCamera(CameraPosition.x, CameraPosition.y, CameraPosition.z,
        heli.position.x, heli.position.y, heli.position.z);
}

void MoveBall(HWND hwnd)
{
    D3DXMATRIX carMatrix, ballMatrix, S, R, T;
    if (ballDirection != D3DXVECTOR3(0, 0, 0)) // ball is moving
    {
        ball.position += ballDirection * 0.1;  // continue to move

        D3DXMatrixScaling(&S, ball.scale, ball.scale, ball.scale);
        D3DXMatrixRotationY(&R, ball.yrot);
        D3DXMatrixTranslation(&T, ball.position.x, ball.position.y, ball.position.z);
        ballMatrix = S * R * T;
        for (int i = 0; i < car_number; i++)
        {
            if (validCar[i] == true)
            {
                D3DXMatrixScaling(&S, cars[i].scale, cars[i].scale, cars[i].scale);
                D3DXMatrixRotationY(&R, cars[i].yrot);
                D3DXMatrixTranslation(&T, cars[i].position.x, cars[i].position.y,
                    cars[i].position.z);
                carMatrix = S * R * T;
                if (Collided(carModel, carMatrix, ballModel, ballMatrix))
                {
                    validCar[i] = false;
                    // reset ball if ball collided with 3d model
                    ballDirection = D3DXVECTOR3(0, 0, 0);
                    score++;
                }
            }
        }

        if (ball.position.x < -100 || ball.position.x > 100 || ball.position.z < -100 ||
            ball.position.z > 100 || ball.position.y < 0)
            ballDirection = D3DXVECTOR3(0, 0, 0);  //reset ball if ball is out of boundary
    }
    else  // ball is not moving
    {
        ball.position = heli.position; // ball is not moving, ready to launch a new one
        if (Mouse_Button(0))  // mouse left button clicked, try to select a target
        {
            POINT pos;
            RECT windowRect;
            GetCursorPos(&pos);
            GetWindowRect(hwnd, &windowRect);
            int x = pos.x - windowRect.left;
            int y = pos.y - windowRect.top;
            float distance = -1;

            D3DXVECTOR3 rayOrigin, rayEnd, rayDirection;
            CalculateRay(x, y, rayOrigin, rayDirection);
            for (int i = 0; i < car_number; i++)
            {
                if (validCar[i] == true)
                {
                    D3DXMatrixScaling(&S, cars[i].scale, cars[i].scale, cars[i].scale);
                    D3DXMatrixRotationY(&R, cars[i].yrot);
                    D3DXMatrixTranslation(&T, cars[i].position.x, cars[i].position.y, cars[i].position.z);
                    carMatrix = S * R * T;

                    if (CollidedModelRay(carModel, carMatrix, rayOrigin, rayDirection))
                    {
                        rayEnd = cars[i].position + D3DXVECTOR3(0,
                            carScale * FindModelHeight(carModel) / 2, 0);
                        D3DXVECTOR3 ray = (rayEnd - rayOrigin);
                        if (distance < 0)  // the first model intersected with ray
                        {
                            distance = D3DXVec3Length(&ray);
                            ballDirection = rayEnd - heli.position;
                        }
                        // if more models intersected with ray, select the closest model
                        if (distance > D3DXVec3Length(&ray))
                        {
                            distance = D3DXVec3Length(&ray);
                            ballDirection = rayEnd - heli.position;
                        }
                    }
                    if (ballDirection == D3DXVECTOR3(0, 0, 0))   // if no target selected
                    {
                        rayEnd = rayOrigin;
                        while (rayEnd.x > -100 && rayEnd.x < 100 && rayEnd.z>-100 &&
                            rayEnd.z < 100 && rayEnd.y>0)
                            rayEnd += rayDirection * 0.1;

                        ballDirection = rayEnd - heli.position;
                    }

                    D3DXVec3Normalize(&ballDirection, &ballDirection);
                }
            }
        }
    }
}  

void DrawText(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);

    RECT rt;
    GetClientRect(hwnd, &rt);

    char s[80];
    sprintf_s(s, "score = %d", score);

    SetBkColor(hdc, RGB(0, 0, 0));
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawText(hdc, s, strlen(s), &rt, DT_LEFT);

    ReleaseDC(hwnd, hdc);
}

bool Game_Init(HWND hwnd)
{
    Direct3D_Init(hwnd, SCREENWIDTH, SCREENHEIGHT, false);
    DirectInput_Init(hwnd);
    DirectSound_Init(hwnd);

    float ratio = (float)SCREENWIDTH / (float)SCREENHEIGHT;
    SetPerspective(D3DX_PI / 3, ratio, 0.1f, 10000.0f);

    // set camera

    d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
    d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
    d3ddev->SetRenderState(D3DRS_AMBIENT, WHITE);

    ground = CreateQuad("grassground.bmp");
    ground->vertices[0].x = -100; ground->vertices[0].y = 0;
    ground->vertices[0].z = 100;
    ground->vertices[1].x = 100; ground->vertices[1].y = 0;
    ground->vertices[1].z = 100;  ground->vertices[1].tu = 100;
    ground->vertices[2].x = -100; ground->vertices[2].y = 0;
    ground->vertices[2].z = -100; ground->vertices[2].tv = 100;
    ground->vertices[3].x = 100; ground->vertices[3].y = 0;
    ground->vertices[3].z = -100;
    ground->vertices[3].tu = 100; ground->vertices[3].tv = 100;

    srand(time(NULL));  // need #include <time.h> 
    carModel = LoadModel("car.x");
    for (int i = 0; i < car_number; i++)
    {
        cars[i].position = D3DXVECTOR3(rand() % 100 - 50, 0, rand() % 100 - 50);
        cars[i].yrot = D3DXToRadian(rand() % 360);     cars[i].scale = carScale;
        validCar[i] = true;
    }
    heliModel = LoadModel("heli.x");
    heli.position = D3DXVECTOR3(0, 2, 5);    heli.scale = heliScale;
    ballModel = LoadModel("ball.x");
    ball.position = heli.position;    ball.scale = ballScale;

    return true;
}

void Game_Run(HWND hwnd)
{
    if (!d3ddev) return;
    DirectInput_Update();
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    if (d3ddev->BeginScene())
    {
        D3DXMATRIX matWorld, S, R, T;

        D3DXMatrixTranslation(&matWorld, 0.0f, 0.0f, 0.0f);
        d3ddev->SetTransform(D3DTS_WORLD, &matWorld);
        d3ddev->SetMaterial(&WHITE_MTRL);
        DrawQuad(ground);

        D3DXMatrixScaling(&S, heli.scale, heli.scale, heli.scale);
        D3DXMatrixRotationY(&R, heli.yrot);
        D3DXMatrixTranslation(&T, heli.position.x, heli.position.y, heli.position.z);
        matWorld = S * R * T;
        d3ddev->SetTransform(D3DTS_WORLD, &matWorld);
        DrawModel(heliModel);

        D3DXMatrixScaling(&S, ball.scale, ball.scale, ball.scale);
        D3DXMatrixRotationY(&R, ball.yrot);
        D3DXMatrixTranslation(&T, ball.position.x, ball.position.y, ball.position.z);
        matWorld = S * R * T;
        d3ddev->SetTransform(D3DTS_WORLD, &matWorld);
        DrawModel(ballModel);

        for (int i = 0; i < car_number; i++)
        {
            if (validCar[i] == true)
            {
                D3DXMatrixScaling(&S, cars[i].scale, cars[i].scale, cars[i].scale);
                D3DXMatrixRotationY(&R, cars[i].yrot);
                D3DXMatrixTranslation(&T, cars[i].position.x, cars[i].position.y,
                    cars[i].position.z);
                matWorld = S * R * T;
                d3ddev->SetTransform(D3DTS_WORLD, &matWorld);
                DrawModel(carModel);
            }
        }
        d3ddev->EndScene();
    }
    d3ddev->Present(NULL, NULL, NULL, NULL);
    
    MoveHeli();
    MoveBall(hwnd);

    if (Key_Down(DIK_ESCAPE))
        gameover = true;
}

void Game_End()
{
    DeleteQuad(ground);
    for (int i = 0; i < car_number; i++)
    {
        DeleteModel(carModel);
    }
    DeleteModel(heliModel);
    DeleteModel(ballModel);

    DirectSound_Shutdown();
    DirectInput_Shutdown();
    Direct3D_Shutdown();
}