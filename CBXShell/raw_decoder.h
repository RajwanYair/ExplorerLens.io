#ifndef _RAW_DECODER_H_
#define _RAW_DECODER_H_

#include <windows.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <atlstr.h>
#include <string>
#include <vector>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs {

// RAW Decoder using LibRaw for professional camera RAW formats
class RAWDecoder
{
public:
	// Get dimensions from RAW file (fast, metadata only)
	static bool GetDimensions(const BYTE* data, size_t size, int* width, int* height);

	// Decode RAW to HBITMAP for thumbnail display
	static HRESULT DecodeToHBITMAP(
		const BYTE* data,
		size_t size,
		HBITMAP* phBitmap,
		int maxWidth = 256,
		int maxHeight = 256);

	// Get camera metadata (make, model)
	static bool GetCameraInfo(
		const BYTE* data,
		size_t size,
		std::wstring& cameraMake,
		std::wstring& cameraModel);

private:
	// Helper: Decode embedded JPEG thumbnail
	static HBITMAP DecodeJPEGThumbnail(const BYTE* jpegData, size_t jpegSize);

	// Helper: Convert LibRaw bitmap to HBITMAP
	static HBITMAP ConvertBitmapToHBITMAP(
		const BYTE* pixels,
		int width,
		int height,
		int colors,
		int bits);
};

} // namespace DarkThumbs

// Legacy namespace for compatibility
namespace RawDecoder
{
	// RAW format detection by extension and magic bytes
	inline bool IsRAWFormat(const wchar_t* ext, const BYTE* data, size_t dataSize)
	{
		if (!ext) return false;

		// Check common RAW extensions
		CString extension(ext);
		extension.MakeLower();

		// Adobe DNG (Digital Negative)
		if (extension == L".dng")
		{
			// DNG is TIFF-based, check for TIFF header
			if (dataSize >= 4)
			{
				// Little-endian TIFF: 0x49 0x49 0x2A 0x00
				// Big-endian TIFF: 0x4D 0x4D 0x00 0x2A
				if ((data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00) ||
					(data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A))
				{
					return true;
				}
			}
			return true; // Trust extension if no data to verify
		}

		// Canon RAW formats
		if (extension == L".cr2" || extension == L".cr3" || extension == L".crw")
		{
			// CR2 is TIFF-based (same header as DNG)
			// CR3 is ISO Base Media Format (similar to MP4)
			if (dataSize >= 4)
			{
				// CR2: TIFF header
				if ((data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00) ||
					(data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A))
				{
					return true;
				}
				// CR3: "ftypcrx " signature at offset 4
				if (dataSize >= 12 && data[4] == 'f' && data[5] == 't' && 
					data[6] == 'y' && data[7] == 'p')
				{
					return true;
				}
			}
			return true;
		}

		// Nikon RAW formats
		if (extension == L".nef" || extension == L".nrw")
		{
			// NEF is TIFF-based with Nikon-specific tags
			if (dataSize >= 4)
			{
				if ((data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00) ||
					(data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A))
				{
					return true;
				}
			}
			return true;
		}

		// Sony RAW formats
		if (extension == L".arw" || extension == L".srf" || extension == L".sr2")
		{
			// ARW is TIFF-based
			if (dataSize >= 4)
			{
				if ((data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00) ||
					(data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A))
				{
					return true;
				}
			}
			return true;
		}

		// Other RAW formats
		if (extension == L".orf" ||  // Olympus
			extension == L".rw2" ||  // Panasonic
			extension == L".pef" ||  // Pentax
			extension == L".raf" ||  // Fujifilm
			extension == L".dcr" ||  // Kodak
			extension == L".mrw" ||  // Minolta
			extension == L".x3f")    // Sigma
		{
			return true;
		}

		return false;
	}

	// Check if Windows Camera Codec Pack is installed
	// This is required for RAW format support in WIC
	inline bool IsCameraCodecPackInstalled()
	{
		// Try to create a WIC factory and check for RAW codec support
		CComPtr<IWICImagingFactory> pFactory;
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pFactory));

		if (FAILED(hr) || !pFactory)
			return false;

		// Query for available decoders
		CComPtr<IEnumUnknown> pEnum;
		hr = pFactory->CreateComponentEnumerator(WICDecoder, WICComponentEnumerateDefault, &pEnum);
		if (FAILED(hr))
			return false;

		// Check if any RAW codec is available
		// The Camera Codec Pack adds decoders for DNG, CR2, NEF, ARW, etc.
		// We'll check by trying to create a decoder for a known RAW GUID
		
		// Note: Windows 10+ includes native DNG support
		// For other formats, Camera Codec Pack or manufacturer codecs are needed
		
		// Check registry for Camera Codec Pack installation
		HKEY hKey;
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{ED9C5406-AAED-43F0-B0D6-C0EAE00F7D42}",
			0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return true; // Camera Codec Pack is installed
		}

		// Check for Windows 10+ native RAW support (DNG codec)
		// GUID_ContainerFormatDng = {E51B51BC-E6A5-4F57-8F4A-BD8F9F4B8D8C}
		CComPtr<IWICBitmapDecoder> pDecoder;
		hr = pFactory->CreateDecoderFromFilename(
			L"test.dng",  // Dummy filename to test decoder availability
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&pDecoder);

		// If we can create a decoder (even for non-existent file), codec is available
		// WINCODEC_ERR_COMPONENTNOTFOUND means no codec
		// Other errors (like file not found) mean codec exists
		if (hr != WINCODEC_ERR_COMPONENTNOTFOUND)
		{
			return true; // Some RAW codec support is available
		}

		return false;
	}

	// Decode RAW file to HBITMAP using Windows Imaging Component (WIC)
	// Requires Windows Camera Codec Pack or manufacturer-specific codecs
	inline HBITMAP DecodeToHBITMAP(IStream* pStream, UINT thumbnailSize)
	{
		if (!pStream)
			return NULL;

		HBITMAP hBitmap = NULL;
		CComPtr<IWICImagingFactory> pFactory;
		CComPtr<IWICBitmapDecoder> pDecoder;
		CComPtr<IWICBitmapFrameDecode> pFrame;
		CComPtr<IWICFormatConverter> pConverter;

		// Create WIC factory
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
			return NULL;


	// Create decoder from stream
	hr = pFactory->CreateDecoderFromStream(pStream, NULL,
		WICDecodeMetadataCacheOnDemand, &pDecoder);
	if (FAILED(hr))
		return NULL;

	// Try to get embedded preview/thumbnail for better performance (on decoder, not frame)
	CComPtr<IWICBitmapSource> pPreview;
	hr = pDecoder->GetPreview(&pPreview);
	if (SUCCEEDED(hr) && pPreview)
	{
		// Use embedded preview if available (faster than decoding full RAW)
		CComPtr<IWICFormatConverter> pPreviewConverter;
		hr = pFactory->CreateFormatConverter(&pPreviewConverter);
		if (SUCCEEDED(hr))
		{
			hr = pPreviewConverter->Initialize(pPreview,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL, 0.0, WICBitmapPaletteTypeCustom);
			if (SUCCEEDED(hr))
			{
				pConverter = pPreviewConverter;
			}
		}
	}

	// If no preview, get first frame
	if (!pConverter)
	{
		hr = pDecoder->GetFrame(0, &pFrame);
		if (FAILED(hr))
			return NULL;
	}

	// Create format converter to 32bpp BGRA
	if (!pConverter)
	{
		hr = pFactory->CreateFormatConverter(&pConverter);
		if (FAILED(hr))
			return NULL;

		hr = pConverter->Initialize(pFrame,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL, 0.0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr))
			return NULL;
	}		// Get image dimensions
		UINT width, height;
		hr = pConverter->GetSize(&width, &height);
		if (FAILED(hr))
			return NULL;

		// Create scaled thumbnail if image is larger than requested size
		CComPtr<IWICBitmapScaler> pScaler;
		UINT scaledWidth = width;
		UINT scaledHeight = height;

		if (width > thumbnailSize || height > thumbnailSize)
		{
			// Calculate scaled dimensions (maintain aspect ratio)
			double scale = (double)thumbnailSize / max(width, height);
			scaledWidth = (UINT)(width * scale);
			scaledHeight = (UINT)(height * scale);

			hr = pFactory->CreateBitmapScaler(&pScaler);
			if (SUCCEEDED(hr))
			{
				hr = pScaler->Initialize(pConverter, scaledWidth, scaledHeight,
					WICBitmapInterpolationModeFant);
				if (FAILED(hr))
				{
					// Scaling failed, use original
					scaledWidth = width;
					scaledHeight = height;
				}
			}
		}

		// Create HBITMAP from WIC bitmap
		CComPtr<IWICBitmapSource> pFinalSource = pScaler ? (IWICBitmapSource*)pScaler : (IWICBitmapSource*)pConverter;

		UINT stride = scaledWidth * 4; // 4 bytes per pixel (32bpp BGRA)
		UINT bufferSize = stride * scaledHeight;
		BYTE* pBuffer = new BYTE[bufferSize];

		hr = pFinalSource->CopyPixels(NULL, stride, bufferSize, pBuffer);
		if (SUCCEEDED(hr))
		{
			// Create DIB (Device Independent Bitmap)
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = scaledWidth;
			bmi.bmiHeader.biHeight = -(LONG)scaledHeight; // Negative for top-down DIB
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;

			HDC hdc = GetDC(NULL);
			void* pBits = NULL;
			hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
			if (hBitmap && pBits)
			{
				memcpy(pBits, pBuffer, bufferSize);
			}
			ReleaseDC(NULL, hdc);
		}

		delete[] pBuffer;
		return hBitmap;
	}

	// Get RAW format name for display
	inline CString GetFormatName(const wchar_t* ext)
	{
		if (!ext) return L"RAW Photo";

		CString extension(ext);
		extension.MakeLower();

		if (extension == L".dng") return L"Adobe DNG (Digital Negative)";
		if (extension == L".cr2") return L"Canon CR2 RAW";
		if (extension == L".cr3") return L"Canon CR3 RAW";
		if (extension == L".crw") return L"Canon CRW RAW";
		if (extension == L".nef") return L"Nikon NEF RAW";
		if (extension == L".nrw") return L"Nikon NRW RAW";
		if (extension == L".arw") return L"Sony ARW RAW";
		if (extension == L".srf") return L"Sony SRF RAW";
		if (extension == L".sr2") return L"Sony SR2 RAW";
		if (extension == L".orf") return L"Olympus ORF RAW";
		if (extension == L".rw2") return L"Panasonic RW2 RAW";
		if (extension == L".pef") return L"Pentax PEF RAW";
		if (extension == L".raf") return L"Fujifilm RAF RAW";
		if (extension == L".dcr") return L"Kodak DCR RAW";
		if (extension == L".mrw") return L"Minolta MRW RAW";
		if (extension == L".x3f") return L"Sigma X3F RAW";

		return L"RAW Photo";
	}
}

#endif // _RAW_DECODER_H_
