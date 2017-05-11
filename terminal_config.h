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

#define TERM_TIMELEFT_EN				(1)			// ������ �������
#define TERM_TX_RX_EN					(1)			// ��������� ���������
#define TERM_TX_RX_DEB_EN				(0)			// �������������� ���������� ��������� ���������
#define TERM_CMD_LOG_EN					(1)			// ��� �������� ������
#define TERM_LR_KEY_EN					(1)			// ����������� ������� ������-�����

// ******************************** keys ************************************

#define TERM_KEY_ENTER					(0x0D)		// ������ ����� �������
#define TERM_KEY_BACKSPACE				(0x08)		// ������ �������� ���������� ��������� �������
#define TERM_KEY_ESCAPE					(0xF0)		// ������ ���������� ��������
#define TERM_KEY_UP						(0xF1)		//
#define TERM_KEY_RIGHT					(0xF2)		//
#define TERM_KEY_DOWN					(0xF3)		//
#define TERM_KEY_LEFT					(0xF4)		//
#define TERM_KEY_RESET					'~'			// ������ ������ cpu	(shift+�)

// **************************************************************************

// ********************** ��������� ��������� *******************************

#define _TERM_VER_				"v1.3"				// ������ ���������
#define TERM_SIZE_TASK          (80)				// ������������ ���������� �����
#define TERM_CMD_BUF_SIZE		(80)				// ������������ ���������� �������� � ������� �������� ������
#define TERM_CMD_LOG_SIZE		(10)				// ��� ��������� ��������� ������
#define TERM_ARGS_BUF_SIZE		(20)				// ������������ ���������� ���������� � �������
#define TERM_ARG_SIZE			(15)				// ������������� ����� ������ ���������
#define CHAR_INTERRUPT			TERM_KEY_ESCAPE 	// ������ ���������� ��������
#define STRING_TERM_ENTER		"\n\r"				// ������� ������
#define STRING_TERM_ARROW		">> "				// ��������� ����� �����
#define ECHO_EN					(1)					// �������� echo
#define RESET_FCN				NVIC_SystemReset	// ������� ������ ����������

// **************************************************************************

// ********************** ��������� ������ � �������� ***********************

#if (TERM_TX_RX_EN == 1)

//#include "RS232_PC/rs232.h"			// HW ��������� rs232
#include "debug.h"

//#define TerminalTx					RS232_TXDataStr			// void fcn(const char*)			// ������� ������ � ��������
#define TerminalTx					DBG					// void fcn(const char*)			// ������� ������ � ��������
#if (ECHO_EN == 1)
#define PutChar			            RS232_PutChar		// �������� ������� ������� ������������ � fifo ���������� ������
#else	// ECHO_EN != 1
#define PutChar
#endif	// ECHO_EN == 1

#else	// TERM_TX_RX_EN != 1
#define TerminalTx
#define PutChar
#endif	// TERM_TX_RX_EN == 1
// **************************************************************************

// ************** ��������� ��������������� ����������� ������ **************

#if (TERM_TX_RX_DEB_EN == 1)

#include "debug.h"					// ����� �������� �����

#define TerminalTxDebug			rtt_printf				// void fcn(str, ...)		// ������� ����������� ������ ��� ������ � ����������

#else	// TERM_TX_RX_DEB_EN != 1

#define TerminalTxDebug

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

#define atoiDec(str)        strtoll((const char*)str, NULL, 10)
#define atoiHex(str)        strtoll((const char*)str, NULL, 16)

#endif // _TERMIANL_CONFIG_H_
