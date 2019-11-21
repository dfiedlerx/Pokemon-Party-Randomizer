#include "StdAfx.h"

PartyPokemon::PartyPokemon()
{
	swappedEndians = false;
}

PartyPokemon::~PartyPokemon()
{
	// Do nothing
}

void PartyPokemon::swapEndians()
{
	this->hitPointsExp = htons(this->hitPointsExp);
	this->attackExp = htons(this->attackExp);
	this->defenseExp = htons(this->defenseExp);
	this->speedExp = htons(this->speedExp);
	this->specialExp = htons(this->specialExp);

	unsigned long experience = this->getExperience();

	if (this->swappedEndians == true) {
		experience = htonl(experience) >> 8;
	} else {
		experience = htonl(experience << 8);
	}

	this->setExperience(experience);
	experience = this->getExperience();

	this->caughtData = htons(this->caughtData);
		
	this->hitPoints = htons(this->hitPoints);
	this->hitPointsMax = htons(this->hitPointsMax);
	this->attack = htons(this->attack);
	this->defense = htons(this->defense);
	this->speed = htons(this->speed);
	this->attackSpecial = htons(this->attackSpecial);
	this->defenseSpecial = htons(this->defenseSpecial);

	this->swappedEndians = !this->swappedEndians;
}

unsigned long PartyPokemon::getExperience()
{
	unsigned long experience = 0;
	memcpy(&experience, &this->experienceFix, 3);

	return experience;
}

void PartyPokemon::setExperience(unsigned long experience)
{
	memcpy(&this->experienceFix, &experience, 3);
}

void PartyPokemon::recalculate()
{
	this->calculateStats();
	this->scaleCurrentHitPoints();
	this->scaleExperience();
}

void PartyPokemon::calculateStats()
{
	this->hitPointsExp = this->hitPointsExp < 1 ? 1 : this->hitPointsExp;
	this->attackExp = this->attackExp < 1 ? 1 : this->attackExp;
	this->defenseExp = this->defenseExp < 1 ? 1 : this->defenseExp;
	this->speedExp = this->speedExp < 1 ? 1 : this->speedExp;
	this->specialExp = this->specialExp < 1 ? 1 : this->specialExp;

	this->previousHitPointsMax = this->hitPointsMax;
	this->hitPointsMax = floor(((this->pokemon.hitPoints+this->DVs.getHP())*2+floor((sqrt(this->hitPointsExp - 1) + 1) / 4))*this->level/100) + 10 + this->level;
	this->attack = htons(floor(((this->pokemon.attack+this->DVs.attack)*2+floor((sqrt(this->attackExp - 1) + 1) / 4))*this->level/100) + 5);
	this->defense = htons(floor(((this->pokemon.defense+this->DVs.defense)*2+floor((sqrt(this->defenseExp - 1) + 1) / 4))*this->level/100) + 5);
	this->speed = htons(floor(((this->pokemon.speed+this->DVs.speed)*2+floor((sqrt(this->speedExp - 1) + 1) / 4))*this->level/100)  + 5);
	this->attackSpecial = htons(floor(((this->pokemon.specialAttack+this->DVs.special)*2+floor((sqrt(this->specialExp - 1) + 1) / 4))*this->level/100) + 5);
	this->defenseSpecial = htons(floor(((this->pokemon.specialDefense+this->DVs.special)*2+floor((sqrt(this->specialExp - 1) + 1) / 4))*this->level/100) + 5);
}

void PartyPokemon::scaleCurrentHitPoints()
{
	if (this->previousHitPointsMax != this->hitPointsMax && this->hitPoints > 1)
	{
		int hpPct = this->hitPoints * 100 / this->previousHitPointsMax;
		this->hitPoints = ceil(this->hitPointsMax * hpPct / 100);

		if (this->hitPoints > this->hitPointsMax) {
			this->hitPoints = this->hitPointsMax;
		}

		if (this->hitPoints < 1) {
			this->hitPoints = 1;
		}
	}
}

void PartyPokemon::scaleExperience()
{
	unsigned long experience = this->getExperience();

	if (this->level < 100) {
		if (experience != pokemon.experience) {
			int prevBaseExp = minExpForLevel(this->level, this->previousPokemon);
			int prevNextExp = minExpForLevel(this->level + 1, this->previousPokemon);
			prevNextExp -= prevBaseExp;
			experience -= prevBaseExp;
			int experiencePct = experience * 100 / prevNextExp;

			int baseExp = minExpForLevel(this->level, this->pokemon);
			int nextExp = minExpForLevel(this->level + 1, this->pokemon);
			int reqExp = nextExp - baseExp;

			experience = baseExp + floor(experiencePct * reqExp / 100);
		}
	} else {
		experience = pokemon.experience;
	}
			
	if (this->level < 100 && experience > minExpForLevel(this->level + 1, pokemon)) {
		experience = minExpForLevel(this->level + 1, pokemon) - 1;
	}

	if (experience > pokemon.experience) {
		experience = pokemon.experience;
	}
			
	this->setExperience(experience);
}
