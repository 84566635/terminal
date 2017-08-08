#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "terminal_config.h"

typedef enum{
	TE_OK = 0,				// ��� ������ �������
	TE_Err = 1,				// ������������ ������
	TE_NotFound = 2,		// ������� �� �������
	TE_ArgErr = 10,			// ������ ��������� � �����������, ��� ����������
	TE_ExecErr = 11,		// ������ � ���������� �������
	TE_WorkInt = 12			// �������� �������� � ������
}TE_Result_e;	// ������ �������� ������ ������ ��������� (Terminal Error Result)

typedef enum{
	TA_OK = 0,				// ������� ������� ���������
	TA_MaxCmd = 1,			// ���������� ������� ��������� ���������
	TA_FcnNull = 2,			// ��������� �� ������� NULL
	TA_EmptyName = 3,		// ������ ��� �������
	TA_RetryName = 4		// ��� ������� ��� ����������
}TA_Result_e;	// ������ �������� ���������� ����� ������� (Terminal Add Result)

typedef enum{
	TC_OK = 0,
	TC_Enter = 1,
	TC_BufFull = 2,
	TC_Reset = 3
}TC_Result_e; // ����� �������� ����� ������ ������� (Terminal Char Result)

typedef enum{
	TDC_None = 0x0000,
	TDC_Time = 0x0001,
	TDC_CPU = 0x0002,
	TDC_All = 0xFFFF
}TypeDefaultCmd_e;

typedef enum{
	TMC_None = 0x0000,
	TMC_PrintStartTime = 0x0001,
	TMC_PrintStopTime = 0x0002,
	TMC_PrintDiffTime = 0x0004,
	TMC_All = 0xFFFF,
}TypeModeCmd_e;

bool CLI_GetIntState();
#define CLI_RetInt()			{ if (CLI_GetIntState()){return TE_WorkInt;}}

void CLI_Init(TypeDefaultCmd_e defCmd);

bool CLI_Execute();

TA_Result_e CLI_AddCmd(const char* name, uint8_t (*fcn)(char**, uint8_t), uint8_t argc, uint16_t mode, const char* descr);

void CLI_PrintTime(void);
void CLI_PrintTimeWithoutRN(void);

TC_Result_e CLI_EnterChar(char c);
void CLI_SetEnterCallback(void (*fcn)(uint8_t* rx_buf, uint16_t length));

bool CLI_GetArgDecByFlag(const char* flag, uint32_t* outValue);
bool CLI_GetArgHexByFlag(const char* flag, uint32_t* outValue);

uint32_t CLI_GetArgDec(uint8_t index);
uint32_t CLI_GetArgHex(uint8_t index);

#endif // _TERMINAL_H
