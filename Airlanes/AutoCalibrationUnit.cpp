#include <vcl.h>
#pragma hdrstop

#include "AutoCalibrationUnit.h"
#include "ProgConfigUnit.h"
#include "CrcUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "RzTabs"
#pragma link "PngSpeedButton"
#pragma link "RzEdit"
#pragma link "RzSpnEdt"
#pragma link "PngBitBtn"
#pragma link "RzPanel"
#pragma link "RzShellDialogs"
#pragma link "PngSpeedButton"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------

#pragma link "wclBluetooth"
#pragma resource "*.dfm"

#ifdef _WIN64
#pragma comment(lib, "wclCommon.a")
#pragma comment(lib, "wclCommunication.a")
#pragma comment(lib, "wclBluetoothFrameworkR.a")
#else
#pragma comment(lib, "wclCommon.lib")
#pragma comment(lib, "wclCommunication.lib")
#pragma comment(lib, "wclBluetoothFrameworkR.lib")
#endif

TFormAutoСalibration *FormAutoСalibration;
//---------------------------------------------------------------------------

__fastcall TFormAutoСalibration::TFormAutoСalibration(TComponent* Owner)
        : TForm(Owner)
{
str_Error = L"Ошибка";

StopCalib = false;
BthConnectTime = 10;
ConnectError = mrOk;

nReadBTH = new TReadBTH(this);
ListBase = new TStringList();

BthFifo = new TFifo(NULL, 1024 * 1024);
CmdFifo = new TFifo(NULL, 1024 * 1024);
RawFifo = new TFifo(this, 1024 * 1024);
PackFifo = new TFifo(this, 1024 * 1024);
ManomRawFifo = new TFifo(this, 1024 * 1024);

ComDataForm = new TComData();
}
//---------------------------------------------------------------------------
void __fastcall TFormAutoСalibration::FormCreate(TObject *Sender)
{
UpdatePortList();
String FileSet = ChangeFileExt(ParamStr(0), L".ini");

// загрузка настроек программы
FormatSettings.DecimalSeparator = L'.';
if (FileExists(FileSet))
  {
  ProgConfig.LoadConfig();
  }
else
  {
  ProgConfig.SaveConfig();
  ProgConfig.LoadConfig();
  }

SetParam();

LabelSearchDev->Left = (GroupBoxSearchDev->Width / 2) - (LabelSearchDev->Width / 2);

RzPageControlCheckUp->TabIndex = 0;
LabelStatusCOM->Left = (GroupBoxSetCOM->Width / 2) - (LabelStatusCOM->Width / 2);

wclRfCommClient->OnData = wclRfCommClientData;

OpenDialog1->Options = TOpenOptions() << ofPathMustExist << ofFileMustExist << ofEnableSizing;
OpenDialog1->FileName = L"";
OpenDialog1->Filter = L"Текстовый файл (*.csv)|*.CSV";
OpenDialog1->FilterIndex = 1;

if (ProgConfig.DBPath == L"")
  {
  ProgConfig.DBPath = GetCurrentDir() + L"\\CalibBase.csv";
  RzEditBasePath->Text = ProgConfig.DBPath;
  }
else
  {
  RzEditBasePath->Text = ProgConfig.DBPath;
  }
ReadBase();
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::SetParam(void)
{
ComboBoxPorts->Text = ProgConfig.AutoCalibCfg.NameManomCom;
ComboBoxSpeeds->Text = ProgConfig.AutoCalibCfg.SpeedManomCom;
RzSpinEditManDelay->Value = StrToIntDef(ProgConfig.AutoCalibCfg.ManDelay, 200);
RzSpinEditPoint1->Value = StrToIntDef(ProgConfig.AutoCalibCfg.Point1, 5);
RzSpinEditPoint2->Value = StrToIntDef(ProgConfig.AutoCalibCfg.Point2, 10);
RzSpinEditPoint3->Value = StrToIntDef(ProgConfig.AutoCalibCfg.Point3, 20);
RzSpinEditPoint4->Value = StrToIntDef(ProgConfig.AutoCalibCfg.Point4, 30);
RzSpinEditPoint5->Value = StrToIntDef(ProgConfig.AutoCalibCfg.Point5, 40);
RzSpinEditDelay->Value = StrToIntDef(ProgConfig.AutoCalibCfg.DelayPres, 30);
RzSpinEditResetThreshold->Value = StrToIntDef(ProgConfig.AutoCalibCfg.ResetThreshold, 60);

RzEditBasePath->Text = ProgConfig.DBPath;

RzEditSurname->Text = ProgConfig.CalibratorCfg.Surname;
RzEditName->Text    = ProgConfig.CalibratorCfg.Name;
RzEditPatron->Text  = ProgConfig.CalibratorCfg.Patron;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ReadBase(void)
{
TFileStream *FileStream = NULL;
if (FileExists(ProgConfig.DBPath)) FileStream = new TFileStream(ProgConfig.DBPath, fmOpenWrite);
else FileStream = new TFileStream(ProgConfig.DBPath, fmCreate | fmOpenWrite);
__int64 FileBaseSize = FileStream->Size;
if (FileStream) delete FileStream;
if (FileBaseSize > SIZE_MEMORY)
  {
  String Hdr = L"Ошибка!";
  String Msg = L"Файл базы больше 200Мб! Открыть невозможно";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
  return;
  }
ListBase->Clear();
if (FileBaseSize)
  {
  ListBase->LoadFromFile(ProgConfig.DBPath);
  }
else
  {
  String Str = L"ID чипа;День,Месяц,Год;Время;Модель;Серийный номер;Версия прошивки;Версия оборуд.;МАС ВТН;S/N манометра;AvtoCalibration v.;1к1д;2к1д;3к1д;1к2д;2к2д;3к2д;Фамилия;Имя;Отчество";
  ListBase->Add(Str);
  }
}
//---------------------------------------------------------------------------


void __fastcall TFormAutoСalibration::UpdatePortList(void)
{
TComPort Port;
ComboBoxPorts->Clear();

for (int i = 1; i <= 256; i++)
  {
  String S = L"\\\\.\\com" + IntToStr(i);
  ComboBoxPorts->Items->Add(L"com" + IntToStr(i));
  }
if (ComboBoxPorts->Items->Count) ComboBoxPorts->ItemIndex = 0;
}
//---------------------------------------------------------------------------


void __fastcall TFormAutoСalibration::ButtonOpenPortClick(TObject *Sender)
{
String SPort = ComboBoxPorts->Text;
if (!SPort.Length()) return;

String SSpeed = ComboBoxSpeeds->Text;
if (!SSpeed.Length()) return;

int ComSpeedTable[18] = { 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 128000, 230400, 256000, 460800, 921600 };
int ComSpeedIndex = ComboBoxSpeeds->ItemIndex;
int ComSpeed = ComSpeedTable[ComSpeedIndex];

int Res = OpenPort(SPort, ComSpeed);
if (!Res)
  {
  //получаем серийный номер манометра (если он найден на текущем порту)
  TComStatus Status = ComSendCmdWithReply(MAN_GET_SERIAL_CMD);

  // если статус не COM_OK - делаем вывод, что манометр на порту не найден
  if (Status != COM_OK)
    {
    String Msg = GetComStatusString(Status);
    ShowMessage(L"Манометр не найден, " + Msg);
    ClosePort();
    }
  else
    {
    unsigned Value = (ComDataForm->Data[0] << 24) | (ComDataForm->Data[1] << 16) | (ComDataForm->Data[2] << 8) | ComDataForm->Data[3];
    ManomSerial = reinterpret_cast<float &>(Value);

    LabelStatusCOM->Caption = L"Манометр подключен S/N: " + FloatToStr(ManomSerial);
    LabelStatusCOM->Font->Color = clGreen;
    LabelStatusCOM->Left = (GroupBoxSetCOM->Width / 2) - (LabelStatusCOM->Width / 2);

    ComSendCmdWithReply(MAN_SET_ZERO_CMD); // Сброс манометра
    StartManomThread();                    // Запуск потока манометра
    }
  }
else
  {
  LabelStatusCOM->Caption = L"Ошибка подключения манометра";
  LabelStatusCOM->Font->Color = clRed;
  LabelStatusCOM->Left = (GroupBoxSetCOM->Width / 2) - (LabelStatusCOM->Width / 2);
  }
}
//---------------------------------------------------------------------------

int TFormAutoСalibration::OpenPort (String APortName, int APortSpeed)
{
int Status = 1;

try
  {
  int Result = Port.Open(L"\\\\.\\" + APortName);
  if (Result) throw Exception(Port.GetLastErrStr());

  Result = Port.Setup(APortSpeed, 8, NOPARITY, ONESTOPBIT);
  if (Result) throw Exception(Port.GetLastErrStr());

  Result = Port.Timeouts(MAXDWORD, 0, 0, 0, 0);
  if (Result) throw Exception(Port.GetLastErrStr());

  Result = Port.Buffers(32768, 32768);
  if (Result) throw Exception(Port.GetLastErrStr());

  Result = Port.Purge(true, true);
  if (Result) throw Exception(Port.GetLastErrStr());

  ComboBoxPorts->Enabled = false;
  ComboBoxSpeeds->Enabled = false;
  ButtonOpenPort->Enabled = false;
  ButtonClosePort->Enabled = true;

  Status = 0;
  }
catch (Exception &Exc)
  {
  Port.Close();
  }

return Status;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonClosePortClick(TObject *Sender)
{
Disconnect(MODE_DIS_MANOM);
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::ClosePort (void)
{
ComboBoxSpeeds->Enabled = true;
ComboBoxPorts->Enabled = true;
ButtonOpenPort->Enabled = true;
ButtonClosePort->Enabled = false;
Port.Close();
LabelStatusCOM->Caption = L"Манометр отключен";
LabelStatusCOM->Font->Color = clRed;
LabelStatusCOM->Left = (GroupBoxSetCOM->Width / 2) - (LabelStatusCOM->Width / 2);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonSendBinClick(TObject *Sender)
{
String Hdr = L"Внимание!";
String Msg = L"Вы уверены что хотите начать процесс калибровки?\r\n";
if (!ProbaThread || !ManomThread)
  {
  Hdr = L"Ошибка!";
  Msg = L"";
  if (!ProbaThread) Msg = L"Прибор не подключен по BTH! ";
  if (!ManomThread) Msg += L"Манометр не подключен!";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);
  return;
  }
int Result = MessageBox(Handle, Msg.w_str(), Hdr.w_str(), MB_YESNO | MB_ICONWARNING);
if (Result == IDNO) return;
LabelInfo->Caption = L"Запуск процесса колибровки";

ButtonUpdateDeviceList->Enabled = false;
ButtonDisconnect->Enabled = false;
TabSheetSettings->Enabled = false;
PngSpeedButtonStopCalib->Enabled = true;

int Status = CalibProcess();

switch (Status)
  {
  case 0: // Успешно завершено
    {
    Disconnect(MODE_DIS_BTH);
    BthConnectProcess(0);
    WriteBase(); // запись в базу
    FindListBase(); // чтение
    LabelInfo->Caption = L"Калибровка успешно завершена";
    Msg = L"Калибровка успешно завершена!";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    break;
    }
  case 1: // Прервана пользователем
    {
    LabelInfo->Caption = L"Калибровка прервана пользователем";
    Msg = L"Калибровка прервана пользователем!";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    break;
    }
  default:
    {
    LabelInfo->Caption = L"Ошибка при калибровке";
    Hdr = L"Ошибка!";
    Msg = L"Ошибка при калибровке!";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    break;
    }
  }

ListBoxDeviceBTH->Enabled = false;
ButtonUpdateDeviceList->Enabled = true;
ButtonDisconnect->Enabled = true;
TabSheetSettings->Enabled = true;
PngSpeedButtonStopCalib->Enabled = false;

}
//---------------------------------------------------------------------------

TComStatus TFormAutoСalibration::ComSendCmdWithReply (int Index)
{

int RxDataCount = 0, Count = 0;
TComStatus Status = COM_OK;

switch (Index)
  {
  case MAN_GET_PRES_CMD:   { RxDataCount = 19; break; } // команда "Считать первичную переменную (давление)"
  case MAN_SET_ZERO_CMD:   { RxDataCount = 15; break; } // команда "Выполнить коррекцию нуля"
  case MAN_GET_SERIAL_CMD: { RxDataCount = 22; break; } // команда "Сведения о приборе"
  default: { return Status = COM_ERR_UNK_CMD; }         // неизвестная команда
  }

// отправляем команду
Status = ComSendCmd(Index);
if (Status != COM_OK) { return Status; }

// ожидаем
Sleep(RzSpinEditManDelay->Value);

// получаем ответ
while (1)
  {
  if (RxDataCount <= 0) { break; }
  Port.Read(&TmpBuffer, sizeof(TmpBuffer), Count);
  if (!Count) { return Status = COM_ERR_EMPTY_REPLY; }
  if (Count > RxDataCount) { return Status = COM_ERR_LONG_REPLY; }
  RxDataCount -= Count;
  }

// парсим ответ
Status = ComParseCmd((unsigned char*) &TmpBuffer, ComDataForm);
if (Status != COM_OK) { return Status; }

return Status;
}
//---------------------------------------------------------------------------

TComStatus TFormAutoСalibration::ComSendCmd (int Index)
{
String CmdStr, Msg;
byte DataBuffer[256];
int Data, DataSize = 0;

TWords *Words = new TWords(MAP_SOURCE);
TComStatus Status = COM_OK;

switch (Index)
  {
  case MAN_GET_PRES_CMD:   { CmdStr = L"0xFF 0xFF 0xFF 0x82 0xFF 0xFF 0xFF 0xFF 0x00 0x01 0x00 0x83"; break; }
  case MAN_SET_ZERO_CMD:   { CmdStr = L"0xFF 0xFF 0xFF 0x82 0xFF 0xFF 0xFF 0xFF 0x00 0x25 0x00 0xA7"; break; }
  case MAN_GET_SERIAL_CMD: { CmdStr = L"0xFF 0xFF 0xFF 0x82 0xFF 0xFF 0xFF 0xFF 0x00 0xAE 0x00 0x2C"; break; }
  default: { return Status = COM_ERR_UNK_CMD; }
  }

// записываем команду в строку
Words->SetNewString(CmdStr);

// чистим TX-буфер
memset(&DataBuffer, 0, sizeof(DataBuffer));

while (1)
  {
  // парсим команду из заполненной строки
  CmdStr = Words->GetMeanWord();
  if (!CmdStr.Length()) break;
  if (CmdStr == ".") continue;
  if (CmdStr == ",") continue;
  if (CmdStr == ":") continue;
  if (CmdStr == ";") continue;
  if (CmdStr == "-") continue;
  if (CmdStr == "+") continue;
  if (!TryStrToInt(CmdStr, Data))
    {
    Status = COM_ERR_WRONG_CMD;
    DataSize = -1;
    break;
    }

  // заполняем TX-буфер
  DataBuffer[DataSize++] = Data;
  if (DataSize >= 256) { Status = COM_ERR_LONG_CMD; break; }
  }

// отправляем команду в порт
if (DataSize > 0)
  {
  if (Port.Write(&DataBuffer, DataSize)) { Status = COM_ERR_WRITE; }
  }

delete Words;
return Status;
}
//---------------------------------------------------------------------------

TComStatus TFormAutoСalibration::ComParseCmd(unsigned char *Src, TComData *AComData)
{
// проверка корректности преамбулы
if (Src[0] != 0xFF) return COM_ERR_WRONG_PREAMB;
if (Src[1] != 0xFF) return COM_ERR_WRONG_PREAMB;
if (Src[2] != 0xFF) return COM_ERR_WRONG_PREAMB;

// проверка корректности стартового бита
if ((Src[3] != 0x82) && (Src[3] != 0x86)) return COM_ERR_WRONG_SBIT;

// проверка корректности размера данных
int DataSize = Src[10];
if ((DataSize == 0) || (DataSize > 25)) return COM_ERR_WRONG_DATASIZE;

// проверка корректности контрольной суммы
byte CheckSum = Src[13 + DataSize];
byte RealCheckSum = ChecksumXOR((byte*) &Src[3], 10 + DataSize);
if (CheckSum != RealCheckSum) return COM_ERR_WRONG_CRC;

// очищаем структуру
memset(AComData, 0x00, sizeof(TComData));

// заполняем поле преамбулы
memcpy(&AComData->Preamb, &Src[0], sizeof(AComData->Preamb));

// заполняем поле стартового символа
AComData->StartSymb = Src[3];

// заполняем поле адреса
memcpy(&AComData->Addr, &Src[4], sizeof(AComData->Addr));

// заполняем поле команды
AComData->Cmd = Src[9];

// заполняем поле размера полезных данных
AComData->DataSize = DataSize;

// заполняем поле статуса
memcpy(&AComData->Status, &Src[11], sizeof(AComData->Status));

// заполняем поле буфера с данными
memcpy(&AComData->Data, &Src[13], DataSize);

// заполняем поле контрольной суммы
AComData->CheckSum = CheckSum;

return COM_OK;
}
//---------------------------------------------------------------------------

String TFormAutoСalibration::GetComStatusString(TComStatus Status)
{
String Msg;

switch (Status)
  {
  case COM_OK:                 { Msg = L"Ошибок не обнаружено!"; break; }
  case COM_ERR_UNK_CMD:        { Msg = L"Ошибка! Неизвестная команда!"; break; }
  case COM_ERR_WRONG_CMD:      { Msg = L"Ошибка! Некорректная команда!"; break; }
  case COM_ERR_LONG_CMD:       { Msg = L"Ошибка! Слишком длинная команда!"; break; }
  case COM_ERR_WRONG_PREAMB:   { Msg = L"Ошибка! Некорректная преамбула!"; break; }
  case COM_ERR_WRONG_SBIT:     { Msg = L"Ошибка! Некорректный START-бит!"; break; }
  case COM_ERR_WRONG_DATASIZE: { Msg = L"Ошибка! Некорректный размер данных!"; break; }
  case COM_ERR_WRONG_CRC:      { Msg = L"Ошибка! Некорректная CRC-сумма!"; break; }
  case COM_ERR_WRITE:          { Msg = L"Ошибка! Ошибка записи в COM-порт!"; break; }
  case COM_ERR_EMPTY_REPLY:    { Msg = L"Ошибка! Ответ пуст!"; break; }
  case COM_ERR_LONG_REPLY:     { Msg = L"Ошибка! Слишком длинный ответ!"; break; }
  default:                     { Msg = L"Ошибка! Неизвестная ошибка!"; break; }
  }

return Msg;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonUpdateDeviceListClick(TObject *Sender)
{

if (ButtonUpdateDeviceList->Caption == L"Обновить")
  {
  ListBoxDeviceBTH->Enabled = false;
  BthDevicesList.clear();
  ListBoxDeviceBTH->Clear();

  SetSearchBthLabel(L"Идёт поиск устройств", 1);
  //ActivityIndicatorBth->Animate = true;
  ButtonUpdateDeviceList->Caption = L"Стоп";

  Application->ProcessMessages();
  if (wclBluetoothManager->Active) wclBluetoothManager->Close();
  //wclBluetoothManager->Close();

  int Result = wclBluetoothManager->Open();
  if (Result != WCL_E_SUCCESS)
    {
    String Hdr = str_Error;
    String Msg = L"Ошибка при подключении стека Bluetooh (code " + IntToHex(Result, 8) + ")";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    Disconnect(MODE_DIS_BTH);
    return;
    }

  StartBthDiscovery();
  }
else
  {
  Disconnect(MODE_DIS_BTH);
  ListBoxDeviceBTH->Enabled = true;
  return;
  }

}
//---------------------------------------------------------------------------

void TFormAutoСalibration::StartBthDiscovery (void)
{
// запускаем первый шаг поиска устройств (dlClassic)
StartOperationTimer(1, 10);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::Disconnect (int AModeDisconnect)
{
TimerDifference->Enabled = false;
if (AModeDisconnect & MODE_DIS_BTH)
  {
  TDateTime TimerDisProba = Now() + (0.5 / (24.0 * 3600.0));
  if (ProbaThread)
    {
    SendCommandBth(0x13); // открытие клапана
    SendCommandBth(0x16); // выключение компрессора
    bool Done = false;
    while (!Done)
      {
      Application->ProcessMessages();
      if (Now() > TimerDisProba) Done = true;
      }
    ProbaThread->Terminate();
    ProbaThread->WaitFor();
    delete ProbaThread;
    ProbaThread = NULL;
    }
  btDisconnect();
  ListBoxDeviceBTH->Enabled = true;
  VerificationSetBthDef();
  }
if (AModeDisconnect & MODE_DIS_MANOM)
  {
  if (ManomThread)
    {
    ManomThread->Terminate();
    ManomThread->WaitFor();
    delete ManomThread;
    ManomThread = NULL;
    }
  ClosePort();
  RzEditManom->Text = L"0";
  }
Sleep(1000);
}
//---------------------------------------------------------------------------

int TFormAutoСalibration::DeviceDefinition (String ADevice)
{
int Res = 1;

ADevice = ADevice.Delete(ADevice.Pos(" "), ADevice.Length());

if (ADevice == L"MD-01")               return Res = 0;
if (ADevice == L"MD-01M")              return Res = 0;
if (ADevice == L"KR02ru")              return Res = 0;
if (ADevice == L"KR03ru")              return Res = 0;
if (ADevice == L"KR04ru")              return Res = 0;
if (ADevice == L"KR05ru")              return Res = 0;
if (ADevice == L"KR06ru")              return Res = 0;
if (ADevice == L"IN22M")               return Res = 0;
if (ADevice == L"ICAR7")               return Res = 0;
if (ADevice == L"ICAR")                return Res = 0;
if (ADevice == L"ICAR8")               return Res = 0;
if (ADevice == L"KAMA")                return Res = 0;
if (ADevice == L"CardiolinkECG")       return Res = 0;
if (ADevice == L"CardiolinkBP")        return Res = 0;
if (ADevice == L"CardiolinkECG/BP")    return Res = 0;
if (ADevice == L"CardiolinkECG/BP/S")  return Res = 0;
if (ADevice == L"MD-01")               return Res = 0;
if (ADevice == L"MD-01M")              return Res = 0;

return Res;
}
//---------------------------------------------------------------------------

bool __fastcall TFormAutoСalibration::BthConnectDevice (void)
{
int Status = 1, Result;
int Index = ListBoxDeviceBTH->ItemIndex;

if (Index == -1)
  {
  String Hdr = str_Error;
  String Msg = L"Выберите устройство для подключения";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
  return Status;
  }

// данные о подключаемом устройстве
BthDeviceInfo = BthDevicesList[Index];

TwclBluetoothRadio *Radio = GetRadio(BthDeviceInfo.Type);
if (!Radio)
  {
  String Hdr = str_Error;
  String Msg = L"Адаптер Bluetooh не подключён.";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
  return Status;
  }

if (BthDeviceInfo.Type != dkBle)
  {
  // подключаем как dlClassic
  wclRfCommClient->Address = BthDeviceInfo.Addr;
  wclRfCommClient->Authentication = true;
  wclRfCommClient->Encryption = true;
  wclRfCommClient->Timeout = BthConnectTime * 1000;

  Result = wclRfCommClient->Connect(Radio);
  if (Result == WCL_E_SUCCESS) { return 0; }
  }
else
  {
  // подключаем как dlBle
  wclGattClient->Address = BthDeviceInfo.Addr;
  wclGattClient->ConnectOnRead = false;

  Result = wclGattClient->Connect(Radio);
  if (Result == WCL_E_SUCCESS) { return 0; }
  }

String Hdr = str_Error;
String Msg = L"Ошибка при запуске подключения к устройству (code " + IntToHex(Result, 8) + ")";
MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);

return Status;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ListBoxDeviceBTHDblClick(TObject *Sender)
{
Disconnect(MODE_DIS_BTH);
ListBoxDeviceBTH->Enabled = true;
if ((ListBoxDeviceBTH->ItemIndex > -1) && (ItemBoxMouse > -1))
  {
  ListBoxDeviceBTH->Enabled = false;
  BthConnectProcess(1);
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ListBoxDeviceBTHKeyPress(TObject *Sender, System::WideChar &Key)
{
Disconnect(MODE_DIS_BTH);
ListBoxDeviceBTH->Enabled = true;
if ((ListBoxDeviceBTH->ItemIndex > -1) && (Key == VK_RETURN))
  {
  ListBoxDeviceBTH->Enabled = false;
  BthConnectProcess(1);
  }
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::BthConnectProcess(int ProcessBTH)
{
int Status = mrAbort;
LabelInfo->Caption = L"Подготовка...";

// подключаем или проверяем подключено ли выбранное устройство ВТН

// подключаем выбранное устройство ВТН
ConnectError == mrNone;
int Result = BthConnectDevice(); // подключаем выбранное устройство ВТН
if (Result)
  {
  ConnectError = mrOk;
  }

while (1)
  {
  // ожидаем подключения устройства
  Application->ProcessMessages(); Sleep(10);

  if ((ConnectError == mrOk) && (GetClientState() == csConnected))
    {
    Status = mrOk;
    break;
    }

  if (ConnectError == mrAbort)
    {
    ConnectError = mrOk;
    Status = mrAbort;
    //break;
    return;
    }
  }

int Res = GetDeviceParams(ProcessBTH); // Запускаем считывание (прибор уже должен быть подключен)

if (Res == mrAbort || Res == mrCancel || ConnectError == mrAbort)
  {
  // Рвём соединение по BTH, если прибор подключен
  Disconnect(MODE_DIS_BTH);
  // Возвращяемся обратно
  ConnectError = mrOk;
  }
return;
}
//---------------------------------------------------------------------------

int __fastcall TFormAutoСalibration::GetDeviceParams (int AProcessBTH)
{
int Result = -1;

nReadBTH->Process(AProcessBTH);

switch (nReadBTH->EndResult)
  {
  case mrOk:
    {
    Result = 0;
    break;
    }
  case mrCancel:
    {
    return nReadBTH->EndResult;
    }
  case mrAbort:
    {
    return nReadBTH->EndResult;
    }
  }
FindListBase();

UpdateCalibFormData(); // Считывание информации из памяти прибора
StartBthThread();      // Запуск потоков BTH

if (GroupBoxDifference->Enabled) TimerDifference->Enabled = true;
else TimerDifference->Enabled = false;

return Result;
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::UpdateCalibFormData (void)
{
LabelMemModel->Caption = ProgReadBTH.sDeviceName;
LabelMemSN->Caption = ProgReadBTH.sDevSerial;
LabelMemSW->Caption = ProgReadBTH.sDevSw;
LabelMemHW->Caption = ProgReadBTH.sDevHw;

LabelMemCalibCoef11->Caption = ProgReadBTH.uDevCalibCoef[0][0];
LabelMemCalibCoef12->Caption = ProgReadBTH.uDevCalibCoef[0][1];
LabelMemCalibCoef13->Caption = ProgReadBTH.uDevCalibCoef[0][2];

LabelMemCalibCoef21->Caption = ProgReadBTH.uDevCalibCoef[1][0];
LabelMemCalibCoef22->Caption = ProgReadBTH.uDevCalibCoef[1][1];
LabelMemCalibCoef23->Caption = ProgReadBTH.uDevCalibCoef[1][2];
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::StartBthThread (void)
{
ProbaThread = new TProbaThread(true, this);
ProbaThread->BthSearchTime = BthConnectTime;
ProbaThread->Fifo = RawFifo;

ProbaThread->ProcessState = ppsIdle;
ProbaThread->ThreadCmd = 0;
ProbaThread->Resume();
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::StartManomThread (void)
{
ManomThread = new TManomThread(true, this);
ManomThread->ManomFifo = ManomRawFifo;
ManomThread->ComData = ComDataForm;
ManomThread->ProcessState = ppsIdle;
ManomThread->ThreadCmd = 0;
ManomThread->Resume();
}
//---------------------------------------------------------------------------

// расчет давления для датчика 1
// вход: беззнаковые данные с АЦП [0..X]
// результат: [мм]
int TFormAutoСalibration::CalcPres1Level (int Sample, int Index)
{
Sample = (Sample >= ProgReadBTH.uDevCalibCoef[Index][1]) ? (Sample - ProgReadBTH.uDevCalibCoef[Index][0]) : (0);
return ((Sample * ProgReadBTH.uDevCalibCoef[Index][2]));
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonOpenValveClick(TObject *Sender)
{
SendCommandBth(0x13);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonCloseValveClick(TObject *Sender)
{
SendCommandBth(0x14);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonEnPumpClick(TObject *Sender)
{
SendCommandBth(0x15);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonDisPumpClick(TObject *Sender)
{
SendCommandBth(0x16);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::SendCommandBth(int BthTreadCmd)
{
if (ProbaThread)
  {
  int Notify = THR_SEND_CMD_BTH;
  ProbaThread->SendThreadCmd(Notify, BthTreadCmd);
  }
else
  {
  String Hdr = L"Ошибка";
  String Msg = L"Прибор не подключен по BTH!";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonDisconnectClick(TObject *Sender)
{
Disconnect(MODE_DIS_BTH);
}
//---------------------------------------------------------------------------

int __fastcall TFormAutoСalibration::CalibProcess(void)
{
StopCalib = false;
bool Done = false;
int State = 1;
int Res = -1;

int PointCount = 0;
int CalibPointCount = 0;
int DeviceSens1 = 0;
int DeviceSens2 = 0;
int Sens1Zero = 0;
int Sens2Zero = 0;
TDateTime Timeout = Now();
TDateTime ManomTimeout = Now();
float ManomSens = 0.0;
float PrevManomSens = 0.0;
int StepPoint[5];

StepPoint[0] = RzSpinEditPoint1->Value;  // 5 kpa
StepPoint[1] = RzSpinEditPoint2->Value;  // 10
StepPoint[2] = RzSpinEditPoint3->Value;  // 20
StepPoint[3] = RzSpinEditPoint4->Value;  // 30
StepPoint[4] = RzSpinEditPoint5->Value;  // 40

double PumpTimeout = 10.0;
double SensTimeout = (double) (RzSpinEditDelay->Value);

int ResetThreshold = RzSpinEditResetThreshold->Value;

while (!Done)
  {
  Application->ProcessMessages();
  if (ResetThreshold < ManomSens) return Res = -1;

  if (StopCalib)
    {
    Done = true;
    Res = 1;
    StopCalib = false;
    }

  switch (State)
    {
    case 1: // подготовка к началу процесса калибровки
      {
      SendCommandBth(0x13); // открытие клапана
      ManomTimeout = Now() + (1.0 / (24.0 * 3600.0));
      State = 2;
      break;
      }
    case 2:
      {
      if (Now() < ManomTimeout) break;
      ProbaThread->GetSensValue(&DeviceSens1, &DeviceSens2);
      Sens1Zero = DeviceSens1;
      Sens2Zero = DeviceSens2;

      State = 4;
      break;
      }
    case 3: // сохранение значений 1...5 точек
      {
      ProbaThread->GetSensValue(&DeviceSens1, &DeviceSens2);
      ManomThread->GetSensValue(&ManomSens);

      CalibSampleTable1[CalibPointCount] = DeviceSens1;
      CalibSampleTable2[CalibPointCount] = DeviceSens2;

      ManomSensTable[CalibPointCount] = ManomSens;

      if (++CalibPointCount >= 5) { State = 9; } // если набрано нужное колличество точек - завершаем калибровку
      else State = 4;

      break;
      }
    case 4: // устанавливаем таймаут ожидания накачки, закрываем клапана и включаем компрессор
      {
      SendCommandBth(0x14); // закрытие клапана
      SendCommandBth(0x15); // включение компрессора
      Timeout = Now() + (PumpTimeout / (24.0 * 3600.0));
      State = 5;
      break;
      }
    case 5:
      {
      ProbaThread->GetSensValue(&DeviceSens1, &DeviceSens2);
      ManomThread->GetSensValue(&ManomSens);
      if (ManomSens > ((float)(StepPoint[PointCount] + 0.05))) // давление достигло необходимого значения
        {
        SendCommandBth(0x16); // выключение компрессора
        State = 6;
        PointCount++;
        }

      if (Now() >= Timeout) State = 99;
      break;
      }
    case 6: // отключаем компрессор и устанавливаем таймаут ожидания установления давления
      {
      ManomThread->GetSensValue(&ManomSens);
      PrevManomSens = ManomSens;

      Timeout = Now() + (SensTimeout / (24.0 * 3600.0));
      ManomTimeout = Now() + (0.4 / (24.0 * 3600.0));
      State = 7;
      break;
      }
    case 7: // ожидаем установления давления
      {
      LabelInfo->Caption = L"калибровка точки " + IntToStr(CalibPointCount + 1);
      State = 8;
      break;
      }
    case 8: // проверяем утечку
      {
      if (Now() < ManomTimeout) break;
      ManomTimeout = Now() + (0.4 / (24.0 * 3600.0));

      ManomThread->GetSensValue(&ManomSens);
      if ((fabs(ManomSens - PrevManomSens)) > 3.0) { State = 99; break; }             // проверка на отсоединение регистратора
      if ((fabs(ManomSens - PrevManomSens)) > 0.002) { PrevManomSens = ManomSens; }   // проверка на утечку
      else { State = 3; }

      if (Now() >= Timeout) State = 99;

      break;
      }
    case 9: // завершение калибровки
      {
      CalibCalcParams(Sens1Zero, CalibSampleTable1, ManomSensTable, 0, CalibPointCount); // расчет и запись коэфицентов калибровки 1-го датчика
      CalibCalcParams(Sens2Zero, CalibSampleTable2, ManomSensTable, 1, CalibPointCount); // расчет и запись коэфицентов калибровки 2-го датчика

      State = 99;

      Done = true;
      Res = 0;
      break;
      }
    case 99:
      {
      Done = true;
      break;
      }
    }
  }

SendCommandBth(0x13); // открытие клапана
SendCommandBth(0x16); // выключение компрессора
return Res;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::CalibCalcParams (int CalibZeroLevel, float *CalibSampleTable, float *CalibPresTable, u8 CalibSensIndex, int CalibPointCount)
{
// расчет коэфицентов калибровки
float NSum = CalibPointCount, XSum = 0, YSum = 0, X2Sum = 0, XYSum = 0;

for (int i = 0; i < CalibPointCount; i++)
  {
  float X = ((float) CalibPresTable[i]);
  float Y = (float) CalibSampleTable[i];

  XSum += X;
  YSum += Y;
  X2Sum += (X * X);
  XYSum += (X * Y);
  }

float D0 = (NSum * X2Sum) - (XSum * XSum);
if (!D0) D0 = 1.0;
float A1 = ((YSum * X2Sum) - (XYSum * XSum)) / D0;
float A2 = ((NSum * XYSum) - (XSum * YSum)) / D0;

int K0 = (int) Round(A1);
if (K0 < 0) K0 = 0;
int K1 = CalibZeroLevel;
int K2 = (int) Round((7.5 / A2) * 65536.0);
if (K1 < K0) K1 = K0;

DevCalibCoef[CalibSensIndex][0] = K0 & 0xFFFF;
DevCalibCoef[CalibSensIndex][1] = K1 & 0xFFFF;
DevCalibCoef[CalibSensIndex][2] = K2 & 0xFFFF;

// для дебага обновляем таблицу калибровки прибора
ProgReadBTH.uDevCalibCoef[CalibSensIndex][0] = DevCalibCoef[CalibSensIndex][0];
ProgReadBTH.uDevCalibCoef[CalibSensIndex][1] = DevCalibCoef[CalibSensIndex][1];
ProgReadBTH.uDevCalibCoef[CalibSensIndex][2] = DevCalibCoef[CalibSensIndex][2];
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonDefSetupClick(TObject *Sender)
{
ProgConfig.SetDefault();
ProgConfig.DBPath = GetCurrentDir() + L"\\CalibBase.csv";
SetParam();
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::VerificationSetBthDef(void)
{
ButtonUpdateDeviceList->Caption = L"Обновить";
RzEditSens1->Text = L"0";
RzEditSens2->Text = L"0";
RzEditSensMean->Text = L"0";

LabelMemModel->Caption = L"Нет данных";
LabelMemSN->Caption = L"Нет данных";
LabelMemSW->Caption = L"Нет данных";
LabelMemHW->Caption = L"Нет данных";

LabelMemCalibCoef11->Caption = L"0000";
LabelMemCalibCoef12->Caption = L"0000";
LabelMemCalibCoef13->Caption = L"0000";

LabelMemCalibCoef21->Caption = L"0000";
LabelMemCalibCoef22->Caption = L"0000";
LabelMemCalibCoef23->Caption = L"0000";
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::PngSpeedButtonStopCalibClick(TObject *Sender)
{
StopCalib = true;
}
//---------------------------------------------------------------------------


void __fastcall TFormAutoСalibration::FormClose(TObject *Sender, TCloseAction &Action)
{
ProgConfig.AutoCalibCfg.NameManomCom = ComboBoxPorts->Text;
ProgConfig.AutoCalibCfg.SpeedManomCom = ComboBoxSpeeds->Text;
ProgConfig.AutoCalibCfg.ManDelay = RzSpinEditManDelay->Text;
ProgConfig.AutoCalibCfg.Point1 = RzSpinEditPoint1->Text;
ProgConfig.AutoCalibCfg.Point2 = RzSpinEditPoint2->Text;
ProgConfig.AutoCalibCfg.Point3 = RzSpinEditPoint3->Text;
ProgConfig.AutoCalibCfg.Point4 = RzSpinEditPoint4->Text;
ProgConfig.AutoCalibCfg.Point5 = RzSpinEditPoint5->Text;
ProgConfig.AutoCalibCfg.DelayPres = RzSpinEditDelay->Text;
ProgConfig.AutoCalibCfg.ResetThreshold = RzSpinEditResetThreshold->Text;

ProgConfig.DBPath = RzEditBasePath->Text;

ProgConfig.CalibratorCfg.Surname = RzEditSurname->Text;
ProgConfig.CalibratorCfg.Name = RzEditName->Text;
ProgConfig.CalibratorCfg.Patron = RzEditPatron->Text;

ProgConfig.SaveConfig();
StopCalib = true;
int ModeDisconnect = MODE_DIS_BTH + MODE_DIS_MANOM;
Disconnect(ModeDisconnect);
}
//---------------------------------------------------------------------------
void __fastcall TFormAutoСalibration::wclRfCommClientConnect(TObject *Sender, const int Error)
{
if (Error == WCL_E_SUCCESS)
  {
  BthTimer = Now() + ((double) 1500 / (24.0 * 3600.0 * 1000.0));
  ConnectError = mrOk;
  }
else
  {
  ConnectErrorCode = Error;
  StartOperationTimer(10, 10);
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclRfCommClientData (System::TObject* Sender, const void * Buffer, const unsigned Size)
{
byte *Src = (byte*) Buffer;
int Count = Size;
while (Count--) BthFifo->PutByte(*Src++);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclBluetoothManagerDeviceFound(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address)
{
// запрашиваем тип устройства BTH
TwclBluetoothDeviceType BthType = DiscoveryState;

// запрашиваем имя устройства BTH
String DevName;
int Result = Radio->GetRemoteName(Address, DevName);
if (Result != WCL_E_SUCCESS) { return; }

// запрашиваем индекс адаптера BTH
int RadioIndex = GetBthRadioIndex(Radio);

// сравниваем выбранное и найденое устройство
if (!DeviceDefinition(DevName)) { AddNewBthDevice(DevName, Address, BthType, RadioIndex); }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclBluetoothManagerPinRequest(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address, String &Pin)
{
Pin = BTH_PIN_CODE;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ButtonOpenClick(TObject *Sender)
{
if (OpenDialog1->Execute())
  {
  ProgConfig.DBPath = OpenDialog1->FileName;
  std::auto_ptr <TFileStream> FileStream(new TFileStream(ProgConfig.DBPath, fmCreate | fmOpenWrite));
  if (FileStream->Size > SIZE_MEMORY)
    {
    String Hdr = L"Ошибка!";
    String Msg = L"Файл базы больше 200Мб! Открыть невозможно";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    return;
    }
  ReadBase();

  // проверку бы какую
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::WriteBase(void)
{
String aStr = L"";
aStr = ProgReadBTH.sDevChipId + L";";
TDateTime DateTime = Now();
aStr += DateTime.DateString() + L";";
aStr += DateTime.TimeString() + L";";
aStr += ProgReadBTH.sDeviceName + L";";
aStr += ProgReadBTH.sDevSerial + L";";
aStr += ProgReadBTH.sDevSw + L";";
aStr += ProgReadBTH.sDevHw + L";";
aStr += IntToStr(BthDevicesList[ListBoxDeviceBTH->ItemIndex].Addr) + L";";
aStr += FloatToStr(ManomSerial) + L";";
aStr += AUTOCALIBRATION_VERSION;
aStr += L";";
aStr += IntToStr((int)ProgReadBTH.uDevCalibCoef[0][0]) + L";";
aStr += IntToStr((int)ProgReadBTH.uDevCalibCoef[0][1]) + L";";
aStr += IntToStr((int)ProgReadBTH.uDevCalibCoef[0][2]) + L";";
aStr += IntToStr((int)ProgReadBTH.uDevCalibCoef[1][0]) + L";";
aStr += IntToStr((int)ProgReadBTH.uDevCalibCoef[1][1]) + L";";
aStr += IntToStr((int)ProgReadBTH.uDevCalibCoef[1][2]) + L";";

aStr += RzEditSurname->Text + L";";
aStr += RzEditName->Text + L";";
aStr += RzEditPatron->Text + L";";

if (FindIndex != -1)
  {
  ListBase->Strings[FindIndex] = aStr;
  }
else
  {
  ListBase->Add(aStr);
  }
DeleteFile(ProgConfig.DBPath);
ListBase->SaveToFile(ProgConfig.DBPath);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::FindListBase(void)
{
FindIndex = -1;
memset(&InfoBaseCalib, 0, sizeof(InfoBaseCalib));

InfoBaseCalib.ChipID = "Нет Данных";
InfoBaseCalib.Date = "Нет Данных";
InfoBaseCalib.Time = "Нет Данных";
InfoBaseCalib.DevName = "Нет Данных";
InfoBaseCalib.DevSerial = "Нет Данных";
InfoBaseCalib.DevSwVer = "Нет Данных";
InfoBaseCalib.DevHwVer = "Нет Данных";
InfoBaseCalib.BthAddr = "Нет Данных";
InfoBaseCalib.ManomSerial = "Нет Данных";
InfoBaseCalib.ProgVersion = "Нет Данных";
InfoBaseCalib.CalibCoef11 = "0000";
InfoBaseCalib.CalibCoef12 = "0000";
InfoBaseCalib.CalibCoef13 = "0000";
InfoBaseCalib.CalibCoef21 = "0000";
InfoBaseCalib.CalibCoef22 = "0000";
InfoBaseCalib.CalibCoef23 = "0000";
InfoBaseCalib.Surname = "Нет Данных";
InfoBaseCalib.Name = "Нет Данных";
InfoBaseCalib.Patron = "Нет Данных";

for (int i = 0; i < ListBase->Count; i++)
  {
  if (ListBase->Strings[i].Pos(ProgReadBTH.sDevChipId))
    {
    FindIndex = i;
    SplitBase(ListBase->Strings[FindIndex]);
    break;
    }
  }
SetDeviseParam();
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::SetDeviseParam()
{
InfoBaseCalib.ChipID;
InfoBaseCalib.Date;
InfoBaseCalib.Time;
LabelDbModel->Caption = InfoBaseCalib.DevName;
LabelDbSN->Caption = InfoBaseCalib.DevSerial;
LabelDbSW->Caption = InfoBaseCalib.DevSwVer;
LabelDbHW->Caption = InfoBaseCalib.DevHwVer;
InfoBaseCalib.ManomSerial;
InfoBaseCalib.ProgVersion;
LabelDbCalibCoef11->Caption = InfoBaseCalib.CalibCoef11;
LabelDbCalibCoef12->Caption = InfoBaseCalib.CalibCoef12;
LabelDbCalibCoef13->Caption = InfoBaseCalib.CalibCoef13;
LabelDbCalibCoef21->Caption = InfoBaseCalib.CalibCoef21;
LabelDbCalibCoef22->Caption = InfoBaseCalib.CalibCoef22;
LabelDbCalibCoef23->Caption = InfoBaseCalib.CalibCoef23;
InfoBaseCalib.Surname;
InfoBaseCalib.Name;
InfoBaseCalib.Patron;
}
//---------------------------------------------------------------------------
void TFormAutoСalibration::SplitBase (String FindStr)
{
// разделяем строку
TStringList *List = NULL;

try
  {
  List = new TStringList();

  List->StrictDelimiter = true;
  List->Delimiter = ';';
  List->DelimitedText = FindStr;

  if (List->Count)
    {
    InfoBaseCalib.ChipID = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.Date = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.Time = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.DevName = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.DevSerial = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.DevSwVer = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.DevHwVer = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.BthAddr = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.ManomSerial = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.ProgVersion = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.CalibCoef11 = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.CalibCoef12 = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.CalibCoef13 = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.CalibCoef21 = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.CalibCoef22 = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.CalibCoef23 = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.Surname = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.Name = List->Strings[0];
    List->Delete(0);
    }
  if (List->Count)
    {
    InfoBaseCalib.Patron = List->Strings[0];
    List->Delete(0);
    }
  }
catch (...)
  {
  }

if (List) delete List;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::ListBoxDeviceBTHMouseMove (TObject *Sender, TShiftState Shift, int X, int Y)
{
ItemBoxMouse = ListBoxDeviceBTH->ItemAtPos(TPoint(X, Y), true);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::RzPageControlCheckUpChanging(TObject *Sender,
          int NewIndex, bool &AllowChange)
{
if (NewIndex == 1)
  {
  int LenSurname = RzEditSurname->Text.Trim().Length();
  int LenName = RzEditName->Text.Trim().Length();
  int LenPatron = RzEditPatron->Text.Trim().Length();

  if ((!LenSurname), (!LenName), (!LenPatron))
    {
    AllowChange = false;
    String Hdr = L"Ошибка!";
    String Msg = L"Введите ФИО!";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::TimerDifferenceTimer(TObject *Sender)
{
float Sens1, Sens2, SensMean;

switch(RadioGroupDifference->ItemIndex)
  {
  case 1:
    {
    Sens1 = (float)CalcPres1Level(DevCalib1, 0) / 65536.0;
    Sens2 = (float)CalcPres1Level(DevCalib2, 1) / 65536.0;
    SensMean = abs(ManomCalib * 7.501 - (Sens1 + Sens2) / 2.0);
    RzEditDifference->Text = FloatToStrF(SensMean, ffFixed, 8, 3);
    break;
    }
  default:
    {
    Sens1 = (float)CalcPres1Level(DevCalib1, 0) / (65536.0 * 7.501);
    Sens2 = (float)CalcPres1Level(DevCalib2, 1) / (65536.0 * 7.501);
    SensMean = abs(ManomCalib - (Sens1 + Sens2) / 2.0);
    RzEditDifference->Text = FloatToStrF(SensMean, ffFixed, 8, 3);
    break;
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclBluetoothManagerDiscoveringCompleted(TObject *Sender,
          TwclBluetoothRadio * const Radio, const int Error)
{
// запускаем второй шаг поиска
if (DiscoveryState == dkClassic)
  {
  // запускаем второй шаг поиска устройств (dlBle)
  StartOperationTimer(2, 10);
  }
else
  {
  // завершение поиска устройств
  StartOperationTimer(3, 10);
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclBluetoothManagerNumericComparison(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address, const DWORD Number,
          bool Confirm)
{
// Accept any pairing.
Confirm = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclBluetoothManagerPasskeyRequest(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address, DWORD Passkey)

{
// Use 123456 as passkey.
Passkey = 123456;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclGattClientConnect(TObject *Sender, const int Error)

{
if (Error == WCL_E_SUCCESS)
  {
  StartOperationTimer(11, 10);
  }
else
  {
  ConnectErrorCode = Error;
  StartOperationTimer(13, 10);
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclGattClientDisconnect(TObject *Sender, const int Reason)

{
((TwclGattClient*) Sender)->Radio->RemoteUnpair(((TwclGattClient*) Sender)->Address);
}
//---------------------------------------------------------------------------

TwclBluetoothRadio* __fastcall TFormAutoСalibration::GetRadio (int AType)
{
TwclBluetoothRadio* Radio;

if (AType != dkBle)
  {
  int Result = wclBluetoothManager->GetClassicRadio(Radio);
  if (Result != WCL_E_SUCCESS) Radio = NULL;
  }
else
  {
  int Result = wclBluetoothManager->GetLeRadio(Radio);
  if (Result != WCL_E_SUCCESS) Radio = NULL;
  }

return Radio;
}
//---------------------------------------------------------------------------

int TFormAutoСalibration::GetClientState (void)
{
// текущее состояние подключения
return (BthDeviceInfo.Type != dkBle) ? (wclRfCommClient->State) : (wclGattClient->State);
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::StartOperationTimer (int ACode, int ATime)
{
// заряжаем таймер для микроопераций
TimerOperation->Enabled = 0;

TimerOperationCode = ACode;
TimerOperation->Interval = ATime;

TimerOperation->Enabled = 1;
}
//---------------------------------------------------------------------------

int TFormAutoСalibration::GetBthRadioIndex (TwclBluetoothRadio *ARadio)
{
// получаем индекс адаптера BTH в списке адаптеров (по умолчанию = 0)
int Index = 0;

for (int i = 0; i < wclBluetoothManager->Count; i++)
  {
  if (wclBluetoothManager->Radios[i] == ARadio) { Index = i; break; }
  }

return Index;
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::AddNewBthDevice (String AName, __int64 AAddr, int ABthType, int ARadioIndex)
{
// если имя пустое, подставляем MAC-адрес
AName = AName.Trim();
if (AName == L"") AName = BthMacToStr(AAddr);

// добавляем устройство BTH в список обнаруженных
TBthDeviceInfo Info;
Info.Name = AName;
Info.Addr = AAddr;
Info.Type = ABthType;
Info.RadioIndex = ARadioIndex;
BthDevicesList.push_back(Info);

// выводим устройство в список для выбора
switch (ABthType)
  {
  case dtClassic: { AName = AName + "  (Bth: Classic)"; break; }
  case dtBle:     { AName = AName + "  (Bth: BLE)"; break; }
  case dtMixed:   { AName = AName + "  (Bth: Mixed)"; break; }
  default:        { AName = AName + "  (Bth: Unknown)"; break; }
  }

ListBoxDeviceBTH->Items->Add(AName);
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::TimerOperationTimer(TObject *Sender)
{
// обработка микроопераций
TimerOperation->Enabled = 0;

switch (TimerOperationCode)
  {
  case 0: // переключаем страницу "инфо о считывании >> данные пациента"
    {
    break;
    }

  case 1: // запуск поиска устройств BTH в режиме dkClassic
    {
    TwclBluetoothRadio *Radio = GetRadio(dkClassic);
    if (!Radio)
      {
      String Hdr = str_Error;
      String Msg = L"Адаптер Bluetooh не подключён.";
      MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
      Disconnect(MODE_DIS_BTH);
      return;
      }

    // ищем устройства dkClassic
    DiscoveryState = dkClassic;

    int Result = Radio->Discover(10, dkClassic);
    if (Result == WCL_E_SUCCESS)
      {
      BthSearchInProgress = 1;
      return;
      }
    else if (Result == WCL_E_BLUETOOTH_HARDWARE_NOT_AVAILABLE)
      {
      String Hdr = str_Error;
      String Msg = L"Адаптер Bluetooh не подключён.";
      MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
      Disconnect(MODE_DIS_BTH);
      }
    else
      {
      String Hdr = str_Error;
      String Msg = L"Ошибка при запуске поиска устройств Bluetooh (code " + IntToHex(Result, 8) + ")";
      MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
      Disconnect(MODE_DIS_BTH);
      }
    break;
    }

  case 2: // запуск поиска устройств BTH в режиме dkBle
    {
    TwclBluetoothRadio *Radio = GetRadio(dkBle);
    if (Radio)
      {
      // ищем устройства dkBle
      DiscoveryState = dkBle;

      int Result = Radio->Discover(10, dkBle);
      if (Result == WCL_E_SUCCESS)
        {
        BthSearchInProgress = 1;
        return;
        }
      }

    StartOperationTimer(3, 10);
    break;
    }

  case 3: // завершение поиска устройств BTH
    {
    BthSearchInProgress = 0;
    ButtonUpdateDeviceList->Caption = L"Обновить";
    ListBoxDeviceBTH->Enabled = true;
    SetSearchBthLabel(L"Поиск завершен", 0);

    if (!BthDevicesList.size())
      {
      ListBoxDeviceBTH->Items->Add(L"Устройства не найдены");
      }
    else
      {
      ListBoxDeviceBTH->ItemIndex = 0;
      }

    break;
    }

  case 10: // неудачное подключение к устройству Bth/Classic
    {
    ErrorInfo = L"Ошибка при подключении устройства (code " + IntToHex(ConnectErrorCode, 8) + ")";

    String Hdr = str_Error;
    String Msg = ErrorInfo;
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);

    ConnectError = mrAbort;
    break;
    }

  case 11: // считывание и настройка сервисов Bth/Ble
    {
    int Status = SetGattParams();
    if (Status) { StartOperationTimer(12, 10); break; }

    ConnectError = mrOk;
    break;
    }

  case 12: // отключение Bth/Ble
    {
    wclGattClient->Disconnect();
    ErrorInfo = L"Сбой при подключении к устройству:\n" + ErrorInfo;

    String Hdr = str_Error;
    String Msg = ErrorInfo;
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);

    ConnectError = mrAbort;
    break;
    }

  case 13:
    {
    ErrorInfo = L"Ошибка при подключении устройства (code " + IntToHex(ConnectErrorCode, 8) + ")";

    String Hdr = str_Error;
    String Msg = ErrorInfo;
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);

    ConnectError = mrAbort;
    break;
    }
  }
}
//---------------------------------------------------------------------------

String TFormAutoСalibration::BthMacToStr (__int64 AAddr)
{
// преобразуем MAC-адрес устройства в строку
String Result;

for (int i = 0; i < 6; i++)
  {
  int Value = (AAddr >> (i * 8)) & 0xFF;
  if (Result != L"") Result = L":" + Result;
  Result = IntToHex(Value, 2) + Result;
  }

return Result;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::btDisconnect (void)
{
// отключаем устройство BTH и все программные инструменты
if (ProbaThread)
  {
  ProbaThread->Terminate();
  while (ProbaThread) { Application->ProcessMessages(); Sleep(10); }
  }

ButtonUpdateDeviceList->Caption = L"Обновить";

if (wclBluetoothManager->Active)
  {
  wclRfCommClient->OnData = wclRfCommClientData;
  wclGattClient->OnCharacteristicChanged = wclGattClientCharacteristicChanged;
  }
wclRfCommClient->Disconnect();
wclGattClient->Disconnect();

// завершаем поиск если активен
TwclBluetoothRadio *Radio = GetRadio(dkClassic);
if (Radio) { if (Radio->Discovering) Radio->Terminate(); }
Radio = GetRadio(dkBle);
if (Radio) { if (Radio->Discovering) Radio->Terminate(); }
DiscoveryState = -1;

SetSearchBthLabel(L"Поиск завершен", 0);
}
//---------------------------------------------------------------------------

void TFormAutoСalibration::SetSearchBthLabel (String AInfo, int AState)
{
LabelSearchDev->Caption = AInfo;

int W1 = LabelSearchDev->Width;
int W2 = (AState) ? (16) : (0);
int W3 = 0;

int W = W1 + W2 + W3;
int X = (GroupBoxSearchDev->Width / 2) - (W / 2);
LabelSearchDev->Left = X;
}
//---------------------------------------------------------------------------

int TFormAutoСalibration::SetGattParams (void)
{
int Status = 1;
BthTimer = Now() + ((double) 1500 / (24.0 * 3600.0 * 1000.0));

do
  {
  // GUID сервиса UART для BLE/Nordic
  String UartServiceGuid = L"{6E400001-B5A3-F393-E0A9-E50E24DCCA9E}";
  String UartCharWrGuid  = L"{6E400002-B5A3-F393-E0A9-E50E24DCCA9E}";
  String UartCharRdGuid  = L"{6E400003-B5A3-F393-E0A9-E50E24DCCA9E}";

  // [запрос и обработка списка сервисов]

  ServiceList.Length = 0;
  int Result = wclGattClient->ReadServices(BleOperFlag(), ServiceList);
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"Ошибка при получении списка сервисов (code " + IntToHex(Result, 8) + ")";
    break;
    }

  if (ServiceList.Length == 0)
    {
    ErrorInfo = L"Ошибка в размере списка сервисов (список пуст)";
    break;
    }

  // проверка списка сервисов
  int UartServiceFound = 0;

  for (int i = 0; i < ServiceList.Length; i++)
    {
    TwclGattService Service = ServiceList[i];
    if (Service.Uuid.IsShortUuid) continue;

    String Str = Sysutils::GUIDToString(Service.Uuid.LongUuid);
    if (Str == UartServiceGuid)
      {
      UartServiceFound = 1;
      UartService = Service;
      break;
      }
    }

  if (!UartServiceFound)
    {
    ErrorInfo = L"Cервис UART отсутствует в списке сервисов";
    break;
    }

  // [запрос и обработка списка характеристик]

  CharacteristicList.Length = 0;
  Result = wclGattClient->ReadCharacteristics(UartService, BleOperFlag(), CharacteristicList);
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"Ошибка при получении списка характеристик (code " + IntToHex(Result, 8) + ")";
    break;
    }

  if (CharacteristicList.Length == 0)
    {
    ErrorInfo = L"Ошибка в размере списка характеристик (список пуст)";
    break;
    }

  // проверка списка характеристик
  int UartCharWrFound = 0;
  int UartCharRdFound = 0;

  for (int i = 0; i < CharacteristicList.Length; i++)
    {
    TwclGattCharacteristic Character = CharacteristicList[i];
    if (Character.Uuid.IsShortUuid) continue;

    String Str = Sysutils::GUIDToString(Character.Uuid.LongUuid);
    if (Str == UartCharWrGuid) { UartCharWrFound = 1; UartCharWr = Character; }
    if (Str == UartCharRdGuid) { UartCharRdFound = 1; UartCharRd = Character; }
    }

  if ((!UartCharWrFound) || (!UartCharRdFound))
    {
    ErrorInfo = L"Характеристики UART отсутствует в списке характеристик";
    break;
    }

  // [подключение оповещений]

  Result = wclGattClient->Subscribe(UartCharRd);
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"Ошибка при подключении оповещений (code " + IntToHex(Result, 8) + ")";
    break;
    }

  Result = wclGattClient->WriteClientConfiguration(UartCharRd, true, BleOperFlag());
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"Ошибка при настройке оповещений (code " + IntToHex(Result, 8) + ")";
    break;
    }

  Status = 0;
  }
while (0);

return Status;
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::wclGattClientCharacteristicChanged (TObject *Sender,
  const WORD Handle, const TwclGattCharacteristicValue Value)
{
// переносим блок данных в FIFO приема
int Count = Value.Length;
if (Count)
  {
  for (int i = 0; i < Count; i++) BthFifo->PutByte(Value[i]);
  }
}
//---------------------------------------------------------------------------

TwclGattOperationFlag TFormAutoСalibration::BleOperFlag (void)
{
// режим работы BLE устройства (пока не используется)
int Index = 0;
switch (Index)
  {
  case 1:  { return goReadFromDevice; }
  case 2:  { return goReadFromCache; }
  default: { return goNone; }
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAutoСalibration::FormDestroy(TObject *Sender)
{
Disconnect(MODE_DIS_BTH);
wclBluetoothManager->Close();
ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

int TFormAutoСalibration::SetClientData (void *Data, unsigned Size, unsigned &Written)
{
int Result;

if (BthDeviceInfo.Type != dkBle)
  {
  // запись блока данных в устройство типа dkClassic
  Result = wclRfCommClient->Write(Data, Size, Written);
  }
else
  {
  // запись блока данных в устройство типа dkBle
  TwclGattCharacteristicValue Value;
  Value.Length = Size;

  byte *Src = (byte*) Data;
  int Count = Size;
  for (int i = 0; i < Count; i++) { Value[i] = Src[i]; }
  Written = Size;

  Result = wclGattClient->WriteCharacteristicValue(UartCharWr, Value);
  }

return Result;
}
//---------------------------------------------------------------------------

