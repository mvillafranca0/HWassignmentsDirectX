#include "MyDirectX.h"

LPD3DXSPRITE sprite_obj;

const string APPTITLE = "Terrain";
const int SCREENWIDTH = 640;
const int SCREENHEIGHT = 480;
/*
int mapLines = 50;   // how many lines for grid
int mapCols = 50;    // how many columns for grid
int numVertRows = mapLines;
int numVertCols = mapCols;
long numVertices = numVertRows * numVertCols;
int numCellRows = numVertRows - 1;
int numCellCols = numVertCols - 1;
long numTris = numCellRows * numCellCols * 2; */

ID3DXMesh* terrainMesh;

unsigned char* height;
float mapScale = 0.25;

D3DXVECTOR3 CameraPosition(
    0.0f, 100.0f, -1.0f);
D3DXVECTOR3 CameraUpDirection(
    0.0f, 1.0f, 0.0f);
D3DXVECTOR3 CameraDirection(
    0.0f, 0.0f, 1.0f);
D3DXVECTOR3 CameraRightDirection(
    1.0f, 0.0f, 0.0f);
float speed = 0.10;

TERRAIN* terrain;

void UpdateCamera()
{
    //travel along camera direction
    D3DXVECTOR3 dir(0, 0, 0);
    if (Key_Down(DIK_W))
        dir = CameraDirection;
    if (Key_Down(DIK_S))
        dir = -CameraDirection;
    if (Key_Down(DIK_D))
        dir = CameraRightDirection;
    if (Key_Down(DIK_A))
        dir = -CameraRightDirection;

    CameraPosition = CameraPosition + dir * speed;
    CameraPosition.y = GetHeight(terrain, CameraPosition.x,
        CameraPosition.z) + 5;

    float yAngel = Mouse_X() / 100.0;
    float pitch = Mouse_Y() / 100.0;

    D3DXMATRIX R, matView;
    //rotate camera's look and up vectors around the camera's right vector
    D3DXMatrixRotationAxis(&R, &CameraRightDirection, pitch);
    D3DXVec3TransformCoord(&CameraDirection, &CameraDirection, &R);
    D3DXVec3TransformCoord(&CameraUpDirection,
        &CameraUpDirection, &R);
    D3DXVec3TransformCoord(&CameraRightDirection,
        &CameraRightDirection, &R);

    D3DXMatrixRotationY(&R, yAngel);
    D3DXVec3TransformCoord(&CameraRightDirection,
        &CameraRightDirection, &R);
    D3DXVec3TransformCoord(&CameraUpDirection,
        &CameraUpDirection, &R);
    D3DXVec3TransformCoord(&CameraDirection, &CameraDirection, &R);

    //set up the camera view matrix
    D3DXVECTOR3 CameraTarget = CameraPosition + CameraDirection;
    D3DXMatrixLookAtLH(&matView, &CameraPosition,
        &CameraTarget, &CameraUpDirection);
    d3ddev->SetTransform(D3DTS_VIEW, &matView);
}

void DrawText(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);

    RECT rt;
    GetClientRect(hwnd, &rt);

    char s[80];
    sprintf_s(s, "Camera Position = %.2f %.2f %.2f \nCamera Direction = %.2f %.2f % .2f", 
        CameraPosition.x, CameraPosition.y, CameraPosition.z,
        CameraDirection.x, CameraDirection.y, CameraDirection.z);

    SetBkColor(hdc, RGB(0, 0, 0));
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawText(hdc, s, strlen(s), &rt, DT_LEFT);
    //TextOut(hdc, 0, 0, s, strlen(s));

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

    terrain = CreateTerrain ("terrain_base_texture.jpg", "heightMap.raw", mapScale = 1);

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
        DrawTerrain(terrain);

        d3ddev->EndScene();
    }
    d3ddev->Present(NULL, NULL, NULL, NULL);

    UpdateCamera();
    DrawText(hwnd);

    if (Key_Down(DIK_ESCAPE))
        gameover = true;
}

void Game_End()
{
    DeleteTerrain(terrain);

    DirectSound_Shutdown();
    DirectInput_Shutdown();
    Direct3D_Shutdown();
}