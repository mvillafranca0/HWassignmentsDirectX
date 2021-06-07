/*
    Beginning Game Programming, Third Edition
    MyDirectX.cpp
*/

#include "MyDirectX.h"

//Direct3D variables
LPDIRECT3D9 d3d = NULL; 
LPDIRECT3DDEVICE9 d3ddev = NULL; 
LPDIRECT3DSURFACE9 backbuffer = NULL;

//DirectInput variables
LPDIRECTINPUT8 dinput = NULL;
LPDIRECTINPUTDEVICE8 dimouse = NULL;
LPDIRECTINPUTDEVICE8 dikeyboard = NULL;
DIMOUSESTATE mouse_state;
char keys[256];

//DirectSound variables
CSoundManager *dsound = NULL;

// Direct3D initialization 
bool Direct3D_Init(HWND hwnd, int width, int height, bool fullscreen)
{
    //initialize Direct3D
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) return false;

    //set Direct3D presentation parameters
    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = (!fullscreen);
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.BackBufferWidth = width;
    d3dpp.BackBufferHeight = height;
    d3dpp.hDeviceWindow = hwnd;

    //create Direct3D device
    d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);

    if (!d3ddev) return false;

    //get a pointer to the back buffer surface
    d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	
	//create sprite handler object
	HRESULT result = D3DXCreateSprite ( d3ddev,	&sprite_obj);
	if (result != D3D_OK)
		return false;

    return true;
}


// Direct3D shutdown
void Direct3D_Shutdown()
{
	if (sprite_obj) sprite_obj->Release();
    if (d3ddev) d3ddev->Release();
    if (d3d) d3d->Release();
}


// Draws a surface to the screen using StretchRect
void DrawSurface(LPDIRECT3DSURFACE9 dest, float x, float y, LPDIRECT3DSURFACE9 source)
{
    //get width/height from source surface
    D3DSURFACE_DESC desc;
    source->GetDesc(&desc);

    //create rects for drawing
    RECT source_rect = {0, 0, (long)desc.Width, (long)desc.Height };
    RECT dest_rect = { (long)x, (long)y, (long)x+desc.Width, (long)y+desc.Height};
    
    //draw the source surface onto the dest
    d3ddev->StretchRect(source, &source_rect, dest, &dest_rect, D3DTEXF_NONE);

}


// Loads a bitmap file into a surface
LPDIRECT3DSURFACE9 LoadSurface(string filename)
{
    LPDIRECT3DSURFACE9 image = NULL;
    
    //get width and height from bitmap file
    D3DXIMAGE_INFO info;
    HRESULT result = D3DXGetImageInfoFromFile(filename.c_str(), &info);
    if (result != D3D_OK)
        return NULL;

    //create surface
    result = d3ddev->CreateOffscreenPlainSurface(
        info.Width,         //width of the surface
        info.Height,        //height of the surface
        D3DFMT_X8R8G8B8,    //surface format
        D3DPOOL_DEFAULT,    //memory pool to use
        &image,             //pointer to the surface
        NULL);              //reserved (always NULL)

    if (result != D3D_OK) return NULL;

    //load surface from file into newly created surface
    result = D3DXLoadSurfaceFromFile(
        image,                  //destination surface
        NULL,                   //destination palette
        NULL,                   //destination rectangle
        filename.c_str(),       //source filename
        NULL,                   //source rectangle
        D3DX_DEFAULT,           //controls how image is filtered
        D3DCOLOR_XRGB(0,0,0),   //for transparency (0 for none)
        NULL);                  //source image info (usually NULL)

    //make sure file was loaded okay
    if (result != D3D_OK) return NULL;

    return image;
}

LPDIRECT3DTEXTURE9 LoadTexture (string filename, D3DCOLOR transcolor)
// transcolor defines the background color and 
// should be transparent when rendering on screen
{
    LPDIRECT3DTEXTURE9 texture = NULL;
    D3DXIMAGE_INFO info;
    HRESULT result;

	  //get width and height from bitmap file
	  result = D3DXGetImageInfoFromFile(filename.c_str(), &info);

	  if (result != D3D_OK)
        return NULL;
	  	result = D3DXCreateTextureFromFileEx (d3ddev,
		filename.c_str(), info.Width, info.Height,         
		1, D3DPOOL_DEFAULT, D3DFMT_UNKNOWN, 
		D3DPOOL_DEFAULT, D3DX_DEFAULT, 
		D3DX_DEFAULT, transcolor, &info, 
		NULL, &texture ); 

	//make sure the texture was loaded correctly
  if (result != D3D_OK)
      return NULL;

  return texture;
}

void Sprite_Draw_Frame(LPDIRECT3DTEXTURE9 texture, int destx, int desty, int framenum, int framew, int frameh, int columns)
{
	D3DXVECTOR3 position( (float)destx, (float)desty, 0 );
	D3DCOLOR white = D3DCOLOR_XRGB(255,255,255);

	RECT rect;
 	rect.left = (framenum % columns) * framew;
	rect.top = (framenum / columns) * frameh;
	rect.right = rect.left + framew;
	rect.bottom = rect.top + frameh;

	sprite_obj->Draw( texture, &rect, NULL, &position, white);
}

void Sprite_Animate(int &frame, int startframe, int endframe, int direction, int &starttime, int delay)
{
	if ((int)GetTickCount() > starttime + delay)
	{
		starttime = GetTickCount();

		frame += direction;
		if (frame > endframe) frame = startframe;
		if (frame < startframe) frame = endframe;
	}	
}

/**
 ** DirectInput initialization
 **/
bool DirectInput_Init(HWND hwnd)
{
    //initialize DirectInput object
    HRESULT result = DirectInput8Create(
        GetModuleHandle(NULL), 
        DIRECTINPUT_VERSION, 
        IID_IDirectInput8,
        (void**)&dinput,
        NULL);

    //initialize the keyboard
    dinput->CreateDevice(GUID_SysKeyboard, &dikeyboard, NULL);
    dikeyboard->SetDataFormat(&c_dfDIKeyboard);
    dikeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    dikeyboard->Acquire();

    //initialize the mouse
    dinput->CreateDevice(GUID_SysMouse, &dimouse, NULL);
    dimouse->SetDataFormat(&c_dfDIMouse);
    dimouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    dimouse->Acquire();
    d3ddev->ShowCursor(false);

    return true;
}

/**
 ** DirectInput update
 **/
void DirectInput_Update()
{
    //update mouse
	dimouse->Acquire();
    dimouse->GetDeviceState(sizeof(mouse_state), (LPVOID)&mouse_state);

    //update keyboard
	dikeyboard->Acquire();
    dikeyboard->GetDeviceState(sizeof(keys), (LPVOID)&keys);
}

/**
 ** Return mouse x movement
 **/
int Mouse_X()
{
    return mouse_state.lX;
}

/**
 ** Return mouse y movement
 **/
int Mouse_Y()
{
    return mouse_state.lY;
}

/**
 ** Return mouse button state
 **/
int Mouse_Button(int button)
{
    return mouse_state.rgbButtons[button] & 0x80;
}

/**
 ** Return key press state
 **/
int Key_Down(int key)
{
    return (keys[key] & 0x80);
}

/**
 ** DirectInput shutdown
 **/
void DirectInput_Shutdown()
{
    if (dikeyboard) 
    {
        dikeyboard->Unacquire();
        dikeyboard->Release();
        dikeyboard = NULL;
    }
    if (dimouse) 
    {
        dimouse->Unacquire();
        dimouse->Release();
        dimouse = NULL;
    }
}

//DirectSound Initialization
bool DirectSound_Init(HWND hwnd)
{
    //create DirectSound manager object
    dsound = new CSoundManager();

    //initialize DirectSound
    HRESULT result;
    result = dsound->Initialize(hwnd, DSSCL_PRIORITY);
    if (result != DS_OK) return false;

    //set the primary buffer format
    result = dsound->SetPrimaryBufferFormat(2, 22050, 16);
    if (result != DS_OK) return false;

    //return success
    return true;
}

void DirectSound_Shutdown()
{
    if (dsound) delete dsound;
}

CSound *LoadSound(string filename)
{
    HRESULT result;

    //create local reference to wave data
    CSound *wave = NULL;

    //attempt to load the wave file
    char s[255];
    sprintf_s(s, "%s", filename.c_str());
    result = dsound->Create(&wave, s);
    if (result != DS_OK) wave = NULL;

    //return the wave
    return wave;
}

void PlaySound(CSound *sound)
{
    sound->Play();
}

void LoopSound(CSound *sound)
{
    sound->Play(0, DSBPLAY_LOOPING);
}

void StopSound(CSound *sound)
{
    sound->Stop();
}
