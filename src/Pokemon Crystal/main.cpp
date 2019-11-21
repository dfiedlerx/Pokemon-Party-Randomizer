#include "stdafx.h"

bool lockedAddr = false;

HANDLE process = nullptr;
std::vector<DWORD> gAddresses;
std::map<unsigned char, sMove> gMoves;
std::map<unsigned char, sPokemon> gPokemons;
DWORD gCorrectAddr = 0;

void loadMoves()
{
	std::cout << "Loading: pp data" << std::endl;

	std::ifstream file = std::ifstream("moves.csv");
	
	if(file.is_open() == false) {
		std::cout << "moves.csv not found" << std::endl;
	}

	file.seekg(0, std::ios_base::beg);
	std::string temp = "";
	while(file.eof() == false) {
		sMove move = sMove();
		
		if (getline(file, temp, ';')) {
			move.index = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			move.name = temp;
		}

		if (getline(file, temp, ';')) {
			move.maxPP = atoi(temp.c_str());
		}

		if (getline(file, temp)) {
			move.note = temp;
		}

		gMoves.insert(std::make_pair(move.index, move));
	}

	file.close();
}

void loadPokemons()
{
	std::cout << "Loading: Pokemon data" << std::endl;

	std::ifstream file = std::ifstream("pokemon.csv");

	if(file.is_open() == false) {
		std::cout << "pokemon.csv not found" << std::endl;
	}

	file.seekg(0, std::ios_base::beg);
	std::string temp = "";
	while(file.eof() == false) {
		sPokemon pokemon = sPokemon();
		
		if (getline(file, temp, ';')) {
			pokemon.index = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.name = temp;
		}

		if (getline(file, temp, ';')) {
			pokemon.level = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.experience = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.hitPoints = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.attack = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.defense = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.specialAttack = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.specialDefense = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.speed = atoi(temp.c_str());
		}

		if (getline(file, temp, ';')) {
			pokemon.moves = temp;
		}

		if (getline(file, temp, ';')) {
			pokemon.levels = temp;
		}

		if (getline(file, temp)) {
			pokemon.note = temp;
		}
		
		int movesCount = std::count(pokemon.moves.begin(), pokemon.moves.end(), ',');
		int levelsCount = std::count(pokemon.levels.begin(), pokemon.levels.end(), ',');
		
		std::string move = "";
		std::string level = "";
		
		std::stringstream stream1(pokemon.moves);
		std::stringstream stream2(pokemon.levels);
		while(getline(stream1, move, ',') && getline(stream2, level, ',')) {
			unsigned char moveId = atoi(move.c_str());
			unsigned char minLevel = atoi(level.c_str());

			sMinMove minMove = {moveId, minLevel};

			pokemon.minMoves.push_back(minMove);
		}

		gPokemons.insert(std::make_pair(pokemon.index, pokemon));
	}

	
	file.close();
}


int minExpForLevel(char level, sPokemon pokemon)
{
	double experience = 0;

	switch(pokemon.experience)
	{
	case 800000:
		experience = floor(level * level * level * 0.8);
	case 1000000:
		experience = floor(level * level * level);
	case 1059860:
		experience = floor(1.2 * level * level * level - 15 * level * level + 100 * level - 140);
	case 1250000:
		experience = floor(level * level * level * 1.25);
	}

	return static_cast<int>(experience);
}

bool randomizeParty(DWORD addr)
{
	addr += 0x0AAA;
	
	char* buff = new char[1];
	SIZE_T loaded;
	ReadProcessMemory(process, (void*)addr, buff, 1, &loaded);

	char pokemonAmount = buff[0];

	if (pokemonAmount < 1 || pokemonAmount > 6) {
		return false;
	}
	
	unsigned char previousPokemonIndex;
	unsigned long experience = 0;


	unsigned short prevMaxHP = 0;
	unsigned short newMaxHP = 0;

	unsigned char pokId = 1 + rand() % 251; 

	for (int i = 0; i < pokemonAmount; i++) {
		pokId = 1 + rand() % 251; 


		DWORD spiecesAddr = addr + OFFSET_SPIECES + (i);
		DWORD partyAddr = addr + OFFSET_PARTY + (MAX_POKEMON_DATA_SIZE * i);
		DWORD seenAddr = addr + OFFSET_POKEDEX_SEEN;
		DWORD caughtAddr = addr + OFFSET_POKEDEX_CAUGHT;

		PartyPokemon partyPokemon = PartyPokemon();
 		ReadProcessMemory(process, (void*)(partyAddr), &partyPokemon.ramData, MAX_POKEMON_DATA_SIZE, &loaded);
		partyPokemon.swapEndians();
	
 		ReadProcessMemory(process, (void*)(spiecesAddr), &previousPokemonIndex, 1, &loaded);

		// Skip if egg
		if (previousPokemonIndex == 0xFD) {
			continue;
		}
		
		partyPokemon.pokemon = gPokemons.at(pokId);
		partyPokemon.previousPokemon = gPokemons.at(previousPokemonIndex);

		partyPokemon.index = partyPokemon.pokemon.index;
		partyPokemon.recalculate();


		// Moves
		{
			unsigned char* moves = partyPokemon.pokemon.getMovesForLevel(partyPokemon.level);
			
			sMove move1 = gMoves.at(moves[0]);
			sMove move2 = gMoves.at(moves[1]);
			sMove move3 = gMoves.at(moves[2]);
			sMove move4 = gMoves.at(moves[3]);

			for (int move = 0; move < MAX_MOVES; move++)
			{
				sMove tempMove = gMoves.at(moves[move]);

				partyPokemon.moves[move] = tempMove.index;
				partyPokemon.movesPP[move] = tempMove.maxPP;
			}
		}

		{
			partyPokemon.swapEndians();
			WriteProcessMemory(process, (void*)(spiecesAddr), &partyPokemon.pokemon.index, 1, &loaded);
			WriteProcessMemory(process, (void*)(partyAddr), &partyPokemon.ramData, MAX_POKEMON_DATA_SIZE, &loaded);
		}

		{
			// Pokedex
			int byteOffset = floor((partyPokemon.index - 1) / 8);
			int bitOffset = (partyPokemon.index - 1) % 8;

			unsigned char seen = 0;
			unsigned char caught = 0;
			
			ReadProcessMemory(process, (void*)(seenAddr + byteOffset), &seen, 1, &loaded);
			ReadProcessMemory(process, (void*)(caughtAddr + byteOffset), &caught, 1, &loaded);
			
			seen |= 1 << bitOffset;
			caught |= 1 << bitOffset;

			WriteProcessMemory(process, (void*)(seenAddr + byteOffset), &seen, 1, &loaded);
			WriteProcessMemory(process, (void*)(caughtAddr + byteOffset), &caught, 1, &loaded);
		}
	}

	return true;
}

int main()
{
	int processId = FindProcessId(L"VisualBoyAdvance.exe");

	if(processId == 0)
	{
		std::cout << "VisualBoyAdvance not running" << std::endl;
		std::cout << "Read the README.txt" << std::endl;
		std::cin.get();
		return 0;
	}

	int tempTime = static_cast<unsigned int>(time(0));
	srand(tempTime);

    char someData[] = "ANT  ";
	someData[3] = 0x50;
	someData[4] = 0x50;
	someData[5] = 0x50;
	size_t size = sizeof(someData);
	size -= 3;
	for(size_t i = 0; i < size; i++) {
		someData[i] += 0x3F;
	}
	size += 3;

	DWORD addr = 0;
	
    std::cout << "Hacking into VisualBoyAdvance." << std::endl;

	unsigned char* b = new unsigned char[1];

    char* ret = GetAddressOfData(processId, someData, size);
    if(ret)
    {
		std::cout << "VisualBoyAdvance has been hacked." << std::endl;

		loadPokemons();
		loadMoves();

		std::cout << "Everything has been loaded." << std::endl;
		SIZE_T loaded;

		memcpy(&addr, &ret, 4);

		std::vector<DWORD>::iterator it = gAddresses.begin();
		
		unsigned char lastBattleMode = 0;
		while(true) {
			if (lockedAddr == false) {
				it++;
				if(it == gAddresses.end()) {
					it = gAddresses.begin();
				}

				addr = (*it);
			}
 			ReadProcessMemory(process, (void*)addr, b, 1, &loaded);

			if(b[0] > 2 || b[0] == lastBattleMode) {
				continue;
			}

			lockedAddr = true;
			lastBattleMode = b[0];

			switch(b[0])
			{
			case 0: 
 				randomizeParty(addr);
				break;
			}
			::Sleep(500);
		}
		
		
        std::cout << b << "\n";
		
		return 0;
    }
    else
    {
        std::cout << "Not found\n";
    }

    return 0;
}
