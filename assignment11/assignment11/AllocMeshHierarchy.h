#include <vector>
#include <d3dx9.h>

// Implements the ID3DXAllocateHierarchy interface.  In order to create and destroy an animation 
// hierarchy using the D3DXLoadMeshHierarchyFromX and D3DXFrameDestroy functions, we must implement
// the ID3DXAllocateHierarchy interface, which defines how meshes and frames are created and 
// destroyed, thereby giving us some flexibility in the construction and destruction process.

struct FrameEx : public D3DXFRAME
{
	D3DXMATRIX toRoot;
};

struct MeshContainerEx : public D3DXMESHCONTAINER
{
	ID3DXMesh*					exSkinnedMesh;		// The skinned mesh (updated with software skinning).
	std::vector<D3DXMATRIX>		exFinalXForms;		// Final transforms for each frame.
	std::vector<D3DXMATRIX*>	exToRootXFormPtrs;	// To root transform for each frame.
};

class AllocMeshHierarchy : public ID3DXAllocateHierarchy 
{
public:
	HRESULT STDMETHODCALLTYPE CreateFrame(THIS_ PCSTR Name, D3DXFRAME** ppNewFrame);                     

	HRESULT STDMETHODCALLTYPE CreateMeshContainer(PCSTR Name, const D3DXMESHDATA* pMeshData,               
		const D3DXMATERIAL* pMaterials, const D3DXEFFECTINSTANCE* pEffectInstances, DWORD NumMaterials, 
		const DWORD *pAdjacency, ID3DXSkinInfo* pSkinInfo, D3DXMESHCONTAINER** ppNewMeshContainer);     

	HRESULT STDMETHODCALLTYPE DestroyFrame(THIS_ D3DXFRAME* pFrameToFree);              
	HRESULT STDMETHODCALLTYPE DestroyMeshContainer(THIS_ D3DXMESHCONTAINER* pMeshContainerBase);
};
