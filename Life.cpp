/*
*
* Copyright 2020 Michael Oliva
*
* A pretty version of Conway's Game of Life written for Windows
* Find out more here: https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
*/

#include <string>
#include <vector>

// SRAND
#include <stdlib.h>
#include <time.h>

// Write to windows console
#include <Windows.h>

// Determine colour pairs
#include <map>

// std::max_element / std::count_if
#include <algorithm>

// std::accumulate
#include <numeric>

struct SCell
{
	enum EState
	{
		ALIVE,
		DEAD
	} state;
	WORD colour;

	SCell(SCell::EState s) : colour(NULL), state(s)			{ /* Empty */ }
	SCell() : colour(NULL), state(SCell::EState::DEAD)		{ /* Empty */ }
	SCell(WORD c, SCell::EState s) : colour(c), state(s)	{ /* Empty */ }

	inline bool operator==(const SCell::EState& comp)  { return state == comp; }
	inline bool operator!=(const SCell::EState& comp)  { return state != comp; }
};

// Print our cells to the console at each iteration
void print(std::vector<std::vector<SCell>>& v, HANDLE h, long long int gen)
{
	COORD location;
	DWORD written;

	WORD white = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

	for (size_t y = 0; y < v.size(); ++y)
	{
		for (size_t x = 0; x < v[y].size(); ++x)
		{
			location.X = x;
			location.Y = y;

			if (v[y][x].colour == NULL)
			{
				// Reset colour to white if we do not have a colours
				::WriteConsoleOutputAttribute(h, &white, 1, location, &written);
			}
			else ::WriteConsoleOutputAttribute(h, &v[y][x].colour, 1, location, &written);

			::WriteConsoleOutputCharacterA(h, v[y][x] == SCell::EState::ALIVE ? "x" : ".", 1, location, &written);
		}
	}

	location.X = 0;
	location.Y = v.size();

	std::string genStr = "Generation: ";
	genStr.append(std::to_string(gen));

	// Let's find how many cells are alive
	const int alive = std::accumulate
	(
		v.begin(), v.end(), 0,
		[](int total, const std::vector<SCell>& inner)
		{
			return total + std::count_if
			(
				inner.begin(), inner.end(),
				[](const SCell& cell) 
				{ 
					return cell.state == SCell::EState::ALIVE;
				}
			);
		}
	);


	genStr.append(" | Alive: ");
	genStr.append(std::to_string(alive));

	LPCSTR lpGenStr = genStr.c_str();

	::WriteConsoleOutputAttribute(h, &white, 1, location, &written);
	::WriteConsoleOutputCharacterA(h, lpGenStr, genStr.length(), location, &written);
}

struct SNeighbours
{
	unsigned int counter;
	WORD colour;
};

SNeighbours neighbours(std::vector<std::vector<SCell>>& v, int x, int y)
{
	SNeighbours neighbours = SNeighbours();

	std::map<WORD, unsigned int> pairs;

	// Let's check up
	if (y < v.size() - 1)
	{
		// We have tiles to check above
		if (v[y + 1][x] == SCell::EState::ALIVE)
		{
			neighbours.counter++;
			pairs[v[y + 1][x].colour] == NULL ? pairs[v[y + 1][x].colour] = 1 : pairs[v[y + 1][x].colour]++;
		}

		// Check the above diagonal
		if (x > 0 && v[y + 1][x - 1] == SCell::EState::ALIVE)
		{
			neighbours.counter++;
			pairs[v[y + 1][x - 1].colour] == NULL ? pairs[v[y + 1][x - 1].colour] = 1 : pairs[v[y + 1][x - 1].colour]++;
		}

		if (x < v[y].size() - 1 && v[y + 1][x + 1] == SCell::EState::ALIVE)
		{
			neighbours.counter++;
			pairs[v[y + 1][x + 1].colour] == NULL ? pairs[v[y + 1][x + 1].colour] = 1 : pairs[v[y + 1][x + 1].colour]++;
		}
	}

	// Let's check down
	if (y > 0)
	{
		// We have tiles to check below
		if (v[y - 1][x] == SCell::EState::ALIVE)
		{
			neighbours.counter++;
			pairs[v[y - 1][x].colour] == NULL ? pairs[v[y - 1][x].colour] = 1 : pairs[v[y - 1][x].colour]++;
		}

		// Check the below diagonal
		if (x > 0 && v[y - 1][x - 1] == SCell::EState::ALIVE)
		{
			neighbours.counter++;
			pairs[v[y - 1][x - 1].colour] == NULL ? pairs[v[y - 1][x - 1].colour] = 1 : pairs[v[y - 1][x - 1].colour]++;
		}

		if (x < v[y].size() - 1 && v[y - 1][x + 1] == SCell::EState::ALIVE)
		{
			neighbours.counter++;
			pairs[v[y - 1][x + 1].colour] == NULL ? pairs[v[y - 1][x + 1].colour] = 1: pairs[v[y - 1][x + 1].colour]++;
		}
	}

	// Now we just need to check the analouge side tiles!
	if (x < v[y].size() - 1 && v[y][x + 1] == SCell::EState::ALIVE)
	{
		neighbours.counter++;
		pairs[v[y][x + 1].colour] == NULL ? pairs[v[y][x + 1].colour] = 1: pairs[v[y][x + 1].colour]++;
	}

	if (x > 0 && v[y][x - 1] == SCell::EState::ALIVE)
	{
		neighbours.counter++;
		pairs[v[y][x - 1].colour] == NULL ? pairs[v[y][x - 1].colour] = 1 : pairs[v[y][x - 1].colour]++;
	}

	// We will only use the colour code if neighbours counter is exactly 3
	if (neighbours.counter == 3)
	{
		// Find the most common colours; let's use it to create our new node!
		using map_type = decltype(pairs)::value_type;
		neighbours.colour = std::max_element
		(
			std::begin(pairs), std::end(pairs),
			[](const map_type& p1, const map_type& p2)
			{
				return p1.second < p2.second;
			}
		)->first;
	}
	

	return neighbours;
}

int main()
{
	// Change these to change the resolution of the cells
	const unsigned int ROWS = 120;
	const unsigned int COLS = 28;

	// This 2d vector stores all of our cells
	std::vector<std::vector<SCell>> tiles(false);
	tiles.resize(COLS, std::vector<SCell>(ROWS));

	// Define time to sleep (ms)
	const unsigned int SLEEP_TIME = 200;

	// Seed srand
	srand(time(NULL));

	// Add a random number of cells
	for (int i = 0; i < (int)((COLS * ROWS) / 2); ++i)
	{
		// Let's define 3 starting cells next to each other
		const unsigned int RAND_LOCATION_Y = rand() % COLS;
		const unsigned int RAND_LOCATION_X = rand() % ROWS;

		tiles[RAND_LOCATION_Y][RAND_LOCATION_X].state = SCell::EState::ALIVE;

		// Let's start this cell off with a random colour!
		for (int idx = 0; idx < rand() % 3 + 1; ++idx)
		{
			// _CHAR_INFO supports 3 colour types, let's pick one
			switch (rand() % 3)
			{
			case 0:
				tiles[RAND_LOCATION_Y][RAND_LOCATION_X].colour |= FOREGROUND_RED;
				break;
			case 1:
				tiles[RAND_LOCATION_Y][RAND_LOCATION_X].colour |= FOREGROUND_GREEN;
				break;
			case 2:
				tiles[RAND_LOCATION_Y][RAND_LOCATION_X].colour |= FOREGROUND_BLUE;
				break;
			}
		}
	}


	// Windows output console handle
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);

	long long int generation = 0;

	while (true)
	{
		// Will replace our current vectors values at each iteration
		std::vector<std::vector<SCell>> repTiles;
		repTiles.resize(COLS, std::vector<SCell>(ROWS));

		// Handle core Life logic
		for (int y = 0; y < COLS; ++y)
		{
			for (int x = 0; x < ROWS; ++x)
			{
				// Let's find info about our neighbours
				SNeighbours neigh = neighbours(tiles, x, y);

				const unsigned int count = neigh.counter;
				if (tiles[y][x] == SCell::EState::ALIVE)
				{
					// Check of over/under population - kill off if needed
					if (count < 2 || count > 3)repTiles[y][x].state = SCell::EState::DEAD;
					else repTiles[y][x] = tiles[y][x];
				}
				else
				{
					if (count == 3)
					{
						// We have exactly 3 neighbours, let's create a new node here!
						repTiles[y][x].state = SCell::EState::ALIVE;
						repTiles[y][x].colour = neigh.colour;
					}			
					else repTiles[y][x].state = SCell::EState::DEAD;
				}
			}
		}

		// Replace our tiles with the new info!
		tiles = repTiles;

		// Update our console
		print(tiles, handle, generation++);
	}
}
