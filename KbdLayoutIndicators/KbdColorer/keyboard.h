﻿//=============================================================================
//
//  Утилита для изменения цвета подсветки клавиш клавиатуры Logitech G213
//
//  Объявления типов, переменных и функций для модуля взаимодействия с поддерживаемыми клавиатурами
//
//
//    Автор: Б.Гарбузов, 2022-2023
//           b.garbuzov@hotmail.com
//
//
//=============================================================================


#pragma once

#include <windows.h>

/// <summary>
/// Модель данных поддерживаемой клавиатуры.
/// </summary>
typedef struct
{
  /// <summary>
  /// USB-идентификатор производителя клавиатуры.
  /// </summary>
  const char* vid;

  /// <summary>
  /// USB-идентификатор клавиатуры в номенклатуре производителя.
  /// </summary>
  const char* pid;

  /// <summary>
  /// Команда для включения определенным цветом подсветки клавиш клавиатуры.
  /// Формат команды - binhex-строка с последовательностью байт команды нужной длины. Разделитель байтов - пробел. Все буквы в верхнем регистре.
  /// Места для подстановки значений красного, зеленого и синего обозначаются &R, &G и &B, соответственно.
  /// Пример:
  ///   "11 FF 0C 3C 00 01 &R &G &B 02 00 00 00 00 00 00 00 00 00 00" - команда размером 20 байт.
  /// </summary>
  const char* command;

} KEYBOARDCOMMAND;


/// <summary>
/// Список поддерживаемых клавиатур.
/// </summary>
extern KEYBOARDCOMMAND* SupportedKeyboards;


/// <summary>
/// Производит поиск подключенной USB-клавиатуры с подсветкой клавиш по имеющемуся списку поддерживаемого оборудования.
/// </summary>
/// <param name="keyboard">Выходной параметр. При успешном открытии найденной клавиатуры и возвращении ее хэндла в этом параметре будет возвращен указатель на информацию о найденной и открытой клавиатуре.</param>
/// <returns>Возвращает хэндл первой из подключенных USB-клавиатур с подсветкой клавиш. Если ничего не удалось найти, то возврщается NULL.</returns>
HANDLE DetectKeyboard (KEYBOARDCOMMAND** keyboard);


/// <summary>
/// Производит поиск заданной подстроки в заданной строке.
/// </summary>
/// <param name="full_string">Строка, по которой производится поиск подстроки.</param>
/// <param name="substring">Искомая подстрока.</param>
/// <returns>Возвращает true, если подстрока substring имеется в строке full_string.</returns>
BOOL FindSubstring (WCHAR* full_string, const char* substring);


/// <summary>
/// Возвращает размер сообщений, которые можно отправлять в указанное USB-устройство. 
/// </summary>
/// <param name="hDevice">Хэндл устройства.</param>
int GetOutputReportLength (HANDLE hDevice);


/// <summary>
/// Проверяет, поддерживает ли клавиатура, идентифицируемая devicePath, запись сообщений, длина которых соответствует длине команды изменения цвета подсветки клавиш из keyboard. 
/// </summary>
/// <param name="devicePath">Путь к устройству (клавиатуре).</param>
/// <param name="keyboard">Информация о поддерживаемой клавиатуре и размере отправляемой в нее команды.</param>
/// <returns>Возвращает доступный для записи хэндл указанного устройства, если оно поддерживает запись команд длиной, указанной в keyboard в поле command.
/// В противном случае возвращает INVALID_HANDLE_VALUE.</returns>
HANDLE OpenDevice (WCHAR* devicePath, KEYBOARDCOMMAND* keyboard);


/// <summary>
/// Формирует и возвращает binhex-строку USB-команды путем замены в строке command конструкций &R, &R и &B на binhex-значения r, g и b, соответственно.
/// </summary>
/// <param name="command">Шаблон, по которому формируется команда, содержащий конструкции &R, &R и &B.</param>
/// <param name="r">Значение красной компоненты цвета.</param>
/// <param name="g">Значение зеленой компоненты цвета.</param>
/// <param name="b">Значение синей компоненты цвета.</param>
/// <returns>Новая BinHex-строка, сформированная из command и r, g, b. Эту строку после использования следует освободить.</returns>
char* PrepareCommand (const char* command, byte r, byte g, byte b);

/// <summary>
/// Записывает в заданный буфре BinHex-представление заданного байта.
/// </summary>
/// <param name="value">Заданный байт.</param>
/// <param name="buf">Адрес буфера, начиная с которого будет записаны символы BinHex-представления заданного байта.</param>
void BinHex (byte value, char* buf);

/// <summary>
/// Преобразует два последовательных шестнадцатиричных символа из указанного буфера в байт.
/// </summary>
/// <param name="buf">Буфер с двумя шестнадцатиричными символами.</param>
/// <returns>Байт, соответствующий двум последовательным шестнадцатиричным символам в указанном буфере.</returns>
byte HexBin (char* buf);

/// <summary>
/// Возвращает длину (в байтах) команды, заданной BinHex-строкой.
/// </summary>
/// <param name="command">BinHex-строка команды.</param>
/// <returns>Длина команды в байтах.</returns>
size_t CommandLength (const char* command);

/// <summary>
/// Отправляет в заданное USB-устройство заданную команду.
/// </summary>
/// <param name="hDevice">Хэндл устройства, в который будет отправлена команда.</param>
/// <param name="command">Отправляемая команда в виде binhex-строки.</param>
void SendCommand (HANDLE hDevice, char* command);

