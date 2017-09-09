#include "PlatformBase.h"
#include <Unknwnbase.h>
#include <memory>
#include <d3d9.h>
#include <d3d11.h>


#include "../Unity/IUnityGraphics.h"
#include "RecorderFactory.h"
#include "IRecorder.h"
#include "../Common/ComPtrCustom.h"


// --------------------------------------------------------------------------
// UnitySetInterfaces

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;

RecorderFactory g_RecorderFactory;

std::unique_ptr<IRecorder> g_Recorder;

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	// Create graphics API implementation upon initialization
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
	}
	
	// Cleanup graphics API implementation upon shutdown
	if (eventType == kUnityGfxDeviceEventShutdown)
	{

	}
}

extern "C" BSTR UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API getCollectionOfSources()
{
	return g_RecorderFactory.getSourceXML();
}



extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API startCapture(BSTR aSymboliclink, int aStreamIndex, int aMediaTypeIndex, void* texture)
{
	if (g_Recorder)
		g_Recorder.reset();

	g_Recorder.reset(g_RecorderFactory.createRecorder(std::wstring(aSymboliclink)));

	if (g_Recorder)
	{
		g_Recorder->setMediaInfoIndex(aMediaTypeIndex);

		try
		{
			CComPtrCustom<IDirect3DSurface9> lSurface;

			IUnknown* l_ptrBaseUnknown = (IUnknown*)texture;

			auto lresult = l_ptrBaseUnknown->QueryInterface(IID_PPV_ARGS(&lSurface));

			if (FAILED(lresult))
			{
				CComPtrCustom<IDirect3DTexture9> lTexture;

				lresult = l_ptrBaseUnknown->QueryInterface(IID_PPV_ARGS(&lTexture));

				if (SUCCEEDED(lresult))
				{
					lresult = lTexture->GetSurfaceLevel(0, &lSurface);
				}
			}


			if (lSurface)
				g_Recorder->startPreview(lSurface);

		}
		catch (...)
		{

		}

	}

}

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API stopCapture()
{
	if (g_Recorder)
	{
		g_Recorder->closeRecorder();

		g_Recorder.reset();
	}
}