
//#include <stdio.h>
#include <stdlib.h>
#include "putty.h"
#include "terminal.h"
#include <fcntl.h>
#include <io.h>
#include <windowsx.h>
#include "puttymem.h"

//PuTTYAttach
struct  copydata_msg_data_list_item g_copydata_msg_data_list_head;
UINT    file_transport = 0;
HWND    g_puttyattach_hwnd = 0;

static  void add_copydata_msg_data_to_list(struct copydata_msg_data * data)
{
    struct  copydata_msg_data_list_item * p_item = &g_copydata_msg_data_list_head;
    while(p_item)
    {
        if(p_item->next)
        {
            p_item = p_item->next;
        }
        else
        {
            struct  copydata_msg_data_list_item * new_item;
            new_item = smalloc(sizeof(struct copydata_msg_data_list_item));
            if(new_item)
            {
                new_item->data = data;
                new_item->next = 0;
                p_item->next = new_item;
            }
			break;
        }
    }
}

static  struct copydata_msg_data * get_copydata_msg_data_list_first_item()
{
    struct  copydata_msg_data_list_item * p_item = g_copydata_msg_data_list_head.next;
    if(p_item)
    {
        struct copydata_msg_data * data = p_item->data;
        g_copydata_msg_data_list_head.next = p_item->next;
        safefree(p_item);
        return data;
    }
    else
    {
       return 0;
    }
}
////////////

void	key_press(HWND hwnd, int wParam)
{
	struct copydata_msg_data * new_data = smalloc(sizeof(struct copydata_msg_data) + sizeof(char));
	if (new_data)
	{
		COPYDATASTRUCT pCopyData;
		int c = wParam & 0xFF;
		if (c < VK_SPACE) {}
		else if (c < VK_END)	//blank space，PAGE UP,PAGE DOWN 
		{
			c = 0;
		}
		else if (c <= VK_HELP)
		{
			if ((c == VK_UP) || (c == VK_DOWN))
			{
				c = 0;
			}
		}
		else if (c < 0x41)		//Num Key
		{
			if ((GetKeyState(VK_CONTROL) & 0x8000) == 0)
			{
				c = 0;
			}
		}
		else if (c < VK_LWIN)  //CHAR key
		{
			if ((GetKeyState(VK_CONTROL) & 0x8000) == 0)
			{
				c = 0;
			}
			else
			{
				c = c - 0x40;
			}
		}
		else if (c < VK_NUMPAD0) {}
		else if (c < VK_F1)	//NUMPAD
		{
			if ((GetKeyState(VK_CONTROL) & 0x8000) == 0)
			{
				c = 0;
			}
		}
		else if (c < VK_OEM_1) {}
		else if (c < 0xE3)
		{
			if ((GetKeyState(VK_CONTROL) & 0x8000) == 0)
			{
				c = 0;
			}
		}
		else {}
		new_data->msg_type = PUTTYATTACH_PUTTY_KEYPRESS_POPUP;
		new_data->data_size = sizeof(char);
		new_data->data[0] = (unsigned char)c;
		new_data->sender_wnd = hwnd;

		pCopyData.dwData = new_data->msg_type; //
		pCopyData.cbData = sizeof(struct copydata_msg_data) + sizeof(char); //
		pCopyData.lpData = (void*)new_data; //
		SendMessage(g_puttyattach_hwnd, WM_COPYDATA, 0, (LPARAM)&pCopyData);
		safefree(new_data);
	}
}
void    recv_dropfiles_msg(HWND hwnd, WPARAM wParam,LPARAM lParam)
{
	HDROP hDrop = (HDROP)wParam;
	POINT mousePos;

	lParam = GetMessagePos();
	mousePos.x = GET_X_LPARAM(lParam);
	mousePos.y = GET_Y_LPARAM(lParam);

	char * p_file = 0;
	int new_len = 0;
	int len = 0;
	for (UINT i = 0; i < DragQueryFileA(hDrop, 0xFFFFFFFF, 0, 0); ++i)
	{
		new_len += DragQueryFileA(hDrop, i, 0, 0);
		new_len += 1;
		p_file = srealloc(p_file, new_len);
		if (p_file)
		{
			DragQueryFileA(hDrop, i, p_file + len, new_len - len);
			p_file[new_len - 1] = '\n';
			len = new_len;
		}
	}

	if (p_file)
	{
		p_file[len-1] = 0;		
		if (g_puttyattach_hwnd)
		{
			struct PuttyDropFiles * p_drop_files = smalloc(sizeof(struct PuttyDropFiles) + len);
			p_drop_files->x = mousePos.x;
			p_drop_files->y = mousePos.y;

            
			p_drop_files->data_size = len;
			memcpy(p_drop_files->data, p_file, len);
			safefree(p_file);

            struct copydata_msg_data * new_data = smalloc(sizeof(struct copydata_msg_data) + sizeof(struct PuttyDropFiles) + len);
			if (new_data)
			{
				COPYDATASTRUCT pCopyData;
				new_data->sender_wnd = hwnd;
				new_data->msg_type = PUTTYATTACH_PUTTYDROPFILES_POPUP;
				new_data->data_size = sizeof(struct PuttyDropFiles) + len;
				memcpy(new_data->data, p_drop_files, new_data->data_size);
				safefree(p_drop_files);

				pCopyData.dwData = new_data->msg_type; //
				pCopyData.cbData = sizeof(struct copydata_msg_data) + new_data->data_size; //
				pCopyData.lpData = (void*)new_data; //
				SendMessage(g_puttyattach_hwnd, WM_COPYDATA, 0, (LPARAM)&pCopyData);
				safefree(new_data);

                /*new_data->sender_wnd = g_puttyattach_hwnd;
                new_data->msg_type = PUTTYATTACH_PUTTYDROPFILES_POPUP;
                new_data->data_size = sizeof(struct PuttyDropFiles) + len;
				memcpy(new_data->data, p_drop_files, new_data->data_size);
                add_copydata_msg_data_to_list(new_data);*/
			}
		}
		else
		{
			safefree(p_file);
		}

	}
	DragFinish(hDrop);
}
void	recv_copyData_msg(HWND hwnd, WPARAM wParam, LPARAM lParam, Terminal * term)
{
	PCOPYDATASTRUCT pCopyData = (PCOPYDATASTRUCT)lParam;
	switch (pCopyData->dwData)
	{
	case PUTTYATTACH_INIT_EXCHANGE:
	{
		struct copydata_msg_data * p_data = (struct copydata_msg_data *)pCopyData->lpData;
		if (p_data)
		{
			g_puttyattach_hwnd = p_data->sender_wnd;
			struct copydata_msg_data * new_data = smalloc(sizeof(struct copydata_msg_data) + p_data->data_size);
			if (new_data)
			{
				memcpy(new_data, p_data, sizeof(struct copydata_msg_data) + p_data->data_size);
				add_copydata_msg_data_to_list(new_data);
			}
		}
		break;
	}
	case PUTTYATTACH_FILETRANSOPRT_ACCEPT:
	{
		struct copydata_msg_data * p_data = (struct copydata_msg_data *)pCopyData->lpData;
		if (p_data && (p_data->data_size == sizeof(unsigned char)))
		{
			file_transport = *((unsigned char*)p_data->data);
		}
		break;
	}
	case PUTTYATTACH_JUST_TERM_OUT_ACCEPT:
	{
		struct copydata_msg_data * p_data = (struct copydata_msg_data *)pCopyData->lpData;
		if (p_data && (p_data->data_size > 0))
		{
			if (term) term_data(term, p_data->data, p_data->data_size);
		}
		break;
	}
	}
	/*if (pCopyData->dwData & PUTTYATTACH_MSGTYPE_GETDATA)
	{
		struct copydata_msg_data * p_data = (struct copydata_msg_data *)pCopyData->lpData;
		if (p_data)
		{
            g_puttyattach_hwnd = p_data->sender_wnd;
			struct copydata_msg_data * new_data = smalloc(sizeof(struct copydata_msg_data) + p_data->data_size);
			if (new_data)
			{
				memcpy(new_data, p_data, sizeof(struct copydata_msg_data) + p_data->data_size);
                add_copydata_msg_data_to_list(new_data);
			}
		}
	}
	else
	{
		switch (pCopyData->dwData)
		{
		case PUTTYATTACH_FILETRANSOPRT_ACCEPT:
		{
			struct copydata_msg_data * p_data = (struct copydata_msg_data *)pCopyData->lpData;
			if (p_data && (p_data->data_size == sizeof(unsigned char)))
			{
				file_transport = *((unsigned char*)p_data->data);
			}
			break;
		}
		case PUTTYATTACH_JUST_TERM_OUT_ACCEPT:
		{
			struct copydata_msg_data * p_data = (struct copydata_msg_data *)pCopyData->lpData;
			if (p_data && (p_data->data_size > 0))
			{
                struct copydata_msg_data * new_data = smalloc(sizeof(struct copydata_msg_data) + p_data->data_size);
    			if (new_data)
    			{
                    memcpy(new_data, p_data, sizeof(struct copydata_msg_data) + p_data->data_size);
                    add_copydata_msg_data_to_list(new_data);
    			}
			}
			break;
		}
		}
	}*/
}
void	send_excharng_copyData_msg(HWND hwnd, Terminal * term)
{
	//PuTTYAttach send WM_COPYDATA to get some datas,putty send WM_COPYDATA carry the data give to PuTTYAttach.
	struct copydata_msg_data * msg_data = get_copydata_msg_data_list_first_item();
    while(msg_data)
    {
        if(msg_data->sender_wnd) 
        {
			switch (msg_data->msg_type)
			{
			case PUTTYATTACH_INIT_EXCHANGE:
			{
				struct copydata_msg_data * send_data = smalloc(sizeof(struct copydata_msg_data) + sizeof(unsigned char));
				if (send_data)
				{
					memcpy(send_data, msg_data, sizeof(struct copydata_msg_data));
					send_data->data_size = sizeof(unsigned char);
					*((unsigned char *)send_data->data) = 1;
					send_data->sender_wnd = hwnd;
					COPYDATASTRUCT pCopyData;
					pCopyData.dwData = PUTTYATTACH_INIT_EXCHANGE; //
					pCopyData.cbData = sizeof(struct copydata_msg_data) + sizeof(unsigned char); //
					pCopyData.lpData = (void*)send_data; //
					SendMessage(msg_data->sender_wnd, WM_COPYDATA, 0, (LPARAM)&pCopyData);
					safefree(send_data);
				}
				break;
			}
			}           
        }
        safefree(msg_data);
        msg_data = get_copydata_msg_data_list_first_item();
    }
}
void  stdout_printf(const char * d, int len)
{

	fwrite(d, len, 1, stdout);
	fflush(stdout);
}
void  stderr_printf(const char * d, int len)
{
	fwrite(d, len, 1, stderr);
	fflush(stderr);
}

size_t std_win_seat_output(Terminal * term, const void *data, size_t len)
{
	//Receive raw data from the network
	if (file_transport)
	{
		stdout_printf(data, len);
		return 0;
	}
	else
	{
		return term_data(term, data, len);
	}
}

void std_term_out(const char * data, int len)
{
	//term print out
	stderr_printf(data,len);
}
void    read_from_stdin(Terminal *term)
{
	unsigned int bread, avail;
	char buffer[8200];
	bread = 0;
	HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
	if (in != INVALID_HANDLE_VALUE)
	{
		PeekNamedPipe(in, buffer, 1, &bread, &avail, NULL);
		//check to see if there is any data to read from stdout
		while (bread)
		{
			bread = 0;
			if (ReadFile(in, buffer, sizeof(buffer) - 8, &bread, NULL))
			{
				if (bread)
				{
					if (backend_sendok(term->backend))
					{
						backend_send(term->backend, buffer, bread);
					}

				}
			}
			bread = 0;
			PeekNamedPipe(in, buffer, 1, &bread, &avail, NULL);
		}
	}

}

/////////////////////////////////////////////////////////////////////////////////
//stdin read thread
typedef struct stdin_handle_list_node stdin_handle_list_node;
struct stdin_handle_list_node {
	stdin_handle_list_node *next, *prev;
};


static stdin_handle_list_node stdin_ready_head[1];
static CRITICAL_SECTION stdin_ready_critsec[1];

/*
 * Event object used by all subthreads to signal that they've just put
 * something on the ready list, i.e. that the ready list is non-empty.
 */
static HANDLE stdin_ready_event = INVALID_HANDLE_VALUE;

typedef size_t(*stdin_handle_inputfn_t)(
	struct stdin_handle_input *h, const void *data, size_t len, int err);

struct stdin_handle_input {
	/* the handle itself */
	stdin_handle_list_node ready_node;       /* for linking on to the ready list */
	char buffer[8200];                 /* the data read from the handle */
	DWORD len;                         /* how much data that was */
	stdin_handle_inputfn_t stdin_gotdata;
	HANDLE ev_from_main;               /* event used to signal back to us */
	bool moribund;                     /* are we going to kill this soon? */
	bool done;                         /* request subthread to terminate */
	bool defunct;                      /* has the subthread already gone? */
	bool busy;                         /* operation currently in progress? */
	Terminal * term;
};

struct HandleWait {
	HANDLE handle;
	handle_wait_callback_fn_t callback;
	void *callback_ctx;

	int index;                    /* sort key for tree234 */
};

static void stdin_add_to_ready_list(stdin_handle_list_node *node)
{
	/*
	 * Called from subthreads, when their handle has done something
	 * that they need the main thread to respond to. We append the
	 * given list node to the end of the ready list, and set
	 * stdin_ready_event to signal to the main thread that the ready list is
	 * now non-empty.
	 */
	EnterCriticalSection(stdin_ready_critsec);
	node->next = stdin_ready_head;
	node->prev = stdin_ready_head->prev;
	node->next->prev = node->prev->next = node;
	SetEvent(stdin_ready_event);
	LeaveCriticalSection(stdin_ready_critsec);
}

static void stdin_remove_from_ready_list(stdin_handle_list_node *node)
{
	/*
	 * Called from the main thread, just before destroying a 'struct
	 * handle' completely: as a precaution, we make absolutely sure
	 * it's not linked on the ready list, just in case somehow it
	 * still was.
	 */
	EnterCriticalSection(stdin_ready_critsec);
	node->next->prev = node->prev;
	node->prev->next = node->next;
	node->next = node->prev = node;
	LeaveCriticalSection(stdin_ready_critsec);
}

static void stdin_handle_destroy(struct stdin_handle_input *h)
{
	CloseHandle(h->ev_from_main);
	stdin_remove_from_ready_list(&h->ready_node);
	sfree(h);
}

static void stdin_handle_free(struct stdin_handle_input *h)
{
	if (h == 0) return;
	if (h->busy)
	{
		/*
		 * If the handle is currently busy, we cannot immediately free
		 * it, because its subthread is in the middle of something.
		 * (Exception: foreign handles don't have a subthread.)
		 *
		 * Instead we must wait until it's finished its current
		 * operation, because otherwise the subthread will write to
		 * invalid memory after we free its context from under it. So
		 * we set the moribund flag, which will be noticed next time
		 * an operation completes.
		 */
		h->moribund = true;
	}
	else if (h->defunct)
	{
		/*
		 * There isn't even a subthread; we can go straight to
		 * handle_destroy.
		 */
		stdin_handle_destroy(h);
	}
	else
	{
		/*
		 * The subthread is alive but not busy, so we now signal it
		 * to die. Set the moribund flag to indicate that it will
		 * want destroying after that.
		 */
		h->moribund = true;
		h->done = true;
		h->busy = true;
		SetEvent(h->ev_from_main);
	}
}


static void stdin_handle_ready(struct stdin_handle_input *h)
{
	if (h->moribund)
	{
		/*
		 * A moribund handle is one which we have either already
		 * signalled to die, or are waiting until its current I/O op
		 * completes to do so. Either way, it's treated as already
		 * dead from the external user's point of view, so we ignore
		 * the actual I/O result. We just signal the thread to die if
		 * we haven't yet done so, or destroy the handle if not.
		 */
		if (h->done)
		{
			stdin_handle_destroy(h);
		}
		else
		{
			h->done = true;
			h->busy = true;
			SetEvent(h->ev_from_main);
		}
		return;
	}

	h->busy = false;
	/*
	 * A signal on an input handle means data has arrived.
	 */
	if (h->len == 0)
	{
		/*
		 * EOF, or (nearly equivalently) read error.
		 */
		h->defunct = true;
		h->stdin_gotdata(h, NULL, 0, -1);
	}
	else
	{
		h->stdin_gotdata(h, h->buffer, h->len, 0);
		SetEvent(h->ev_from_main);
		h->busy = true;
	}
}

static void stdin_handle_ready_callback(void *vctx)
{
	/*
	 * Called when the main thread detects stdin_ready_event, indicating
	 * that at least one handle is on the ready list. We empty the
	 * whole list and process the handles one by one.
	 *
	 * It's possible that other handles may be destroyed, and hence
	 * taken _off_ the ready list, during this processing. That
	 * shouldn't cause a deadlock, because according to the API docs,
	 * it's safe to call EnterCriticalSection twice in the same thread
	 * - the second call will return immediately because that thread
	 * already owns the critsec. (And then it takes two calls to
	 * LeaveCriticalSection to release it again, which is just what we
	 * want here.)
	 */
	EnterCriticalSection(stdin_ready_critsec);
	while (stdin_ready_head->next != stdin_ready_head) {
		stdin_handle_list_node *node = stdin_ready_head->next;
		node->prev->next = node->next;
		node->next->prev = node->prev;
		node->next = node->prev = node;
		stdin_handle_ready(container_of(node, struct stdin_handle_input, ready_node));
	}
	LeaveCriticalSection(stdin_ready_critsec);
}

static inline void stdin_ensure_ready_event_setup(void)
{
	if (stdin_ready_event == INVALID_HANDLE_VALUE) {
		stdin_ready_head->prev = stdin_ready_head->next = stdin_ready_head;
		InitializeCriticalSection(stdin_ready_critsec);
		stdin_ready_event = CreateEvent(NULL, false, false, NULL);
		add_handle_wait(stdin_ready_event, stdin_handle_ready_callback, NULL);
	}
}

static size_t stdin_got_data(struct stdin_handle_input *h, const void *data, size_t len, int err)
{
	if (len == 0) return 0;

	if (backend_sendok(h->term->backend))
	{
		backend_send(h->term->backend, data, len);
	}
	return len;
}
static DWORD WINAPI stdin_handle_input_threadfunc(void *param)
{
	struct stdin_handle_input *ctx = (struct stdin_handle_input *) param;
	HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
	while (1)
	{
		if (ReadFile(in, ctx->buffer, sizeof(ctx->buffer) - 8, &ctx->len, 0))
		{
			if (ctx->len == 0) continue;
			stdin_add_to_ready_list(&ctx->ready_node);
			WaitForSingleObject(ctx->ev_from_main, INFINITE);
			if (ctx->done)
			{
				stdin_add_to_ready_list(&ctx->ready_node);
				break;
			}
		}
		else
		{
			if (ctx->len == 0)
			{
				stdin_add_to_ready_list(&ctx->ready_node);
				break;
			}
		}

	}
	return 0;

}

static struct stdin_handle_input *stdin_handle_input_new(Terminal * term, stdin_handle_inputfn_t stdin_gotdata)
{
	struct stdin_handle_input *h = snew(struct stdin_handle_input);
	DWORD in_threadid; /* required for Win9x */
	memset(h, 0, sizeof(struct stdin_handle_input));
	h->ev_from_main = CreateEvent(NULL, false, false, NULL);
	h->defunct = false;
	h->moribund = false;
	h->done = false;
	h->stdin_gotdata = stdin_gotdata;
	h->term = term;
	stdin_ensure_ready_event_setup();

	HANDLE hThread = CreateThread(NULL, 0, stdin_handle_input_threadfunc,
		h, 0, &in_threadid);
	if (hThread)
		CloseHandle(hThread);          /* we don't need the thread handle */
	h->busy = true;

	return h;
}

static struct stdin_handle_input * g_stdin_handle_input;
void    stdin_input_new(Terminal * term)
{
	g_stdin_handle_input = stdin_handle_input_new(term, stdin_got_data);
}

void    stdin_input_free()
{
	stdin_handle_free(g_stdin_handle_input);
}



