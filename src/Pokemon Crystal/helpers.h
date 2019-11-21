#pragma once

DWORD FindProcessId(const std::wstring& processName);
char* GetAddressOfData(DWORD pid, const char *data, size_t len);

extern HANDLE process;
extern std::vector<DWORD> gAddresses;
extern std::map<unsigned char, sMove> gMoves;
extern std::map<unsigned char, sPokemon> gPokemons;
extern DWORD gCorrectAddr;
