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
			unsigned short moveId = atoi(move.c_str());
			unsigned short minLevel = atoi(level.c_str());

			sMinMove minMove = {moveId, minLevel};

			pokemon.minMoves.push_back(minMove);
		}

		gPokemons.insert(std::make_pair(pokemon.index, pokemon));
	}

	
	file.close();
}


int minExpForLevel(char level, sPokemon pokemon)
{
	switch(pokemon.experience)
	{
	case 800000:
		return floor(level * level * level * 0.8);
	case 1000000:
		return floor(level * level * level);
	case 1059860:
		return floor(1.2 * level * level * level - 15 * level * level + 100 * level - 140);
	case 1250000:
		return floor(level * level * level * 1.25);
	}

	return 0;
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
	unsigned char level = 0;
	unsigned long  experience = 0;
	unsigned short hpExp = 0;
	unsigned short atkExp = 0;
	unsigned short defExp = 0;
	unsigned short spdExp = 0;
	unsigned short spcExp = 0;

	unsigned short prevMaxHP = 0;
	unsigned short curHP = 0;
	unsigned short newMaxHP = 0;
	unsigned short newAtk = 0;
	unsigned short newDef = 0;
	unsigned short newSAtk = 0;
	unsigned short newSDef = 0;
	unsigned short newSpd = 0;

	sDV DV = sDV();
	unsigned char pokId = 1 + rand() % 251; 

	sPokemon pokemon;
	sPokemon prevPokemon;

	for (int i = 0; i < pokemonAmount; i++) {
		pokId = 1 + rand() % 251; 


		DWORD spiecesAddr = addr + OFFSET_SPIECES + (i);
		DWORD partyAddr = addr + OFFSET_PARTY + (i * MAX_POKEMON_DATA_SIZE);
		DWORD seenAddr = addr + OFFSET_POKEDEX_SEEN;
		DWORD caughtAddr = addr + OFFSET_POKEDEX_CAUGHT;
	
 		ReadProcessMemory(process, (void*)(spiecesAddr), &previousPokemonIndex, 1, &loaded);

		// Skip if egg
		if (previousPokemonIndex == 0xFD) {
			continue;
		}
		
		ReadProcessMemory(process, (void*)(partyAddr + LEVEL), &level, 1, &loaded);

		pokemon = gPokemons.at(pokId);
		prevPokemon = gPokemons.at(previousPokemonIndex);

		// Stats
		{
			ReadProcessMemory(process, (void*)(partyAddr + DVS), &DV.DVs, sizeof(DV.DVs), &loaded);
			
			ReadProcessMemory(process, (void*)(partyAddr + HP_EXP), &hpExp, sizeof(short), &loaded);
			ReadProcessMemory(process, (void*)(partyAddr + ATK_EXP), &atkExp, sizeof(short), &loaded);
			ReadProcessMemory(process, (void*)(partyAddr + DEF_EXP), &defExp, sizeof(short), &loaded);
			ReadProcessMemory(process, (void*)(partyAddr + SPD_EXP), &spdExp, sizeof(short), &loaded);
			ReadProcessMemory(process, (void*)(partyAddr + SPC_EXP), &spcExp, sizeof(short), &loaded);
			ReadProcessMemory(process, (void*)(partyAddr + CUR_HP), &curHP, sizeof(short), &loaded);

			
			curHP = htons(curHP);
			hpExp = htons(hpExp);
			atkExp = htons(atkExp);
			defExp = htons(defExp);
			spdExp = htons(spdExp);
			spcExp = htons(spcExp);

			if (hpExp < 1) {
				hpExp = 1;
			}

			if (atkExp < 1) {
				atkExp = 1;
			}

			if (defExp < 1) {
				defExp = 1;
			}

			if (spdExp < 1) {
				spdExp = 1;
			}

			if (spcExp < 1) {
				spcExp = 1;
			}
			
			prevMaxHP = floor(((prevPokemon.hitPoints+DV.getHP())*2+floor((sqrt(hpExp - 1) + 1) / 4))*level/100) + 10 + level;
			newMaxHP = floor(((pokemon.hitPoints+DV.getHP())*2+floor((sqrt(hpExp - 1) + 1) / 4))*level/100) + 10 + level;
			newAtk = htons(floor(((pokemon.attack+DV.attack)*2+floor((sqrt(atkExp - 1) + 1) / 4))*level/100) + 5);
			newDef = htons(floor(((pokemon.defense+DV.defense)*2+floor((sqrt(defExp - 1) + 1) / 4))*level/100) + 5);
			newSAtk = htons(floor(((pokemon.specialAttack+DV.special)*2+floor((sqrt(spcExp - 1) + 1) / 4))*level/100) + 5);
			newSDef = htons(floor(((pokemon.specialDefense+DV.special)*2+floor((sqrt(spcExp - 1) + 1) / 4))*level/100) + 5);
			newSpd = htons(floor(((pokemon.speed+DV.speed)*2+floor((sqrt(spdExp - 1) + 1) / 4))*level/100)  + 5);
			
			if(prevMaxHP != newMaxHP && curHP > 1)
			{
				int hpPct = curHP * 100 / prevMaxHP;
				curHP = ceil(newMaxHP * hpPct / 100);

				if(curHP > newMaxHP) {
					curHP = newMaxHP;
				}

				if(curHP < 1) {
					curHP = 1;
				}
			}

			newMaxHP = htons(newMaxHP);
			curHP = htons(curHP);
			
			WriteProcessMemory(process, (void*)(partyAddr + CUR_HP), &curHP, sizeof(short), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + MAX_HP), &newMaxHP, sizeof(short), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + CUR_ATK), &newAtk, sizeof(short), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + CUR_DEF), &newDef, sizeof(short), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + CUR_SATK), &newSAtk, sizeof(short), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + CUR_SDEF), &newSDef, sizeof(short), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + CUR_SPD), &newSpd, sizeof(short), &loaded);
		}

		// Moves
		{
			unsigned char* moves = pokemon.getMovesForLevel(level);
			
			sMove move1 = gMoves.at(moves[0]);
			sMove move2 = gMoves.at(moves[1]);
			sMove move3 = gMoves.at(moves[2]);
			sMove move4 = gMoves.at(moves[3]);
			
			WriteProcessMemory(process, (void*)(partyAddr + MOVE_1), &move1.index, sizeof(char), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + MOVE_2), &move2.index, sizeof(char), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + MOVE_3), &move3.index, sizeof(char), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + MOVE_4), &move4.index, sizeof(char), &loaded);

			WriteProcessMemory(process, (void*)(partyAddr + MOVE_1_PP), &move1.maxPP, sizeof(char), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + MOVE_2_PP), &move2.maxPP, sizeof(char), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + MOVE_3_PP), &move3.maxPP, sizeof(char), &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + MOVE_4_PP), &move4.maxPP, sizeof(char), &loaded);
		}

		// Fix Exp
		{
			experience = 0;
			ReadProcessMemory(process, (void*)(partyAddr + EXPERIENCE), &experience, 3, &loaded);
			experience = htonl(experience << 8);

			if (level < 100) {
				if (prevPokemon.experience != pokemon.experience) {
					int prevBaseExp = minExpForLevel(level, prevPokemon);
					int prevNextExp = minExpForLevel(level + 1, prevPokemon);
					prevNextExp -= prevBaseExp;
					experience -= prevBaseExp;
					int experiencePct = experience * 100 / prevNextExp;

					
					int baseExp = minExpForLevel(level, pokemon);
					int nextExp = minExpForLevel(level + 1, pokemon);
					int reqExp = nextExp - baseExp;

					experience = baseExp + floor(experiencePct * reqExp / 100);
				}
			} else {
				experience = pokemon.experience;
			}
			
			if (level < 100 && experience > minExpForLevel(level + 1, pokemon)) {
				experience = minExpForLevel(level + 1, pokemon) - 1;
			}

			if (experience > pokemon.experience) {
				experience = pokemon.experience;
			}

			
			experience = htonl(experience) >> 8;
			
 			WriteProcessMemory(process, (void*)(partyAddr + EXPERIENCE), &experience, 3, &loaded);
		}

		{
			// Pokedex
			int byteOffset = floor((pokemon.index - 1) / 8);
			int bitOffset = (pokemon.index - 1) % 8;

			unsigned char seen = 0;
			unsigned char caught = 0;
			
			ReadProcessMemory(process, (void*)(seenAddr + byteOffset), &seen, 1, &loaded);
			ReadProcessMemory(process, (void*)(caughtAddr + byteOffset), &caught, 1, &loaded);
			
			seen |= 1 << bitOffset;
			caught |= 1 << bitOffset;

			WriteProcessMemory(process, (void*)(seenAddr + byteOffset), &seen, 1, &loaded);
			WriteProcessMemory(process, (void*)(caughtAddr + byteOffset), &caught, 1, &loaded);
		}

		{
			WriteProcessMemory(process, (void*)(spiecesAddr), &pokemon.index, 1, &loaded);
			WriteProcessMemory(process, (void*)(partyAddr + SPIECES), &pokemon.index, 1, &loaded);
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

	srand(time(0));
    char someData[] = "ANT  ";
	someData[3] = 0x50;
	someData[4] = 0x50;
	someData[5] = 0x50;
	size_t size = sizeof(someData);
	size -= 3;
	for(int i = 0; i < size; i++) {
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
