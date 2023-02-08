//=============================================================================
//
//  Утилита для изменения цвета подсветки клавиш клавиатуры Logitech G213
//
//  Главный запускаемый модуль утилиты
//
//
//    Автор: Б.Гарбузов, 2022-2023
//
//
//=============================================================================


#include <windows.h>
#include "colorer.h"
#include "keyboard.h"
#include "..\..\KbdLayoutMonitor\debug.h"


/// <summary>
/// Отправляет клавиатуре Logitech G213 USB-команду смены подсветки на заданный цвет.
/// </summary>
/// <param name="r">Красный.</param>
/// <param name="g">Зеленый.</param>
/// <param name="b">Синий.</param>
void SetKeyboardColor (byte r, byte g, byte b)
{
  KEYBOARDCOMMAND* keyboard;
  HANDLE hDevice = DetectKeyboard(&keyboard);
  if (hDevice == INVALID_HANDLE_VALUE)
  {
#ifdef _DEBUG
    DumpMessage(L"No supported keyboard");
#endif
  }
  else
  {
    char* command = PrepareCommand(keyboard->command, r, g, b);
    if (command == NULL)
    {
#ifdef _DEBUG
      DumpMessage(L"PrepareCommand failed");
#endif
    }
    else
    {
#ifdef _DEBUG
      DumpMessage(L"Send command %s to keyboard %s %s", command, keyboard->vid, keyboard->pid);
#endif
      SendCommand(hDevice, command);
      free(command);
    }
  }
} // SetKeyboardColor


/// <summary>
/// Точка входа.
/// </summary>
/// <param name="hInstance">Хэндл текущего запущенного экземпляра приложения.</param>
/// <param name="hPrevInstance">NULL</param>
/// <param name="lpCmdLine">Параметры командной строки.</param>
/// <param name="nCmdShow">SW_SHOW</param>
/// <returns>0</returns>
int APIENTRY wWinMain (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  if (DebugInstanceHandle == 0) DebugInstanceHandle = hInstance;
#ifdef _DEBUG
  DumpMessage(L"Started KbdColorer %s", lpCmdLine);
/*
  HANDLE hFile = NULL;
  DWORD dwPos = 0;
  TCHAR* buf = NULL;
  SYSTEMTIME tm;
  buf = new TCHAR[1024];
  GetLocalTime(&tm);
  hFile = CreateFile(TEXT("C:\\KbdColorerDump.txt"), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  dwPos = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
  DWORD dwBytes = wsprintf(buf, L"%02d:%02d:%02d.%03d KbdColorer started\r\n", tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds)*2;
  DWORD dwBytesWritten;
  LockFile(hFile, dwPos, 0, dwBytes, 0);
  WriteFile(hFile, buf, dwBytes, &dwBytesWritten, NULL);
  UnlockFile(hFile, dwPos, 0, dwBytes, 0);
  CloseHandle(hFile);
*/
#endif
  int result = -1;
  COLOREDLAYOUT* coloredLayout = SupportedLayouts;
  while (coloredLayout->layout != NULL)
  {
    if (FindSubstring(lpCmdLine, coloredLayout->layout))
    {
      // Нашли цвет для указанной в командной строке раскладки клавиатуры.
      SetKeyboardColor(coloredLayout->red, coloredLayout->green, coloredLayout->blue);
      result = 0;
      break;
    }
    coloredLayout++;
  }
  return result;
} // wWinMain


