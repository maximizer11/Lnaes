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

TFormAuto�alibration *FormAuto�alibration;
//---------------------------------------------------------------------------

__fastcall TFormAuto�alibration::TFormAuto�alibration(TComponent* Owner)
        : TForm(Owner)
{
str_Error = L"������";

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
void __fastcall TFormAuto�alibration::FormCreate(TObject *Sender)
{
UpdatePortList();
String FileSet = ChangeFileExt(ParamStr(0), L".ini");

// �������� �������� ���������
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
OpenDialog1->Filter = L"��������� ���� (*.csv)|*.CSV";
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

void TFormAuto�alibration::SetParam(void)
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

void __fastcall TFormAuto�alibration::ReadBase(void)
{
TFileStream *FileStream = NULL;
if (FileExists(ProgConfig.DBPath)) FileStream = new TFileStream(ProgConfig.DBPath, fmOpenWrite);
else FileStream = new TFileStream(ProgConfig.DBPath, fmCreate | fmOpenWrite);
__int64 FileBaseSize = FileStream->Size;
if (FileStream) delete FileStream;
if (FileBaseSize > SIZE_MEMORY)
  {
  String Hdr = L"������!";
  String Msg = L"���� ���� ������ 200��! ������� ����������";
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
  String Str = L"ID ����;����,�����,���;�����;������;�������� �����;������ ��������;������ ������.;��� ���;S/N ���������;AvtoCalibration v.;1�1�;2�1�;3�1�;1�2�;2�2�;3�2�;�������;���;��������";
  ListBase->Add(Str);
  }
}
//---------------------------------------------------------------------------


void __fastcall TFormAuto�alibration::UpdatePortList(void)
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


void __fastcall TFormAuto�alibration::ButtonOpenPortClick(TObject *Sender)
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
  //�������� �������� ����� ��������� (���� �� ������ �� ������� �����)
  TComStatus Status = ComSendCmdWithReply(MAN_GET_SERIAL_CMD);

  // ���� ������ �� COM_OK - ������ �����, ��� �������� �� ����� �� ������
  if (Status != COM_OK)
    {
    String Msg = GetComStatusString(Status);
    ShowMessage(L"�������� �� ������, " + Msg);
    ClosePort();
    }
  else
    {
    unsigned Value = (ComDataForm->Data[0] << 24) | (ComDataForm->Data[1] << 16) | (ComDataForm->Data[2] << 8) | ComDataForm->Data[3];
    ManomSerial = reinterpret_cast<float &>(Value);

    LabelStatusCOM->Caption = L"�������� ��������� S/N: " + FloatToStr(ManomSerial);
    LabelStatusCOM->Font->Color = clGreen;
    LabelStatusCOM->Left = (GroupBoxSetCOM->Width / 2) - (LabelStatusCOM->Width / 2);

    ComSendCmdWithReply(MAN_SET_ZERO_CMD); // ����� ���������
    StartManomThread();                    // ������ ������ ���������
    }
  }
else
  {
  LabelStatusCOM->Caption = L"������ ����������� ���������";
  LabelStatusCOM->Font->Color = clRed;
  LabelStatusCOM->Left = (GroupBoxSetCOM->Width / 2) - (LabelStatusCOM->Width / 2);
  }
}
//---------------------------------------------------------------------------

int TFormAuto�alibration::OpenPort (String APortName, int APortSpeed)
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

void __fastcall TFormAuto�alibration::ButtonClosePortClick(TObject *Sender)
{
Disconnect(MODE_DIS_MANOM);
}
//---------------------------------------------------------------------------

void TFormAuto�alibration::ClosePort (void)
{
ComboBoxSpeeds->Enabled = true;
ComboBoxPorts->Enabled = true;
ButtonOpenPort->Enabled = true;
ButtonClosePort->Enabled = false;
Port.Close();
LabelStatusCOM->Caption = L"�������� ��������";
LabelStatusCOM->Font->Color = clRed;
LabelStatusCOM->Left = (GroupBoxSetCOM->Width / 2) - (LabelStatusCOM->Width / 2);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonSendBinClick(TObject *Sender)
{
String Hdr = L"��������!";
String Msg = L"�� ������� ��� ������ ������ ������� ����������?\r\n";
if (!ProbaThread || !ManomThread)
  {
  Hdr = L"������!";
  Msg = L"";
  if (!ProbaThread) Msg = L"������ �� ��������� �� BTH! ";
  if (!ManomThread) Msg += L"�������� �� ���������!";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);
  return;
  }
int Result = MessageBox(Handle, Msg.w_str(), Hdr.w_str(), MB_YESNO | MB_ICONWARNING);
if (Result == IDNO) return;
LabelInfo->Caption = L"������ �������� ����������";

ButtonUpdateDeviceList->Enabled = false;
ButtonDisconnect->Enabled = false;
TabSheetSettings->Enabled = false;
PngSpeedButtonStopCalib->Enabled = true;

int Status = CalibProcess();

switch (Status)
  {
  case 0: // ������� ���������
    {
    Disconnect(MODE_DIS_BTH);
    BthConnectProcess(0);
    WriteBase(); // ������ � ����
    FindListBase(); // ������
    LabelInfo->Caption = L"���������� ������� ���������";
    Msg = L"���������� ������� ���������!";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    break;
    }
  case 1: // �������� �������������
    {
    LabelInfo->Caption = L"���������� �������� �������������";
    Msg = L"���������� �������� �������������!";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    break;
    }
  default:
    {
    LabelInfo->Caption = L"������ ��� ����������";
    Hdr = L"������!";
    Msg = L"������ ��� ����������!";
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

TComStatus TFormAuto�alibration::ComSendCmdWithReply (int Index)
{

int RxDataCount = 0, Count = 0;
TComStatus Status = COM_OK;

switch (Index)
  {
  case MAN_GET_PRES_CMD:   { RxDataCount = 19; break; } // ������� "������� ��������� ���������� (��������)"
  case MAN_SET_ZERO_CMD:   { RxDataCount = 15; break; } // ������� "��������� ��������� ����"
  case MAN_GET_SERIAL_CMD: { RxDataCount = 22; break; } // ������� "�������� � �������"
  default: { return Status = COM_ERR_UNK_CMD; }         // ����������� �������
  }

// ���������� �������
Status = ComSendCmd(Index);
if (Status != COM_OK) { return Status; }

// �������
Sleep(RzSpinEditManDelay->Value);

// �������� �����
while (1)
  {
  if (RxDataCount <= 0) { break; }
  Port.Read(&TmpBuffer, sizeof(TmpBuffer), Count);
  if (!Count) { return Status = COM_ERR_EMPTY_REPLY; }
  if (Count > RxDataCount) { return Status = COM_ERR_LONG_REPLY; }
  RxDataCount -= Count;
  }

// ������ �����
Status = ComParseCmd((unsigned char*) &TmpBuffer, ComDataForm);
if (Status != COM_OK) { return Status; }

return Status;
}
//---------------------------------------------------------------------------

TComStatus TFormAuto�alibration::ComSendCmd (int Index)
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

// ���������� ������� � ������
Words->SetNewString(CmdStr);

// ������ TX-�����
memset(&DataBuffer, 0, sizeof(DataBuffer));

while (1)
  {
  // ������ ������� �� ����������� ������
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

  // ��������� TX-�����
  DataBuffer[DataSize++] = Data;
  if (DataSize >= 256) { Status = COM_ERR_LONG_CMD; break; }
  }

// ���������� ������� � ����
if (DataSize > 0)
  {
  if (Port.Write(&DataBuffer, DataSize)) { Status = COM_ERR_WRITE; }
  }

delete Words;
return Status;
}
//---------------------------------------------------------------------------

TComStatus TFormAuto�alibration::ComParseCmd(unsigned char *Src, TComData *AComData)
{
// �������� ������������ ���������
if (Src[0] != 0xFF) return COM_ERR_WRONG_PREAMB;
if (Src[1] != 0xFF) return COM_ERR_WRONG_PREAMB;
if (Src[2] != 0xFF) return COM_ERR_WRONG_PREAMB;

// �������� ������������ ���������� ����
if ((Src[3] != 0x82) && (Src[3] != 0x86)) return COM_ERR_WRONG_SBIT;

// �������� ������������ ������� ������
int DataSize = Src[10];
if ((DataSize == 0) || (DataSize > 25)) return COM_ERR_WRONG_DATASIZE;

// �������� ������������ ����������� �����
byte CheckSum = Src[13 + DataSize];
byte RealCheckSum = ChecksumXOR((byte*) &Src[3], 10 + DataSize);
if (CheckSum != RealCheckSum) return COM_ERR_WRONG_CRC;

// ������� ���������
memset(AComData, 0x00, sizeof(TComData));

// ��������� ���� ���������
memcpy(&AComData->Preamb, &Src[0], sizeof(AComData->Preamb));

// ��������� ���� ���������� �������
AComData->StartSymb = Src[3];

// ��������� ���� ������
memcpy(&AComData->Addr, &Src[4], sizeof(AComData->Addr));

// ��������� ���� �������
AComData->Cmd = Src[9];

// ��������� ���� ������� �������� ������
AComData->DataSize = DataSize;

// ��������� ���� �������
memcpy(&AComData->Status, &Src[11], sizeof(AComData->Status));

// ��������� ���� ������ � �������
memcpy(&AComData->Data, &Src[13], DataSize);

// ��������� ���� ����������� �����
AComData->CheckSum = CheckSum;

return COM_OK;
}
//---------------------------------------------------------------------------

String TFormAuto�alibration::GetComStatusString(TComStatus Status)
{
String Msg;

switch (Status)
  {
  case COM_OK:                 { Msg = L"������ �� ����������!"; break; }
  case COM_ERR_UNK_CMD:        { Msg = L"������! ����������� �������!"; break; }
  case COM_ERR_WRONG_CMD:      { Msg = L"������! ������������ �������!"; break; }
  case COM_ERR_LONG_CMD:       { Msg = L"������! ������� ������� �������!"; break; }
  case COM_ERR_WRONG_PREAMB:   { Msg = L"������! ������������ ���������!"; break; }
  case COM_ERR_WRONG_SBIT:     { Msg = L"������! ������������ START-���!"; break; }
  case COM_ERR_WRONG_DATASIZE: { Msg = L"������! ������������ ������ ������!"; break; }
  case COM_ERR_WRONG_CRC:      { Msg = L"������! ������������ CRC-�����!"; break; }
  case COM_ERR_WRITE:          { Msg = L"������! ������ ������ � COM-����!"; break; }
  case COM_ERR_EMPTY_REPLY:    { Msg = L"������! ����� ����!"; break; }
  case COM_ERR_LONG_REPLY:     { Msg = L"������! ������� ������� �����!"; break; }
  default:                     { Msg = L"������! ����������� ������!"; break; }
  }

return Msg;
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonUpdateDeviceListClick(TObject *Sender)
{

if (ButtonUpdateDeviceList->Caption == L"��������")
  {
  ListBoxDeviceBTH->Enabled = false;
  BthDevicesList.clear();
  ListBoxDeviceBTH->Clear();

  SetSearchBthLabel(L"��� ����� ���������", 1);
  //ActivityIndicatorBth->Animate = true;
  ButtonUpdateDeviceList->Caption = L"����";

  Application->ProcessMessages();
  if (wclBluetoothManager->Active) wclBluetoothManager->Close();
  //wclBluetoothManager->Close();

  int Result = wclBluetoothManager->Open();
  if (Result != WCL_E_SUCCESS)
    {
    String Hdr = str_Error;
    String Msg = L"������ ��� ����������� ����� Bluetooh (code " + IntToHex(Result, 8) + ")";
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

void TFormAuto�alibration::StartBthDiscovery (void)
{
// ��������� ������ ��� ������ ��������� (dlClassic)
StartOperationTimer(1, 10);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::Disconnect (int AModeDisconnect)
{
TimerDifference->Enabled = false;
if (AModeDisconnect & MODE_DIS_BTH)
  {
  TDateTime TimerDisProba = Now() + (0.5 / (24.0 * 3600.0));
  if (ProbaThread)
    {
    SendCommandBth(0x13); // �������� �������
    SendCommandBth(0x16); // ���������� �����������
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

int TFormAuto�alibration::DeviceDefinition (String ADevice)
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

bool __fastcall TFormAuto�alibration::BthConnectDevice (void)
{
int Status = 1, Result;
int Index = ListBoxDeviceBTH->ItemIndex;

if (Index == -1)
  {
  String Hdr = str_Error;
  String Msg = L"�������� ���������� ��� �����������";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
  return Status;
  }

// ������ � ������������ ����������
BthDeviceInfo = BthDevicesList[Index];

TwclBluetoothRadio *Radio = GetRadio(BthDeviceInfo.Type);
if (!Radio)
  {
  String Hdr = str_Error;
  String Msg = L"������� Bluetooh �� ���������.";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
  return Status;
  }

if (BthDeviceInfo.Type != dkBle)
  {
  // ���������� ��� dlClassic
  wclRfCommClient->Address = BthDeviceInfo.Addr;
  wclRfCommClient->Authentication = true;
  wclRfCommClient->Encryption = true;
  wclRfCommClient->Timeout = BthConnectTime * 1000;

  Result = wclRfCommClient->Connect(Radio);
  if (Result == WCL_E_SUCCESS) { return 0; }
  }
else
  {
  // ���������� ��� dlBle
  wclGattClient->Address = BthDeviceInfo.Addr;
  wclGattClient->ConnectOnRead = false;

  Result = wclGattClient->Connect(Radio);
  if (Result == WCL_E_SUCCESS) { return 0; }
  }

String Hdr = str_Error;
String Msg = L"������ ��� ������� ����������� � ���������� (code " + IntToHex(Result, 8) + ")";
MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);

return Status;
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ListBoxDeviceBTHDblClick(TObject *Sender)
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

void __fastcall TFormAuto�alibration::ListBoxDeviceBTHKeyPress(TObject *Sender, System::WideChar &Key)
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

void TFormAuto�alibration::BthConnectProcess(int ProcessBTH)
{
int Status = mrAbort;
LabelInfo->Caption = L"����������...";

// ���������� ��� ��������� ���������� �� ��������� ���������� ���

// ���������� ��������� ���������� ���
ConnectError == mrNone;
int Result = BthConnectDevice(); // ���������� ��������� ���������� ���
if (Result)
  {
  ConnectError = mrOk;
  }

while (1)
  {
  // ������� ����������� ����������
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

int Res = GetDeviceParams(ProcessBTH); // ��������� ���������� (������ ��� ������ ���� ���������)

if (Res == mrAbort || Res == mrCancel || ConnectError == mrAbort)
  {
  // ��� ���������� �� BTH, ���� ������ ���������
  Disconnect(MODE_DIS_BTH);
  // ������������ �������
  ConnectError = mrOk;
  }
return;
}
//---------------------------------------------------------------------------

int __fastcall TFormAuto�alibration::GetDeviceParams (int AProcessBTH)
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

UpdateCalibFormData(); // ���������� ���������� �� ������ �������
StartBthThread();      // ������ ������� BTH

if (GroupBoxDifference->Enabled) TimerDifference->Enabled = true;
else TimerDifference->Enabled = false;

return Result;
}
//---------------------------------------------------------------------------

void TFormAuto�alibration::UpdateCalibFormData (void)
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

void TFormAuto�alibration::StartBthThread (void)
{
ProbaThread = new TProbaThread(true, this);
ProbaThread->BthSearchTime = BthConnectTime;
ProbaThread->Fifo = RawFifo;

ProbaThread->ProcessState = ppsIdle;
ProbaThread->ThreadCmd = 0;
ProbaThread->Resume();
}
//---------------------------------------------------------------------------

void TFormAuto�alibration::StartManomThread (void)
{
ManomThread = new TManomThread(true, this);
ManomThread->ManomFifo = ManomRawFifo;
ManomThread->ComData = ComDataForm;
ManomThread->ProcessState = ppsIdle;
ManomThread->ThreadCmd = 0;
ManomThread->Resume();
}
//---------------------------------------------------------------------------

// ������ �������� ��� ������� 1
// ����: ����������� ������ � ��� [0..X]
// ���������: [��]
int TFormAuto�alibration::CalcPres1Level (int Sample, int Index)
{
Sample = (Sample >= ProgReadBTH.uDevCalibCoef[Index][1]) ? (Sample - ProgReadBTH.uDevCalibCoef[Index][0]) : (0);
return ((Sample * ProgReadBTH.uDevCalibCoef[Index][2]));
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonOpenValveClick(TObject *Sender)
{
SendCommandBth(0x13);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonCloseValveClick(TObject *Sender)
{
SendCommandBth(0x14);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonEnPumpClick(TObject *Sender)
{
SendCommandBth(0x15);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonDisPumpClick(TObject *Sender)
{
SendCommandBth(0x16);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::SendCommandBth(int BthTreadCmd)
{
if (ProbaThread)
  {
  int Notify = THR_SEND_CMD_BTH;
  ProbaThread->SendThreadCmd(Notify, BthTreadCmd);
  }
else
  {
  String Hdr = L"������";
  String Msg = L"������ �� ��������� �� BTH!";
  MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonDisconnectClick(TObject *Sender)
{
Disconnect(MODE_DIS_BTH);
}
//---------------------------------------------------------------------------

int __fastcall TFormAuto�alibration::CalibProcess(void)
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
    case 1: // ���������� � ������ �������� ����������
      {
      SendCommandBth(0x13); // �������� �������
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
    case 3: // ���������� �������� 1...5 �����
      {
      ProbaThread->GetSensValue(&DeviceSens1, &DeviceSens2);
      ManomThread->GetSensValue(&ManomSens);

      CalibSampleTable1[CalibPointCount] = DeviceSens1;
      CalibSampleTable2[CalibPointCount] = DeviceSens2;

      ManomSensTable[CalibPointCount] = ManomSens;

      if (++CalibPointCount >= 5) { State = 9; } // ���� ������� ������ ����������� ����� - ��������� ����������
      else State = 4;

      break;
      }
    case 4: // ������������� ������� �������� �������, ��������� ������� � �������� ����������
      {
      SendCommandBth(0x14); // �������� �������
      SendCommandBth(0x15); // ��������� �����������
      Timeout = Now() + (PumpTimeout / (24.0 * 3600.0));
      State = 5;
      break;
      }
    case 5:
      {
      ProbaThread->GetSensValue(&DeviceSens1, &DeviceSens2);
      ManomThread->GetSensValue(&ManomSens);
      if (ManomSens > ((float)(StepPoint[PointCount] + 0.05))) // �������� �������� ������������ ��������
        {
        SendCommandBth(0x16); // ���������� �����������
        State = 6;
        PointCount++;
        }

      if (Now() >= Timeout) State = 99;
      break;
      }
    case 6: // ��������� ���������� � ������������� ������� �������� ������������ ��������
      {
      ManomThread->GetSensValue(&ManomSens);
      PrevManomSens = ManomSens;

      Timeout = Now() + (SensTimeout / (24.0 * 3600.0));
      ManomTimeout = Now() + (0.4 / (24.0 * 3600.0));
      State = 7;
      break;
      }
    case 7: // ������� ������������ ��������
      {
      LabelInfo->Caption = L"���������� ����� " + IntToStr(CalibPointCount + 1);
      State = 8;
      break;
      }
    case 8: // ��������� ������
      {
      if (Now() < ManomTimeout) break;
      ManomTimeout = Now() + (0.4 / (24.0 * 3600.0));

      ManomThread->GetSensValue(&ManomSens);
      if ((fabs(ManomSens - PrevManomSens)) > 3.0) { State = 99; break; }             // �������� �� ������������ ������������
      if ((fabs(ManomSens - PrevManomSens)) > 0.002) { PrevManomSens = ManomSens; }   // �������� �� ������
      else { State = 3; }

      if (Now() >= Timeout) State = 99;

      break;
      }
    case 9: // ���������� ����������
      {
      CalibCalcParams(Sens1Zero, CalibSampleTable1, ManomSensTable, 0, CalibPointCount); // ������ � ������ ����������� ���������� 1-�� �������
      CalibCalcParams(Sens2Zero, CalibSampleTable2, ManomSensTable, 1, CalibPointCount); // ������ � ������ ����������� ���������� 2-�� �������

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

SendCommandBth(0x13); // �������� �������
SendCommandBth(0x16); // ���������� �����������
return Res;
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::CalibCalcParams (int CalibZeroLevel, float *CalibSampleTable, float *CalibPresTable, u8 CalibSensIndex, int CalibPointCount)
{
// ������ ����������� ����������
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

// ��� ������ ��������� ������� ���������� �������
ProgReadBTH.uDevCalibCoef[CalibSensIndex][0] = DevCalibCoef[CalibSensIndex][0];
ProgReadBTH.uDevCalibCoef[CalibSensIndex][1] = DevCalibCoef[CalibSensIndex][1];
ProgReadBTH.uDevCalibCoef[CalibSensIndex][2] = DevCalibCoef[CalibSensIndex][2];
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonDefSetupClick(TObject *Sender)
{
ProgConfig.SetDefault();
ProgConfig.DBPath = GetCurrentDir() + L"\\CalibBase.csv";
SetParam();
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::VerificationSetBthDef(void)
{
ButtonUpdateDeviceList->Caption = L"��������";
RzEditSens1->Text = L"0";
RzEditSens2->Text = L"0";
RzEditSensMean->Text = L"0";

LabelMemModel->Caption = L"��� ������";
LabelMemSN->Caption = L"��� ������";
LabelMemSW->Caption = L"��� ������";
LabelMemHW->Caption = L"��� ������";

LabelMemCalibCoef11->Caption = L"0000";
LabelMemCalibCoef12->Caption = L"0000";
LabelMemCalibCoef13->Caption = L"0000";

LabelMemCalibCoef21->Caption = L"0000";
LabelMemCalibCoef22->Caption = L"0000";
LabelMemCalibCoef23->Caption = L"0000";
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::PngSpeedButtonStopCalibClick(TObject *Sender)
{
StopCalib = true;
}
//---------------------------------------------------------------------------


void __fastcall TFormAuto�alibration::FormClose(TObject *Sender, TCloseAction &Action)
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
void __fastcall TFormAuto�alibration::wclRfCommClientConnect(TObject *Sender, const int Error)
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

void __fastcall TFormAuto�alibration::wclRfCommClientData (System::TObject* Sender, const void * Buffer, const unsigned Size)
{
byte *Src = (byte*) Buffer;
int Count = Size;
while (Count--) BthFifo->PutByte(*Src++);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::wclBluetoothManagerDeviceFound(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address)
{
// ����������� ��� ���������� BTH
TwclBluetoothDeviceType BthType = DiscoveryState;

// ����������� ��� ���������� BTH
String DevName;
int Result = Radio->GetRemoteName(Address, DevName);
if (Result != WCL_E_SUCCESS) { return; }

// ����������� ������ �������� BTH
int RadioIndex = GetBthRadioIndex(Radio);

// ���������� ��������� � �������� ����������
if (!DeviceDefinition(DevName)) { AddNewBthDevice(DevName, Address, BthType, RadioIndex); }
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::wclBluetoothManagerPinRequest(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address, String &Pin)
{
Pin = BTH_PIN_CODE;
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::ButtonOpenClick(TObject *Sender)
{
if (OpenDialog1->Execute())
  {
  ProgConfig.DBPath = OpenDialog1->FileName;
  std::auto_ptr <TFileStream> FileStream(new TFileStream(ProgConfig.DBPath, fmCreate | fmOpenWrite));
  if (FileStream->Size > SIZE_MEMORY)
    {
    String Hdr = L"������!";
    String Msg = L"���� ���� ������ 200��! ������� ����������";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    return;
    }
  ReadBase();

  // �������� �� �����
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::WriteBase(void)
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

void __fastcall TFormAuto�alibration::FindListBase(void)
{
FindIndex = -1;
memset(&InfoBaseCalib, 0, sizeof(InfoBaseCalib));

InfoBaseCalib.ChipID = "��� ������";
InfoBaseCalib.Date = "��� ������";
InfoBaseCalib.Time = "��� ������";
InfoBaseCalib.DevName = "��� ������";
InfoBaseCalib.DevSerial = "��� ������";
InfoBaseCalib.DevSwVer = "��� ������";
InfoBaseCalib.DevHwVer = "��� ������";
InfoBaseCalib.BthAddr = "��� ������";
InfoBaseCalib.ManomSerial = "��� ������";
InfoBaseCalib.ProgVersion = "��� ������";
InfoBaseCalib.CalibCoef11 = "0000";
InfoBaseCalib.CalibCoef12 = "0000";
InfoBaseCalib.CalibCoef13 = "0000";
InfoBaseCalib.CalibCoef21 = "0000";
InfoBaseCalib.CalibCoef22 = "0000";
InfoBaseCalib.CalibCoef23 = "0000";
InfoBaseCalib.Surname = "��� ������";
InfoBaseCalib.Name = "��� ������";
InfoBaseCalib.Patron = "��� ������";

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

void TFormAuto�alibration::SetDeviseParam()
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
void TFormAuto�alibration::SplitBase (String FindStr)
{
// ��������� ������
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

void __fastcall TFormAuto�alibration::ListBoxDeviceBTHMouseMove (TObject *Sender, TShiftState Shift, int X, int Y)
{
ItemBoxMouse = ListBoxDeviceBTH->ItemAtPos(TPoint(X, Y), true);
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::RzPageControlCheckUpChanging(TObject *Sender,
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
    String Hdr = L"������!";
    String Msg = L"������� ���!";
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::TimerDifferenceTimer(TObject *Sender)
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

void __fastcall TFormAuto�alibration::wclBluetoothManagerDiscoveringCompleted(TObject *Sender,
          TwclBluetoothRadio * const Radio, const int Error)
{
// ��������� ������ ��� ������
if (DiscoveryState == dkClassic)
  {
  // ��������� ������ ��� ������ ��������� (dlBle)
  StartOperationTimer(2, 10);
  }
else
  {
  // ���������� ������ ���������
  StartOperationTimer(3, 10);
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::wclBluetoothManagerNumericComparison(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address, const DWORD Number,
          bool Confirm)
{
// Accept any pairing.
Confirm = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::wclBluetoothManagerPasskeyRequest(TObject *Sender,
          TwclBluetoothRadio * const Radio, const __int64 Address, DWORD Passkey)

{
// Use 123456 as passkey.
Passkey = 123456;
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::wclGattClientConnect(TObject *Sender, const int Error)

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

void __fastcall TFormAuto�alibration::wclGattClientDisconnect(TObject *Sender, const int Reason)

{
((TwclGattClient*) Sender)->Radio->RemoteUnpair(((TwclGattClient*) Sender)->Address);
}
//---------------------------------------------------------------------------

TwclBluetoothRadio* __fastcall TFormAuto�alibration::GetRadio (int AType)
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

int TFormAuto�alibration::GetClientState (void)
{
// ������� ��������� �����������
return (BthDeviceInfo.Type != dkBle) ? (wclRfCommClient->State) : (wclGattClient->State);
}
//---------------------------------------------------------------------------

void TFormAuto�alibration::StartOperationTimer (int ACode, int ATime)
{
// �������� ������ ��� �������������
TimerOperation->Enabled = 0;

TimerOperationCode = ACode;
TimerOperation->Interval = ATime;

TimerOperation->Enabled = 1;
}
//---------------------------------------------------------------------------

int TFormAuto�alibration::GetBthRadioIndex (TwclBluetoothRadio *ARadio)
{
// �������� ������ �������� BTH � ������ ��������� (�� ��������� = 0)
int Index = 0;

for (int i = 0; i < wclBluetoothManager->Count; i++)
  {
  if (wclBluetoothManager->Radios[i] == ARadio) { Index = i; break; }
  }

return Index;
}
//---------------------------------------------------------------------------

void TFormAuto�alibration::AddNewBthDevice (String AName, __int64 AAddr, int ABthType, int ARadioIndex)
{
// ���� ��� ������, ����������� MAC-�����
AName = AName.Trim();
if (AName == L"") AName = BthMacToStr(AAddr);

// ��������� ���������� BTH � ������ ������������
TBthDeviceInfo Info;
Info.Name = AName;
Info.Addr = AAddr;
Info.Type = ABthType;
Info.RadioIndex = ARadioIndex;
BthDevicesList.push_back(Info);

// ������� ���������� � ������ ��� ������
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

void __fastcall TFormAuto�alibration::TimerOperationTimer(TObject *Sender)
{
// ��������� �������������
TimerOperation->Enabled = 0;

switch (TimerOperationCode)
  {
  case 0: // ����������� �������� "���� � ���������� >> ������ ��������"
    {
    break;
    }

  case 1: // ������ ������ ��������� BTH � ������ dkClassic
    {
    TwclBluetoothRadio *Radio = GetRadio(dkClassic);
    if (!Radio)
      {
      String Hdr = str_Error;
      String Msg = L"������� Bluetooh �� ���������.";
      MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
      Disconnect(MODE_DIS_BTH);
      return;
      }

    // ���� ���������� dkClassic
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
      String Msg = L"������� Bluetooh �� ���������.";
      MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
      Disconnect(MODE_DIS_BTH);
      }
    else
      {
      String Hdr = str_Error;
      String Msg = L"������ ��� ������� ������ ��������� Bluetooh (code " + IntToHex(Result, 8) + ")";
      MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONWARNING);
      Disconnect(MODE_DIS_BTH);
      }
    break;
    }

  case 2: // ������ ������ ��������� BTH � ������ dkBle
    {
    TwclBluetoothRadio *Radio = GetRadio(dkBle);
    if (Radio)
      {
      // ���� ���������� dkBle
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

  case 3: // ���������� ������ ��������� BTH
    {
    BthSearchInProgress = 0;
    ButtonUpdateDeviceList->Caption = L"��������";
    ListBoxDeviceBTH->Enabled = true;
    SetSearchBthLabel(L"����� ��������", 0);

    if (!BthDevicesList.size())
      {
      ListBoxDeviceBTH->Items->Add(L"���������� �� �������");
      }
    else
      {
      ListBoxDeviceBTH->ItemIndex = 0;
      }

    break;
    }

  case 10: // ��������� ����������� � ���������� Bth/Classic
    {
    ErrorInfo = L"������ ��� ����������� ���������� (code " + IntToHex(ConnectErrorCode, 8) + ")";

    String Hdr = str_Error;
    String Msg = ErrorInfo;
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);

    ConnectError = mrAbort;
    break;
    }

  case 11: // ���������� � ��������� �������� Bth/Ble
    {
    int Status = SetGattParams();
    if (Status) { StartOperationTimer(12, 10); break; }

    ConnectError = mrOk;
    break;
    }

  case 12: // ���������� Bth/Ble
    {
    wclGattClient->Disconnect();
    ErrorInfo = L"���� ��� ����������� � ����������:\n" + ErrorInfo;

    String Hdr = str_Error;
    String Msg = ErrorInfo;
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);

    ConnectError = mrAbort;
    break;
    }

  case 13:
    {
    ErrorInfo = L"������ ��� ����������� ���������� (code " + IntToHex(ConnectErrorCode, 8) + ")";

    String Hdr = str_Error;
    String Msg = ErrorInfo;
    MessageBox(Handle, Msg.c_str(), Hdr.c_str(), MB_OK | MB_ICONERROR);

    ConnectError = mrAbort;
    break;
    }
  }
}
//---------------------------------------------------------------------------

String TFormAuto�alibration::BthMacToStr (__int64 AAddr)
{
// ����������� MAC-����� ���������� � ������
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

void __fastcall TFormAuto�alibration::btDisconnect (void)
{
// ��������� ���������� BTH � ��� ����������� �����������
if (ProbaThread)
  {
  ProbaThread->Terminate();
  while (ProbaThread) { Application->ProcessMessages(); Sleep(10); }
  }

ButtonUpdateDeviceList->Caption = L"��������";

if (wclBluetoothManager->Active)
  {
  wclRfCommClient->OnData = wclRfCommClientData;
  wclGattClient->OnCharacteristicChanged = wclGattClientCharacteristicChanged;
  }
wclRfCommClient->Disconnect();
wclGattClient->Disconnect();

// ��������� ����� ���� �������
TwclBluetoothRadio *Radio = GetRadio(dkClassic);
if (Radio) { if (Radio->Discovering) Radio->Terminate(); }
Radio = GetRadio(dkBle);
if (Radio) { if (Radio->Discovering) Radio->Terminate(); }
DiscoveryState = -1;

SetSearchBthLabel(L"����� ��������", 0);
}
//---------------------------------------------------------------------------

void TFormAuto�alibration::SetSearchBthLabel (String AInfo, int AState)
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

int TFormAuto�alibration::SetGattParams (void)
{
int Status = 1;
BthTimer = Now() + ((double) 1500 / (24.0 * 3600.0 * 1000.0));

do
  {
  // GUID ������� UART ��� BLE/Nordic
  String UartServiceGuid = L"{6E400001-B5A3-F393-E0A9-E50E24DCCA9E}";
  String UartCharWrGuid  = L"{6E400002-B5A3-F393-E0A9-E50E24DCCA9E}";
  String UartCharRdGuid  = L"{6E400003-B5A3-F393-E0A9-E50E24DCCA9E}";

  // [������ � ��������� ������ ��������]

  ServiceList.Length = 0;
  int Result = wclGattClient->ReadServices(BleOperFlag(), ServiceList);
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"������ ��� ��������� ������ �������� (code " + IntToHex(Result, 8) + ")";
    break;
    }

  if (ServiceList.Length == 0)
    {
    ErrorInfo = L"������ � ������� ������ �������� (������ ����)";
    break;
    }

  // �������� ������ ��������
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
    ErrorInfo = L"C����� UART ����������� � ������ ��������";
    break;
    }

  // [������ � ��������� ������ �������������]

  CharacteristicList.Length = 0;
  Result = wclGattClient->ReadCharacteristics(UartService, BleOperFlag(), CharacteristicList);
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"������ ��� ��������� ������ ������������� (code " + IntToHex(Result, 8) + ")";
    break;
    }

  if (CharacteristicList.Length == 0)
    {
    ErrorInfo = L"������ � ������� ������ ������������� (������ ����)";
    break;
    }

  // �������� ������ �������������
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
    ErrorInfo = L"�������������� UART ����������� � ������ �������������";
    break;
    }

  // [����������� ����������]

  Result = wclGattClient->Subscribe(UartCharRd);
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"������ ��� ����������� ���������� (code " + IntToHex(Result, 8) + ")";
    break;
    }

  Result = wclGattClient->WriteClientConfiguration(UartCharRd, true, BleOperFlag());
  if (Result != WCL_E_SUCCESS)
    {
    ErrorInfo = L"������ ��� ��������� ���������� (code " + IntToHex(Result, 8) + ")";
    break;
    }

  Status = 0;
  }
while (0);

return Status;
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::wclGattClientCharacteristicChanged (TObject *Sender,
  const WORD Handle, const TwclGattCharacteristicValue Value)
{
// ��������� ���� ������ � FIFO ������
int Count = Value.Length;
if (Count)
  {
  for (int i = 0; i < Count; i++) BthFifo->PutByte(Value[i]);
  }
}
//---------------------------------------------------------------------------

TwclGattOperationFlag TFormAuto�alibration::BleOperFlag (void)
{
// ����� ������ BLE ���������� (���� �� ������������)
int Index = 0;
switch (Index)
  {
  case 1:  { return goReadFromDevice; }
  case 2:  { return goReadFromCache; }
  default: { return goNone; }
  }
}
//---------------------------------------------------------------------------

void __fastcall TFormAuto�alibration::FormDestroy(TObject *Sender)
{
Disconnect(MODE_DIS_BTH);
wclBluetoothManager->Close();
ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

int TFormAuto�alibration::SetClientData (void *Data, unsigned Size, unsigned &Written)
{
int Result;

if (BthDeviceInfo.Type != dkBle)
  {
  // ������ ����� ������ � ���������� ���� dkClassic
  Result = wclRfCommClient->Write(Data, Size, Written);
  }
else
  {
  // ������ ����� ������ � ���������� ���� dkBle
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

