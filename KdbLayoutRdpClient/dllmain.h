//=============================================================================
//
//  Модуль интеграции с RDP-клиентом
//
//  Объявления библиотеки RDP-клиента, через которую производится информирование KbdLayoutMonitor-а об изменении раскладки клавиатуры внутри RDP-сессии.
//
//
//    Автор: Б.Гарбузов, 2022-2023
//
//
//=============================================================================


#pragma once
#include <windows.h>
#include <cchannel.h>
#include <TlHelp32.h>


// Имя виртуального RDP-канала.
#define VIRTUAL_CHANNEL_NAME "KbdL"

/// <summary>
/// Функции API виртуального RDP-канала.
/// </summary>
CHANNEL_ENTRY_POINTS VirtualChannelEntryPoints;

/// <summary>
/// Хэндл RDP-соединения.
/// </summary>
HANDLE RdpConnectionHandle;

/// <summary>
/// Хэндл виртуального RDP-канала.
/// </summary>
DWORD VirtualChannelHandle;

/// <summary>
/// Точка входа, которую вызывает mstsc в момент своей инициализации.
/// Тут мы производим организацию виртуального RDP-канала для получения инфрормцаии об переключении раскладки клавиатуры от KbdLayoutMonitor-а, запущенного на RDP-сервере.
/// </summary>
/// <param name="pEntryPoints">Функции API виртуального RDP-канала.</param>
/// <returns>TRUE - успех, FALSE - какая-то ошибка (при возврате значения FALSE RDP-клиент выгрузит нашу dll).</returns>
extern "C" __declspec(dllexport) BOOL VCAPITYPE VirtualChannelEntry (PCHANNEL_ENTRY_POINTS pEntryPoints);


/// <summary>
/// Функция, вызываемая RDP-клиентом для уведомления о событиях RDP-соединения.
/// </summary>
VOID VCAPITYPE VirtualChannelInitEventProc (LPVOID pInitHandle, UINT event, LPVOID pData, UINT dataLength);

/// <summary>
/// Функция, вызываемая RDP-клиентом для уведомления о событиях виртуального RDP-канала.
/// </summary>
VOID VCAPITYPE VirtualChannelOpenEventProc (DWORD openHandle, UINT event, LPVOID pData, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags);

/// <summary>
/// Возвращает идентификатор основного (и единственного) потока процесса KbdLayountMonitor.exe.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
/// </summary>
/// <returns>Возвращает идентификатор основного (и единственного) потока процесса KbdLayountMonitor.exe.</returns>
DWORD FindKdbLayoutMonitorThreadId ();

