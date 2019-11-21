#pragma once

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <thread>
#include <tlhelp32.h>
#include <map>

#pragma comment( lib, "Ws2_32.lib")

// Cleanup
extern struct sPokemon;
int minExpForLevel(char level, sPokemon pokemon);
/// End Cleanup

#include "datatypes.h"
#include "helpers.h"

#include "PartyPokemon.h"

