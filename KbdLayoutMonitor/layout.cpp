//=============================================================================
//
//  Утилита для мониторинга за переключениями раскладки клавиатуры
//
//  Модуль для определения раскладки клавиатуры в активном окне и запуска нужного процесса
//  по событию переключения раскладки.
//
//
//    Автор: Б.Гарбузов, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#include "layout.h"
#include "conhost.h"
#include "rasterhost.h"
#include "debug.h"


/// <summary>
/// Проверяет текущую раскладку клавиатуры и сигнализирует вызовом метода OnLayoutChange, если она изменилась.
/// </summary>
void CheckKeyboardLayout ()
{
  HKL layout = GetCurrentKeyboardLayout();
  if ((layout != 0) && (layout != CurrentLayout))
  {
    CurrentLayout = layout;
    OnLayoutChange();
  }
} // CheckKeyboardLayout


/// <summary>
/// Вызывается при изменении раскладки клавиатуры.
/// </summary>
void OnLayoutChange ()
{
#ifdef _DEBUG
  TCHAR* buf = new TCHAR[1024];
  wsprintf(buf, L"Текущая раскладка: %p", CurrentLayout);
  DumpMessage(buf);
  delete[] buf;
#endif

  if (false/*удаленная RDP-станция*/)
  {
    // Проверить переменную окружения:
    // SESSIONNAME=Console - клиент (с которого подключено RDP)
    // SESSIONNAME=RDP#... - хост (к которому подключено RDP)
    // Для RDP использовать виртуальные каналы RDP для отправки текущей раскладки на хост для подсветки хостовой клавиатуры.
    // !!! Эта часть переедет в отдельное запускаемое приложение-индикатор.
  }

  if (LayoutIndicatorApp != NULL)
  {
    // Запускаем заданный в командной строке процесс на событие смены текущей раскладки клавиатуры.
    RunLayoutIndicatorApp();
  }
} // OnLayoutChange


/// <summary>
/// Возвращает идентификатор текущей раскладки клавиатуры.
/// </summary>
/// <returns>Идентификатор текущей раскладки клавиатуры: 67699721 - латинская раскладка, 68748313 - русская раскладка.</returns>
HKL GetCurrentKeyboardLayout ()
{
  HKL result = 0;
#ifdef _DEBUG
  TCHAR* buf;
#endif
  HWND hwnd = GetForegroundWindow();
  if (hwnd != 0)
  {
    if (CurrentForegroundWindow != hwnd)
    {
      CurrentForegroundWindow = hwnd;
      TCHAR* title = new TCHAR[256];
      GetClassName(CurrentForegroundWindow, title, 256);
      CurrentForegroundWindowIsConsole = (_wcsicmp(title, L"ConsoleWindowClass") == 0);
      CurrentForegroundWindowIsRDP = (_wcsicmp(title, L"TscShellContainerClass") == 0);
#ifdef _DEBUG
      buf = new TCHAR[1024];
      if (CurrentForegroundWindowIsConsole) wsprintf(buf, L"Активное окно консоли: %p %s", CurrentForegroundWindow, title);
        else if (CurrentForegroundWindowIsRDP) wsprintf(buf, L"Активное окно RDP: %p %s", CurrentForegroundWindow, title);
          else wsprintf(buf, L"Активное окно: %p %s", CurrentForegroundWindow, title);
      DumpMessage(buf);
      delete[] buf;
#endif
      delete[] title;
    }
    DWORD idProcess;
    DWORD idThread = GetWindowThreadProcessId(hwnd, &idProcess);
    if (CurrentForegroundWindowIsConsole)
    {
      // Активно консольное окно: для определения раскладки нужно перебрать потоки процесса ConHost.exe, связанного с этим консольным окном (процесс окна является родительским для искомого ConHost.exe).
      DWORD conhostId = FindConhost(idProcess);
      result = FindMessageQueueThreadKeyboardLayout(conhostId);
    }
    else if (CurrentForegroundWindowIsRDP)
    {
      // ЭТО НЕ ВЗЛЕТЕЛО, Т.К. ХРЕН НАРИСУЕШЬ СОДЕРЖИМОЕ RDP-ОКНА В ПОЛНОМ РАЗМЕРЕ НА DC В ПАМЯТИ.
      // --Активно RDP-окно: для определения раскладки на удаленном компьютере, отображаемом в RDP-окне, пробуем отрисовать его в полный размер в памяти на bitmap-е,
      // вырезаем из получившегося bitmap-а часть с индикатором языка, сравниваем ее с ранее вырезанным куском. Если куски отличаются, то прогоняем свежий кусок bitmap-а
      // через нейросеть классификации на Eng и Rus и выдаем соответствующий код раскладки клавиатуры.
//      result = DetectRasterKeyboardLayout(hwnd);
      // Получаем сигналы о смене раскладки клавиатуры в RDP-сессии через сообщение-сигнал WM_KEYBOARD_LAYOUT_CHANGED от KdbLayoutRdpClient-а.
      if (RDPWindowLayout != 0)
      {
        result = RDPWindowLayout;
      }
      else
      {
        result = CurrentLayout;
      }
    }
    if (result == 0)
    {
      // Не удалось определить раскладку клавиатуры в консоли или активна не консоль, а обычное окно: определяем раскладку клавиатуры по потоку этого окна.
      if (CurrentForegroundWindowThreadId != idThread)
      {
        CurrentForegroundWindowThreadId = idThread;
#ifdef _DEBUG
        buf = new TCHAR[1024];
        wsprintf(buf, L"Идентификатор потока активного окна: %d", CurrentForegroundWindowThreadId);
        DumpMessage(buf);
        delete[] buf;
#endif
      }
      result = GetKeyboardLayout(idThread);
    }
  }
  return result;
} // GetCurrentKeyboardLayout


/// <summary>
/// Запускает приложение-индикатор раскладки клавиатуры с передачей ему в командной строке идентификатора текущей раскладки клавиатуры.
/// </summary>
void RunLayoutIndicatorApp ()
{
  TCHAR currentDir [MAX_PATH];
  GetCurrentDirectory(MAX_PATH, currentDir);
  TCHAR* cmdLine = new TCHAR[MAX_PATH];
  wsprintf(cmdLine, L"%s\\%s %p", currentDir, LayoutIndicatorApp, CurrentLayout);
#ifdef _DEBUG
  TCHAR* buf = new TCHAR[1024];
  wsprintf(buf, L"Запускаем приложение-индикатор: %s %p", LayoutIndicatorApp, CurrentLayout);
  DumpMessage(buf);
  delete[] buf;
#endif
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  // Можно перехватить запуск отладчиком: \HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\KbdColorer.exe Debugger:REG_SZ=vsjitdebugger.exe
  if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
#ifdef _DEBUG
    buf = new TCHAR[1024];
    wsprintf(buf, L"Ошибка запуска приложения-индикатора: 0x%X", GetLastError());
    DumpMessage(buf);
    delete[] buf;
#endif
  }
  else
  {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
  delete[] cmdLine;
} // RunLayoutIndicatorApp
