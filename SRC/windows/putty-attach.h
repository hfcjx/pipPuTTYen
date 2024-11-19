#ifndef PUTTY_ATTACH_H
#define PUTTY_ATTACH_H
//PuTTYAttach
#include <Windows.h>
void  stdout_printf(const char * d, int len);
void  stderr_printf(const char * d, int len);
void  read_from_stdin(Terminal *term);
void std_term_out(const char * data, int len);
void stdin_input_new(Terminal * term);
void    stdin_input_free();
size_t std_win_seat_output(Terminal * term, const void *data, size_t len);
void	recv_copyData_msg(HWND hwnd, WPARAM wParam, LPARAM lParam, Terminal * term);
void	send_excharng_copyData_msg(HWND hwnd, Terminal * term);
void    recv_dropfiles_msg(HWND hwnd, WPARAM wParam,LPARAM lParam);
void	key_press(HWND hwnd, int wParam);

struct copydata_msg_data
{
	HWND    sender_wnd;
	int		msg_type;
	int     data_size;
	unsigned char   data[];
};
struct PuttyDropFiles
{
	int  x;
	int  y;
	int     data_size;
	unsigned char   data[];
};

struct copydata_msg_data_list_item
{
    struct copydata_msg_data * data;
    struct copydata_msg_data_list_item * next;
};
#define PUTTYATTACH_INIT_EXCHANGE					 1
#define PUTTYATTACH_FILETRANSOPRT_ACCEPT             2
#define PUTTYATTACH_JUST_TERM_OUT_ACCEPT             3
#define PUTTYATTACH_PUTTYDROPFILES_POPUP		     4
#define PUTTYATTACH_PUTTY_KEYPRESS_POPUP		     5


extern UINT    file_transport;

#endif
