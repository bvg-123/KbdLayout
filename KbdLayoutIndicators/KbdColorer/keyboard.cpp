//=============================================================================
//
//  Утилита для изменения цвета подсветки клавиш клавиатуры Logitech G213
//
//  Модуль поиска поддерживаемой клавиатуры
//
//
//    Автор: Б.Гарбузов, 2022-2023
//
//
//=============================================================================

#include <windows.h>
#include <Setupapi.h>
#include <Hidsdi.h>
#include "keyboard.h"
#include "..\..\KbdLayoutMonitor\debug.h"


/// <summary>
/// Производит поиск подключенной USB-клавиатуры с подсветкой клавиш по имеющемуся списку поддерживаемого оборудования.
/// </summary>
/// <param name="keyboard">Выходной параметр. При успешном открытии найденной клавиатуры и возвращении ее хэндла в этом параметре будет возвращен указатель на информацию о найденной и открытой клавиатуре.</param>
/// <returns>Возвращает хэндл первой из подключенных USB-клавиатур с подсветкой клавиш. Если ничего не удалось найти, то возврщается INVALID_HANDLE_VALUE.</returns>
HANDLE DetectKeyboard (KEYBOARDCOMMAND** keyboard)
{
  HANDLE result = INVALID_HANDLE_VALUE;
  *keyboard = NULL;

  GUID hidGuid;
  HidD_GetHidGuid(&hidGuid);
  HDEVINFO hDeviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDeviceInfo == INVALID_HANDLE_VALUE)
  {
#ifdef _DEBUG
    DumpMessage(L"SetupDiGetClassDevs failed: 0x%X", GetLastError());
#endif
  }
  else
  {
    // Перебираем все подключенные к USB HID-устройства и находим среди них первую клавиатуру, параметры которой соответствуют одной из поддерживаемых клавиатур.
    DWORD memberIndex = 0;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    while (SetupDiEnumDeviceInterfaces(hDeviceInfo, NULL, &hidGuid, memberIndex++, &deviceInterfaceData))
    {
      ULONG requiredSize;
      SetupDiGetDeviceInterfaceDetail(hDeviceInfo, &deviceInterfaceData, NULL, 0, &requiredSize, 0);
      PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetail;
      deviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(requiredSize);
      if (deviceDetail == NULL)
      {
#ifdef _DEBUG
        DumpMessage(L"malloc PSP_DEVICE_INTERFACE_DETAIL_DATA failed: 0x%X", GetLastError());
#endif
      }
      else
      {
        deviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
        if (!SetupDiGetDeviceInterfaceDetail(hDeviceInfo, &deviceInterfaceData, deviceDetail, requiredSize, &requiredSize, NULL))
        {
#ifdef _DEBUG
          DumpMessage(L"SetupDiGetDeviceInterfaceDetail failed: 0x%X", GetLastError());
#endif
        }
        else
        {
          // Проверяем, входит ли очередное подключенное HID-устройство в список поддерживаемых клавиатур.
          KEYBOARDCOMMAND* kbd = SupportedKeyboards;
          while (kbd->vid != NULL)
          {
            if (FindSubstring(deviceDetail->DevicePath, kbd->vid) && FindSubstring(deviceDetail->DevicePath, kbd->pid))
            {
              WCHAR* devicePath = NULL;
              devicePath = deviceDetail->DevicePath;
              if (devicePath != NULL)
              {
                result = OpenDevice(devicePath, kbd);
                if (result != INVALID_HANDLE_VALUE)
                {
                  // Нашли нужное устройство.
                  *keyboard = kbd;
                  break;
                }
              }
            }
            kbd++;
          }
        }
        free(deviceDetail);
      }
      if (result != INVALID_HANDLE_VALUE) break; // Нашли нужное устройство.
    }
    SetupDiDestroyDeviceInfoList(hDeviceInfo);
  }
  return result;
} // DetectKeyboard


/// <summary>
/// Производит поиск заданной подстроки в заданной строке.
/// </summary>
/// <param name="full_string">Строка, по которой производится поиск подстроки.</param>
/// <param name="substring">Искомая подстрока.</param>
/// <returns>Возвращает true, если подстрока substring имеется в строке full_string.</returns>
BOOL FindSubstring (WCHAR* full_string, const char* substring)
{
  unsigned int c = 0, d = 0;
  while (true)
  {
    if (substring[d] == '\0') return true;
    if (full_string[c] == '\0') return false;
    d = (full_string[c] == substring[d]) ? d + 1 : 0;
    if (c++ > MAX_PATH) return false;
  }
} // FindSubstring


/// <summary>
/// Возвращает размер сообщений, которые можно отправлять в указанное USB-устройство. 
/// </summary>
/// <param name="hDevice">Хэндл устройства.</param>
int GetOutputReportLength (HANDLE hDevice)
{
  int result = 0;
  PHIDP_PREPARSED_DATA data;
  if (!HidD_GetPreparsedData(hDevice, &data))
  {
#ifdef _DEBUG
    DumpMessage(L"HidD_GetPreparsedData failed: 0x%X", GetLastError());
#endif
  }
  else
  {
    HIDP_CAPS caps;
    NTSTATUS status = HidP_GetCaps(data, &caps);
    if (status != HIDP_STATUS_SUCCESS)
    {
#ifdef _DEBUG
      DumpMessage(L"HidP_GetCaps failed: 0x%X", status);
#endif
    }
    else
    {
      result = caps.OutputReportByteLength;
    }
    HidD_FreePreparsedData(data);
  }
  return result;
} // GetOutputReportLength


/// <summary>
/// Проверяет, поддерживает ли клавиатура, идентифицируемая devicePath, запись сообщений, длина которых соответствует длине команды изменения цвета подсветки клавиш из keyboard. 
/// </summary>
/// <param name="devicePath">Путь к устройству (клавиатуре).</param>
/// <param name="keyboard">Информация о поддерживаемой клавиатуре и размере отправляемой в нее команды.</param>
/// <returns>Возвращает доступный для записи хэндл указанного устройства, если оно поддерживает запись команд длиной, указанной в keyboard в поле command.
/// В противном случае возвращает INVALID_HANDLE_VALUE.</returns>
HANDLE OpenDevice (WCHAR* devicePath, KEYBOARDCOMMAND* keyboard)
{
  // Пробуем открыть заданное устройство сначала на чтение и запись, если не удалось, то только на запись. 
  HANDLE hDevice = CreateFile(devicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if (hDevice == INVALID_HANDLE_VALUE)
  {
#ifdef _DEBUG
    // https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes
    DumpMessage(L"Unable to obtain read/write permissions for the HID device %s: 0x%X", devicePath, GetLastError());
#endif
    hDevice = CreateFile(devicePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
#ifdef _DEBUG
      DumpMessage(L"Unable to obtain write permissions for the HID device %s: 0x%X", devicePath, GetLastError());
#endif
    }
  }
  if (hDevice != INVALID_HANDLE_VALUE)
  {
    // Удалось открыть устройство на чтение и запись или только на запись.
    // Проверяем, поддерживает ли клавиатура по указанному пути протокол с нужным размером сообщения (одна и та же физическая клавиатура может оказаться представлена разными devicePath с разными правами доступа и разными размерами принимаемых и отправляемых сообщений).
    int outputReportLength = GetOutputReportLength(hDevice);
    size_t commandReportLength = CommandLength(keyboard->command);
    if (outputReportLength == commandReportLength)
    {
#ifdef _DEBUG
      DumpMessage(L"Found suitable USB-keyboard %s with write report size %d", devicePath, commandReportLength);
#endif
    }
    else
    {
      // Поддерживаемый устройством, открытым по указанному пути, размер отправляемых в него сообщений не соответствует размеру требуемой команды. Это устройство или путь к нему не подходят.
      CloseHandle(hDevice);
      hDevice = INVALID_HANDLE_VALUE;
    }
  }
  return hDevice;
} // OpenDevice


/// <summary>
/// Формирует и возвращает binhex-строку USB-команды путем замены в строке command конструкций &R, &G и &B на binhex-значения r, g и b, соответственно.
/// </summary>
/// <param name="command">Шаблон, по которому формируется команда, содержащий конструкции &R, &G и &B.</param>
/// <param name="r">Значение красной компоненты цвета.</param>
/// <param name="g">Значение зеленой компоненты цвета.</param>
/// <param name="b">Значение синей компоненты цвета.</param>
/// <returns>Новая BinHex-строка, сформированная из command и r, g, b. Эту строку после использования следует освободить вызовом free.</returns>
char* PrepareCommand (const char* command, byte r, byte g, byte b)
{
  char* result;
  result = _strdup(command);
  size_t count = strlen(result);
  char* prevChar = NULL;
  char* currentChar = result;
  while (count > 0)
  {
    if ((prevChar != NULL) && (*prevChar == '&'))
    {
      // Нашли &-конструкцию.
      if (*currentChar == 'R') BinHex(r, prevChar);
        else if (*currentChar == 'G') BinHex(g, prevChar);
          else if (*currentChar == 'B') BinHex(b, prevChar);
    }
    prevChar = currentChar;
    currentChar++;
    count--;
  }
  return result;
} // PrepareCommand


/// <summary>
/// Записывает в заданный буфер BinHex-представление заданного байта.
/// </summary>
/// <param name="value">Заданный байт.</param>
/// <param name="buf">Адрес буфера, начиная с которого будет записаны символы BinHex-представления заданного байта.</param>
void BinHex (byte value, char* buf)
{
  const char* hex = "0123456789ABCDEF";
  *buf = hex[(value >> 4) & 0x0F]; // старший полубайт
  *(buf + 1) = hex[value & 0x0F];  // младший полубайт
} // BinHex


/// <summary>
/// Преобразует два последовательных шестнадцатиричных символа из указанного буфера в байт.
/// </summary>
/// <param name="buf">Буфер с двумя шестнадцатиричными символами.</param>
/// <returns>Байт, соответствующий двум последовательным шестнадцатиричным символам в указанном буфере.</returns>
byte HexBin (char* buf)
{
  byte result;
  char hi = *buf;
  char lo = *(buf + 1);
  if (('0' <= hi) && (hi <= '9')) result = (hi-'0') << 4;
    else result = (hi-'A'+10) << 4;
  if (('0' <= lo) && (lo <= '9')) result = result | (lo-'0');
    else result = result | (lo-'A'+10);
  return result;
} // HexBin


/// <summary>
/// Возвращает длину (в байтах) команды, заданной BinHex-строкой.
/// </summary>
/// <param name="command">BinHex-строка команды.</param>
/// <returns>Длина команды в байтах.</returns>
size_t CommandLength (const char* command)
{
  return (strlen(command)+1)/3;  // +1, т.к. в конце строки после посленднего BinHex-байта нет пробела.
} // CommandLength


/// <summary>
/// Отправляет в заданное USB-устройство заданную команду.
/// </summary>
/// <param name="hDevice">Хэндл устройства, в который будет отправлена команда.</param>
/// <param name="command">Отправляемая команда в виде binhex-строки.</param>
void SendCommand (HANDLE hDevice, char* command)
{
  size_t len = CommandLength(command);
  byte* buf = new byte[len];
  char* commandPos = command;
  byte* bufPos = buf;
  size_t count = len;
  while (count > 0)
  {
    byte b = HexBin(commandPos);
    *bufPos = b;
    commandPos+=3;
    bufPos++;
    count--;
  }
  if (!HidD_SetOutputReport(hDevice, buf, (ULONG)len))          // HidD_SetOutputReport == control endpoint в нотации USB. 
  {
#ifdef _DEBUG
    DumpMessage(L"HidD_SetOutputReport(hDevice=%p, buf=\"%hs\", len=%d) failed: 0x%X", hDevice, command, len, GetLastError());
#endif
  }
  delete[] buf;
} // SendCommand
