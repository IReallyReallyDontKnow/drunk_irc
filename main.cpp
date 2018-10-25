#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <thread>
#include <libircclient.h>
#include <stdio.h>
#include "resource.h"
#include "header.h"
#include "stack.h"

const char g_szClassName[] = "mainWinClass";
const char textWinName[] = "textOutWinClass";
const char textInWinName[] = "textInWinClass";

serverAddress tempAddress;

perm_stack server_list_memory;

char* buffAddr;
uint16_t buffPort;
char* buffName;
char* buffNick;

int server_index = -1;

HINSTANCE hInst;
int showCmd;

HWND textfield;
HWND inputbar;
HWND button;

RECTL rect;

LPTSTR replace(std::string str, std::string from, std::string to) {
    size_t start_pos = str.find(from);
    str.replace(start_pos, from.length(), to);
    return (LPTSTR)str.c_str();
}

//******//
//******//

typedef struct
{
	char 	* channel;
	char 	* nick;

} irc_ctx_t;

void addlog (const char * fmt, ...)
{
	FILE * fp;
	char buf[1024];
	va_list va_alist;

	va_start (va_alist, fmt);
#if defined (_WIN32)
	_vsnprintf (buf, sizeof(buf), fmt, va_alist);
#else
	vsnprintf (buf, sizeof(buf), fmt, va_alist);
#endif
	va_end (va_alist);

	//printf ("%s\n", buf);

	SetWindowText(textfield, replace("%s\n","%s",buf));

	if ( (fp = fopen ("irctest.log", "ab")) != 0 )
	{
		fprintf (fp, "%s\n", buf);
		fclose (fp);
	}
}


void dump_event (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	char buf[512];
	int cnt;

	buf[0] = '\0';

	for ( cnt = 0; cnt < count; cnt++ )
	{
		if ( cnt )
			strcat (buf, "|");

		strcat (buf, params[cnt]);
	}


	addlog ("Event \"%s\", origin: \"%s\", params: %d [%s]", event, origin ? origin : "NULL", cnt, buf);
}


void event_join (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	dump_event (session, event, origin, params, count);
	irc_cmd_user_mode (session, "+i");
	irc_cmd_msg (session, params[0], "Hi all");
}


void event_connect (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
	dump_event (session, event, origin, params, count);

	irc_cmd_join (session, ctx->channel, 0);
}


void event_privmsg (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	dump_event (session, event, origin, params, count);

	printf ("'%s' said me (%s): %s\n",
		origin ? origin : "someone",
		params[0], params[1] );
}


void dcc_recv_callback (irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length)
{
	static int count = 1;
	char buf[12];

	switch (status)
	{
	case LIBIRC_ERR_CLOSED:
	    SetWindowText(textfield, replace("DCC %d: chat closed\n","%d",(const int)id));
		//printf ("DCC %d: chat closed\n", id);
		break;

	case 0:
		if ( !data )
		{
		    SetWindowText(textfield, replace("DCC %d: chat connected\n","%d",id));
			//printf ("DCC %d: chat connected\n", id);
			irc_dcc_msg	(session, id, "Hehe");
		}
		else
		{
            SetWindowText(textfield, replace(replace("DCC %d: %s\n","%d",id),"%s",data));
			//printf ("DCC %d: %s\n", id, data);
			sprintf (buf, "DCC [%d]: %d", id, count++);
			irc_dcc_msg	(session, id, buf);
		}
		break;

	default:
        SetWindowText(textfield, replace(replace("DCC %d: error %s\n","%d",id),"%s",irc_strerror(status)));
		//printf ("DCC %d: error %s\n", id, irc_strerror(status));
		break;
	}
}


void dcc_file_recv_callback (irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length)
{
	if ( status == 0 && length == 0 )
	{
		printf ("File sent successfully\n");

		if ( ctx )
			fclose ((FILE*) ctx);
	}
	else if ( status )
	{
		printf ("File sent error: %d\n", status);

		if ( ctx )
			fclose ((FILE*) ctx);
	}
	else
	{
		if ( ctx )
			fwrite (data, 1, length, (FILE*) ctx);
		printf ("File sent progress: %d\n", length);
	}
}


void event_channel (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	char nickbuf[128];

	if ( count != 2 )
		return;

	printf ("'%s' said in channel %s: %s\n",
		origin ? origin : "someone",
		params[0], params[1] );

	if ( !origin )
		return;

	irc_target_get_nick (origin, nickbuf, sizeof(nickbuf));

	if ( !strcmp (params[1], "quit") )
		irc_cmd_quit (session, "of course, Master!");

	if ( !strcmp (params[1], "help") )
	{
		irc_cmd_msg (session, params[0], "quit, help, dcc chat, dcc send, ctcp");
	}

	if ( !strcmp (params[1], "ctcp") )
	{
		irc_cmd_ctcp_request (session, nickbuf, "PING 223");
		irc_cmd_ctcp_request (session, nickbuf, "FINGER");
		irc_cmd_ctcp_request (session, nickbuf, "VERSION");
		irc_cmd_ctcp_request (session, nickbuf, "TIME");
	}

	if ( !strcmp (params[1], "dcc chat") )
	{
		irc_dcc_t dccid;
		irc_dcc_chat (session, 0, nickbuf, dcc_recv_callback, &dccid);
		printf ("DCC chat ID: %d\n", dccid);
	}

	if ( !strcmp (params[1], "dcc send") )
	{
		irc_dcc_t dccid;
		irc_dcc_sendfile (session, 0, nickbuf, "irctest.c", dcc_file_recv_callback, &dccid);
		printf ("DCC send ID: %d\n", dccid);
	}

	if ( !strcmp (params[1], "topic") )
		irc_cmd_topic (session, params[0], 0);
	else if ( strstr (params[1], "topic ") == params[1] )
		irc_cmd_topic (session, params[0], params[1] + 6);

	if ( strstr (params[1], "mode ") == params[1] )
		irc_cmd_channel_mode (session, params[0], params[1] + 5);

	if ( strstr (params[1], "nick ") == params[1] )
		irc_cmd_nick (session, params[1] + 5);

	if ( strstr (params[1], "whois ") == params[1] )
		irc_cmd_whois (session, params[1] + 5);
}


void irc_event_dcc_chat (irc_session_t * session, const char * nick, const char * addr, irc_dcc_t dccid)
{
	printf ("DCC chat [%d] requested from '%s' (%s)\n", dccid, nick, addr);

	irc_dcc_accept (session, dccid, 0, dcc_recv_callback);
}


void irc_event_dcc_send (irc_session_t * session, const char * nick, const char * addr, const char * filename, unsigned long size, irc_dcc_t dccid)
{
	FILE * fp;
	printf ("DCC send [%d] requested from '%s' (%s): %s (%lu bytes)\n", dccid, nick, addr, filename, size);

	if ( (fp = fopen ("file", "wb")) == 0 )
		abort();

	irc_dcc_accept (session, dccid, fp, dcc_file_recv_callback);
}

void event_numeric (irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
	char buf[24];
	sprintf (buf, "%d", event);

	dump_event (session, buf, origin, params, count);
}

void loopRun(irc_session_t*);


//*********//
//*********//


void add_server(perm_stack_part input){
    std::ofstream listFile;
    listFile.open("serverlist.txt", std::ios::app);
    listFile << input.address.name << ":" << input.address.address << ":" << input.address.port << ":" << input.address.nick << ":" << "\n";
    listFile.close();
}

int8_t addressTypeCheck(char* input){
    int index = 0;
    int dotCount = 0;
    if(input[0] != '\n'){
        while(index < strlen(input)){
            if(input[index] == '.'){
                dotCount++;
            }
            index++;
        }
        if(dotCount == 3){
            //IPADDRESS
            char* newInput;
            newInput = new char[strlen(input) + 1];
            strcpy(newInput, input);
            strcat(newInput, ".");
            input = newInput;
            int IP[4] = {0,0,0,0};
            int numberCounter = 0;
            int convert[3]={0,0,0};
            int convCount = 0;
            int decimalCount = 1;

            for(int i = 0; i < strlen(input); i++){
                if(input[i] == '.'){

                    convCount--;
                    for(; convCount >= 0; convCount--){
                        if(convert[convCount] != 0){
                            IP[numberCounter] += (convert[convCount] * decimalCount);

                            decimalCount *= 10;
                        }
                    }
                    decimalCount = 1;
                    numberCounter++;
                    for(int i=0;i<3;i++){
                        convert[i] = 0;
                    }
                    convCount = 0;
                }
                else{
                    if(input[i]>=48 && input[i]<=57){
                        convert[convCount] = (input[i] - 48);
                        convCount++;
                    }
                    else{
                        return -1;
                    }
                }
            }
            for(int i=0; i<4; i++){
                if(IP[i] > 255 || IP[i] < 0){
                    return -1;
                }
            }
            return 1;
        }
        else{
            if(dotCount == 1 || dotCount == 2){
                //HOSTNAME
                return 0;
            }
            else{
                return -1;
            }
        }
    }
    else{
        return -1;
    }
}

bool loadList(temp_stack& para_stack){
    std::ifstream listFile;
    listFile.open("serverlist.txt", std::ios::out|std::ios::in);
    if(listFile.is_open()){
        std::string line;
        while(std::getline(listFile,line)){
            int str_index = 0;
            std::string paraArray;

            while(str_index < line.length()){
                if(line[str_index]!=':'){
                    paraArray += line[str_index];
                }
                else{
                    para_stack.push(paraArray);
                    paraArray = "";
                }
                str_index++;
            }
        }
        listFile.close();
        return true;
    }
    else{
        listFile.open("serverlist.txt", std::ios::out|std::ios::in|std::ios::trunc);
        std::cout << "No file present. Crating new file.";
        listFile.close();
        return false;
    }
}
/*
LRESULT CALLBACK inputBoxProc(HWND inputbar, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg){
        case WM_CLOSE:
            DestroyWindow(inputbar);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        case WM_CREATE:
        break;
        case WM_PAINT:

        break;
        default:

        return DefWindowProc(inputbar, msg, wParam, lParam);
    }
    return 0;
}
*/
LRESULT CALLBACK textBoxProc(HWND textfield, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg){
        case WM_CLOSE:
            DestroyWindow(textfield);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        case WM_CREATE:
        break;
        case WM_PAINT:

        break;
        default:

        return DefWindowProc(textfield, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;

        case WM_KEYDOWN:
            switch (LOWORD(wParam)){
                case VK_RETURN:
                    char* line;
                    GetDlgItemText(hwnd, IDC_INPUTBOX, (LPTSTR)line, 100);
                    SetWindowText(inputbar, "");
                break;
            }
        break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
            case IDC_BUTTON:
                char* line;
                GetDlgItemText(hwnd, IDC_INPUTBOX, (LPTSTR)line, 100);
                SetWindowText(inputbar, "");
            break;
            }
        break;
        case WM_CREATE:
            {
            GetWindowRect(hwnd, (LPRECT)&rect);

            textfield = CreateWindow(
                "EDIT",
                0,
                WS_BORDER | WS_CHILD | WS_VSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | ES_READONLY | ES_LEFT,
                0,
                0,
                300,
                600,
                hwnd, NULL ,hInst, NULL);

            inputbar = CreateWindow(
                "EDIT",
                0,
                WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL | ES_MULTILINE | ES_LEFT,
                0,
                ((float)(rect.bottom - rect.top) / 100) * 85,
                ((float)(rect.right - rect.left) / 100) * 50,
                ((float)(rect.bottom - rect.top) / 100) * 25,
                hwnd, (HMENU)IDC_INPUTBOX, hInst, NULL);

            button = CreateWindow(
                "BUTTON",
                "ENTER",
                WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                ((float)(rect.right - rect.left) / 100) * 50,
                ((float)(rect.bottom - rect.top) / 100) * 85,
                ((float)(rect.right - rect.left) / 100) * 25,
                ((float)(rect.bottom - rect.top) / 100) * 25,
                hwnd, (HMENU)IDC_BUTTON, hInst, NULL);

            ShowWindow(textfield,showCmd);
            ShowWindow(inputbar,showCmd);
            ShowWindow(button, showCmd);
            UpdateWindow(textfield);
            UpdateWindow(inputbar);
            UpdateWindow(button);
        }
        break;
        case WM_SIZE:
            GetWindowRect(hwnd, (LPRECT)&rect);

            MoveWindow(textfield,
                0,
                0,
                ((float)(rect.right - rect.left) / 100) * 75,
                ((float)(rect.bottom - rect.top) / 100) * 85,
                TRUE);

            MoveWindow(inputbar,
                0,
                ((float)(rect.bottom - rect.top) / 100) * 85,
                ((float)(rect.right - rect.left) / 100) * 50,
                ((float)(rect.bottom - rect.top) / 100) * 25,
                TRUE);

            MoveWindow(button,
                ((float)(rect.right - rect.left) / 100) * 50,
                ((float)(rect.bottom - rect.top) / 100) * 85,
                ((float)(rect.right - rect.left) / 100) * 25,
                ((float)(rect.bottom - rect.top) / 100) * 25,
                TRUE);

        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

BOOL CALLBACK SaveDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
            case IDOK:
                {
                int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_ADDRESS));
                int lenNam = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NAME));
                int lenNik = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NICK));

                buffAddr = (char*)GlobalAlloc(GPTR, len + 1);
                buffName = (char*)GlobalAlloc(GPTR, lenNam + 1);
                buffNick = (char*)GlobalAlloc(GPTR, lenNik + 1);

                GetDlgItemText(hwndDlg, IDC_ADDRESS, buffAddr, len + 1);
                GetDlgItemText(hwndDlg, IDC_NAME, buffName, lenNam + 1);
                GetDlgItemText(hwndDlg, IDC_NICK, buffNick, lenNik + 1);

                buffPort = GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE);
                EndDialog(hwndDlg, IDOK);
                }
            break;

            case IDCANCEL:
                EndDialog(hwndDlg, IDCANCEL);
            break;

            case IDSTORNO:
                EndDialog(hwndDlg, IDSTORNO);
            break;
            }
        }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        temp_stack init_stack;
        loadList(init_stack);
        int index = 0;
        int list_index = 0;
        std::string temp_list[4];
        if((init_stack.depth_counter)%4==0){
            while(index < init_stack.depth_counter){
                temp_list[list_index]=init_stack.pop();
                index++;
                if(list_index<3 && list_index>=0){
                    list_index++;
                }
                else{
                    list_index=0;
                    server_list_memory.push(temp_list[3],temp_list[2],atoi(temp_list[1].c_str()),temp_list[0]);
                    for(int i=0;i<4;i++){
                        temp_list[i]="";
                    }
                }
            }
        }
        else{
            std::cout << "Something horribly wrong with txt file.";
        }
        int loader_index=server_list_memory.depth_counter-1;
        while(loader_index >= 0){
            SendDlgItemMessage(hwndDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)server_list_memory.read(loader_index).name.c_str());
            loader_index--;
        }
    }
    return TRUE;

    case WM_CLOSE:
    {
        EndDialog(hwndDlg, -1);
    }
    return TRUE;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        switch(wmId)
        {

        {
        case IDC_ADD:
        int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SAVEDIL), hwndDlg, SaveDlg);

        if (ret == IDOK){
            if(addressTypeCheck(buffAddr)!= -1){
                SendDlgItemMessage(hwndDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)buffName);
                perm_stack_part temp_address_struct;
                temp_address_struct.address.address = buffAddr;
                temp_address_struct.address.name = buffName;
                temp_address_struct.address.port = buffPort;
                temp_address_struct.address.nick = buffNick;
                add_server(temp_address_struct);
                server_list_memory.push(temp_address_struct.address.name,temp_address_struct.address.address,temp_address_struct.address.port,temp_address_struct.address.nick);

            }
            else{
                MessageBox(hwndDlg, "Wrong address format.", "Error", MB_OK | MB_ICONINFORMATION);
            }
        }
        else if (ret == -1){
            MessageBox(hwndDlg, "Failed to open dialog.", "Error", MB_OK | MB_ICONINFORMATION);
        }
        break;
        }


        {
        case IDC_REMOVE:
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
            int select_index = (int)SendMessage(hwndList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            BOOL err = server_list_memory.del_index(select_index);
            SendMessage(hwndList, LB_DELETESTRING, (WPARAM)select_index, (LPARAM)0);
            if(err == -1){
                std::cout << "Could not delete this server.";
            }

        break;
        }

        case IDC_START:
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
            EndDialog(hwndDlg,(int)SendMessage(hwndList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0));
        }
        break;

        case IDC_LIST:
        {
            if(wmEvent == LBN_DBLCLK){
                HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
                EndDialog(hwndDlg,(int)SendMessage(hwndList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0));

            }
        }
        break;
        }
    }
    return TRUE;
    }
    return FALSE;
}

void loopRun(irc_session_t * s){
    if(irc_run(s)){
        std::cout << "Error with connection loop.";
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

    hInst=hInstance;
    showCmd = nShowCmd;
    InitCommonControls();
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    WNDCLASSEX txWc;

    txWc.cbSize        = sizeof(WNDCLASSEX);
    txWc.style         = 0;
    txWc.lpfnWndProc   = textBoxProc;
    txWc.cbClsExtra    = 0;
    txWc.cbWndExtra    = 0;
    txWc.hInstance     = hInstance;
    txWc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    txWc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    txWc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    txWc.lpszMenuName  = NULL;
    txWc.lpszClassName = textWinName;
    txWc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&txWc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
/*
    WNDCLASSEX inpWc;

    inpWc.cbSize        = sizeof(WNDCLASSEX);
    inpWc.style         = 0;
    inpWc.lpfnWndProc   = inputBoxProc;
    inpWc.cbClsExtra    = 0;
    inpWc.cbWndExtra    = 0;
    inpWc.hInstance     = hInstance;
    inpWc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    inpWc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    inpWc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    inpWc.lpszMenuName  = NULL;
    inpWc.lpszClassName = textInWinName;
    inpWc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&inpWc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
*/
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "DrunkIRC",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    server_index = DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgMain);
    if(server_index == -1)
    {
        return 0;
    }
    ShowWindow(hwnd,nShowCmd);
    UpdateWindow(hwnd);

    //******//

    irc_callbacks_t callbacks;
    irc_ctx_t ctx;
    memset(&callbacks, 0, sizeof(callbacks));

    callbacks.event_connect = event_connect;
	callbacks.event_join = event_join;
	callbacks.event_nick = dump_event;
	callbacks.event_quit = dump_event;
	callbacks.event_part = dump_event;
	callbacks.event_mode = dump_event;
	callbacks.event_topic = dump_event;
	callbacks.event_kick = dump_event;
	callbacks.event_channel = event_channel;
	callbacks.event_privmsg = event_privmsg;
	callbacks.event_notice = dump_event;
	callbacks.event_invite = dump_event;
	callbacks.event_umode = dump_event;
	callbacks.event_ctcp_rep = dump_event;
	callbacks.event_ctcp_action = dump_event;
	callbacks.event_unknown = dump_event;
	callbacks.event_numeric = event_numeric;

	callbacks.event_dcc_chat_req = irc_event_dcc_chat;
	callbacks.event_dcc_send_req = irc_event_dcc_send;

    irc_session_t * session = irc_create_session(&callbacks);
    if(!session){
        std::cout << "Error with creating session.";
    }

    ctx.channel = "#general";
    ctx.nick = "NaMe";

    irc_set_ctx(session, &ctx);

    irc_option_set(session, LIBIRC_OPTION_STRIPNICKS);

    if(irc_connect(session, "irc.bazina.tk", 6667, NULL, "NaMe","nAmE","UNKNOWN")){
        std::cout << "Error with connecting.";
    }

    std::thread loopThread(loopRun,session);

    //******//

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    loopThread.join();

    return Msg.wParam;
}
