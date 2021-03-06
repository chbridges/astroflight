#ifndef LEVEL_H
#define LEVEL_H

#include <glad/glad.h>

#include "game_objects.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

class Level
{
private:
	std::string name;
	bool valid = true;
	unsigned int score = 0;

	std::vector<PointMass> pointMasses;
	std::vector<Planet> planets;
	std::vector<Moon> moons;
	std::vector<BlackHole> blackHoles;
	std::vector<PointMass*> physicsCore;
	std::vector<Star> stars;
	std::vector<Box> boxes;

public:
	Level(const std::string filePath)
		: name(filePath.substr(7, filePath.length() - 11))
	{
		std::ifstream levelFile;

		levelFile.open(filePath, std::ios::in);
		if (levelFile.is_open())
		{
			unsigned int nObjects;

			// Process PointMass
			levelFile >> nObjects;
			GLfloat mass, posX, posY, vX, vY;

			while (nObjects--)
			{
				levelFile >> mass >> posX >> posY >> vX >> vY;
				PointMass temp(mass, posX, posY);
				pointMasses.push_back(temp);
			}

			// Process Planet
			levelFile >> nObjects;

			if (nObjects < 2)
			{
				std::cout << "Error: Level needs at least 2 planets" << std::endl;
				valid = false;
			}

			GLfloat radius, r, g, b;

			while (nObjects--)
			{
				levelFile >> mass >> radius >> r >> g >> b >> posX >> posY >> vX >> vY;
				Planet temp(mass, radius, r, g, b, posX, posY, vX, vY);
				planets.push_back(temp);
			}

			// Process Moon
			levelFile >> nObjects;
			unsigned int planetIndex;
			GLfloat distance, angle;
			bool clockwise;

			while (nObjects--)
			{
				levelFile >> mass >> radius >> r >> g >> b >> planetIndex >> distance >> angle >> clockwise;
				if (planetIndex > planets.size())
				{
					std::cout << "Error: Planet index out of range" << std::endl;
					continue;
				}
				Moon temp(mass, radius, r, g, b, planets[planetIndex], distance, glm::radians(angle), clockwise);
				moons.push_back(temp);
			}

			// Process BlackHole
			levelFile >> nObjects;
			while (nObjects--)
			{
				levelFile >> mass >> posX >> posY >> vX >> vY;
				BlackHole temp(mass, posX, posY, vX, vY);
				blackHoles.push_back(temp);
			}

			/*
			std::cout << "Point masses generated: " << pointMasses.size() << std::endl;
			std::cout << "Planets generated: " << planets.size() << std::endl;
			std::cout << "Moons generated: " << moons.size() << std::endl;
			std::cot << "Black holes generated: " << blackHoles.size() << std::endl;
			*/
			

			levelFile.close();
		}
		else
		{
			std::cout << "Could not open " << filePath << std::endl;
		}

	}

	
	// Generate physics core
	void genPhysics()
	{
		// PointMasses
		for (auto & pm : pointMasses)
			physicsCore.push_back(dynamic_cast<PointMass*>(&pm));
		// Planets
		for (auto & p : planets)
			physicsCore.push_back(dynamic_cast<PointMass*>(&p));
		// Moons
		for (auto & m : moons)
			physicsCore.push_back(dynamic_cast<PointMass*>(&m));
		// Black holes
		for (auto & bh : blackHoles)
			physicsCore.push_back(dynamic_cast<PointMass*>(&bh));

		planets[0].setTerraforming(100);
		planets[1].setTerraforming(100);
	}


	// Move objects in level
	void updatePhysics()
	{
		for (auto & pm : pointMasses)
			pm.move();
		for (auto & planet : planets)
			planet.move();
		for (auto & moon : moons)
			moon.move();
		for (auto & box : boxes)
			box.move();
	}

	void updateScore(const int value)
	{
		score += value;
	}

	// Getter functions
	// ----------------
	std::string getName() const
	{
		return name;
	}
	bool isValid() const
	{
		return valid;
	}
	unsigned int getScore() const
	{
		return score;
	}
	std::vector<PointMass>& getPointMasses()
	{
		return pointMasses;
	}
	std::vector<Planet>& getPlanets()
	{
		return planets;
	}
	std::vector<Moon>& getMoons()
	{
		return moons;
	}
	std::vector<BlackHole>& getBlackHoles()
	{
		return blackHoles;
	}
	std::vector<PointMass*>& getPhysics()
	{
		return physicsCore;
	}
	std::vector<Box>& getBoxes()
	{
		return boxes;
	}
};

#endif