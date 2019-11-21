#include <string>

#define OFFSET_SPIECES 0x01
#define OFFSET_PARTY 0x08
#define OFFSET_POKEDEX_SEEN 0x01C2
#define OFFSET_POKEDEX_CAUGHT 0x01E2

#define MAX_POKEMON_DATA_SIZE 0x30

#define SPIECES 0

#define EXPERIENCE 8

#define MOVE_1 2
#define MOVE_2 3
#define MOVE_3 4
#define MOVE_4 5

#define MOVE_1_PP 23
#define MOVE_2_PP 24
#define MOVE_3_PP 25
#define MOVE_4_PP 26

#define HP_EXP 11
#define ATK_EXP 13
#define DEF_EXP 15
#define SPD_EXP 17
#define SPC_EXP 19
#define DVS 21


#define LEVEL 31

#define CUR_HP 34
#define MAX_HP 36
#define CUR_ATK 38
#define CUR_DEF 40
#define CUR_SPD 42
#define CUR_SATK 44
#define CUR_SDEF 46


struct sDV

{
	union
	{
		struct
		{
			unsigned char defense : 4;
			unsigned char attack : 4;
			unsigned char special : 4;
			unsigned char speed : 4;
		};

		unsigned short DVs;
	};

	char getHP()
	{
		return (char)(attack&1 == 1 ? 8 : 0) + (defense&1 == 1 ? 4 : 0) + (speed&1 == 1 ? 2 : 0) + (special&1 == 1 ? 1 : 0);
	}
};

struct sMinMove
{
	unsigned char moveId;
	unsigned char minLevel;
};

struct sMove
{
public:
	unsigned char index;
	
	std::string name;

	unsigned char maxPP;
	std::string note;
};

struct sPokemon
{
public:
	unsigned char index;

	std::string name;

	unsigned char level;
	unsigned long experience;

	unsigned short hitPoints;
	unsigned short attack;
	unsigned short defense;
	unsigned short specialAttack;
	unsigned short specialDefense;
	unsigned short speed;

	std::vector<sMinMove> minMoves;
	
	std::string moves;
	std::string levels;
	std::string note;

	unsigned char* getMovesForLevel(unsigned char level)
	{
		unsigned char* moves = new unsigned char[4];
		moves[0] = 0;
		moves[1] = 0;
		moves[2] = 0;
		moves[3] = 0;
		
		std::vector<sMinMove>::iterator it;
		for(it = minMoves.begin(); it != minMoves.end(); it++) {
			if((*it).minLevel > level) {
				break;
			}
		}
		int maxMoves = 0;
		for(maxMoves = 0; maxMoves < 4 && it != minMoves.begin(); maxMoves++) {
			it--;
		}

		for(int i = 0; i < maxMoves; i++) {
			if(moves[0] != (*it).moveId && moves[1] != (*it).moveId && moves[2] != (*it).moveId && moves[3] != (*it).moveId) {
				moves[i] = (*it).moveId;
			} else {
				i--;
				maxMoves--;
			}
			it++;
		}

		return moves;
	}
};
