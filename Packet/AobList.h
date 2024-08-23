﻿#ifndef __AOB_LIST_H__
#define __AOB_LIST_H__

#include<Windows.h>
#include<string>

/*
	v20Xかv30XあたりからPacket Senderの対策が追加されています
		1) Thread IDの確認, Main Thread以外からSendPacketを呼び出すと検出されます, Packet Senderを使わない限りは回避不要です
		2) Return Addressの確認, SendPacketが呼ばれた時のReturn Addressがexeの範囲外のだと検出されます
		3) Memoryの確認, Return Address - 0x05がcall SendPacketでない場合に検出されます
		4) Memoryの確認, Return Adressがret (0xC3)の場合も検出されます
		5) Return Addressの確認, SendPacketが呼ばれた時のReturn Addressがexeのthemidaのvirtualizerの範囲内だと3, 4が無視されます

	以下のメモリを探しSendPacketのフックに利用します
		call SendPacket // Bypass Memory Check
		add rsp,XX // Bypass Return Address Check, FakeReturn
		ret

	フックから_SendPacketの呼び出す時の書き方
		sub rsp,XX // 適当に利用した処理でStackがずれる分を事前に調整します
		mov rax,FakeReturn // ここがRetun Addressとなります
		push rax
		mov rax,_SendPacket // push + retで任意のReturn Addressを設置したcallを実行します
		push rax
		ret
*/

// Send Hook
std::wstring AOB_SendPacket[] = {
	#ifdef _WIN64
	// JMS v414.1 and TWMS v263.3, 1422C1F40 1st result
	L"48 89 54 24 10 48 89 4C 24 08 56 57 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? E9",
	// v410.2 from v403.1
	L"48 89 54 24 10 48 89 4C 24 08 56 57 48 81 EC ?? ?? ?? ?? 48 C7 84 24 ?? ?? ?? ?? FE FF FF FF 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? E9",
	#else
	// v131.0
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 10 53 56 8B F1 8D 9E ?? ?? ?? ?? 8B CB 89 5D F0 E8 ?? ?? ?? ?? 8B 46 10 33 C9 3B C1 89 4D FC 0F 84",
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 10 53 56 8B F1 8D 9E 80 00 00 00 57 8B CB 89 5D F0 E8 ?? ?? ?? ?? 8B 46 0C 33 FF 3B C7",
	// v188.0 to v302.0
	L"6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC ?? 56 57 A1 ?? ?? ?? ?? 33 C4 50 8D 44 24 ?? 64 A3 00 00 00 00 8B F9 8D 87 ?? ?? ?? ?? 50 8D 4C 24 ?? E8",
	#endif
};

#ifdef _WIN64
std::wstring AOB_SendPacket_EH[] = {
	// TWMS v263.3, 1422BE740
	L"48 89 4C 24 08 48 83 EC ?? E8 ?? ?? ?? ?? 48 89 44 24 ?? 48 8B 44 24 ?? 8B 80 ?? ?? ?? ?? 89 44 24 ?? 8B 54 24 ?? 48 8B 4C 24 ?? E8 ?? ?? ?? ?? 48 8B 44 24 ?? 8B 88 ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 C0 85 C0 75 17 48 8B 44 24 ?? 8B 88 ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 C0 85 C0 74 27 E8 ?? ?? ?? ?? 48 89 44 24 ?? 48 8B 44 24 ?? 8B 80 ?? ?? ?? ?? 89 44 24 ?? 8B 54 24 ?? 48 8B 4C 24 ?? E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 89 44 24 ?? 48 83 7C 24 ?? 00 74 0F 48 8B 54 24 ?? 48 8B 4C 24 ?? E8",
	// v414.1
	L"48 89 4C 24 08 48 83 EC 48 E8 ?? ?? ?? ?? 48 89 44 24 30 48 8B 44 24 50 8B 40 1C 89 44 24 20 8B 54 24 20 48 8B 4C 24 30 E8 ?? ?? ?? ?? E8",
	// v410.2
	L"48 89 4C 24 08 48 83 EC 38 E8 ?? ?? ?? ?? 48 8B 4C 24 40 8B 51 1C 48 8B C8 E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 89 44 24 20 48 83 7C 24 20 00 74 0F 48 8B 54 24 40 48 8B 4C 24 20 E8 ?? ?? ?? ?? 48 83 C4 38 C3",
	// v403.1
	L"48 89 4C 24 08 48 83 EC 28 E8 ?? ?? ?? ?? 48 8B 4C 24 30 8B 51 1C 48 8B C8 E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8B 54 24 30 48 8B C8 E8 ?? ?? ?? ?? 48 83 C4 28 C3",
};

DWORD Offset_SendPacket_EH_Ret[] = {
	// TWMS v263.3
	0xA6,
	// v414.1
	0x4E,
	// v410.2
	0x3F,
	// v403.1
	0x30,
};

DWORD Offset_SendPacket_EH_CClientSocket[] = {
	// TWMS v263.3
	0x85,
	// v414.1
	0x2D,
	// older
	0x1E,
	0x1E,
};

#else
std::wstring AOB_EnterSendPacket[] = {
	// v164.0 to v186.1
	L"FF 74 24 04 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? C3",
	// v164.0 to v186.1
	L"8B 44 24 04 8B 0D ?? ?? ?? ?? 50 E8 ?? ?? ?? ?? C3",
};
#endif

// Recv Hook
std::wstring AOB_ProcessPacket[] = {
	#ifdef _WIN64
	// TWMS v263.3, 1422C4450 by chuichui
	L"48 89 54 24 10 48 89 4C 24 08 48 81 EC ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? 48 33 C4 48 ?? ?? ?? ?? ?? ?? ?? 48 8D 4C 24 34 E8",
	// v425.2, 141F0ABB0 by chuichui
	L"48 89 54 24 10 48 89 4C 24 08 48 81 EC ?? ?? ?? ?? 48 ?? ?? ?? ?? ?? ?? 48 33 C4 48 ?? ?? ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 C0 85 C0 75 05 E9 ?? ?? ?? ?? E9",
	// v414.1
	L"48 89 54 24 10 48 89 4C 24 08 48 81 EC ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 C0 85 C0 75 05 E9 ?? ?? ?? ?? E9",
	// v410.2
	L"48 89 54 24 10 48 89 4C 24 08 48 81 EC ?? ?? ?? ?? 48 C7 84 24 ?? ?? ?? ?? FE FF FF FF 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 C0 85 C0 75",
	// v403.1
	L"48 89 54 24 10 48 89 4C 24 08 56 57 48 81 EC ?? ?? ?? ?? 48 C7 84 24 ?? ?? ?? ?? ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 C0 85 C0 75 05 E9 ?? ?? ?? ?? E9",
	#else
	// v131.0
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 51 A1 ?? ?? ?? ?? 56 57 8B F9 8D 4D EC 89 45 F0 E8 ?? ?? ?? ?? 8B 75 08 83 65 FC 00 8B CE E8 ?? ?? ?? ?? 0F B7 C0 8D 48 F7 83 F9 07 77",
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 51 A1 ?? ?? ?? ?? 56 57 8B F9 8D 4D EC 89 45 F0 E8 ?? ?? ?? ?? 8B 75 08 83 65 FC 00 8B CE E8 ?? ?? ?? ?? 0F B7",
	// v188.0 to v302.0
	L"6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 08 53 56 57 A1 ?? ?? ?? ?? 33 C4 50 8D 44 24 18 64 A3 00 00 00 00 8B F9 8B 1D ?? ?? ?? ?? 89 5C 24 14 85 DB 74",
	#endif
};
// Format Hook
std::wstring AOB_COutPacket[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9E7A0, may be duplicated
	L"48 89 5C 24 10 48 89 4C 24 08 57 48 83 EC ?? 48 8B D9 33 FF 48 89 B9",
	// v425.2, 140983C70 by chuichui
	L"48 89 5C 24 10 48 89 74 24 18 48 89 4C 24 08 57 48 83 EC 20 8B FA 48 8B F1 48 83 C1 08 E8 ?? ?? ?? ?? 90 33 C0 89",
	// v414.1
	L"48 89 5C 24 10 48 89 4C 24 08 57 48 83 EC 20 8B DA 48 8B F9 48 83 C1 08 48 C7 01 00 00 00 00 4C 8D 44 24 40 BA 00 01 00 00 E8",
	// v410.2 from v403.1
	L"48 89 4C 24 ?? 57 48 83 EC ?? 48 C7 44 24 ?? ?? ?? ?? ?? 48 89 5C 24 ?? 8B DA 48 8B F9 48 83 C1 ?? 48 C7 01 00 00 00 00",
	// TWMS v246
	L"48 89 4C 24 ?? 57 48 83 EC ?? 48 C7 44 24 ?? ?? ?? ?? ?? 48 89 5C 24 ?? 8B ?? 48 8B D9 48 C7 41 08 00 00 00 00 BA 04 01 00 00 48 8D 0D ?? ?? ?? ?? E8",
	#else
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 51 56 8B F1 83 66 04 00 8D 45 F3 50 8D 4E 04 68 00 01 00 00 89 75 EC E8 ?? ?? ?? ?? FF 75 08 83 65 FC 00 8B CE E8",
	// v188.0
	L"6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 51 56 A1 ?? ?? ?? ?? 33 C4 50 8D 44 24 0C 64 A3 00 00 00 00 8B F1 89 74 24 08 68 04 01 00 00 B9 ?? ?? ?? ?? C7 46 04 00 00 00 00 E8",
	#endif
};

#ifndef _WIN64
std::wstring AOB_COutPacket_Old[] = {
	// v131.0
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 51 56 8B F1 83 66 04 00 8D 45 F3 50 8D 4E 04 68 00 01 00 00 89 75 EC E8 ?? ?? ?? ?? FF 75 0C 83 65 FC 00 FF 75 08 8B CE E8",
};
#endif

std::wstring AOB_Encode1[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9EB70
	L"48 89 5C 24 08 57 48 83 EC ?? 48 8B D9 0F B6 FA 8B 89 ?? ?? ?? ?? 8D 51 01 3B 93 ?? ?? ?? ?? 76",
	// v410.2
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B D9 0F B6 FA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CB E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75",
	// v403.1
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B F9 0F B6 DA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CF E8 ?? ?? ?? ?? 8B 57 10 0F B6 CB 48 03 57 08 E8 ?? ?? ?? ?? 01 47 10 48 8B 5C 24 30 48 83 C4 20 5F C3",
	#else
	// v164.0 to v186.1
	L"56 8B F1 6A 01 E8 ?? ?? ?? ?? 8B 4E 08 8B 46 04 8A 54 24 08 88 14 08 FF 46 08 5E C2 04 00",
	// v188.0
	L"56 8B F1 8B 46 04 57 8D 7E 04 85 C0 74 03 8B 40 FC 8B 4E 08 41 3B C8 76 1E 8B 07 85 C0 74 03 8B 40 FC 03 C0 3B C8 77 FA 8D 4C 24 0C 51 6A 00 50 8B CF E8",
	#endif
};

std::wstring AOB_Encode2[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9EC70
	L"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ?? 0F B7 F2 48 8B D9 8B 91 ?? ?? ?? ?? 8D 42 02 3B 81 ?? ?? ?? ?? 76",
	// v410.2
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B D9 0F B7 FA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CB E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75",
	// v403.1
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B F9 0F B7 DA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CF E8 ?? ?? ?? ?? 8B 57 10 0F B7 CB 48 03 57 08 E8 ?? ?? ?? ?? 01 47 10 48 8B 5C 24 30 48 83 C4 20 5F C3",
	#else
	// v164.0 to v186.1
	L"56 8B F1 6A 02 E8 ?? ?? ?? ?? 8B 4E 08 8B 46 04 66 8B 54 24 08 66 89 14 08 83 46 08 02 5E C2 04 00",
	// v188.0
	L"56 8B F1 8B 46 04 57 8D 7E 04 85 C0 74 03 8B 40 FC 8B 4E 08 83 C1 02 3B C8 76 1E 8B 07 85 C0 74 03 8B 40 FC 03 C0 3B C8 77 FA 8D 4C 24 0C 51 6A 00 50 8B CF E8 ?? ?? ?? ?? 8B 56 08 8B 07 66 8B 4C 24 0C 66 89 0C 02 83 46 08 02 5F 5E C2 04 00",
	#endif
};

std::wstring AOB_Encode4[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9ED00
	L"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ?? 8B F2 48 8B D9 8B 91 ?? ?? ?? ?? 8D 42 04 3B 81 ?? ?? ?? ?? 76",
	// v410.2
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B D9 8B FA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CB E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75",
	// v403.1
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B F9 8B DA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CF E8 ?? ?? ?? ?? 8B 57 10 8B CB 48 03 57 08 E8 ?? ?? ?? ?? 01 47 10 48 8B 5C 24 30 48 83 C4 20 5F C3",
	#else
	// v164.0 to v186.1
	L"56 8B F1 6A 04 E8 ?? ?? ?? ?? 8B 4E 08 8B 46 04 8B 54 24 08 89 14 08 83 46 08 04 5E C2 04 00",
	// v188.0
	L"56 8B F1 8B 46 04 57 8D 7E 04 85 C0 74 03 8B 40 FC 8B 4E 08 83 C1 04 3B C8 76 1E 8B 07 85 C0 74 03 8B 40 FC 03 C0 3B C8 77 FA 8D 4C 24 0C 51 6A 00 50 8B CF E8 ?? ?? ?? ?? 8B 56 08 8B 07 8B 4C 24 0C 89 0C 02 83 46 08 04 5F 5E C2 04 00",
	#endif
};

#ifdef _WIN64
std::wstring AOB_Encode8[] = {
	// TWMS v263.3, 140A9EEF0
	L"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ?? 48 8B F2 48 8B D9 8B 91 ?? ?? ?? ?? 8D 42 08 3B 81 ?? ?? ?? ?? 76",
	// v410.2
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B D9 48 8B FA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CB E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75",
	// v403.1
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B F9 48 8B DA 48 8D 4C 24 38 E8 ?? ?? ?? ?? 8B D0 48 8B CF E8 ?? ?? ?? ?? 8B 57 10 48 8B CB 48 03 57 08 E8 ?? ?? ?? ?? 01 47 10 48 8B 5C 24 30 48 83 C4 20 5F C3",
};
#endif

std::wstring AOB_EncodeStr[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9EFB0
	L"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC ?? 48 8B 02 45 33 F6 48 8B FA 48 8B D9 48 85 C0 74 05 8B 48 FC EB 03 41 8B CE 8B B3 ?? ?? ?? ?? 8D 56 02 03 D1 3B 93 ?? ?? ?? ?? 76",
	// v410.2
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B D9 48 8B FA 48 8B CA E8 ?? ?? ?? ?? 8B D0 48 8B CB E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75",
	// v403.1
	L"48 89 5C 24 08 57 48 83 EC 20 48 8B F9 48 8B DA 48 8B CA E8 ?? ?? ?? ?? 8B D0 48 8B CF E8 ?? ?? ?? ?? 8B 57 10 48 8B CB 48 03 57 08 E8 ?? ?? ?? ?? 01 47 10 48 8B 5C 24 30 48 83 C4 20 5F C3",
	#else
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 56 8B F1 8B 45 08 83 65 FC 00 85 C0 74 05 8B 40 FC EB 02 33 C0 83 C0 02 50 8B CE E8",
	// v188.0
	L"6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 51 56 57 A1 ?? ?? ?? ?? 33 C4 50 8D 44 24 10 64 A3 00 00 00 00 8B F1 8B 44 24 20 C7 44 24 18 00 00 00 00 85 C0 74",
	#endif
};

std::wstring AOB_EncodeBuffer[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9F150
	L"48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC ?? 48 8B EA 41 8B F8 8B 91 ?? ?? ?? ?? 48 8B D9 42 8D 04 02 3B 81 ?? ?? ?? ?? 76",
	// v410.2
	L"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B DA 41 8B F0 41 8B D0 48 8B F9 E8 ?? ?? ?? ?? 48 8B 47 08 48 85 C0 75",
	// v403.1
	L"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B DA 41 8B F0 41 8B D0 48 8B F9 E8 ?? ?? ?? ?? 8B 47 10 48 03 47 08 85 F6 7E 18 8B D6 0F 1F 00 0F B6 0B 48 8D 5B 01 88 08 48 8D 40 01 48 83 EA 01 75 ED 01 77 10 48 8B 74 24 38 48 8B 5C 24 30 48 83 C4 20 5F C3",
	#else
	// v164.0 to v186.1
	L"56 57 8B 7C 24 10 8B F1 57 E8 ?? ?? ?? ?? 8B 46 04 03 46 08 57 FF 74 24 10 50 E8 ?? ?? ?? ?? 01 7E 08 83 C4 0C 5F 5E C2 08 00",
	// v188.0
	L"53 56 8B F1 8B 46 04 57 8D 7E 04 85 C0 74 03 8B 40 FC 8B 4E 08 8B 5C 24 14 03 CB 3B C8 76 1E 8B 07 85 C0 74 03 8B 40 FC 03 C0 3B C8 77 FA 8D 54 24 14 52 6A 00 50 8B CF E8 ?? ?? ?? ?? 8B 4E 08 8B 44 24 10 03 0F 53 50 51 E8 ?? ?? ?? ?? 01 5E 08 83 C4 0C 5F 5E 5B C2 08 00",
	#endif
};

std::wstring AOB_Decode1[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9A630
	L"48 89 5C 24 10 48 89 4C 24 08 57 48 83 EC 50 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 B5 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 A1 00 00 00 E8 ?? ?? ?? ?? 8B 4B 1C 8B C1 48 03 43 08 83 FF 01",
	// v414.1
	L"48 89 5C 24 18 48 89 4C 24 08 57 48 83 EC 40 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 48 03 53 08 44 8B C7 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 0F B6 44 24 58 48 8B 5C 24 60 48 83 C4 40 5F C3",
	// v410.2
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 48 8B 41 08 48 85 C0 75 13 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 74 0B 83 78 FC 00 77 1A 48 85 C0 75 05 45 33 C0 EB 04 44 8B 40 FC 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 44 8B 43 10 44 2B C2 48 03 53 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 0F B6 44 24 58 48 83 C4 40 5B C3",
	// v403.1
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 8B 51 1C 44 8B 41 10 44 2B C2 48 03 51 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 0F B6 44 24 58 48 83 C4 40 5B C3",
	#else
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 14 0F B7 51 0C 8B 41 08 83 65 FC 00 53 56 8B 71 14 2B D6 57 03 C6 83 FA 01",
	// v188.0
	L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 14 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 65 F0 89 4D E8 0F B7 41 0C 8B 51 14 8B 71 08 2B C2 C7 45 FC 00 00 00 00 83 F8 01",
	#endif
};

std::wstring AOB_Decode2[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9A6D0
	L"48 89 5C 24 10 48 89 4C 24 08 57 48 83 EC 50 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 B5 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 A1 00 00 00 E8 ?? ?? ?? ?? 8B 4B 1C 8B C1 48 03 43 08 83 FF 02",
	// v414.1
	L"48 89 5C 24 18 48 89 4C 24 08 57 48 83 EC 40 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 48 03 53 08 44 8B C7 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 0F B7 44 24 58 48 8B 5C 24 60 48 83 C4 40 5F C3",
	// v410.2
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 48 8B 41 08 48 85 C0 75 13 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 74 0B 83 78 FC 00 77 1A 48 85 C0 75 05 45 33 C0 EB 04 44 8B 40 FC 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 44 8B 43 10 44 2B C2 48 03 53 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 0F B7 44 24 58 48 83 C4 40 5B C3",
	// v403.1
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 8B 51 1C 44 8B 41 10 44 2B C2 48 03 51 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 0F B7 44 24 58 48 83 C4 40 5B C3",
	#else
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 14 0F B7 51 0C 8B 41 08 83 65 FC 00 53 56 8B 71 14 2B D6 57 03 C6 83 FA 02",
	// v188.0
	L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 14 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 65 F0 89 4D E8 0F B7 41 0C 8B 51 14 8B 71 08 2B C2 C7 45 FC 00 00 00 00 83 F8 02",
	#endif
};

std::wstring AOB_Decode4[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9AA70
	L"48 89 5C 24 10 48 89 4C 24 08 57 48 83 EC 50 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 B5 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 A1 00 00 00 E8 ?? ?? ?? ?? 8B 4B 1C 8B C1 48 03 43 08 83 FF 04",
	// v414.1
	L"48 89 5C 24 18 48 89 4C 24 08 57 48 83 EC 40 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 48 03 53 08 44 8B C7 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 8B 44 24 58 48 8B 5C 24 60 48 83 C4 40 5F C3",
	// v410.2
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 48 8B 41 08 48 85 C0 75 13 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 74 0B 83 78 FC 00 77 1A 48 85 C0 75 05 45 33 C0 EB 04 44 8B 40 FC 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 44 8B 43 10 44 2B C2 48 03 53 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 8B 44 24 58 48 83 C4 40 5B C3",
	// v403.1
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 8B 51 1C 44 8B 41 10 44 2B C2 48 03 51 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 8B 44 24 58 48 83 C4 40 5B C3",
	#else
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 14 0F B7 51 0C 8B 41 08 83 65 FC 00 53 56 8B 71 14 2B D6 57 03 C6 83 FA 04",
	// v188.0
	L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 14 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 65 F0 89 4D E8 0F B7 41 0C 8B 51 14 8B 71 08 2B C2 C7 45 FC 00 00 00 00 83 F8 04",
	#endif
};

#ifdef _WIN64
std::wstring AOB_Decode8[] = {
	// TWMS v263.3, 140A9AA70
	L"48 89 5C 24 10 48 89 4C 24 08 57 48 83 EC 50 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 B5 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 A1 00 00 00 E8 ?? ?? ?? ?? 8B 4B 1C 8B C1 48 03 43 08 83 FF 08 72 ?? 48",
	// v414.1
	L"48 89 5C 24 18 48 89 4C 24 08 57 48 83 EC 40 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 75 05 45 33 C0 EB 09 44 8B 40 FC 45 85 C0 75 0C 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 48 03 53 08 44 8B C7 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 48 8B 44 24 58 48 8B 5C 24 60 48 83 C4 40 5F C3",
	// v410.2
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 48 8B 41 08 48 85 C0 75 13 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 74 0B 83 78 FC 00 77 1A 48 85 C0 75 05 45 33 C0 EB 04 44 8B 40 FC 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 44 8B 43 10 44 2B C2 48 03 53 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 48 8B 44 24 58 48 83 C4 40 5B C3",
	// v403.1
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 8B D9 8B 51 1C 44 8B 41 10 44 2B C2 48 03 51 08 48 8D 4C 24 58 E8 ?? ?? ?? ?? 01 43 1C 48 8B 44 24 58 48 83 C4 40 5B C3",
};
#endif

std::wstring AOB_DecodeStr[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9ABB0
	L"48 89 5C 24 18 48 89 74 24 20 48 89 54 24 10 48 89 4C 24 08 57 41 54 41 55 41 56 41 57 48 83 EC 60 48 8B F2 4C 8B F1 45 33 ED 44 89 6C 24 20 4C 89 2A C7 44 24 20 01 00 00 00 8B 59 10 2B 59 1C 48 8B 41 08 48 85 C0 75 18 B9 B5 00 00 00 E8 ?? ?? ?? ?? 49 8B 46 08 48 85 C0 75",
	// v414.1
	L"48 89 5C 24 18 48 89 54 24 10 48 89 4C 24 08 56 57 41 56 48 83 EC 40 48 8B F2 48 8B D9 33 FF 89 7C 24 20 48 89 3A C7 44 24 20 01 00 00 00 44 8B 71 10 44 2B 71 1C 48 8B 41 08 48 85 C0 75 13 B9 A7 00 00 00 E8",
	// v410.2
	L"48 8B C4 48 89 50 10 48 89 48 08 57 48 83 EC 50 48 C7 40 D0 FE FF FF FF 48 89 58 18 48 89 70 20 48 8B F2 48 8B D9 33 FF 89 78 C8 48 89 3A C7 40 C8 01 00 00 00 48 8B 41 08 48 85 C0 75 13 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 74 0E 83 78 FC 00 77 17 48 85 C0 74 03 8B 78 FC 44 8B C7 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 8B 53 1C 44 8B 43 10 44 2B C2 48 03 53 08 48 8B CE E8 ?? ?? ?? ?? 01 43 1C 48 8B C6 48 8B 5C 24 70 48 8B 74 24 78 48 83 C4 50 5F C3",
	// v403.1
	L"48 89 54 24 10 48 89 4C 24 08 57 48 83 EC 50 48 C7 44 24 28 FE FF FF FF 48 89 5C 24 70 48 8B FA 48 8B D9 33 C0 89 44 24 20 48 89 02 C7 44 24 20 01 00 00 00 8B 51 1C 44 8B 41 10 44 2B C2 48 03 51 08 48 8B CF E8 ?? ?? ?? ?? 01 43 1C 48 8B C7 48 8B 5C 24 70 48 83 C4 50 5F C3",
	#else
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 18 53 56 57 89 65 F0 6A 01 33 FF 8B F1 5B",
	// v188.0
	L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 14 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 65 F0 8B F1 89 75 E8 C7 45 EC 00 00 00 00 8B 7D 08 B8 01 00 00 00 89 45 FC C7 07 00 00 00 00 0F B7 56 0C 8B 4E 08 89 45 EC 8B 46 14 2B D0 52 03 C1 50 57 E8",
	#endif
};

std::wstring AOB_DecodeBuffer[] = {
	#ifdef _WIN64
	// TWMS v263.3, 140A9AE10
	L"48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 48 89 4C 24 08 41 56 48 83 EC 50 41 8B F0 48 8B DA 48 8B F9 44 8B 71 10 44 2B 71 1C 48 8B 41 08 48 85 C0 75 18 B9 B5 00 00 00 E8",
	// v414.1
	L"48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 48 89 4C 24 08 41 56 48 83 EC 40 41 8B F0 4C 8B F2 48 8B D9 8B 79 10 2B 79 1C 48 8B 41 08 48 85 C0 75 18 B9 A7 00 00 00 E8",
	// v410.2
	L"48 89 4C 24 08 57 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 58 48 89 74 24 60 41 8B F8 48 8B F2 48 8B D9 48 8B 41 08 48 85 C0 75 13 B9 A7 00 00 00 E8 ?? ?? ?? ?? 48 8B 43 08 48 85 C0 74 0B 83 78 FC 00 77 1A 48 85 C0 75 05 45 33 C0 EB 04 44 8B 40 FC 33 D2 B9 93 00 00 00 E8 ?? ?? ?? ?? 44 8B 43 1C 44 8B 4B 10 45 2B C8 4C 03 43 08 8B D7 48 8B CE E8 ?? ?? ?? ?? 01 43 1C 48 8B 5C 24 58 48 8B 74 24 60 48 83 C4 40 5F C3",
	// v403.1
	L"48 89 4C 24 08 53 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 41 8B C0 4C 8B D2 48 8B D9 44 8B 41 1C 44 8B 49 10 45 2B C8 4C 03 41 08 8B D0 49 8B CA E8 ?? ?? ?? ?? 01 43 1C 48 83 C4 40 5B C3",
	#else
	// v164.0 to v186.1
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 EC 14 83 65 FC 00 53 56 8B F1 0F B7 46 0C",
	// v188.0
	L"55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 00 00 00 00 50 83 EC 14 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 F4 64 A3 00 00 00 00 89 65 F0 8B F1 89 75 E8 0F B7 46 0C 8B 4E 14 8B 56 08 8B 7D 0C 2B C1 03 CA C7 45 FC 00 00 00 00 3B C7 73",
	#endif
};

#endif