#pragma once

#include <Windows.h>
#include <Urlmon.h>   // URLOpenBlockingStreamW()
#include <atlbase.h>  // CComPtr

#include <iostream>
#include <string>
#include <fstream>
#include <thread>

#pragma comment( lib, "Urlmon.lib" )

namespace download
{
	class c_internal_callback : public IBindStatusCallback
	{
	public:
		float* progress;
		
		c_internal_callback(float* _progress)
		{
			progress = _progress;
		}
		
		STDMETHOD(OnStartBinding)( DWORD dwReserved, IBinding __RPC_FAR* pib)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetPriority)( LONG __RPC_FAR* pnPriority)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnLowResource)(DWORD reserved)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnProgress)(ULONG ulProgress,ULONG ulProgressMax,ULONG ulStatusCode, LPCWSTR wszStatusText);

		STDMETHOD(OnStopBinding)(HRESULT hresult,LPCWSTR szError)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetBindInfo)(DWORD __RPC_FAR* grfBINDF,BINDINFO __RPC_FAR* pbindinfo)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnDataAvailable)(DWORD grfBSCF,DWORD dwSize, FORMATETC __RPC_FAR* pformatetc,STGMEDIUM __RPC_FAR* pstgmed)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnObjectAvailable)(REFIID riid,IUnknown __RPC_FAR* punk)
		{
			return E_NOTIMPL;
		}
		STDMETHOD_(ULONG, AddRef)()
		{
			return 0;
		}

		STDMETHOD_(ULONG, Release)()
		{
			return 0;
		}

		STDMETHOD(QueryInterface)(REFIID riid,void __RPC_FAR* __RPC_FAR* ppvObject)
		{
			return E_NOTIMPL;
		}
	};

	inline HRESULT c_internal_callback::OnProgress(ULONG ulProgress, ULONG ulProgressMax,
		ULONG ulStatusCode, LPCWSTR wszStatusText)
	{
		if (progress && ulProgress >= 0 && ulProgressMax >= 0)
		{
			auto p = ((float)ulProgress - 0.f) / ((float)ulProgressMax - 0.f);
			*progress = p;
		}
		return S_OK;
	}
	
	inline bool download_to_memory(const std::string& url, std::string& out, float* progress)
	{
		c_internal_callback callback(progress);
		
		IStream* stream;

		auto hr = URLOpenBlockingStream(nullptr, url.c_str(), &stream, 0, &callback);

		if (FAILED(hr))
		{
			std::cout << "Could not connect" << std::endl;
			return false;
		}

		char buff[1];
		std::string s;
		unsigned long bytesRead;
		while (true)
		{
			stream->Read(buff, 1, &bytesRead);
			if (0U == bytesRead)
				break;
			s.append(buff, bytesRead);
		}
		stream->Release();

		out = s;
		return true;
	}

	inline bool download_to_file(const std::string& url, const std::string& file, float* progress)
	{
		c_internal_callback callback(progress);
		return !FAILED(URLDownloadToFile(NULL, url.c_str(), file.c_str(), 0, &callback));
	}

}
