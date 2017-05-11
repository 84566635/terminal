#include "terminal.h"
#include "string_split.h"
#include "terminal_time.h"
#include "terminal_log.h"
#include "queue.h"


extern uint8_t _strcmp(const char* str1, const char* str2);
extern uint32_t _strlen(const char* strSrc);

static uint8_t _help_cmd(char** argv, uint8_t argc);
static uint8_t _reset_cpu(char** argv, uint8_t argc);
static uint8_t _settime_cmd(char** argv, uint8_t argc);
static uint8_t _gettime_cmd(char** argv, uint8_t argc);
static uint8_t _cpur_cmd(char** argv, uint8_t argc);
static uint8_t _cpuw_cmd(char** argv, uint8_t argc);
static uint8_t _cpurb_cmd(char** argv, uint8_t argc);

extern TTime _def_time;

static void (*enter_callback)(uint8_t* rx_buf, uint16_t length);

// ************************* interrupt function ***************************

static bool _interrupt_operation = false;

/// \brief �������� ������� ���������� �������� (����� ������, ������ ���������)
/// \return {bool} - true: ��������� �������� ��������
bool inline Termial_GetIntState(){

	bool res = _interrupt_operation;
	_interrupt_operation = false;
	return res;
}


// ************************************************************************

// *********************** Terminal fcns **********************************

typedef struct{
    uint8_t (*fcn)(char** argv, uint8_t argc);			// ������� ���������� ��������
    const char* name;									// ��� �������, �� ������� ���������� �������
    const char* description;							// �������� ������� (���������, �������� �������� � �.�.)
}TermCmd;		// ��������� ��������� ������� ���������

struct{
	Queue_s symbols;							// ������� �������� ��������� ���������������
	char buf_enter[TERM_CMD_BUF_SIZE + 1];		// ����� �����
	char buf_enter_exec[TERM_CMD_BUF_SIZE + 1];	// ����� �����, ��� ���������� ������
	int16_t buf_curPos;							// ������� ������� �������
	int16_t buf_cntr;							// �������� ������ �����
	int16_t buf_cntr_exec;						// �������� ������ ����� ��� ���������� ������
    TermCmd cmds[TERM_SIZE_TASK];				// ������� ���������
    uint8_t countCommand;						// ���������� ������
    uint8_t executeState;						// ��������� ��������� (�������� ������ ��� ���)
}Terminal;		// ��������

TermCmd* _findTermCmd(const char* cmdName);

static volatile args _input_args;

/// \brief ������������� ���������
/// \return none
void TerminalInit(TypeDefaultCmd_e defCmd){

	TerminalTx("\r\n****************************************************");
	TerminalTx("\r\n|                                                  |");
	TerminalTx("\r\n|                   Terminal "); TerminalTx(_TERM_VER_); TerminalTx("                  |");
	TerminalTx("\r\n|           sw ver.: "); TerminalTx(__DATE__); TerminalTx(" ");TerminalTx(__TIME__);TerminalTx("          |");
	TerminalTx("\r\n|                                                  |");
	TerminalTx("\r\n****************************************************");
	TerminalTx("\r\n");

    Terminal.countCommand = 0;
    Terminal.buf_curPos = 0;
    Terminal.buf_cntr = 0;
    Terminal.buf_cntr_exec = 0;
    Terminal.buf_enter[Terminal.buf_cntr] = '\0';
    Terminal.buf_enter[TERM_CMD_BUF_SIZE] = '\0';
    Terminal.buf_enter_exec[Terminal.buf_cntr_exec] = '\0';
    Terminal.buf_enter_exec[TERM_CMD_BUF_SIZE] = '\0';
    Terminal.executeState = 0;

    setTime(&_def_time, 0, 0, 0);

    TerminalAddCmd("help", 		_help_cmd, 		"������ �� �������� ���������");
    TerminalAddCmd("~", 		_reset_cpu, 	"reset cpu");

    if (defCmd & TDC_Time)
    {
    	TerminalAddCmd("settime", 	_settime_cmd, 	"������ ������� �����\n\r\tsettime [h] [m] [s]");
    	TerminalAddCmd("gettime", 	_gettime_cmd, 	"������� ������� �����");
    }

    if (defCmd & TDC_CPU)
    {
    	TerminalAddCmd("cpur", 		_cpur_cmd, 		"��������� �� ������ ����������\n\r\tcpur [addr]\n\r\t\taddr - 32-��������� �������� ������, �� �������� ������");
    	TerminalAddCmd("cpuw", 		_cpuw_cmd, 		"�������� �� ������ ����������\n\r\tcpuw [addr] [data]\n\r\t\taddr - 32-��������� �������� ������, �� �������� ������\n\r\t\tdata - 32-��������� �������� ������������ ������");
    	TerminalAddCmd("cpurb", 	_cpurb_cmd,		"��������� ���� �� ���������� ������� � ������\n\r\tcpurb [addr] [length]\n\r\t\taddr - 32-��������� �������� ������, �� �������� �������� ������\n\r\t\tlength - ���������� ��������� ���������");
    }

    TerminalTx("\r\nCount base command: %d", Terminal.countCommand);
    TerminalTx("\r\nMax command: %d", TERM_SIZE_TASK);
    TerminalTx("\r\n");

    _input_args.argv = (char**) _malloc(sizeof(char*) * TERM_ARGS_BUF_SIZE);
    for(uint8_t i = 0; i < TERM_ARGS_BUF_SIZE; i++)
    	_input_args.argv[i] = _malloc(sizeof(char) * (TERM_ARG_SIZE + 1));

    Q_Init(&Terminal.symbols, 3, sizeof(char), QUEUE_FORCED_PUSH_POP_Msk);

#if (TERM_CMD_LOG_EN == 1)
    TERM_LogInit();
#endif

    printArrow();
}

/// \brief ��������� �������
/// \param {const char*} str - ������ � �������� � ������� ���������� � ���
/// \return {TE_Result_e} - ��������� ���������� �������
TE_Result_e TerminalExecString(const char* str)
{
	split((char*)str, " ", (args*) &_input_args);

	for(uint8_t i = 0; i < _input_args.argc;i++)
		TerminalTxDebug("\r\n: %s", _input_args.argv[i]);

	TerminalPrintTime();
	TE_Result_e result = TerminalExec(_input_args.argv, _input_args.argc);
	TerminalPrintTime();

	ArgDestroy((args*)&_input_args);

	return result;
}

/// \brief ��������� �������
/// \param {char*} argv - ��������� (�������� � ���������)
/// \param {uint8_t} argc - ���������� ����������
/// \return {TE_Result_e} - ��������� ���������� �������
TE_Result_e TerminalExec(char** argv, uint8_t argc)
{
    if (argc < 1)
        return TE_ArgErr;

    TermCmd* cmd = _findTermCmd(argv[0]);

    if (cmd != NULL){
    	Terminal.executeState = 1;
    	TE_Result_e result = cmd->fcn(argv, argc);
    	Terminal.executeState = 0;
    	return result;
    }

    return TE_NotFound;
}

/// \brief ������� ���������� ������� � ��������
/// \param {const char*} name - ��� �������, �� ������� ��� ����� ����������� � ���������
/// \param {uint8_t (*)(char**, uint8_t)} fcn - ������� ���������� �������
/// \param {const char*} descr - �������� ������� (��� ������� help)
/// \return {TA_Result_e} - ��������� ������� �������� �������
TA_Result_e TerminalAddCmd(const char* name, uint8_t (*fcn)(char**, uint8_t), const char* descr)
{
    if (Terminal.countCommand >= TERM_SIZE_TASK)
        return TA_MaxCmd;

    if (fcn == NULL)
        return TA_FcnNull;

    if (_strlen((char*)name) == 0)
        return TA_EmptyName;

    uint8_t i = 0;
    for(; i < Terminal.countCommand; i++)
        if (_strcmp((char*) Terminal.cmds[i].name, (char*) name))
            return TA_RetryName;

    uint8_t countCmd = Terminal.countCommand;
    Terminal.cmds[countCmd].fcn = fcn;
    Terminal.cmds[countCmd].name = name;
    Terminal.cmds[countCmd].description = descr;
    Terminal.countCommand++;

    return TA_OK;
}

/// \brief ������� ��������� ���������� ������� � ��������
/// \param {uint8_t} code - ��� ���������� ���������� �������
/// \return none
void TerminalPrintResultExec(uint8_t code)
{
	switch(code){
		case TE_NotFound:	TerminalTx("\n\rerr: ������� �� �������. ��������� ���� �������");break;
		case TE_ArgErr: 	TerminalTx("\n\rerr: ������� �������� ��� ����� �� �������� ��������");break;
		case TE_ExecErr:	TerminalTx("\n\rerr: ������ ��� ���������� �������");break;
		case TE_WorkInt:	TerminalTx("\n\rerr: �������� ��������");break;
		default:break;
	}

	printArrow();
}

/// \brief �������� ����������� ����� ������ �������
/// \param {char**} argv - ����� ����������
/// \param {uint8_t} argc - ���������� ����������
/// \param {const char*} flag - ������� ���� (������������� 2 ���� ������, � ������ ('-') ����� ������ ������������� ������� �������� ����� �����, ������ ������������� ����, ������������� ��������� ��������.
/// \return int8_t - � ������ ������� �����, ���������� ��� ������ ����� ����������, � ���� ������ �������� -1.
int8_t TerminalGetValueByFlag(char** argv, uint8_t argc, const char* flag)
{
	for(uint8_t i = 0; i < argc; i++)
	{
		if (_strcmp(argv[i], flag))
		{
			if ((argc > (i + 1)) || (flag[0] != '-'))
				{return i;}
			else
				{return -1;}
		}
	}

	return -1;
}

/// \brief ����� ������ ������ � ���������� ��������
/// \return none
void TerminalViewCommandList(){
	TerminalTxDebug("\nList commands:");
    uint16_t i = 0;
    for(; i < Terminal.countCommand; i++){
    	TerminalTxDebug("\n%-10s: 0x%X", Terminal.cmds[i].name, Terminal.cmds[i].fcn);
    }
}

/// \brief ����� ������� ����� ��������� � ���������
/// \param {const char*} cmdName - ��� ������� �������
/// \return {TermCmd*} - ��������� ������ (NULL, ���� ������� �� �������)
TermCmd* _findTermCmd(const char* cmdName)
{
    uint8_t i = 0;
    for(; i < Terminal.countCommand; i++){

    	char* name1 = (char*) Terminal.cmds[i].name;
    	char* name2 = (char*) cmdName;

    	int res = _strcmp(name1, name2);

        if (res){
            return &Terminal.cmds[i];
        }
    }

    return NULL;
}

// ************************************************************************

// ************************* sys cmd terminal *****************************

uint8_t _help_cmd(char** argv, uint8_t argc)
{
	TerminalTx("\r\nCount command: %d", (int) Terminal.countCommand);
	TerminalTx("\r\n[] - ������������ ��������\r\n<> - �� ������������ ��������\r\n| - ����� ����� �����������");
	uint16_t i = 1;
	for(; i < Terminal.countCommand; i++){
		TerminalTx("\r\n\n%-10s - %s", Terminal.cmds[i].name, Terminal.cmds[i].description);
		TerminalTx("\r\n-----------------------------------------------------------------");
	}

	return TE_OK;
}

uint8_t _reset_cpu(char** argv, uint8_t argc)
{
	RESET_FCN();

	return TE_OK;
}

uint8_t _settime_cmd(char** argv, uint8_t argc)
{
	if (argc < 4)
		return TE_ArgErr;

	uint32_t h = atoiDec(argv[1]);
	uint8_t m = atoiDec(argv[2]);
	uint8_t s = atoiDec(argv[3]);

	SysTimeReset();

	setTime(&_def_time, h, m, s);

	return TE_OK;
}

uint8_t _gettime_cmd(char** argv, uint8_t argc)
{
	TerminalTx("\r\nTime: ");
	TerminalPrintTime();

	return TE_OK;
}

uint8_t _cpur_cmd(char** argv, uint8_t argc)
{
	if (argc < 2)
		return TE_ArgErr;

	uint32_t* v = (uint32_t*) (uint32_t) atoiHex(argv[1]);

	TerminalTx("\r\n0x%08X: 0x%08X", (int) v, (int) *v);

	return TE_OK;
}

uint8_t _cpuw_cmd(char** argv, uint8_t argc)
{
	if (argc < 3)
		return TE_ArgErr;


	uint32_t* v = (uint32_t*) (uint32_t) atoiHex(argv[1]);
	uint32_t d = atoiHex(argv[2]);
	*v = d;

	TerminalTx("\r\n0x%08X: 0x%08X", (int) v, (int) *v);

	return TE_OK;
}

uint8_t _cpurb_cmd(char** argv, uint8_t argc)
{
	if (argc < 3)
		return TE_ArgErr;

	uint32_t* v = (uint32_t*) (uint32_t) atoiHex(argv[1]);
	uint32_t c = atoiDec(argv[2]);

	for(uint32_t i = 0; i < c; i++)
	{
		TerminalTx("\r\n0x%08X: 0x%08X", (int) v, (int)*v);
		v++;
	}


	return TE_OK;
}

// ************************************************************************

/// \brief ����� �������� ������� � ��������
/// \return none
void TerminalPrintTime()
{
	uint32_t ms = Terminal_GetMs();
	TTime t = generateTimeMSec(getTimeMSec(&_def_time) + ms);
	TerminalTx("\r\n%02d:%02d:%02d.%03d", (int) t.hour, (int) t.minute, (int) t.second, (int) t.msec);
}

/// \brief ����� �������� ������� � �������� ��� �������� ������
/// \return none
void TerminalPrintTimeWithoutRN()
{
	uint32_t ms = Terminal_GetMs();
	TTime t = generateTimeMSec(getTimeMSec(&_def_time) + ms);
	TerminalTx("%02d:%02d:%02d.%03d", (int) t.hour, (int) t.minute, (int) t.second, (int) t.msec);
}

static void _UpdateCmd(const char* newCmd)
{
	PutChar('\r');
	printArrowWithoutN();

	memcpy(Terminal.buf_enter, newCmd, TERM_CMD_BUF_SIZE);
	Terminal.buf_cntr = TERM_CMD_BUF_SIZE;


	uint8_t remCntr = 0;
	for(uint8_t i = 0; i < TERM_CMD_BUF_SIZE; i++)
	{
		if ((Terminal.buf_enter[i] == '\0') && (Terminal.buf_cntr == TERM_CMD_BUF_SIZE))
		{
			Terminal.buf_cntr = i;
		}

		if (i > Terminal.buf_cntr)
		{
			remCntr++;
			PutChar(' ');
		}
		else
			{PutChar(Terminal.buf_enter[i]);}
	}

	Terminal.buf_curPos = Terminal.buf_cntr;
	for(uint8_t i = 0; i < remCntr; i++)
		{PutChar(0x08);}

}

static void _AddChar(char c)
{
#if (TERM_LR_KEY_EN == 1)
	if (Terminal.buf_curPos != Terminal.buf_cntr)
	{
		for(uint8_t i = Terminal.buf_cntr + 1; i > Terminal.buf_curPos; i--)
		{
			Terminal.buf_enter[i] = Terminal.buf_enter[i-1];
		}

		Terminal.buf_enter[Terminal.buf_curPos] = c;
		Terminal.buf_cntr++;
		Terminal.buf_curPos++;
		Terminal.buf_enter[Terminal.buf_cntr] = '\0';

		uint8_t tmpPos = Terminal.buf_curPos;
		_UpdateCmd(Terminal.buf_enter);

		for(uint8_t pos = 0; pos < Terminal.buf_cntr - tmpPos; pos++)
		{
			PutChar(0x08);
			Terminal.buf_curPos--;
		}
	}
	else
	{
		PutChar(c);
		Terminal.buf_enter[Terminal.buf_curPos] = c;
		Terminal.buf_cntr++;
		Terminal.buf_curPos++;
		Terminal.buf_enter[Terminal.buf_cntr] = '\0';
	}

#else
	PutChar(c);
	Terminal.buf_enter[Terminal.buf_curPos] = c;
	Terminal.buf_cntr++;
	Terminal.buf_curPos++;
	Terminal.buf_enter[Terminal.buf_cntr] = '\0';
#endif
}

static void _RemChar()
{
#if (TERM_LR_KEY_EN == 1)

	if (Terminal.buf_curPos != Terminal.buf_cntr)
	{
		PutChar(0x08);
		PutChar(' ');
		PutChar(0x08);

		// ��������� ��������� �������
		uint8_t tmpPos = Terminal.buf_curPos - 1;

		// ������� ��� ����������� ������� �����
		for(uint8_t i = tmpPos; i < Terminal.buf_cntr; i++)
		{
			Terminal.buf_enter[i] = Terminal.buf_enter[i+1];
		}

		Terminal.buf_curPos--;
		Terminal.buf_cntr--;
		Terminal.buf_enter[Terminal.buf_cntr] = '\0';

		_UpdateCmd(Terminal.buf_enter);

		for(uint8_t pos = 0; pos < Terminal.buf_cntr - tmpPos; pos++)
		{
			PutChar(0x08);
			Terminal.buf_curPos--;
		}
	}
	else
	{
		PutChar(0x08);
		PutChar(' ');
		PutChar(0x08);

		Terminal.buf_curPos--;
		Terminal.buf_cntr--;
		Terminal.buf_enter[Terminal.buf_cntr] = '\0';
	}
#else
	PutChar(0x08);
	PutChar(' ');
	PutChar(0x08);

	Terminal.buf_curPos--;
	Terminal.buf_cntr--;
	Terminal.buf_enter[Terminal.buf_cntr] = '\0';
#endif
}

/// \brief �������� ����� ������ � ����� ���������
/// \param {char} c - ����������� ������
/// \return {TC_Result_e} - ��������� �������� ���������� �������
TC_Result_e TerminalPutChar(char c)
{
	/*
	uint8_t* write_buffer = (uint8_t*)Terminal.buf_enter;
	int16_t* write_cntr = &Terminal.buf_cntr;

	if (Terminal.executeState == 1)
	{
		write_buffer = &Terminal.buf_cntr_exec;
		write_cntr = &Terminal.buf_cntr_exec;
	}
	 */

	Q_Push(&Terminal.symbols, &c);
	/*
	TerminalTx("\r\nPrintQ: ");
	for(uint8_t i = 0; i < Terminal.symbols._cntr; i++)
	{TerminalTx("%02X ",*((char*) (Terminal.symbols.ptrObj + i)));}
	 */

	uint8_t arr_up[] = {0x1B, 0x5B, 0x41};
	uint8_t arr_down[] = {0x1B, 0x5B, 0x42};
	uint8_t arr_right[] = {0x1B, 0x5B, 0x43};
	uint8_t arr_left[] = {0x1B, 0x5B, 0x44};
	uint8_t arr_esc[] = {0x1B, 0x1B, 0x1B};

	if (Q_IsEqual(&Terminal.symbols, arr_up, 3))
		{c = TERM_KEY_UP;}
	else if (Q_IsEqual(&Terminal.symbols, arr_down, 3))
		{c = TERM_KEY_DOWN;}
	else if (Q_IsEqual(&Terminal.symbols, arr_right, 3))
		{c = TERM_KEY_RIGHT;}
	else if (Q_IsEqual(&Terminal.symbols, arr_left, 3))
		{c = TERM_KEY_LEFT;}
	else if (Q_IsEqual(&Terminal.symbols, arr_esc, 3))
		{c = TERM_KEY_ESCAPE;}

	bool isValidKey = ((Terminal.buf_cntr < TERM_CMD_BUF_SIZE) ||
						(c == TERM_KEY_BACKSPACE) ||
						(c == TERM_KEY_ENTER)	||
						(c == CHAR_INTERRUPT));

	bool isAlphaBet = (((c > 0x2F) && (c < 0x3A)) ||
						((c > 0x60) && (c < 0x7B)) ||
						((c > 0x40) && (c < 0x5B)) ||
						(c == 0x20) || (c == '_') || (c == '-'));

	//TerminalTx("\r\nKey Code: 0x%02X", c);
	if (isValidKey)
	{
		switch(c)
		{
			case TERM_KEY_ENTER:
			{
				Terminal.buf_enter[Terminal.buf_cntr] = '\0';

				if (enter_callback != NULL)
					enter_callback((uint8_t*)Terminal.buf_enter, (uint16_t)Terminal.buf_cntr + 1);

#if (TERM_CMD_LOG_EN == 1)
				TERM_LogCmdPush(Terminal.buf_enter);
				TERM_CurReset();
#endif
				Terminal.buf_curPos = 0;
				Terminal.buf_cntr = 0;
				Terminal.buf_enter[Terminal.buf_cntr] = '\0';

				return TC_Enter;
			}break;
			case CHAR_INTERRUPT:		{_interrupt_operation = true;TerminalTx("\r\nKey ESC");}break;
			case TERM_KEY_BACKSPACE:	{if (Terminal.buf_curPos > 0){_RemChar();}}break;
			case TERM_KEY_RESET:		{return TC_Reset;}break;
			case TERM_KEY_DOWN:
			{
#if (TERM_CMD_LOG_EN == 1)
				const char* ptrCmd = TERM_GetNextCmd();
				if (ptrCmd != NULL)
					{_UpdateCmd(ptrCmd);}
#endif
			}break;
			case TERM_KEY_UP:
			{
#if (TERM_CMD_LOG_EN == 1)
				const char* ptrCmd = TERM_GetLastCmd();
				if (ptrCmd != NULL)
					{_UpdateCmd(ptrCmd);}
#endif
			}break;
			case TERM_KEY_LEFT:
			{
#if (TERM_LR_KEY_EN == 1)
				if (Terminal.buf_curPos > 0)
				{
					Terminal.buf_curPos--;
					PutChar(0x08);
				}
#endif
			}break;
			case TERM_KEY_RIGHT:
			{
#if (TERM_LR_KEY_EN == 1)
				if (Terminal.buf_curPos < Terminal.buf_cntr)
				{
					PutChar(Terminal.buf_enter[Terminal.buf_curPos]);
					Terminal.buf_curPos++;
				}
#endif
			}break;
			default:
			{
				if (isAlphaBet)
				{
					if (Terminal.buf_cntr < TERM_CMD_BUF_SIZE)
						{_AddChar(c);}
				}
			}break;
		}
	}
	else {
		return TC_BufFull;
	}

	return TC_OK;
}

/// \brief ������ ������� ���������� ����� �������
/// \param {(*)(uint8_t, uint16_t)} - ������� ����������
/// \return none
void TerminalSetEnterCallback(void (*fcn)(uint8_t* rx_buf, uint16_t length))
{
	enter_callback = fcn;
}
