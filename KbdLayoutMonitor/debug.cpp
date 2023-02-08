//=============================================================================
//
//  Утилита для мониторинга за переключениями раскладки клавиатуры
//
//  Модуль для формирования строк отладочных печатей и вывода их в окно отладчика и в лог-файл.
//
//
//    Автор: Б.Гарбузов, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#include <stddef.h>
#include "debug.h"


/// <summary>
/// Хэндл dll-ки или exe-файла, из кода которого вызывается DumpMessage и рядом с которым нужно писать лог-файл.
/// </summary>
HMODULE DebugInstanceHandle = 0;


/// <summary>
/// Печатает сообщение в отладочную консоль VS.
/// </summary>
/// <param name="message">Печатаемое сообщение.</param>
/// <param name="...">Опциональный список значений, подставляемых вместо конструкций с процентами.</param>
void DumpMessage (LPCWSTR message, ...)
{
#ifdef _DEBUG
  TCHAR* msgBuffer = new TCHAR[1024];
  va_list args;
  va_start(args, message);
  wvsprintf(msgBuffer, message, args);
  va_end(args);
  TCHAR* buf = new TCHAR[1024];
  SYSTEMTIME tm;
  GetLocalTime(&tm);
  DWORD dwBytes = wsprintf(buf, L"%02d:%02d:%02d.%03d %s\r\n", tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, msgBuffer)*2;
  OutputDebugString(buf);
  TCHAR* path = new TCHAR[MAX_PATH];
  if (GetModuleFileName(DebugInstanceHandle, path, MAX_PATH))
  {
    TCHAR* lastSlash = wcsrchr(path, '\\');
    if (lastSlash)
    {
      *(lastSlash + 1) = '\0';
      wcscat_s(path, MAX_PATH, DEBUGLOGFILENAME);
    }
    else
    {
      wcscpy_s(path, MAX_PATH, DEBUGLOGFILENAME);
    }
  }
  else
  {
    wcscpy_s(path, MAX_PATH, DEBUGLOGFILENAME);
  }
  HANDLE hFile = CreateFile(path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    DWORD dwPos = SetFilePointer(hFile, 0, NULL, FILE_END);
    DWORD dwBytesWritten;
    LockFile(hFile, dwPos, 0, dwBytes, 0);
    WriteFile(hFile, buf, dwBytes, &dwBytesWritten, NULL);
    UnlockFile(hFile, dwPos, 0, dwBytes, 0);
    CloseHandle(hFile);
  }
  delete[] path;
  delete[] buf;
  delete[] msgBuffer;
#endif
} // DumpMessage
