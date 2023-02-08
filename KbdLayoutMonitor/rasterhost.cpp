//=============================================================================
//
//  Утилита для мониторинга за переключениями раскладки клавиатуры
//
//  Модуль определения раскладки клавиатуры по растровому изображению RDP-окна
//
//
//    Автор: Б.Гарбузов, 2022-2023
//
//
//=============================================================================


#include "rasterhost.h"
#include "debug.h"


/// <summary>
/// Определяет раскладку клавиатуры на удаленном компьютере, десктоп которого отображается в заданном RDP-окне.
/// Для определения раскладки по растровому изображению в окне пробуем отрисовать его в полный размер в памяти на bitmap-е,
/// вырезаем из получившегося bitmap-а часть с индикатором языка, сравниваем ее с ранее вырезанным куском. Если куски отличаются, то прогоняем свежий кусок bitmap-а
/// через нейросеть классификации на Eng и Rus и выдаем соответствующий код раскладки клавиатуры.
/// </summary>
/// <param name="hwnd">Хэндл окна с растровым изображением декстопа удаленного компьютера, на котоорм присутствует изображение индикатора раскладки клавиатуры.</param>
/// <returns>Возвращает раскладку клавиатуры, распознанную по отображаемому в окне растровому изображению. Если распознать раскладку не удалось, то возвращает 0.</returns>
HKL DetectRasterKeyboardLayout (HWND hwnd)
{
  CaptureWindowRaster(hwnd, L"rdp.bmp");
  return 0;
} // DetectRasterKeyboardLayout


void CaptureWindowRaster2 (HWND hWnd, const wchar_t* fileName)
{
  HDC hDCMem = CreateCompatibleDC(NULL);
  RECT rect;
  GetWindowRect(hWnd, &rect);
  HBITMAP hBmp = NULL;
  HDC hDC = GetDC(hWnd);
  hBmp = CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
  if (!hBmp)
  {
#ifdef _DEBUG
    DumpMessage(L"CreateCompatibleDC failed");
#endif
    ReleaseDC(hWnd, hDC);
  }
  else
  {
    HGDIOBJ hOld = SelectObject(hDCMem, hBmp);
    SendMessage(hWnd, WM_PRINT, (WPARAM)hDCMem, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);
    BITMAP bmpScreen;
    GetObject(hDCMem, sizeof(BITMAP), &bmpScreen);
    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    if (hDIB == NULL)
    {
#ifdef _DEBUG
      DumpMessage(L"GlobalAlloc failed");
#endif
    }
    else
    {
      char* lpbitmap = (char*)GlobalLock(hDIB);
      GetDIBits(hDC, hBmp, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
      DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
      bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
      bmfHeader.bfSize = dwSizeofDIB;
      bmfHeader.bfType = 0x4D42;
      HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile == INVALID_HANDLE_VALUE)
      {
#ifdef _DEBUG
        DumpMessage(L"CreateFile failed");
#endif
      }
      else
      {
        DWORD dwBytesWritten;
        WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
        CloseHandle(hFile);
      }
      GlobalUnlock(hDIB);
      GlobalFree(hDIB);
    }
    DeleteObject(hBmp);
    SelectObject(hDCMem, hOld);
    DeleteObject(hDCMem);
    ReleaseDC(hWnd, hDC);
  }
} // CaptureWindowRaster2


/// <summary>
/// Сохраняет растровое изображение содержимого заданного окна в заданный bmp-файл.
/// </summary>
/// <param name="hWnd">Хэндл окна.</param>
/// <param name="fileName">Имя bmp-файла.</param>
void CaptureWindowRaster (HWND hWnd, const wchar_t* fileName)
{
  HBITMAP hbmScreen { NULL };
  BITMAP bmpScreen;
  DWORD dwBytesWritten = 0;
  DWORD dwSizeofDIB = 0;
  HANDLE hFile = NULL;
  char* lpbitmap = NULL;
  HANDLE hDIB = NULL;
  DWORD dwBmpSize = 0;
  const HDC hdcScreen = GetDC(hWnd);
  const HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
  if (!hdcMemDC)
  {
#ifdef _DEBUG
    DumpMessage(L"CreateCompatibleDC failed");
#endif
    ReleaseDC(hWnd, hdcScreen);
  }
  else
  {
    RECT rcClient;
//    GetClientRect(hWnd, &rcClient);
    rcClient.left = 0;
    rcClient.top = 0;
    rcClient.right = 1920;
    rcClient.bottom = 1080;
    SetStretchBltMode(hdcScreen, HALFTONE);
    hbmScreen = CreateCompatibleBitmap(hdcScreen, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
    if (!hbmScreen)
    {
#ifdef _DEBUG
      DumpMessage(L"CreateCompatibleBitmap failed");
#endif
      ReleaseDC(hWnd, hdcScreen);
    }
    else
    {
      SelectObject(hdcMemDC, hbmScreen);
      SendMessage(hWnd, WM_PAINT, (WPARAM)hdcMemDC, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);
      if (false/*!BitBlt(hdcMemDC, 0, 0, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, hdcScreen, 0, 0, SRCCOPY)*/)
      {
#ifdef _DEBUG
        DumpMessage(L"BitBlt failed");
#endif
        DeleteObject(hdcMemDC);
        DeleteObject(hbmScreen);
        ReleaseDC(hWnd, hdcScreen);
      }
      else
      {
        GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);
        BITMAPFILEHEADER   bmfHeader;
        BITMAPINFOHEADER   bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmpScreen.bmWidth;
        bi.biHeight = bmpScreen.bmHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;
        dwBmpSize = ((bmpScreen.bmWidth*bi.biBitCount+31)/32)*4*bmpScreen.bmHeight;
        hDIB = GlobalAlloc(GHND, dwBmpSize);
        if (hDIB == NULL)
        {
#ifdef _DEBUG
          DumpMessage(L"GlobalAlloc failed");
#endif
        }
        else
        {
          lpbitmap = (char*)GlobalLock(hDIB);
          GetDIBits(hdcScreen, hbmScreen, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
          dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
          bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
          bmfHeader.bfSize = dwSizeofDIB;
          bmfHeader.bfType = 0x4D42;
          hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
          if (hFile == INVALID_HANDLE_VALUE)
          {
#ifdef _DEBUG
            DumpMessage(L"CreateFile failed");
#endif
          }
          else
          {
            WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
            WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
            WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
            CloseHandle(hFile);
          }
          GlobalUnlock(hDIB);
          GlobalFree(hDIB);
        }
        DeleteObject(hbmScreen);
        DeleteObject(hdcMemDC);
        ReleaseDC(hWnd, hdcScreen);
      }
    }
  }
} // CaptureWindowRaster