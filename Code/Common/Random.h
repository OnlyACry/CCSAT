#pragma once
#include"Config/TypeDef.h"

#include<random>
#ifndef RANDOM_DEF
#define RANDOM_DEF
namespace WorkSpace{
class RandomGen {
	std::mt19937 pseudoRandNumGen;
public:

	void initial(int seed) {
		pseudoRandNumGen = std::mt19937(seed);
	}
	void initial(Str& seed) {
		if (!seed.empty()) {
			pseudoRandNumGen = std::mt19937(stoi(seed));
		}
	}
	
	int rand(const int& lowerBound, int& upperBound) {
		return std::uniform_int_distribution<int>(lowerBound, upperBound)(pseudoRandNumGen);
	}
	int rand(const int upperBound) {
		return std::uniform_int_distribution<int>(0, upperBound - 1)(pseudoRandNumGen);
	}

	RandomGen() : pseudoRandNumGen(sCast<std::mt19937>(time(nullptr) + clock())) {}
};

} // namespace WorkSpace
#endif
