//=============================================================================
//
//  Утилита для мониторинга за переключениями раскладки клавиатуры
//
//  Главный модуль утилиты.
//  Запускает указанный в командной строке exe-шник с передачей в него идентификатора
//  раскладки клавиатуры, на которую произошло переключение.
//
//
//    Автор: Б.Гарбузов, 2022-2023
//
//
//=============================================================================


#include <windows.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "main.h"
#include "debug.h"
#include "layout.h"
#include "RdpClient.h"


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
  if (_wcsicmp(lpCmdLine, L"/exit") == 0)
  {
    // Сигнализируем ранее запущенному экземпляру приложения о необходимости завершить его работу, а также завершаем данный текущий экземпляр приложения.
    if (AppThreadId == 0)
    {
#ifdef _DEBUG
      DumpMessage(L"Ранее запущенный экземпляр приложения KbdLayoutMonitor отсутствует, завершать нечего.");
#endif
    }
    else
    {
      PostThreadMessage(AppThreadId, WM_QUIT, 0, 0);
    }
  }
  else if (AppThreadId == 0)
  {
    // Это первый запущенный экземпляр приложения.
/* -Вместо специфической MS RDP реализации используем нейросеть для распознавания раскладки клавиатуры в RDP-окне путем сегментации и анализа изображения индикатора
   раскладки клавиатуры в RDP-окне. ТУХЛЯК НАРИСОВАТЬ ОКНО ПОЛНОСТЬЮ БЕЗ ОБРЕЗКИ */
    // Если запущены вне RDP-сессии, т.е. на клиенте, то регистрируем свой RDP Addin для ловли изменений раскладки клавиатуры от RDP-сервера.
    if (!GetSystemMetrics(SM_REMOTESESSION))
    {
      // Запущены на компьютере без RDP-сессии, либо в качестве RDP-клиента.
      RegisterClientRDP();
    }
    else
    {
      // Запущены на компьютере в RDP-сессии.
      RegisterServerRDP();
    }

    // Обрабатываем сообщения главного потока приложения в ожидании сигнала завершения работы.
#ifdef _DEBUG
    DumpMessage(L"Это первый запущенный экземпляр приложения KbdLayoutMonitor. Обрабатываем сообщения в ожидании сигнала завершения...");
#endif
    if (wcslen(lpCmdLine) > 0) LayoutIndicatorApp = lpCmdLine;
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);  // создаем очередь сообщений для текущего потока.
    AppThreadId = GetCurrentThreadId();
    BOOL hasMessage;
    while (true)
    {
      hasMessage = PeekMessage(&msg, (HWND)-1, 0, 0, PM_REMOVE);
      if (hasMessage)
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) break;
        if (msg.message == WM_KEYBOARD_LAYOUT_CHANGED)
        {
          // Отрабатываем сообщение-сигнал о смене раскладки клавиатуры в текущем окне RDP-сессии.
          // Как быть, если локально была русская раскладка и тут мы переключаемся на окно RDP-сессии, в которой раскладка латинская?
          // Было бы неплохо по сообщению-сигналу от KbdLayoutRdpClient записывать полученную из RDP-сессии раскладку клавиатуры куда-то в привязке к хэндлу окна этой RDP-сессии.
          // А при запуске RDP-сессии при открытии канала в KbdLayoutRdpClient каким-то образом принудительно запрашивать от запущенного на серверном конце RDP-сессии
          // KbdLayoutRdpColorer-а текущую раскладку клавиатуры в KdbLayoutMonitor-е, запущенном там же в RDP-сессии.
#ifdef _DEBUG
          DumpMessage(L"KbdLayoutMonitor: Получен сигнал изменения раскладки клавиатуры из текущей RDP-сессии через KbdLayoutRdpClient: 0x%X", msg.lParam);
#endif
          HKL layout = (HKL)msg.lParam;
          RDPWindowLayout = layout;
          if ((layout != 0) && (layout != CurrentLayout))
          {
            CurrentLayout = layout;
            OnLayoutChange();
          }
        }
      }
      Sleep(100);
      // Периодический проверяем текущую раскладку клавиатуры и сигнализируем, если она изменилась.
      CheckKeyboardLayout();
    }
#ifdef _DEBUG
    DumpMessage(L"Получен сигнал завершения работы KbdLayoutMonitor, завершаемся.");
#endif
  }
  else
  {
    // Любой повторно запущенный без ключа /exit экземпляр приложения просто завершаем, т.к. запущен должен быть только один первый и основной экземпляр.
#ifdef _DEBUG
    DumpMessage(L"Данный второй экземпляр приложения KbdLayoutMonitor запущен без ключа /exit, завершаем его.");
#endif
  }
  return 0;
} // wWinMain


/// <summary>
/// Регистрирует, dll-ку для виртуального RDP-канала, через который от KbdLayoutMonitor-а, запущенного на RDP-сервере, будут приходить события изменений раскладки клавиатуры.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-client-registration.
/// Регистрация производится по следующему пути в реестре:
/// [HKEY_CURRENT_USER\Software\Microsoft\Terminal Server Client\Default\Addins\KbdLayoutMonitor]
///   "Name" = "полный путь к лежащей рядом с нами KdbLayoutRdpClient.dll."
/// </summary>
void RegisterClientRDP ()
{
  HKEY hKey;
  LSTATUS status = RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Terminal Server Client\\Default\\AddIns\\KbdLayoutMonitor", &hKey);
  if (status != ERROR_SUCCESS)
  {
#ifdef _DEBUG
    DumpMessage(L"RegCreateKey(HKCU\\Software\\Microsoft\\Terminal Server Client\\Default\\Addins\\KbdLayoutMonitor) failed: 0x%X", GetLastError());
#endif
  }
  else
  {
    DWORD size = MAX_PATH + 1;
    LPWSTR dllPath = (LPWSTR)malloc(size * 2);
    DWORD len = GetFullPathName(L"KdbLayoutRdpClient.dll", size, dllPath, NULL);
    if (len == 0)
    {
#ifdef _DEBUG
      DumpMessage(L"GetCurrentDirectory failed: 0x%X", GetLastError());
#endif
    }
    else
    {
      status = RegSetValueEx(hKey, L"Name", 0, REG_SZ, (byte*)dllPath, (len+1)*sizeof(TCHAR));
      if (status != ERROR_SUCCESS)
      {
#ifdef _DEBUG
        DumpMessage(L"RegSetValueEx failed: 0x%X", status);
#endif
      }
    }
    free(dllPath);
    RegCloseKey(hKey);
  }
} // RegisterClientRDP


/// <summary>
/// Регистрирует серверное приложение виртуального RDP-канала, запускаемое в RDP-сессии на RDP-сервере, и отправлющее на клиента по виртуальному RDP-каналу команды смены цвета подсветки клавиатуры.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-server-application
///              https://learn.microsoft.com/en-us/windows/win32/termserv/monitoring-session-connections-and-disconnections.
/// Регистрация производится по следующему пути в реестре:
/// [HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Terminal Server\Addins\KbdLayoutMonitor]
///   "Name" = "RdpColorer"
///   "Type" = 3
/// </summary>
void RegisterServerRDP ()
{
  HKEY hKey;
  LSTATUS status = RegCreateKey(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Terminal Server\\AddIns\\KbdLayoutMonitor", &hKey);
  if (status != ERROR_SUCCESS)
  {
#ifdef _DEBUG
    DumpMessage(L"RegCreateKey(HKLM\\System\\CurrentControlSet\\Control\\Terminal Server\\AddIns\\KbdLayoutMonitor) failed: 0x%X", GetLastError());
#endif
  }
  else
  {
    const byte* name = (byte*)L"RdpColorer";
    status = RegSetValueEx(hKey, L"Name", 0, REG_SZ, name, (10+1)*sizeof(TCHAR));
    if (status != ERROR_SUCCESS)
    {
#ifdef _DEBUG
      DumpMessage(L"RegSetValueEx(Name) failed: 0x%X", status);
#endif
    }
    else
    {
      DWORD type = 3;
      status = RegSetValueEx(hKey, L"Type", 0, REG_DWORD, (byte*)&type, sizeof(DWORD));
      if (status != ERROR_SUCCESS)
      {
#ifdef _DEBUG
        DumpMessage(L"RegSetValueEx(Type) failed: 0x%X", status);
#endif
      }
    }
    RegCloseKey(hKey);
  }
} // RegisterServerRDP
