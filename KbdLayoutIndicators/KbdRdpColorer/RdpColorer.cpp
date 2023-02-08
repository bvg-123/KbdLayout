//=============================================================================
//
//  Утилита для отправки через RDP-канал сигнала смены раскладки клавиатуры на RDP-хосте
//
//  Главный запускаемый модуль утилиты
//
//
//    Автор: Б.Гарбузов, 2022-2023
//
//
//=============================================================================


#include <windows.h>
#include <wtsapi32.h>
#include "RdpColorer.h"
#include "..\..\KbdLayoutMonitor\debug.h"



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
  DumpMessage(L"Started KbdRdpColorer %s", lpCmdLine);
#endif
  if (_wcsicmp(lpCmdLine, L"/exit") == 0)
  {
    // Сигнализируем ранее запущенному экземпляру приложения о необходимости завершить его работу, а также завершаем данный текущий экземпляр приложения.
    if (AppThreadId == 0)
    {
#ifdef _DEBUG
      DumpMessage(L"Ранее запущенный экземпляр приложения KbdRdpColorer отсутствует, завершать нечего.");
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
    // Обрабатываем сообщения главного потока приложения в ожидании сигнала завершения работы.
#ifdef _DEBUG
    DumpMessage(L"Это первый запущенный экземпляр приложения KbdRdpColorer. Обрабатываем сообщения в ожидании сигнала завершения...");
#endif
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);  // создаем очередь сообщений для текущего потока.
    AppThreadId = GetCurrentThreadId();
    TryPostLayoutFromCommandLine(lpCmdLine); // если первый экземпляр запустили с идентификатором раскладки клавиатуры в командной строке, то отправляем этот идентификатор себе через очередь сообщений.
    BOOL hasMessage;
    while (true)
    {
      hasMessage = PeekMessage(&msg, (HWND)-1, 0, 0, PM_REMOVE);
      if (hasMessage)
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) break;
        if (msg.message == WM_KEYBOARD_LAYOUT_CHANGED) SendKeyboardLayout((long)msg.lParam);
      }
      Sleep(100);
    }
#ifdef _DEBUG
    DumpMessage(L"Получен сигнал завершения работы KbdRdpColorer, завершаемся.");
#endif
    if (RdpChannelHandle != NULL)
    {
      WTSVirtualChannelClose(RdpChannelHandle);
      RdpChannelHandle = NULL;
    }
  }
  else
  {
    // Любой повторно запущенный без ключа /exit экземпляр приложения просто завершаем предварительно отправив первому экземпляру сигнал о новой раскладке клавиатуры, переданной в командной строке.
    if (!TryPostLayoutFromCommandLine(lpCmdLine))
    {
#ifdef _DEBUG
      DumpMessage(L"Данный второй экземпляр приложения KbdRdpColorer запущен без ключа /exit, завершаем его.");
#endif
    }
  }
  return 0;
} // wWinMain


/// <summary>
/// Информирует первый запущенный экземпляр приложения о новой раскладке клавиатуры, идентификатор которой указан в командной строке в шестнадцатиричном виде.
/// </summary>
/// <param name="lpCmdLine">Указатель на командную строку.</param>
/// <returns>Возвращает TRUE, если в командной строке был указан идентификатор раскладки клавиатуры, который был отправлен первому экземпляру приложения.</returns>
bool TryPostLayoutFromCommandLine (LPWSTR lpCmdLine)
{
  bool result = FALSE;
  if (wcslen(lpCmdLine) > 0)
  {
    long layout = wcstol(lpCmdLine, NULL, 16);
#ifdef _DEBUG
    DumpMessage(L"Отправляем первому экземпляру KbdRdpColorer раскладку клавиатуры %d (0x%X) из командной строки этого экземпляра KbdRdpColorer.", layout, layout);
#endif
    PostThreadMessage(AppThreadId, WM_KEYBOARD_LAYOUT_CHANGED, 0, layout);
    result = TRUE;
  }
  return result;
} // TryPostLayoutFromCommandLine


/// <summary>
/// Отправляет клиентской части виртуального RDP-канала (KbdLayoutRdpClient) сигнал о смене раскладки клавиатуры в окне RDP-сессии.
/// </summary>
/// <param name="layout">Идентификатор новой раскладки клавиатуры.</param>
void SendKeyboardLayout (long layout)
{
#ifdef _DEBUG
  DumpMessage(L"SendKeyboardLayout %d (0x%X)", layout, layout);
#endif
  if (RdpChannelHandle == NULL)
  {
    // Если виртуальный RDP-канал еще не открыт, то открываем его.
    RdpChannelHandle = WTSVirtualChannelOpenEx(WTS_CURRENT_SESSION, (LPSTR)"KbdL", 0);
    if (RdpChannelHandle == NULL)
    {
#ifdef _DEBUG
      DumpMessage(L"WTSVirtualChannelOpenEx failed: 0x%X", GetLastError());
#endif
    }
  }
  if (RdpChannelHandle != NULL)
  {
    ULONG len = 8;
    PCHAR buf = new CHAR[len];
    len = wsprintfA(buf, "%X", layout);
    ULONG wLen = 0;
    bool res = WTSVirtualChannelWrite(RdpChannelHandle, buf, len, &wLen);
    if (!res)
    {
      DWORD err = GetLastError();
#ifdef _DEBUG
      DumpMessage(L"WTSVirtualChannelWrite(RdpChannelHandle=%d, buf='%s', len=%d, wLen=%d) failed: 0x%X", RdpChannelHandle, buf, len, wLen, err);
#endif
      if (err == ERROR_PIPE_NOT_CONNECTED)
      {
        // Похоже на переподключение RDP-клиента. Старый канал похерился. Пробуем открыть и использовать новый канал.
        WTSVirtualChannelClose(RdpChannelHandle);
        RdpChannelHandle = WTSVirtualChannelOpenEx(WTS_CURRENT_SESSION, (LPSTR)"KbdL", 0);
        if (RdpChannelHandle == NULL)
        {
#ifdef _DEBUG
          DumpMessage(L"Reconnect WTSVirtualChannelOpenEx failed: 0x%X", GetLastError());
#endif
        }
        else
        {
          // Удалось переоткрыть новый канал. Повторяем отправку сигнала о смене раскладки клавиатуры.
          bool res = WTSVirtualChannelWrite(RdpChannelHandle, buf, len, &wLen);
          if (!res)
          {
#ifdef _DEBUG
            DumpMessage(L"Reconnected WTSVirtualChannelWrite(RdpChannelHandle=%d, buf='%s', len=%d, wLen=%d) failed: 0x%X", RdpChannelHandle, buf, len, wLen, GetLastError());
#endif
          }
        }
      }
    }
    delete[] buf;
  }
} // SendKeyboardLayout
