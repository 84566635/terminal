/*************************************************************************
 *
 * 				���������������� ���� ���������
 * 	��� ������������� ������� ���������, ��������� ����������� ���
 * 	��������� ��������� � ������, � ���������� ������������ ����������
 * 	�������� ������������� �������.
 *
 * 	�������� ���������:
 *
 * 	TERM_SIZE_TASK 		- ������������ ���������� ������ ���������
 * 	TERM_CMD_BUF_SIZE	- ������������ ����� 1 �������� �������
 *	ECHO_EN				- ���������� ��� ��� ������� ������ �������
 *
 *
 *	��� ������ ���������� ���������� ��������� ��������� �������:
 *	TerminalTx			- ������� ����������� ����� � ��� ���������
 *							������ ����� ��������� fcn(const char*, ...) (������ printf)
 *	PutChar				- ������� ����������� ����� 1 ������� � ��� ���������
 *							������ ����� ��������� fcn(char) [��������� ��� echo]
 *
 *	��������� �������:
 *	��������� ������� �������� � ���������� � ������ ������ uint32_t SysTickCtr;
 *
 *	�������������� ������ "Terminal_GetMs", ����� �� ��������� �����������
 *	����� � ������������.
 *
 *	---------------------------------------------------------------------
 *
 *	������ ������ ��������� � ����� Readme.h
 *
 ************************************************************************/

#ifndef _TERMIANL_CONFIG_H_
#define _TERMIANL_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>				// strtol, itoa

#define TERM_TIMELEFT_EN				(0)			// ������ �������
#define TERM_TX_RX_EN					(1)			// ��������� ���������
#define TERM_TX_RX_DEB_EN				(1)			// �������������� ���������� ��������� ���������
#define TERM_CMD_LOG_EN					(1)			// ��� �������� ������
#define TERM_LR_KEY_EN					(1)			// ����������� ������� ������-�����
#define TERM_DEFAULT_ALLOC_EN			(1)			//
#define TERM_DEFAULT_STRING_EN			(1)			//
#define TERM_PRINT_ERROR_EXEC_EN		(1)			// 

// ******************************** keys ************************************

#define _KEY_INIT(c)					((char)c)

#define TERM_KEY_ENTER					(_KEY_INIT(0x0D))		// ������ ����� �������
#define TERM_KEY_BACKSPACE				(_KEY_INIT(0x7F))		// ������ �������� ���������� ��������� �������
#define TERM_KEY_ESCAPE					(_KEY_INIT(0xF0))		// ������ ���������� ��������
#define TERM_KEY_UP						(_KEY_INIT(0xF1))		//
#define TERM_KEY_RIGHT					(_KEY_INIT(0xF2))		//
#define TERM_KEY_DOWN					(_KEY_INIT(0xF3))		//
#define TERM_KEY_LEFT					(_KEY_INIT(0xF4))		//
#define TERM_KEY_RESET					'~'			// ������ ������ cpu	(shift+�)

// **************************************************************************

// ********************** ��������� ��������� *******************************

#define _TERM_VER_				"v1.4"				// ������ ���������
#define TERM_SIZE_TASK          (80)				// ������������ ���������� �����
#define TERM_CMD_BUF_SIZE		(80)				// ������������ ���������� �������� � ������� �������� ������
#define TERM_CMD_LOG_SIZE		(10)				// ��� ��������� ��������� ������
#define TERM_ARGS_BUF_SIZE		(20)				// ������������ ���������� ���������� � �������
#define TERM_ARG_SIZE			(15)				// ������������� ����� ������ ���������
#define CHAR_INTERRUPT			TERM_KEY_ESCAPE 	// ������ ���������� ��������
#define STRING_TERM_ENTER		"\n\r"				// ������� ������
#define STRING_TERM_ARROW		">> "				// ��������� ����� �����
#define ECHO_EN					(1)					// �������� echo
#define RESET_FCN()					// ������� ������ ����������

// **************************************************************************

// ********************** ��������� ������ � �������� ***********************

#if (TERM_TX_RX_EN == 1)
#include <stdio.h>
extern void COM_Print(const char* str);
extern char dbgbuffer[256];
#define COM_Printf(...){sprintf(dbgbuffer,__VA_ARGS__);COM_Print(dbgbuffer);}
#define CLI_Printf		COM_Printf
extern void COM_Putc(char c);
#if (ECHO_EN == 1)
#define CLI_PutChar		COM_Putc
#else	// ECHO_EN != 1
#define CLI_PutChar
#endif	// ECHO_EN == 1

#else	// TERM_TX_RX_EN != 1
#define CLI_Printf
#define CLI_PutChar
#endif	// TERM_TX_RX_EN == 1
// **************************************************************************

// ************** ��������� ��������������� ����������� ������ **************

#if (TERM_TX_RX_DEB_EN == 1)
#define CLI_DPrintf		printf
#else	// TERM_TX_RX_DEB_EN != 1
#define CLI_DPrintf
#endif	// TERM_TX_RX_DEB_EN == 1

// **************************************************************************

// ************************ ��������� ������ ������� ************************

#if (TERM_TIMELEFT_EN == 1)

extern volatile uint64_t SysTickCtr;							// ���������� �������� ������� ������

#define Terminal_GetUs()			((float)SysTickCtr * 10)	// ������� ������������ ��������� ����� � us
#define Terminal_GetFastUs()		(SysTickCtr << 3)			// ������� ������������ ��������� ����� � us (�� ������)
#define Terminal_GetMs()			((float)SysTickCtr / 100)	// ������� ������������ ��������� ����� � ms
#define Terminal_GetFastMs()		(SysTickCtr >> 7)			// ������� ������������ ��������� ����� � ms (�� ������)
#define SysTimeReset()				{SysTickCtr = 0;}			// ������� ������������ ��������� �����
#define delay_ms(ms)				{uint32_t ms_cntr_67 = Terminal_GetMs(); while(((uint32_t)Terminal_GetMs() - ms_cntr_67) < (ms)) {}}
#define delay_us(us)				{uint32_t us_cntr_67 = Terminal_GetUs(); while(((uint32_t)Terminal_GetUs() - us_cntr_67) < (us)) {}}

#else	// TERM_TIMELEFT_EN != 1

#define Terminal_GetUs()			(0)
#define Terminal_GetMs()			(0)
#define SysTimeReset()				{}
#define delay_ms(ms)				{}


#endif	// TERM_TIMELEFT_EN == 1

// **************************************************************************

// ********************** memory allocate functions *************************

#if (TERM_DEFAULT_ALLOC_EN == 1)
#include "lib/cli_malloc.h"
#define cli_malloc		_malloc
#define cli_free		_free
#else
#define cli_malloc		// your implementation
#define cli_free		// your implementation
#endif

// **************************************************************************

// *************************** string functions *****************************

#if (TERM_DEFAULT_STRING_EN == 1)
#include <string.h>
#define cli_memcpy		memcpy
#else
#define cli_memcpy		// your implementation
#endif

// **************************************************************************

#define CLI_GetDecString(str)        ((uint32_t) strtoll((const char*)str, NULL, 10))
#define CLI_GetHexString(str)        ((uint32_t) strtoll((const char*)str, NULL, 16))

#endif // _TERMIANL_CONFIG_H_
