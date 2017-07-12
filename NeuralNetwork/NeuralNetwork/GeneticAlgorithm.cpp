#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <iostream>

enum eValues : char
{
	_0,
	_1,
	_2,
	_3,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	_ADD, //10
	_SUB, //11
	_MUL, //12
	_DIV, //13
	__COUNT,
};

const int locTargetValue = 154;
const int locChromosomeSize = 20;
const int locMaxIterations = 400;
const int locPopulationSize = 10; //Needs to be even!!
const float locCrossoverRate = 0.1f;
const float locMutateRate = 0.07f;

struct Chromosome
{
	Chromosome()
	{
		for (int i = 0; i < locChromosomeSize; ++i)
		{
			myGenes[i] = rand() % __COUNT;
		}

		myFitness = 0.f;
		myResult = 0;
	}

	char myGenes[locChromosomeSize];
	float myFitness;

	float myResult;
};

bool IsNumber(char aGene)
{
	return aGene != _ADD && aGene != _SUB && aGene != _MUL && aGene != _DIV;
}

bool IsOperator(char aGene)
{
	return !IsNumber(aGene);
}

int ResolveChromosome(Chromosome& aChromosome)
{
	float sum = 0;
	bool expectsNumber = true;
	char currOperator = __COUNT;

	for (int i = 0; i < locChromosomeSize; ++i)
	{
		char gene = aChromosome.myGenes[i];

		if (expectsNumber)
		{
			if(!IsNumber(gene))
				continue;

			if (currOperator == __COUNT)
			{
				sum = static_cast<float>(gene);
			}
			else
			{
				if (currOperator == _ADD)
					sum += gene;
				else if (currOperator == _SUB)
					sum -= gene;
				else if (currOperator == _MUL)
					sum *= gene;
				else if (currOperator == _DIV)
				{
					if (gene == 0.f)
					{
						aChromosome.myGenes[i] = _ADD;
						sum += gene;
					}
					else
					{
						sum /= gene;
					}
				}
			}

			expectsNumber = false;
		}
		else
		{
			if(!IsOperator(gene))
				continue;

			currOperator = gene;
			expectsNumber = true;
		}
	}

	if (expectsNumber)
		return INT_MIN;

	return sum;
}

void PrintChromosome(const Chromosome& aChromosome)
{
	bool expectsNumber = true;

	for (int i = 0; i < locChromosomeSize; ++i)
	{
		char gene = aChromosome.myGenes[i];

		if (expectsNumber)
		{
			if (!IsNumber(gene))
				continue;

			std::cout << int(gene) << " ";

			expectsNumber = false;
		}
		else
		{
			if (!IsOperator(gene))
				continue;

			if (gene == _ADD)
				std::cout << "+ ";
			else if (gene == _SUB)
				std::cout << "- ";
			else if (gene == _MUL)
				std::cout << "* ";
			else if (gene == _DIV && gene != 0)
				std::cout << "/ ";

			expectsNumber = true;
		}
	}
}

float RandomFloat()
{
	return static_cast<float>(rand()) / (RAND_MAX + 1);
}

Chromosome RandomRoulette(Chromosome* someChromosomes, float aTotalFitness)
{
	float slice = RandomFloat() * aTotalFitness;
	float tally = 0.f;

	for (int i = 0; i < locPopulationSize; ++i)
	{
		tally += someChromosomes[i].myFitness;

		if (tally >= slice)
			return someChromosomes[i];
	}

	return someChromosomes[0];
}

void Crossover(Chromosome& aChromosomeA, Chromosome& aChromosomeB)
{
	if (RandomFloat() > locCrossoverRate)
		return;

	int startPoint = rand() & locChromosomeSize;
	for (int i = startPoint; i < locChromosomeSize; ++i)
	{
		char temp = aChromosomeA.myGenes[i];
		aChromosomeA.myGenes[i] = aChromosomeB.myGenes[i];
		aChromosomeB.myGenes[i] = temp;
	}
}

void Mutate(Chromosome& aChromosome)
{
	for (int i = 0; i < locChromosomeSize; ++i)
	{
		if (RandomFloat() > locMutateRate)
			continue;

		aChromosome.myGenes[i] = rand() % __COUNT;
	}
}

int main()
{
	srand(time(NULL));

	Chromosome currentGeneration[locPopulationSize];
	for (int i = 0; i < locPopulationSize; ++i)
	{
		currentGeneration[i] = Chromosome();
	}

	bool foundSolution = false;
	Chromosome validSolution;
	int generationCount = 1;

	while(!foundSolution && generationCount < locMaxIterations)
	{
		//Generate fitness for all the Chromosomes
		float totalFitness = 0.f;
		for (int j = 0; j < locPopulationSize; ++j)
		{
			Chromosome& chromosome = currentGeneration[j];
			chromosome.myResult = ResolveChromosome(chromosome);

			if (chromosome.myResult == locTargetValue)
			{
				foundSolution = true;
				validSolution = chromosome;
				break;
			}
			else
			{
				chromosome.myFitness = 1.f / (locTargetValue - chromosome.myResult);
			}

			totalFitness += chromosome.myFitness;
		}


		//Crossover
		Chromosome tempGeneration[locPopulationSize];
		for (int j = 0; j < locPopulationSize; j += 2)
		{
			Chromosome roulette1 = RandomRoulette(currentGeneration, totalFitness);
			Chromosome roulette2 = RandomRoulette(currentGeneration, totalFitness);

			Crossover(roulette1, roulette2);

			tempGeneration[j] = roulette1;
			tempGeneration[j + 1] = roulette2;
		}

		//Copy new generation into our current generation
		//Mutate in this loop aswell
		for (int j = 0; j < locPopulationSize; ++j)
		{
			currentGeneration[j] = tempGeneration[j];

			Mutate(currentGeneration[j]);
		}

		++generationCount;
	}
	
	if (foundSolution)
	{
		std::cout << "Found a solution! (Target value was " << locTargetValue << ")" << std::endl;
		PrintChromosome(validSolution);
		int apa = 5;
	}
	else
	{
		std::cout << "Found no solution in " << locMaxIterations << " generations" << std::endl;
	}


	int apa = 5;
}