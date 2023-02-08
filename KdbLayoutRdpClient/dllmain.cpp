//=============================================================================
//
//  Модуль интеграции с RDP-клиентом
//
//  Библиотека RDP-клиента, через которую производится информирование KbdLayoutMonitor-а об изменении раскладки клавиатуры внутри RDP-сессии.
//
//
//    Автор: Б.Гарбузов, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#include "..\KbdLayoutMonitor\debug.h"
#include "..\KbdLayoutMonitor\RdpClient.h"
#include "dllmain.h"


BOOL APIENTRY DllMain (HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
  if (DebugInstanceHandle == 0) DebugInstanceHandle = hModule;
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
//#ifdef _DEBUG
//      DumpMessage(L"KbdLayoutRdpClient: DLL_PROCESS_ATTACH");
//#endif
      break;
    case DLL_THREAD_ATTACH:
//#ifdef _DEBUG
//      DumpMessage(L"KbdLayoutRdpClient: DLL_THREAD_ATTACH");
//#endif
      break;
    case DLL_THREAD_DETACH:
//#ifdef _DEBUG
//      DumpMessage(L"KbdLayoutRdpClient: DLL_THREAD_DETACH");
//#endif
      break;
    case DLL_PROCESS_DETACH:
//#ifdef _DEBUG
//      DumpMessage(L"KbdLayoutRdpClient: DLL_PROCESS_DETACH");
//#endif
      break;
  }
  return TRUE;
} // DllMain


/// <summary>
/// Точка входа, которую вызывает mstsc в момент своей инициализации.
/// Тут мы производим организацию виртуального RDP-канала для получения инфрормцаии об переключении раскладки клавиатуры от KbdLayoutMonitor-а, запущенного на RDP-сервере.
/// </summary>
/// <param name="pEntryPoints">Функции API виртуального RDP-канала.</param>
/// <returns>TRUE - успех, FALSE - какая-то ошибка (при возврате значения FALSE RDP-клиент выгрузит нашу dll).</returns>
BOOL VCAPITYPE VirtualChannelEntry (PCHANNEL_ENTRY_POINTS pEntryPoints)
{
#ifdef _DEBUG
  DumpMessage(L"KbdLayoutRdpClient: VirtualChannelEntry");
#endif
  BOOL result;

  // Сохраняем для последующего использования список указателей на функции API виртуального RDP-канала.
  memcpy(&VirtualChannelEntryPoints, pEntryPoints, sizeof(CHANNEL_ENTRY_POINTS));

  // Резервируем имя виртуального канала.
  CHANNEL_DEF channelDef;
  ZeroMemory(&channelDef, sizeof(CHANNEL_DEF));
  strcpy_s(channelDef.name, VIRTUAL_CHANNEL_NAME);
  channelDef.options = CHANNEL_OPTION_ENCRYPT_RDP | CHANNEL_OPTION_COMPRESS_RDP;
  UINT res = VirtualChannelEntryPoints.pVirtualChannelInit((LPVOID*)&RdpConnectionHandle, &channelDef, 1, VIRTUAL_CHANNEL_VERSION_WIN2000, (PCHANNEL_INIT_EVENT_FN)VirtualChannelInitEventProc);
  if (res != CHANNEL_RC_OK)
  {
    result = FALSE;
#ifdef _DEBUG
    DumpMessage(L"VirtualChannelEntryPoints.pVirtualChannelInit failed: 0x%X", res);
#endif
  }
  else
  {
    result = TRUE;
  }
  return result;
} // VirtualChannelEntry


/// <summary>
/// Функция, вызываемая RDP-клиентом для уведомления о событиях RDP-соединения.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/api/cchannel/nc-cchannel-channel_init_event_fn.
/// </summary>
/// <param name="pInitHandle">Handle to the client connection. This is the handle returned in the ppInitHandle parameter of the VirtualChannelInit function.</param>
/// <param name="event">Indicates the event that caused the notification. This parameter can be one of the following values:
///   CHANNEL_EVENT_INITIALIZED (0) - The Remote Desktop Connection (RDC) client initialization has been completed.The pData parameter is NULL.
///   CHANNEL_EVENT_CONNECTED (1) - A connection has been established with an RD Session Host server that supports virtual channels.The pData parameter is a pointer to a null - terminated string with the name of the server.
///   CHANNEL_EVENT_V1_CONNECTED (2) - A connection has been established with an RD Session Host server that does not support virtual channels.The pData parameter is NULL.
///   CHANNEL_EVENT_DISCONNECTED (3) - The connection to the RD Session Host server has been disconnected.The pData parameter is NULL.
///   CHANNEL_EVENT_TERMINATED (4) - The client has been terminated.The pData parameter is NULL.
///   CHANNEL_EVENT_REMOTE_CONTROL_START (5) - A remote control operation has been started.The pData parameter is NULL.
///   CHANNEL_EVENT_REMOTE_CONTROL_STOP (6) - A remote control operation has been terminated.The pData parameter is a pointer to a null - terminated string containing the name of the server.</param>
/// <param name="pData">Pointer to additional data for the event. The type of data depends on the event, as described previously in the event descriptions.</param>
/// <param name="dataLength">Specifies the size, in bytes, of the data in the pData buffer.</param>
/// <returns>This function does not return a value.</returns>
VOID VCAPITYPE VirtualChannelInitEventProc (LPVOID pInitHandle, UINT event, LPVOID pData, UINT dataLength)
{
  switch (event)
  {
    case CHANNEL_EVENT_INITIALIZED:
    {
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_INITIALIZED");
#endif
      break;
    }
    case CHANNEL_EVENT_CONNECTED:
    {
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_CONNECTED: %s", pData);
#endif
      UINT res = VirtualChannelEntryPoints.pVirtualChannelOpen(RdpConnectionHandle, &VirtualChannelHandle, (PCHAR)VIRTUAL_CHANNEL_NAME, (PCHANNEL_OPEN_EVENT_FN)VirtualChannelOpenEventProc);
      if (res != CHANNEL_RC_OK)
      {
#ifdef _DEBUG
        DumpMessage(L"VirtualChannelEntryPoints.pVirtualChannelOpen failed: 0x%X", res);
#endif
      }
      else
      {
#ifdef _DEBUG
        DumpMessage(L"Virtual Channel Open success");
#endif
      }
      break;
    }
    case CHANNEL_EVENT_V1_CONNECTED:
    {
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_V1_CONNECTED");
#endif
      break;
    }
    case CHANNEL_EVENT_DISCONNECTED:
    {
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_DISCONNECTED");
#endif
      break;
    }
    case CHANNEL_EVENT_TERMINATED:
    {
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_TERMINATED");
#endif
      break;
    }
    default:
    {
      break;
    }
  }
} // VirtualChannelInitEventProc


/// <summary>
/// Функция, вызываемая RDP-клиентом для уведомления о событиях виртуального RDP-канала.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/api/cchannel/nc-cchannel-channel_open_event_fn
/// </summary>
/// <param name="openHandle">Handle to the virtual channel. This is the handle returned in the pOpenHandle parameter of the VirtualChannelOpen function.</param>
/// <param name="event">Indicates the event that caused the notification. This parameter can be one of the following values:
///   CHANNEL_EVENT_DATA_RECEIVED   - The virtual channel received data from the server end.
///                                   pData is a pointer to a chunk of the data.
///                                   dataLength indicates the size of this chunk.
///                                   totalLength indicates the total size of the data written by the server.
///   CHANNEL_EVENT_WRITE_CANCELLED - A write operation started by a VirtualChannelWrite call has been canceled.
///                                   pData is the value specified in the pUserData parameter of VirtualChannelWrite.
///                                   A write operation is canceled when the client session is disconnected.This notification enables you to free any memory associated 
///                                   with the write operation.
///   CHANNEL_EVENT_WRITE_COMPLETE  - A write operation started by a VirtualChannelWrite call has been completed.
///                                   pData is the value specified in the pUserData parameter of VirtualChannelWrite.</param>
/// <param name="pData">Pointer to additional data for the event. The type of data depends on the event, as described previously in the event descriptions.
/// If event is CHANNEL_EVENT_DATA_RECEIVED, the data written by the server is broken into chunks of not more than CHANNEL_CHUNK_LENGTH bytes.
/// The dataFlags parameter indicates whether the current chunk is at the beginning, middle, or end of the block of data written by the server.
/// Note that the size of this parameter can be greater than the value specified by the dataLength parameter.
/// The application should read only the number of bytes specified by dataLength.</param>
/// <param name="dataLength">Specifies the size, in bytes, of the data in the pData buffer.</param>
/// <param name="totalLength">Specifies the total size, in bytes, of the data written by a single write operation to the server end of the virtual channel.</param>
/// <param name="dataFlags">Provides information about the chunk of data being received in a CHANNEL_EVENT_DATA_RECEIVED event. The following bit flags will be set.
///   CHANNEL_FLAG_FIRST  - The chunk is the beginning of the data written by a single write operation.
///                         Use bitwise comparisons when comparing this flag.
///   CHANNEL_FLAG_LAST   - The chunk is the end of the data written by a single write operation.
///                         Use bitwise comparisons when comparing this flag.
///   CHANNEL_FLAG_MIDDLE - This is the default. The chunk is in the middle of a block of data written by a single write operation.
///                         Do not use bitwise comparisons to compare this flag value directly. Instead, use bitwise comparisons to determine that the flag value is
///                         not CHANNEL_FLAG_FIRST or CHANNEL_FLAG_LAST. This is done by using the following comparison:
///                         Result = !(flags& CHANNEL_FLAG_FIRST) && !(flags& CHANNEL_FLAG_LAST).
///   CHANNEL_FLAG_ONLY   - Combines the CHANNEL_FLAG_FIRSTand CHANNEL_FLAG_LAST values.The chunk contains all the data from a single write operation.
///                         Use bitwise comparisons when comparing this flag.</param>
/// <returns>This function has no return values.</returns>
VOID VCAPITYPE VirtualChannelOpenEventProc (DWORD openHandle, UINT event, LPVOID pData, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags)
{
  switch (event)
  {
    case CHANNEL_EVENT_DATA_RECEIVED:
    {
      TCHAR* buf = NULL;
      if (dataLength > 0)
      {
        rsize_t size = (rsize_t)dataLength + 1;
        buf = new TCHAR[size];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, buf, size, (char*)pData, _TRUNCATE);
      }
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_DATA_RECEIVED: pData='%s', dataLength=%d, totalLength=%d, dataFlags=0x%X", buf, dataLength, totalLength, dataFlags);
#endif
      DWORD kdbLayoutMonitorThreadId = FindKdbLayoutMonitorThreadId();
#ifdef _DEBUG
      DumpMessage(L"FindKdbLayoutMonitorThreadId()=%d", kdbLayoutMonitorThreadId);
#endif
      if (kdbLayoutMonitorThreadId != 0)
      {
        long layout = wcstol(buf, NULL, 16);
#ifdef _DEBUG
        DumpMessage(L"PostThreadMessage(kdbLayoutMonitorThreadId=%d, WM_KEYBOARD_LAYOUT_CHANGED, 0, layout=0x%X)", kdbLayoutMonitorThreadId, layout);
#endif
        PostThreadMessage(kdbLayoutMonitorThreadId, WM_KEYBOARD_LAYOUT_CHANGED, 0, layout);
      }
      if (buf) delete[] buf;
      break;
    }
    case CHANNEL_EVENT_WRITE_CANCELLED:
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_WRITE_CANCELLED");
#endif
      break;
    case CHANNEL_EVENT_WRITE_COMPLETE:
#ifdef _DEBUG
      DumpMessage(L"CHANNEL_EVENT_WRITE_COMPLETE");
#endif
      break;
  default:
    break;
  }
} // VirtualChannelOpenEventProc


/// <summary>
/// Возвращает идентификатор основного (и единственного) потока процесса KbdLayoutMonitor.exe.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
/// </summary>
/// <returns>Возвращает идентификатор основного (и единственного) потока процесса KbdLayountMonitor.exe.</returns>
DWORD FindKdbLayoutMonitorThreadId ()
{
  DWORD result = 0;
  HANDLE hSnap;
  PROCESSENTRY32 pe32;
  hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap != INVALID_HANDLE_VALUE)
  {
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnap, &pe32))
    {
      do
      {
        if (lstrcmpi(L"KbdLayoutMonitor.exe", pe32.szExeFile) == 0)
        {
          // Нашли процесс KbdLayoutMonitor.exe.
          THREADENTRY32 te32;
          HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
          if (hThreadSnap != INVALID_HANDLE_VALUE)
          {
            te32.dwSize = sizeof(THREADENTRY32);
            if (Thread32First(hThreadSnap, &te32))
            {
              do
              {
                if (te32.th32OwnerProcessID == pe32.th32ProcessID)
                {
                  // Нашли среди всех потоков системы первый поток процесса KbdLayoutMonitor.exe.
                  result = te32.th32ThreadID;
                  break;
                }
              }
              while (Thread32Next(hThreadSnap, &te32));
            }
            CloseHandle(hThreadSnap);
          }
        }
      } while (Process32Next(hSnap, &pe32));
    }
    CloseHandle(hSnap);
  }
  return result;
} // FindConhost
