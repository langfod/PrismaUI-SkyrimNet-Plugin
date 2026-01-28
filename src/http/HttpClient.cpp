#include "pch.h"
#include "HttpClient.h"
#include <windows.h>
#include <winhttp.h>
#include <mutex>

#pragma comment(lib, "winhttp.lib")

namespace SkyrimNetUI {
namespace Http {

static std::mutex httpMutex;

std::string Get(const std::wstring& url) {
  std::lock_guard<std::mutex> lock(httpMutex);
  
  HINTERNET hSession = WinHttpOpen(L"SkyrimNet/1.0",
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) {
    DWORD error = GetLastError();
    logger::error("GET WinHttpOpen failed with error code: {} (0x{:X})", error, error);
    return "";
  }

  URL_COMPONENTS urlComp = { 0 };
  urlComp.dwStructSize = sizeof(urlComp);
  wchar_t hostName[256] = { 0 };
  wchar_t urlPath[1024] = { 0 };
  urlComp.lpszHostName = hostName;
  urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
  urlComp.lpszUrlPath = urlPath;
  urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

  if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp)) {
    logger::warn("WinHttpCrackUrl failed: {}", GetLastError());
    WinHttpCloseHandle(hSession);
    return "";
  }

  HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName,
                                       urlComp.nPort, 0);
  if (!hConnect) {
    logger::warn("WinHttpConnect failed: {}", GetLastError());
    WinHttpCloseHandle(hSession);
    return "";
  }

  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath,
                                           NULL, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           0);
  if (!hRequest) {
    logger::warn("WinHttpOpenRequest failed: {}", GetLastError());
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return "";
  }

  BOOL bResults = WinHttpSendRequest(hRequest,
                                     WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                     WINHTTP_NO_REQUEST_DATA, 0,
                                     0, 0);

  std::string response;
  if (bResults) {
    bResults = WinHttpReceiveResponse(hRequest, NULL);
  }

  if (bResults) {
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    do {
      dwSize = 0;
      if (WinHttpQueryDataAvailable(hRequest, &dwSize)) {
        char* pszOutBuffer = new char[dwSize + 1];
        ZeroMemory(pszOutBuffer, dwSize + 1);

        if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
          response.append(pszOutBuffer, dwDownloaded);
        }
        delete[] pszOutBuffer;
      }
    } while (dwSize > 0);
  }

  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);

  return response;
}

// Helper to convert wstring to string for logging (ASCII-safe URLs only)
static std::string WideToNarrow(const std::wstring& wide) {
  if (wide.empty()) return {};
  int size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
  std::string result(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), result.data(), size, nullptr, nullptr);
  return result;
}

bool Post(const std::wstring& url, const std::string& jsonData) {
  std::lock_guard<std::mutex> lock(httpMutex);
  
  logger::info("POST Request - URL: {}", WideToNarrow(url));
  logger::info("POST Request - Payload: {}", jsonData);
  
  HINTERNET hSession = WinHttpOpen(L"SkyrimNet/1.0",
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) {
    DWORD error = GetLastError();
    logger::error("WinHttpOpen failed with error code: {} (0x{:X})", error, error);
    return false;
  }

  URL_COMPONENTS urlComp = { 0 };
  urlComp.dwStructSize = sizeof(urlComp);
  wchar_t hostName[256] = { 0 };
  wchar_t urlPath[1024] = { 0 };
  urlComp.lpszHostName = hostName;
  urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
  urlComp.lpszUrlPath = urlPath;
  urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

  if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp)) {
    DWORD error = GetLastError();
    logger::error("WinHttpCrackUrl failed with error code: {} (0x{:X})", error, error);
    WinHttpCloseHandle(hSession);
    return false;
  }

  HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName,
                                       urlComp.nPort, 0);
  if (!hConnect) {
    DWORD error = GetLastError();
    logger::error("WinHttpConnect failed with error code: {} (0x{:X})", error, error);
    WinHttpCloseHandle(hSession);
    return false;
  }

  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", urlComp.lpszUrlPath,
                                           NULL, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           0);
  if (!hRequest) {
    DWORD error = GetLastError();
    logger::error("WinHttpOpenRequest failed with error code: {} (0x{:X})", error, error);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return false;
  }

  std::wstring headers = L"Content-Type: application/json\r\n";
  
  BOOL bResults = WinHttpSendRequest(hRequest,
                                     headers.c_str(), (DWORD)headers.length(),
                                     (LPVOID)jsonData.c_str(), (DWORD)jsonData.length(),
                                     (DWORD)jsonData.length(), 0);

  if (!bResults) {
    DWORD error = GetLastError();
    logger::error("WinHttpSendRequest failed with error code: {} (0x{:X})", error, error);
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return false;
  }

  bool success = false;
  bResults = WinHttpReceiveResponse(hRequest, NULL);
  if (!bResults) {
    DWORD error = GetLastError();
    logger::error("WinHttpReceiveResponse failed with error code: {} (0x{:X})", error, error);
  } else {
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX,
                        &statusCode, &statusCodeSize,
                        WINHTTP_NO_HEADER_INDEX);
    
    logger::info("POST request status code: {}", statusCode);
    
    // Read response body for error details
    if (statusCode >= 400) {
      std::string responseBody;
      DWORD dwSize = 0;
      DWORD dwDownloaded = 0;
      do {
        dwSize = 0;
        if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
          char* pszOutBuffer = new char[dwSize + 1];
          ZeroMemory(pszOutBuffer, dwSize + 1);
          if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
            responseBody.append(pszOutBuffer, dwDownloaded);
          }
          delete[] pszOutBuffer;
        }
      } while (dwSize > 0);
      
      if (!responseBody.empty()) {
        logger::error("Server error response: {}", responseBody);
      }
    }
    
    success = (statusCode >= 200 && statusCode < 300);
  }

  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);

  return success;
}

} // namespace Http
} // namespace SkyrimNetUI
