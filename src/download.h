#pragma once

#include <Windows.h>
#include <Urlmon.h>   // URLOpenBlockingStreamW()
#include <atlbase.h>  // CComPtr

#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <array>

#include "file_tools.h"

#include <WinInet.h>

#pragma comment(lib, "wininet")
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

	__forceinline std::string replace_all(std::string str, const std::string& search, const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = str.find(search, pos)) != std::string::npos)
			str.replace(pos, search.length(), replace), pos += search.length();
		return str;
	}

	inline bool download_to_string(std::string& out, const std::string& url)
	{
		file_tools::log_to_file("Downloading to string " + url);
		
		constexpr auto to_search = "|n"; constexpr auto to_replace = "\r\n";
		const HINTERNET internet_handle = InternetOpen("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
		if (internet_handle)
		{
			HINTERNET url_handle = InternetOpenUrl(internet_handle, url.c_str(), NULL, NULL, NULL, NULL);
			if (url_handle)
			{
				std::array<char, 2000> buf;
				DWORD read_size;
				do
				{
					InternetReadFile(url_handle, buf.data(), 2000, &read_size);
					out.append(buf.data(), read_size);
					buf.fill(0);
				} while (read_size);
				InternetCloseHandle(url_handle);
				InternetCloseHandle(internet_handle);
				out = replace_all(out, to_search, to_replace);
				return true;
			}
			return false;
		}
		InternetCloseHandle(internet_handle);
		return false;
	}

	inline std::string download_to_string(const std::string& url)
	{
		std::string out;
		download_to_string(out, url);
		return out;
	}
	
	inline bool download_to_memory(const std::string& url, std::string& out, float* progress)
	{
		file_tools::log_to_file("Downloading to memory " + url);
		
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
		file_tools::log_to_file("Downloading to file " + url + "  " + file);
		c_internal_callback callback(progress);
		return !FAILED(URLDownloadToFile(NULL, url.c_str(), file.c_str(), 0, &callback));
	}

}
