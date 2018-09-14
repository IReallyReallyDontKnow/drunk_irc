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

serverAddress tempAddress;

perm_stack server_list_memory;

char* buffAddr;
uint16_t buffPort;
char* buffName;

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
        //para_stack = new Stack;
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
        return true;
    }
    else{
        listFile.open("serverlist.txt", std::ios::out|std::ios::in|std::ios::trunc);
        std::cout << "NOPE";
        return false;
    }
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

                buffAddr = (char*)GlobalAlloc(GPTR, len + 1);
                buffName = (char*)GlobalAlloc(GPTR, lenNam + 1);

                GetDlgItemText(hwndDlg, IDC_ADDRESS, buffAddr, len + 1);
                GetDlgItemText(hwndDlg, IDC_NAME, buffName, lenNam + 1);

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
        if(init_stack.depth_counter%4==0){
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
            //std::cout << "_" << server_list_memory.read(loader_index).name << "_";
            SendDlgItemMessage(hwndDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)server_list_memory.read(loader_index).name);
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
        switch(LOWORD(wParam))
        {
        case IDC_ADD:
        int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SAVEDIL), hwndDlg, SaveDlg);

        if (ret == IDOK){
            if(addressTypeCheck(buffAddr)!= -1){
                int index = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)buffName);
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
    }
    return TRUE;
    }
    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst=hInstance;
    InitCommonControls();
    return DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgMain);
}
