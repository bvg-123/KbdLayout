//=============================================================================
//
//  ������ ���������� � RDP-��������
//
//  ���������� ���������� RDP-�������, ����� ������� ������������ �������������� KbdLayoutMonitor-� �� ��������� ��������� ���������� ������ RDP-������.
//
//
//    �����: �.��������, 2022-2023
//
//
//=============================================================================


#pragma once
#include <windows.h>
#include <cchannel.h>
#include <TlHelp32.h>


// ��� ������������ RDP-������.
#define VIRTUAL_CHANNEL_NAME "KbdL"

/// <summary>
/// ������� API ������������ RDP-������.
/// </summary>
CHANNEL_ENTRY_POINTS VirtualChannelEntryPoints;

/// <summary>
/// ����� RDP-����������.
/// </summary>
HANDLE RdpConnectionHandle;

/// <summary>
/// ����� ������������ RDP-������.
/// </summary>
DWORD VirtualChannelHandle;

/// <summary>
/// ����� �����, ������� �������� mstsc � ������ ����� �������������.
/// ��� �� ���������� ����������� ������������ RDP-������ ��� ��������� ����������� �� ������������ ��������� ���������� �� KbdLayoutMonitor-�, ����������� �� RDP-�������.
/// </summary>
/// <param name="pEntryPoints">������� API ������������ RDP-������.</param>
/// <returns>TRUE - �����, FALSE - �����-�� ������ (��� �������� �������� FALSE RDP-������ �������� ���� dll).</returns>
extern "C" __declspec(dllexport) BOOL VCAPITYPE VirtualChannelEntry (PCHANNEL_ENTRY_POINTS pEntryPoints);


/// <summary>
/// �������, ���������� RDP-�������� ��� ����������� � �������� RDP-����������.
/// </summary>
VOID VCAPITYPE VirtualChannelInitEventProc (LPVOID pInitHandle, UINT event, LPVOID pData, UINT dataLength);

/// <summary>
/// �������, ���������� RDP-�������� ��� ����������� � �������� ������������ RDP-������.
/// </summary>
VOID VCAPITYPE VirtualChannelOpenEventProc (DWORD openHandle, UINT event, LPVOID pData, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags);

/// <summary>
/// ���������� ������������� ��������� (� �������������) ������ �������� KbdLayountMonitor.exe.
/// �����������: https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
/// </summary>
/// <returns>���������� ������������� ��������� (� �������������) ������ �������� KbdLayountMonitor.exe.</returns>
DWORD FindKdbLayoutMonitorThreadId ();

