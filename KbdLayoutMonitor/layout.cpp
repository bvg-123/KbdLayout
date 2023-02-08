//=============================================================================
//
//  ������� ��� ����������� �� �������������� ��������� ����������
//
//  ������ ��� ����������� ��������� ���������� � �������� ���� � ������� ������� ��������
//  �� ������� ������������ ���������.
//
//
//    �����: �.��������, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#include "layout.h"
#include "conhost.h"
#include "rasterhost.h"
#include "debug.h"


/// <summary>
/// ��������� ������� ��������� ���������� � ������������� ������� ������ OnLayoutChange, ���� ��� ����������.
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
/// ���������� ��� ��������� ��������� ����������.
/// </summary>
void OnLayoutChange ()
{
#ifdef _DEBUG
  TCHAR* buf = new TCHAR[1024];
  wsprintf(buf, L"������� ���������: %p", CurrentLayout);
  DumpMessage(buf);
  delete[] buf;
#endif

  if (false/*��������� RDP-�������*/)
  {
    // ��������� ���������� ���������:
    // SESSIONNAME=Console - ������ (� �������� ���������� RDP)
    // SESSIONNAME=RDP#... - ���� (� �������� ���������� RDP)
    // ��� RDP ������������ ����������� ������ RDP ��� �������� ������� ��������� �� ���� ��� ��������� �������� ����������.
    // !!! ��� ����� �������� � ��������� ����������� ����������-���������.
  }

  if (LayoutIndicatorApp != NULL)
  {
    // ��������� �������� � ��������� ������ ������� �� ������� ����� ������� ��������� ����������.
    RunLayoutIndicatorApp();
  }
} // OnLayoutChange


/// <summary>
/// ���������� ������������� ������� ��������� ����������.
/// </summary>
/// <returns>������������� ������� ��������� ����������: 67699721 - ��������� ���������, 68748313 - ������� ���������.</returns>
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
      if (CurrentForegroundWindowIsConsole) wsprintf(buf, L"�������� ���� �������: %p %s", CurrentForegroundWindow, title);
        else if (CurrentForegroundWindowIsRDP) wsprintf(buf, L"�������� ���� RDP: %p %s", CurrentForegroundWindow, title);
          else wsprintf(buf, L"�������� ����: %p %s", CurrentForegroundWindow, title);
      DumpMessage(buf);
      delete[] buf;
#endif
      delete[] title;
    }
    DWORD idProcess;
    DWORD idThread = GetWindowThreadProcessId(hwnd, &idProcess);
    if (CurrentForegroundWindowIsConsole)
    {
      // ������� ���������� ����: ��� ����������� ��������� ����� ��������� ������ �������� ConHost.exe, ���������� � ���� ���������� ����� (������� ���� �������� ������������ ��� �������� ConHost.exe).
      DWORD conhostId = FindConhost(idProcess);
      result = FindMessageQueueThreadKeyboardLayout(conhostId);
    }
    else if (CurrentForegroundWindowIsRDP)
    {
      // ��� �� ��������, �.�. ���� ��������� ���������� RDP-���� � ������ ������� �� DC � ������.
      // --������� RDP-����: ��� ����������� ��������� �� ��������� ����������, ������������ � RDP-����, ������� ���������� ��� � ������ ������ � ������ �� bitmap-�,
      // �������� �� ������������� bitmap-� ����� � ����������� �����, ���������� �� � ����� ���������� ������. ���� ����� ����������, �� ��������� ������ ����� bitmap-�
      // ����� ��������� ������������� �� Eng � Rus � ������ ��������������� ��� ��������� ����������.
//      result = DetectRasterKeyboardLayout(hwnd);
      // �������� ������� � ����� ��������� ���������� � RDP-������ ����� ���������-������ WM_KEYBOARD_LAYOUT_CHANGED �� KdbLayoutRdpClient-�.
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
      // �� ������� ���������� ��������� ���������� � ������� ��� ������� �� �������, � ������� ����: ���������� ��������� ���������� �� ������ ����� ����.
      if (CurrentForegroundWindowThreadId != idThread)
      {
        CurrentForegroundWindowThreadId = idThread;
#ifdef _DEBUG
        buf = new TCHAR[1024];
        wsprintf(buf, L"������������� ������ ��������� ����: %d", CurrentForegroundWindowThreadId);
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
/// ��������� ����������-��������� ��������� ���������� � ��������� ��� � ��������� ������ �������������� ������� ��������� ����������.
/// </summary>
void RunLayoutIndicatorApp ()
{
  TCHAR currentDir [MAX_PATH];
  GetCurrentDirectory(MAX_PATH, currentDir);
  TCHAR* cmdLine = new TCHAR[MAX_PATH];
  wsprintf(cmdLine, L"%s\\%s %p", currentDir, LayoutIndicatorApp, CurrentLayout);
#ifdef _DEBUG
  TCHAR* buf = new TCHAR[1024];
  wsprintf(buf, L"��������� ����������-���������: %s %p", LayoutIndicatorApp, CurrentLayout);
  DumpMessage(buf);
  delete[] buf;
#endif
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  // ����� ����������� ������ ����������: \HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\KbdColorer.exe Debugger:REG_SZ=vsjitdebugger.exe
  if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
#ifdef _DEBUG
    buf = new TCHAR[1024];
    wsprintf(buf, L"������ ������� ����������-����������: 0x%X", GetLastError());
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
