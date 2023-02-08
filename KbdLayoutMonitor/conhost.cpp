//=============================================================================
//
//  Утилита для мониторинга за переключениями раскладки клавиатуры
//
//  Модуль для поиска нужного GUI-потока среди консольных процессов и потоков
//
//
//    Автор: Б.Гарбузов, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#include "conhost.h"
#include "debug.h"

/// <summary>
/// Возвращает идентификатор процесса conhost.exe, выступающего в качестве владельца консольного окна заданного процесса.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
/// </summary>
/// <param name="dwPid">Идентификатор консольного процесса (процесса, создавшего консольное окно).</param>
/// <returns>Возвращает идентификатор процесса conhost.exe, выступающего в качестве владельца консольного окна заданного процесса.</returns>
DWORD FindConhost (DWORD dwPid)
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
        if (lstrcmpi(L"conhost.exe", pe32.szExeFile) == 0)
        {
          if (pe32.th32ParentProcessID == dwPid)
          {
            // Нашли процесс conhost.exe, для которого родительским является заданный процесс.
            result = pe32.th32ProcessID;
            break;
          }
        }
      } while (Process32Next(hSnap, &pe32));
    }
    CloseHandle(hSnap);
  }
  return result;
} // FindConhost


/// <summary>
/// Сканирует потоки указанного процесса и определяет для потока, имеющего очередь сообщений, его раскладку клавиатуры.
/// https://learn.microsoft.com/en-us/windows/win32/toolhelp/traversing-the-thread-list
/// </summary>
/// <param name="pid">Идентификатор процесса, сканирование потоков которого производится.</param>
/// <returns>Возвращает раскладку клавиатуры потока, имеющего очередь сообщений.</returns>
HKL FindMessageQueueThreadKeyboardLayout (DWORD pid)
{
  HKL result = (HKL)0;
  HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
  THREADENTRY32 te32;
  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap != INVALID_HANDLE_VALUE)
  {
    te32.dwSize = sizeof(THREADENTRY32);
    if (Thread32First(hThreadSnap, &te32))
    {
#ifdef _DEBUG
      HANDLE hFile = NULL;
      DWORD dwPos = 0;
      TCHAR* buf = NULL;
      SYSTEMTIME tm;
      BOOL doDump = TRUE;
      if (doDump)
      {
        buf = new TCHAR[1024];
        GetLocalTime(&tm);
        hFile = CreateFile(TEXT("KbdLayoutMonitorDebugDump.txt"), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        dwPos = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
      }
#endif
      DWORD tidMessageQueueCount = 0;
      do
      {
        if (te32.th32OwnerProcessID == pid)
        {
          tidMessageQueueCount++;
          HKL threadKbdLayout = GetKeyboardLayout(te32.th32ThreadID);
          GUITHREADINFO info;
          info.cbSize = sizeof(GUITHREADINFO);
          BOOL GetGUIThreadInfoResult = GetGUIThreadInfo(te32.th32ThreadID, &info);
#ifdef _DEBUG
          if (doDump)
          {
            DWORD dwBytes = wsprintf(buf, L"%02d:%02d:%02d.%03d pid=%d tid=%d GetGUIThreadInfoResult=%d info.hwndActive=%p layout=%p\r\n", tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, te32.th32OwnerProcessID, te32.th32ThreadID, GetGUIThreadInfoResult, info.hwndActive, threadKbdLayout) * 2;
            DWORD dwBytesWritten;
            LockFile(hFile, dwPos, 0, dwBytes, 0);
            WriteFile(hFile, buf, dwBytes, &dwBytesWritten, NULL);
            UnlockFile(hFile, dwPos, 0, dwBytes, 0);
          }
#endif
          if (GetGUIThreadInfoResult && info.hwndActive != NULL)
          {
            // В нужном потоке есть очередь сообщений и окно. По ним и определяем консольный поток, который отражает состояние раскладки клавиатуры в консоли.
            result = threadKbdLayout;
          }
        }
      } while (Thread32Next(hThreadSnap, &te32));
#ifdef _DEBUG
      if (doDump)
      {
        DWORD dwBytes = wsprintf(buf, L"========================\r\n") * 2;
        DWORD dwBytesWritten;
        LockFile(hFile, dwPos, 0, dwBytes, 0);
        WriteFile(hFile, buf, dwBytes, &dwBytesWritten, NULL);
        UnlockFile(hFile, dwPos, 0, dwBytes, 0);
        CloseHandle(hFile);
        delete[] buf;
      }
#endif
    }
    CloseHandle(hThreadSnap);
  }
  return result;
} // FindMessageQueueThreadKeyboardLayout
