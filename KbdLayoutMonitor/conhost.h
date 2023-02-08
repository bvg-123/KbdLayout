//=============================================================================
//
//  Утилита для мониторинга за переключениями раскладки клавиатуры
//
//  Объявления для модуля поиска нужного GUI-потока среди консольных процессов и потоков
//
//
//    Автор: Б.Гарбузов, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#pragma once
#include <windows.h>
#include <TlHelp32.h>


/// <summary>
/// Возвращает идентификатор процесса conhost.exe, выступающего в качестве владельца консольного окна заданного процесса.
/// Подробности: https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes.
/// </summary>
/// <param name="dwPPid">Идентификатор консольного процесса (процесса, создавшего консольное окно).</param>
/// <returns>Возвращает идентификатор процесса conhost.exe, выступающего в качестве владельца консольного окна заданного процесса.</returns>
DWORD FindConhost (DWORD dwPid);


/// <summary>
/// Сканирует потоки указанного процесса и определяет для потока, имеющего очередь сообщений, его раскладку клавиатуры.
/// https://learn.microsoft.com/en-us/windows/win32/toolhelp/traversing-the-thread-list
/// </summary>
/// <param name="pid">Идентификатор процесса, сканирование потоков которого производится.</param>
/// <returns>Возвращает раскладку клавиатуры потока, имеющего очередь сообщений.</returns>
HKL FindMessageQueueThreadKeyboardLayout (DWORD pid);
