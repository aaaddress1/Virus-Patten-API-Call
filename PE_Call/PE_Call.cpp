#include "stdafx.h"
#include <windows.h>
#include <string>


int StrVal(std::string Data)
{
	return std::stoi(Data);
}

bool ChkPatten(char* Str, char* Patten)
{
	bool retn = true;
	std::string n = "";
	for (int i = 0; Patten[i]; i++)
	{
		if (*(char*)(Patten + i) == '=')
		{
			int LPos = StrVal(n);
			char RChar = *(char*)(Patten + i + 1);
			if (Str[LPos] != RChar)
			{
				retn = false;
				break;
			}
			n = "";
			i += 2;
		}
		else
		{
			n += *(char*)(Patten + i);
		}
	}
	return retn;
}
int GetKernel32Mod()
{
	int dRetn = 0;
	_asm
	{
		Main:
			mov ebx, fs:[0x30] //PEB
			mov ebx, [ebx + 0x0c]//Ldr
			mov ebx, [ebx + 0x1c]//InInitializationOrderModuleList
		Search :
			mov eax, [ebx + 0x08]//Point to Current Modual Base.
			mov ecx, [ebx + 0x20]//Point to Current Name.
			mov ecx, [ecx + 0x18]
			cmp cl, 0x00//Test if Name[25] == \x00.
			mov ebx, [ebx + 0x00]
			jne Search
			mov[dRetn], eax
	}
	return dRetn;
}

int GetProcAddrEx(int ModBase, char* Patten)
{
	int Data = 0;
	_asm
	{
	
		mov esi, ModBase
			mov edx, [esi + 0x3c]//e_ifanw
			add edx, esi//Point To NTHeader Address.
			add edx, 0x18//OptionalHeader
			add edx, 0x60//IMAGE_OPTIONAL_HEADER -> DataDirectory
			add edx, 0x00//Export Directory Offset Addr
			mov edx, [edx]//Export Directory Addr
			add edx, esi

			mov ecx, [edx + 0x18]//Number Of Names

		FindAddrOfNames:
			mov ebx, [edx + 0x20]//Addr Of Name Offset
			add ebx, esi//Point to Addr Of Name
			mov ebx, [ebx + ecx * 4]
			add ebx, esi//Current Name Addr
			dec ecx
			jl ExitFunc


			pushad
			push Patten
			push ebx
			call ChkPatten
			add esp, 0x08
			test al, al
			popad
			jz FindAddrOfNames

			GetAddr :
			inc ecx
			mov ebx, [edx + 0x24]//AddressOfNameOrdinals Offset
			add ebx, esi
			mov cx, [ebx + ecx * 2]//ECX = AddressOfNameOrdinals  + Index As WORD(2 BYTE)
			mov ebx, [edx + 0x1c]//AddressOfFunction Array Offset.
			add ebx, esi
			mov ebx, [ebx + ecx * 4]//Set EDX = Value Of AddressOfFunction[Index] = Offset.
			add esi, ebx
			mov[Data], esi
			ExitFunc :
	}
	return Data;
}



int _tmain(int argc, char* argv[])
{
	int Kernel32Base = GetKernel32Mod();
	HMODULE(WINAPI*MyLoadLibraryA)(LPCTSTR lpFileName) = (HMODULE(WINAPI *)(LPCTSTR))(GetProcAddrEx(Kernel32Base, "0=L;3=d;4=L;11=A"));
	
	int User32Base = (int)MyLoadLibraryA((LPCTSTR)"User32.dll");
	int(WINAPI*MyMessageBoxA)(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType) = (int(WINAPI *)(HWND, LPCTSTR, LPCTSTR, UINT))(GetProcAddrEx(User32Base, "0=M;2=s;3=s;7=B;10=A"));
	MyMessageBoxA(0,(LPCTSTR)"Adr Handsome!", (LPCTSTR)"Adr Say", 0);
	return 0;
}

