// sstart.cpp : Defines the entry point for the console application
// 
// SStart Copyright (c) 2012-2013 Tamer Rizk (trizk[at]inficron.com)
// Adapted from Silent Start by Robert.A.Davies (bobbigmac) admin@robertadavies.co.uk
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HANDLE processes[2] = {NULL,NULL};	 
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	DWORD exitCode = 1;
	
	size_t cmdLength = _mbstrlen(lpCmdLine);

	char *cmdLine;
	int mode = -1;
	int pid = -1;
    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);


	cmdLine = malloc(cmdLength);
	strcpy(cmdLine, lpCmdLine);

	for (mode=-1;++mode<cmdLength;){
		if(cmdLine[mode]<48 || cmdLine[mode]>57){
			break;
		}
	}
					
	if(mode==cmdLength){
		pid = atoi(cmdLine);
		mode = 1;
	}else{
		pid = GetCurrentProcessId();
		mode = 0;
	}

	if( Process32First(snapshot, &pe)) {
        do {
                if (mode==0 && pid==pe.th32ProcessID) {					
					processes[0] = OpenProcess(SYNCHRONIZE, FALSE, pe.th32ParentProcessID);
					break;
				}else if(mode==1 && pid==pe.th32ParentProcessID){
					processes[1] = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
					TerminateProcess(processes[1], 0);
					CloseHandle(processes[1]);
				}	
        } while( Process32Next(snapshot, &pe));
    }
	CloseHandle(snapshot);
	if(mode ==1 ){
		return 0;
	}
	if( cmdLine != NULL && processes[0]) //Could catch this fail-condition better.
	{
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		if(CreateProcessA(NULL,
			cmdLine,
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi )           // Pointer to PROCESS_INFORMATION structure
		){		
			processes[1] = pi.hProcess;
			WaitForMultipleObjects(2,processes,FALSE,INFINITE);

			GetExitCodeProcess(pi.hProcess,&exitCode);	
			TerminateProcess(pi.hProcess, 0);

			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
			CloseHandle(processes[0]);
			return exitCode;	
		}
	}
	CloseHandle(processes[0]);
	return 1;
}

