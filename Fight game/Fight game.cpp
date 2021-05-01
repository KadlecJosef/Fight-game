#include <iostream>
#include <cassert>
#include <initializer_list>
#include <string>
#include <array>
#include <random>
#include <ctime>

static int getRandomNumber(int min, int max)
{
	static std::mt19937 random{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
	std::uniform_int_distribution num(min, max);
	return num(random);
}

//base class for any character object used in the program conrtains most basic atributes

class Creature
{
protected:
	std::string m_name{};
	char m_symbol{};
	int m_health{};
	int m_attack{};
	int m_gold{};
public:
	Creature(std::string_view name, char symbol, int health, int attack, int gold)
		:m_name{ name }, m_symbol{ symbol }, m_health{ health }, m_attack{ attack }, m_gold{ gold }{}

	std::string getName() const { return m_name; }
	char getSymbol() const { return m_symbol; }
	int getHealth() const { return m_health; }
	int getAttack() const { return m_attack; }
	int getGold() const { return m_gold; }

	void reduceHealth(int damage) { m_health -= damage; }
	void addHealth(int healing) { m_health += healing; }
	void addAttack(int buff) { m_attack += buff; }
	bool isDead() const { return m_health <= 0; }
	void addGold(int gold) { m_gold += gold; }
};

//Player class defines player controlled object has the same characteristics everytime outside of the name that user choses

class Player : public Creature
{
private:
	int m_level{ 1 };
public:
	Player(std::string_view name) : Creature(name, '@', 10, 1, 0) {}

	void levelUp()
	{
		++m_level;
		++m_attack;
	}

	bool hasWon() const { return (m_level >= 20); }
	int getLevel() { return m_level; }
};

//Monster class contains base statistics for monsters and random monster generation method

class Monster : public Creature
{
public:
	enum Type
	{
		dragon,
		orc,
		slime,
		max_type
	};

	static const Creature& getDefaultCreature(Type type)
	{
		static std::array<Creature, static_cast<std::size_t>(Type::max_type)> monsterData{
			{	{"Dragon",'D',20,4,100},
				{"Orc",'o',4,2,25},
				{"Slime",'s',1,1,10}	}
		};
		return monsterData.at(static_cast<size_t>(type));
	}

	static Type getRandomMonster()
	{
		int num{ getRandomNumber(0,static_cast<int>(max_type) - 1) };
		return (static_cast<Type>(num));
	}

	Monster(Type type) :Creature{ getDefaultCreature(type) } {}


};

//Potion class is used to increase stats mainly for player class could be used to give additional stats to monsters too

class Potion
{
private:
	std::string m_type{};
	int m_size{};
public:
	void getRandomPotion()
	{
		int getType{ getRandomNumber(0,2) };
		switch (getType)
		{
		case 0:
			m_type = "Health";
			break;
		case 1:
			m_type = "Strength";
			break;
		case 2:
			m_type = "Poison";
			break;
		default:
			std::cout << "GetRandomPotionError\n";
			break;
		}

		m_size = getRandomNumber(0, 2);
	}

	void drinkPotion(Player& p)
	{
		static std::array<int, 3>potionValues{ 1,1,1 };

		std::cout << "You drank " << m_type << " potion\n";
		if (m_type == "Health")
		{
			p.addHealth(potionValues[m_size] + (m_size * 2));
		}
		else if (m_type == "Strength")
		{
			p.addAttack(potionValues[m_size]);
		}
		else if (m_type == "Poison")
		{
			p.reduceHealth(potionValues[m_size]);
		}

	}
};

//one in a three chance to get a potion for the player and use it

void getPotion(Player& p)
{
	if (getRandomNumber(0, 2) == 0)
	{
		char answer{};

		while (true)
		{
			std::cout << "You found mighty potion, do you want to drink it? (y/n)";
			std::cin >> answer;
			std::cin.ignore(32767, '\n');
			if (answer == 'y' || answer == 'Y');
			{
				Potion potion;
				potion.getRandomPotion();
				potion.drinkPotion(p);
				break;
			}
			if (answer == 'n' || answer == 'N')
			{
				std::cout << "Potion fade from your hands";
				break;
			}
		}
	}
	else return;
}

// Automatic response from a monster class to attack player class

bool attackPlayer(Monster& m, Player& p)
{
	p.reduceHealth(m.getAttack());
	std::cout << "You get hit by " << m.getName() << " for " << m.getAttack() << " damage.\n";
	if (p.isDead())
	{
		std::cout << "You died at level " << p.getLevel() << " with " << p.getGold() << " gold.\n";
		std::cout << "Too bad you cant take it with you!";
		return true;
	}
	return false;
}

// Player class attacks the monster class if the monster class is killed it hands out levels and gold

bool attackMonster(Monster& m, Player& p)
{
	m.reduceHealth(p.getAttack());
	std::cout << "You hit the " << m.getName() << " for " << p.getAttack() << " damage.\n";
	if (m.isDead())
	{
		std::cout << "You Killed the " << m.getName() << '\n';
		p.levelUp();
		std::cout << "You are now level " << p.getLevel() << '\n';
		p.addGold(m.getGold());
		std::cout << "You found " << m.getGold() << " gold" << '\n';
		getPotion(p);
		return true;
	}
	return false;
}

// user can choose whenever they want to engage in encounter or try to escape with 50/50 chance 
// if they engage they attack first if they try to escape and fail they get hit
// this function contains one encounter and generation of random monster

void fightMonster(Player& p)
{
	Monster m{ Monster::getRandomMonster() };
	std::cout << "You have encountered a " << m.getName() << '\n';
	char rf{};
	bool endFight{ false };
	do
	{
		std::cout << "(R)un or (F)ight: ";
		std::cin >> rf;
		std::cin.ignore(32767, '\n');
		if (rf == 'r' || rf == 'R')
		{
			endFight = static_cast<bool>(getRandomNumber(0, 1));
			if (endFight)
				std::cout << "You succesfully fled.\n";
			else
			{
				std::cout << "you failed to flee.\n";
				attackPlayer(m, p);
			}
		}
		else if (rf == 'f' || rf == 'F')
		{
			endFight = static_cast<bool>(attackMonster(m, p));
			if (!endFight)
				endFight = static_cast<bool>(attackPlayer(m, p));
		}

	} while (!endFight && !p.isDead());
}

void playGame(Player& p)
{
	while (!p.isDead())
	{
		fightMonster(p);
		if (p.hasWon())
		{
			std::cout << "You won!";
			break;
		}
	}
}

int main()
{

	std::cout << "Enter your name:";
	std::string name{};
	std::cin >> name;

	Player p{ name };
	std::cout << "Welcome " << p.getName() << '\n';
	std::cout << "You have " << p.getHealth() << " health and are carrying " << p.getGold() << " gold.\n";

	playGame(p);

	return 0;
}