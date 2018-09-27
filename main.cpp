#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include "resource.h"
#include "header.h"
#include "stack.h"

HINSTANCE hInst;

const char g_szClassName[] = "mainWinClass";

serverAddress tempAddress;

perm_stack server_list_memory;

char* buffAddr;
uint16_t buffPort;
char* buffName;
char* buffNick;

int server_index;

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
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

BOOL CALLBACK SaveDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
        }
        return TRUE;

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
        EndDialog(hwndDlg, 0);
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
            server_index = (int)SendMessage(hwndList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            }
        break;

        case IDC_LIST:
        {
            if(wmEvent == LBN_DBLCLK){
                HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
                server_index = (int)SendMessage(hwndList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);

            }
        }
        break;
        }
    }
    return TRUE;
    }
    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

    hInst=hInstance;
    InitCommonControls();
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    //Step 1: Registering the Window Class
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

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "The title of my window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgMain);
    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
