//=============================================================================
//
//  ������� ��� ����������� �� �������������� ��������� ����������
//
//  ������� ������ �������.
//  ��������� ��������� � ��������� ������ exe-���� � ��������� � ���� ��������������
//  ��������� ����������, �� ������� ��������� ������������.
//
//
//    �����: �.��������, 2022-2023
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
/// ����� �����.
/// </summary>
/// <param name="hInstance">����� �������� ����������� ���������� ����������.</param>
/// <param name="hPrevInstance">NULL</param>
/// <param name="lpCmdLine">��������� ��������� ������.</param>
/// <param name="nCmdShow">SW_SHOW</param>
/// <returns>0</returns>
int APIENTRY wWinMain (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  if (DebugInstanceHandle == 0) DebugInstanceHandle = hInstance;
  if (_wcsicmp(lpCmdLine, L"/exit") == 0)
  {
    // ������������� ����� ����������� ���������� ���������� � ������������� ��������� ��� ������, � ����� ��������� ������ ������� ��������� ����������.
    if (AppThreadId == 0)
    {
#ifdef _DEBUG
      DumpMessage(L"����� ���������� ��������� ���������� KbdLayoutMonitor �����������, ��������� ������.");
#endif
    }
    else
    {
      PostThreadMessage(AppThreadId, WM_QUIT, 0, 0);
    }
  }
  else if (AppThreadId == 0)
  {
    // ��� ������ ���������� ��������� ����������.
/* -������ ������������� MS RDP ���������� ���������� ��������� ��� ������������� ��������� ���������� � RDP-���� ����� ����������� � ������� ����������� ����������
   ��������� ���������� � RDP-����. ������ ���������� ���� ��������� ��� ������� */
    // ���� �������� ��� RDP-������, �.�. �� �������, �� ������������ ���� RDP Addin ��� ����� ��������� ��������� ���������� �� RDP-�������.
    if (!GetSystemMetrics(SM_REMOTESESSION))
    {
      // �������� �� ���������� ��� RDP-������, ���� � �������� RDP-�������.
      RegisterClientRDP();
    }
    else
    {
      // �������� �� ���������� � RDP-������.
      RegisterServerRDP();
    }

    // ������������ ��������� �������� ������ ���������� � �������� ������� ���������� ������.
#ifdef _DEBUG
    DumpMessage(L"��� ������ ���������� ��������� ���������� KbdLayoutMonitor. ������������ ��������� � �������� ������� ����������...");
#endif
    if (wcslen(lpCmdLine) > 0) LayoutIndicatorApp = lpCmdLine;
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);  // ������� ������� ��������� ��� �������� ������.
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
          // ������������ ���������-������ � ����� ��������� ���������� � ������� ���� RDP-������.
          // ��� ����, ���� �������� ���� ������� ��������� � ��� �� ������������� �� ���� RDP-������, � ������� ��������� ���������?
          // ���� �� ������� �� ���������-������� �� KbdLayoutRdpClient ���������� ���������� �� RDP-������ ��������� ���������� ����-�� � �������� � ������ ���� ���� RDP-������.
          // � ��� ������� RDP-������ ��� �������� ������ � KbdLayoutRdpClient �����-�� ������� ������������� ����������� �� ����������� �� ��������� ����� RDP-������
          // KbdLayoutRdpColorer-� ������� ��������� ���������� � KdbLayoutMonitor-�, ���������� ��� �� � RDP-������.
#ifdef _DEBUG
          DumpMessage(L"KbdLayoutMonitor: ������� ������ ��������� ��������� ���������� �� ������� RDP-������ ����� KbdLayoutRdpClient: 0x%X", msg.lParam);
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
      // ������������� ��������� ������� ��������� ���������� � �������������, ���� ��� ����������.
      CheckKeyboardLayout();
    }
#ifdef _DEBUG
    DumpMessage(L"������� ������ ���������� ������ KbdLayoutMonitor, �����������.");
#endif
  }
  else
  {
    // ����� �������� ���������� ��� ����� /exit ��������� ���������� ������ ���������, �.�. ������� ������ ���� ������ ���� ������ � �������� ���������.
#ifdef _DEBUG
    DumpMessage(L"������ ������ ��������� ���������� KbdLayoutMonitor ������� ��� ����� /exit, ��������� ���.");
#endif
  }
  return 0;
} // wWinMain


/// <summary>
/// ������������, dll-�� ��� ������������ RDP-������, ����� ������� �� KbdLayoutMonitor-�, ����������� �� RDP-�������, ����� ��������� ������� ��������� ��������� ����������.
/// �����������: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-client-registration.
/// ����������� ������������ �� ���������� ���� � �������:
/// [HKEY_CURRENT_USER\Software\Microsoft\Terminal Server Client\Default\Addins\KbdLayoutMonitor]
///   "Name" = "������ ���� � ������� ����� � ���� KdbLayoutRdpClient.dll."
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
/// ������������ ��������� ���������� ������������ RDP-������, ����������� � RDP-������ �� RDP-�������, � ����������� �� ������� �� ������������ RDP-������ ������� ����� ����� ��������� ����������.
/// �����������: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-server-application
///              https://learn.microsoft.com/en-us/windows/win32/termserv/monitoring-session-connections-and-disconnections.
/// ����������� ������������ �� ���������� ���� � �������:
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
