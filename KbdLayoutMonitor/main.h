//=============================================================================
//
//  Утилита для мониторинга за переключениями раскладки клавиатуры
//
//  Объявления для основного модуля утилиты
//
//
//    Автор: Б.Гарбузов, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#pragma once
#include <windows.h>


#pragma comment(linker, "/SECTION:.SHARED,RWS")
#pragma data_seg(".SHARED")
// Идентификатор основного потока первого запущенного экземпляра приложения.
// Размещается в общем сегменте памяти, разделяемом между всеми запускаемыми экземплярами приложения.
// Используется для взаимодействия последующих запускаемых экземпляров приложения с первым из них. 
DWORD AppThreadId = 0;
#pragma data_seg()

/// <summary>
/// Имя приложения-индикатора текущей раскладки клавиатуры, которое запускается по сигналу смены текущей раскладки клавиатуры. В командной строке запускаемого
/// приложения индикатора передается идентификатор текущей раскладки клавиатуры.
/// Если приложение-индикатор не задано, то при смене раскладки ничего не запускается, просто в отладочную консоль VS печатается текущая раскладка клавиатуры.
/// Имя запускаемого при смене текущей раскладки приложения-индикатора можно указать в командной строке.
/// </summary>
TCHAR* LayoutIndicatorApp = NULL;

/// <summary>
/// Текущая раскладка клавиатуры.
/// 67699721 - латинская раскладка, 68748313 - русская раскладка.
/// </summary>
HKL CurrentLayout = 0;

/// <summary>
/// Текущее активное окно.
/// </summary>
HWND CurrentForegroundWindow = 0;

/// <summary>
/// Признак, что текущее активное окно является окном консоли.
/// См. также связанное объявление в layout.h.
/// </summary>
bool CurrentForegroundWindowIsConsole = FALSE;

/// <summary>
/// Признак, что текущее активное окно является RDP-окном (mstsc.exe).
/// См. также связанное объявление в layout.h.
/// </summary>
bool CurrentForegroundWindowIsRDP = FALSE;

/// <summary>
/// Раскладка клавиатуры в RDP-окне, полученная по сообщению-сигналу от KbdLayoutRdpClient.
/// Используется при переключении в RDP-окно из другого окна в качестве текущей RDP-раскладки.
/// См. также связанное объявление в layout.h.
/// </summary>
HKL RDPWindowLayout = 0;

/// <summary>
/// Идентификатор потока текущего активного окна.
/// </summary>
DWORD CurrentForegroundWindowThreadId = 0;

/// <summary>
/// Регистрирует, dll-ку для виртуального RDP-канала, через который от KbdLayoutMonitor-а, запущенного на RDP-сервере, будут приходить события изменений раскладки клавиатуры.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-client-registration.
/// Регистрация производится по следующему пути в реестре:
/// [HKEY_CURRENT_USER\Software\Microsoft\Terminal Server Client\Default\Addins\KbdLayoutMonitor]
///   "Name" = "полный путь к лежащей рядом с нами KdbLayoutMonitorRDP.dll."
/// </summary>
void RegisterClientRDP ();


/// <summary>
/// Регистрирует серверное приложение виртуального RDP-канала, запускаемое в RDP-сессии на RDP-сервере, и отправлющее на клиента по виртуальному RDP-каналу команды смены цвета подсветки клавиатуры.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-server-application
///              https://learn.microsoft.com/en-us/windows/win32/termserv/monitoring-session-connections-and-disconnections.
/// Регистрация производится по следующему пути в реестре:
/// [HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Terminal Server\Addins\KbdLayoutMonitor]
///   "Name" = "RdpColorer"
///   "Type" = 3
/// </summary>
void RegisterServerRDP ();
