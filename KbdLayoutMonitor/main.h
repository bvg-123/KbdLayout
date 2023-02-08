//=============================================================================
//
//  ������� ��� ����������� �� �������������� ��������� ����������
//
//  ���������� ��� ��������� ������ �������
//
//
//    �����: �.��������, 2022-2023
//
//
//=============================================================================


#pragma once
#include <windows.h>


#pragma comment(linker, "/SECTION:.SHARED,RWS")
#pragma data_seg(".SHARED")
// ������������� ��������� ������ ������� ����������� ���������� ����������.
// ����������� � ����� �������� ������, ����������� ����� ����� ������������ ������������ ����������.
// ������������ ��� �������������� ����������� ����������� ����������� ���������� � ������ �� ���. 
DWORD AppThreadId = 0;
#pragma data_seg()

/// <summary>
/// ��� ����������-���������� ������� ��������� ����������, ������� ����������� �� ������� ����� ������� ��������� ����������. � ��������� ������ ������������
/// ���������� ���������� ���������� ������������� ������� ��������� ����������.
/// ���� ����������-��������� �� ������, �� ��� ����� ��������� ������ �� �����������, ������ � ���������� ������� VS ���������� ������� ��������� ����������.
/// ��� ������������ ��� ����� ������� ��������� ����������-���������� ����� ������� � ��������� ������.
/// </summary>
TCHAR* LayoutIndicatorApp = NULL;

/// <summary>
/// ������� ��������� ����������.
/// 67699721 - ��������� ���������, 68748313 - ������� ���������.
/// </summary>
HKL CurrentLayout = 0;

/// <summary>
/// ������� �������� ����.
/// </summary>
HWND CurrentForegroundWindow = 0;

/// <summary>
/// �������, ��� ������� �������� ���� �������� ����� �������.
/// ��. ����� ��������� ���������� � layout.h.
/// </summary>
bool CurrentForegroundWindowIsConsole = FALSE;

/// <summary>
/// �������, ��� ������� �������� ���� �������� RDP-����� (mstsc.exe).
/// ��. ����� ��������� ���������� � layout.h.
/// </summary>
bool CurrentForegroundWindowIsRDP = FALSE;

/// <summary>
/// ��������� ���������� � RDP-����, ���������� �� ���������-������� �� KbdLayoutRdpClient.
/// ������������ ��� ������������ � RDP-���� �� ������� ���� � �������� ������� RDP-���������.
/// ��. ����� ��������� ���������� � layout.h.
/// </summary>
HKL RDPWindowLayout = 0;

/// <summary>
/// ������������� ������ �������� ��������� ����.
/// </summary>
DWORD CurrentForegroundWindowThreadId = 0;

/// <summary>
/// ������������, dll-�� ��� ������������ RDP-������, ����� ������� �� KbdLayoutMonitor-�, ����������� �� RDP-�������, ����� ��������� ������� ��������� ��������� ����������.
/// �����������: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-client-registration.
/// ����������� ������������ �� ���������� ���� � �������:
/// [HKEY_CURRENT_USER\Software\Microsoft\Terminal Server Client\Default\Addins\KbdLayoutMonitor]
///   "Name" = "������ ���� � ������� ����� � ���� KdbLayoutMonitorRDP.dll."
/// </summary>
void RegisterClientRDP ();


/// <summary>
/// ������������ ��������� ���������� ������������ RDP-������, ����������� � RDP-������ �� RDP-�������, � ����������� �� ������� �� ������������ RDP-������ ������� ����� ����� ��������� ����������.
/// �����������: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-server-application
///              https://learn.microsoft.com/en-us/windows/win32/termserv/monitoring-session-connections-and-disconnections.
/// ����������� ������������ �� ���������� ���� � �������:
/// [HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Terminal Server\Addins\KbdLayoutMonitor]
///   "Name" = "RdpColorer"
///   "Type" = 3
/// </summary>
void RegisterServerRDP ();
