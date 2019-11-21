#pragma once

struct PartyPokemon
{
private:
	bool swappedEndians;
	unsigned short previousHitPointsMax;

	void calculateStats();
	void scaleCurrentHitPoints();
	void scaleExperience();

public:
	sPokemon pokemon;
	sPokemon previousPokemon;

	union {
		struct {
			unsigned char index;
			unsigned char heldItem;

			unsigned char moves[MAX_MOVES];

			unsigned short OT_ID;

			unsigned char experienceFix[3];

			unsigned short hitPointsExp;
			unsigned short attackExp;
			unsigned short defenseExp;
			unsigned short speedExp;
			unsigned short specialExp;

			sDV DVs;

			unsigned char movesPP[MAX_MOVES];
			
			unsigned char happiness;
			unsigned char pokerusStatus;
			unsigned short caughtData;
			unsigned char level;
			unsigned char status;
			unsigned char unused;

			unsigned short hitPoints;
			unsigned short hitPointsMax;
			unsigned short attack;
			unsigned short defense;
			unsigned short speed;
			unsigned short attackSpecial;
			unsigned short defenseSpecial;
		};

		unsigned char ramData[MAX_POKEMON_DATA_SIZE];
	};
	
	PartyPokemon();
	~PartyPokemon();

	void swapEndians();
	void recalculate();

	unsigned long getExperience();
	void setExperience(unsigned long experience);
};
