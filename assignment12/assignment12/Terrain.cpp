#include "MyDirectX.h"

TERRAIN* CreateTerrain(string textureFileName, string heightFileName, float mapScale)
{
	TERRAIN* terrainWorld;
	terrainWorld = (TERRAIN*)malloc(sizeof(TERRAIN));
	if (!terrainWorld)
        return NULL;

	HRESULT result;
	result = D3DXCreateTextureFromFile(d3ddev, textureFileName.c_str(), &terrainWorld->tex);
	if (result != D3D_OK)
		terrainWorld->tex = NULL;

	unsigned char *height=NULL;

	int mapLines = 50;   // how many lines for grid
	int mapCols = 50;    // how many columns for grid
	terrainWorld->numVertRows = mapLines;
	terrainWorld->numVertCols = mapCols;

	if (heightFileName.c_str() != NULL)
	{
		ifstream infile;
		infile.open(heightFileName.c_str(), ios_base::binary);
		if (infile)
		{
	        infile.seekg( 0, std::ios::end );
			mapLines = (int)sqrt((float)infile.tellg()/sizeof(char));
			mapCols = mapLines;
			// due to the limit of number of vertices less than 32767 
			if (mapLines > 181) terrainWorld->numVertRows = 181;
			else terrainWorld->numVertRows = mapLines;
			if (mapCols > 181) terrainWorld->numVertCols = 181;
			else terrainWorld->numVertCols = mapCols;

			height = (unsigned char*)malloc(sizeof(unsigned char)*mapLines*mapCols);
			infile.seekg( 0, std::ios::beg );
			infile.read( (char *)height,mapLines*mapCols*sizeof(char));
			infile.close();
		}	
	}
	
	long numVertices = terrainWorld->numVertRows * terrainWorld->numVertCols;
	int numCellRows = terrainWorld->numVertRows-1;
	int numCellCols = terrainWorld->numVertCols-1;
	int numTris = numCellRows*numCellCols*2;
	
	VERTEX *v;
	v=(VERTEX*)malloc(sizeof(VERTEX)*numVertices);

	int k = 0;
	for(int i = 0; i < terrainWorld->numVertRows; i++)
	{
		for(int j = 0; j < terrainWorld->numVertCols; j++)
		{
			v[k].x=j-terrainWorld->numVertCols/2; 
			v[k].z=i-terrainWorld->numVertRows/2;
			if (height != NULL)
				v[k].y=height[i*mapCols+j]*mapScale;
			else
				v[k].y=0;

			v[k].tv=(float)i/mapLines;
			v[k].tu=(float)j/mapCols;

			k++; // Next vertex
		}
	}

	//===========================================
	// Build indices.

	WORD *indices;
	indices = (WORD *)malloc(sizeof(WORD)*numTris*3);
	 
	// Generate indices for each quad.
	k = 0;
	for(int i = 0; i < numCellRows; i++)
	{
		for(int j = 0; j < numCellCols; j++)
		{
			indices[k]     =   i   * terrainWorld->numVertCols + j;
			indices[k + 1] = (i+1) * terrainWorld->numVertCols + j;					
			indices[k + 2] =   i   * terrainWorld->numVertCols + j + 1;
			
			indices[k + 3] = (i+1) * terrainWorld->numVertCols + j;
			indices[k + 4] = (i+1) * terrainWorld->numVertCols + j + 1;
			indices[k + 5] =   i   * terrainWorld->numVertCols + j + 1;

			// next quad
			k += 6;
		}
	}

	
	for(int k = 0; k < numTris*3; k=k+3)
	{
		D3DXVECTOR3 d1, d2;
		d1 = D3DXVECTOR3( v[indices[k + 1]].x - v[indices[k]].x, v[indices[k + 1]].y - v[indices[k]].y, v[indices[k + 1]].z - v[indices[k]].z );
		d2 = D3DXVECTOR3(v[indices[k + 2]].x - v[indices[k+1]].x, v[indices[k + 2]].y - v[indices[k+1]].y, v[indices[k + 2]].z - v[indices[k+1]].z ) ;
		float crossx, crossy, crossz;
		crossx = d1.y * d2.z   -   d1.z * d2.y;
		crossy = d1.z * d2.x   -   d1.x * d2.z;
		crossz = d1.x * d2.y   -   d1.y * d2.x;
		float dist = sqrt( crossx*crossx + crossy*crossy + crossz*crossz);
		v[indices[k]].nx=crossx/dist;
		v[indices[k]].ny=crossy/dist;
		v[indices[k]].nz=crossz/dist;
		v[indices[k+1]].nx=crossx/dist;
		v[indices[k+1]].ny=crossy/dist;
		v[indices[k+1]].nz=crossz/dist;
		v[indices[k+2]].nx=crossx/dist;
		v[indices[k+2]].ny=crossy/dist;
		v[indices[k+2]].nz=crossz/dist;
	}
	
	D3DXCreateMeshFVF( numTris,numVertices, D3DXMESH_MANAGED, D3DFVF_MYVERTEX, d3ddev, &terrainWorld->terrainMesh );
	if (!terrainWorld->terrainMesh)
        return NULL;

	void *VertexPtr = NULL;
	terrainWorld->terrainMesh->LockVertexBuffer( 0, &VertexPtr );
	memcpy( VertexPtr, v, numVertices*sizeof(VERTEX) );
	terrainWorld->terrainMesh->UnlockVertexBuffer();

	void *IndexPtr;
	terrainWorld->terrainMesh->LockIndexBuffer( 0, &IndexPtr );
	memcpy( IndexPtr, indices, 3*numTris*sizeof(WORD) );
	terrainWorld->terrainMesh->UnlockIndexBuffer();

	DWORD* attBuffer = 0;
	terrainWorld->terrainMesh->LockAttributeBuffer(0, &attBuffer);
	for(int i = 0; i < numTris; i++)
	{
		attBuffer[i] = 0; // Always subset 0
	}
	terrainWorld->terrainMesh->UnlockAttributeBuffer();

	delete v;
	delete indices;

	if (height != NULL)
		free(height);

	return terrainWorld;
}

void DrawTerrain(TERRAIN* terrain)
{
	D3DMATERIAL9 WHITE_MTRL = {WHITE, WHITE, WHITE, BLACK, 2.0f};
	d3ddev->SetMaterial(&WHITE_MTRL);		
	d3ddev->SetTexture( 0, terrain->tex );
	
	d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);  
	d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR); 

	terrain->terrainMesh->DrawSubset(0);
}

void DeleteTerrain(TERRAIN* terrain)
{
	if (terrain != NULL)
	{
		if (terrain->terrainMesh != NULL)
			terrain->terrainMesh->Release();
		if (terrain->tex != NULL)
			terrain->tex->Release();
		free(terrain);
	}
}


float GetHeight(TERRAIN* terrainWorld, float x, float z)
{
	float y;

	if (terrainWorld == NULL)
		return 0;

	x = x+terrainWorld->numVertRows/2;
	z = z+terrainWorld->numVertCols/2;

	// Transform from terrain local space to "cell" space.
	// Get the row and column we are in.
	int row = (int)z;
	int col = (int)x;
	
	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D

	VERTEX* v;
	terrainWorld->terrainMesh->LockVertexBuffer(0,(void**)&v);

	float A = v[row*terrainWorld->numVertCols+col].y;
	float B = v[row*terrainWorld->numVertCols+col+1].y;
	float C = v[(row+1)*terrainWorld->numVertCols+col].y;
	float D = v[(row+1)*terrainWorld->numVertCols+col+1].y;

	// unlock the vertex buffer
	terrainWorld->terrainMesh->UnlockVertexBuffer();

	// Where we are relative to the cell.
	float s = x - (float)col;
	float t = z - (float)row;

	
	// If upper triangle ABC.
	if(t < 1.0f - s)
	{
		float uy = B - A;
		float vy = C - A;
		y = A + s*uy + t*vy;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		y = D + (1.0f-s)*uy + (1.0f-t)*vy;
	}
	return y;
}
