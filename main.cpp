#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <ncurses.h>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <math.h>
#include <string>
using namespace std;
int targets;
int tab[50];
pthread_mutex_t blokada;
struct target
{
public:
	int x;
	int y;
	int health;
	string name;
	bool visible;
	bool narysowany;
	bool zlokalizowany;
}target_tab[100];
class Bomber
{
public:
	int *land, targetX, targetY, health, bombs, x, y, oldX, oldY, targetDistance, enemyHealth, targetNumber;
	bool enemy, draw, on_ground, airportRunway, state;
	pthread_t mover;
	pthread_t localisation;
	pthread_t radarr;
	Bomber()
	{
		health = 100;
		bombs = 50;
		enemy = false;
		on_ground = true;
		state = true;
	}
	void setLand(int *tab)
	{
		land = tab;
	}
	void setEnNumber(int tab)
	{
		targetNumber = tab;
	}
	void setTarget(int xx, int yy)
	{
		targetX = xx;
		targetY = yy;
	}
	void setPosition(int xx, int yy)
	{
		x = xx;
		y = yy;
	}
	void fly()
	{
		int number;
		do
		{
			oldX = x;
			oldY = y;
			if (on_ground)
			{
			}
			else
			{
				if (enemy)
				{
					targetDistance = sqrt((targetX - x)*(targetX - x) + (targetY - y)*(targetY - y));
					if (targetX > x)
					{
						x++;
					}
					else
					{
						x--;
					}
					if (targetY > y)
					{
						y++;
					}
					else
					{
						y--;
					}
					if (targetDistance < 2)
					{
						errno = pthread_mutex_lock(&blokada);
						attack();
						errno = pthread_mutex_unlock(&blokada);
					}
				}
				else
				{
					if (50 <= x)
					{
						x--;
					}
					else if (50 >= x)
					{
						x++;
					}
					if (80 <= y)
					{
						y--;
					}
					else if (80 >= y)
					{
						y++;
					}
					if (50 == x && 80 == y && airportRunway)
					{
						on_ground = true;
					}
				}
			}
			usleep(100000);
		} while (state);
	}
	void drawLocalisation()
	{
		do
		{
			if (on_ground)
			{
			}
			if (!on_ground && y <= 50)
			{
				errno = pthread_mutex_lock(&blokada);
				if (land[oldY] > oldX)
				{
					attrset(COLOR_PAIR(5));
					move(oldY, oldX);
					printw(" ");
				}
				else
				{
					attrset(COLOR_PAIR(3));
					move(oldY, oldX);
					printw(" ");
				}
				attrset(COLOR_PAIR(7));
				move(y, x);
				printw(" ");
				refresh();
				errno = pthread_mutex_unlock(&blokada);
			}
			if (!on_ground && y == 51)
			{
				if (land[oldY] > oldX)
				{
					attrset(COLOR_PAIR(5));
					move(oldY, oldX);
					printw(" ");
				}
				else
				{
					attrset(COLOR_PAIR(3));
					move(oldY, oldX);
					printw(" ");
				}
			}
			usleep(100000);
		} while (state);
		attrset(COLOR_PAIR(5));
		move(oldX, oldY);
		printw(" ");
	}
	void attack()
	{
		target_tab[targetNumber].health = 0;
		bombs = 0;
		enemyHealth = 0;
		target_tab[targetNumber].visible = false;
		enemy = false;
	}
	void radar()
	{
		//Funkcja radar, wykrywa wrogie zestawy przeciwrakietowe odpowiedzialne za uszkodzenia
		do {
			for (int i = 0; i < 100; i++)
			{
				if (sqrt((target_tab[i].x - x)*(target_tab[i].x - x) + (target_tab[i].y - y)*(target_tab[i].y - y)) < 10 && target_tab[i].name == "antiaicraftdefense")
				{
					health = health - rand() % 25;
				}
			}
			usleep(100000);
		} while (true);
	}
	int getEnemyHealth()
	{
		return enemyHealth;
	}
	int getX()
	{
		return x;
	}
	int getY()
	{
		return y;
	}
	void setbombs()
	{
		bombs++;
		if (bombs > 50)
		{
			bombs = 50;
		}
	}
	void setHealth()
	{
		health += 10;
		if (health > 100)
		{
			health = 100;
		}
	}
	static void* moveHelper(void* arg)
	{
		Bomber* Bombers = reinterpret_cast<Bomber*>(arg);
		Bombers->fly();
		return 0;
	}
	static void* localisationHelper(void* arg)
	{
		Bomber* Bombers = reinterpret_cast<Bomber*>(arg);
		Bombers->drawLocalisation();
		return 0;
	}
	static void* radarHelper(void* arg)
	{
		Bomber* Bombers = reinterpret_cast<Bomber*>(arg);
		Bombers->radar();
		return 0;
	}
	bool getBoardStatus()
	{
		return on_ground;
	}
	void setBoardStatus()
	{
		on_ground = false;
	}
	bool getEnemyStatus()
	{
		return enemy;
	}
	void setEnemyStatus()
	{
		enemy = true;
	}
	void setairportRunway(bool cos)
	{
		airportRunway = cos;
	}
	void start()
	{
		pthread_create(&mover, 0, moveHelper, this);
		pthread_create(&localisation, 0, localisationHelper, this);
		pthread_create(&radarr, 0, radarHelper, this);
	}
	void join()
	{
		pthread_join(mover, 0);
		pthread_join(localisation, 0);
		pthread_join(radarr, 0);
	}
};
class AirPort
{
public:
	int unit_position[168][50];
	bool runway;
	int x, y, oldX, oldY, *land;
	int how_many_targets_is;
	int how_many_jets_need;
	Bomber *bombers[6];
	bool tab[100];
	int tab2[100];
	pthread_t mechanic;
	pthread_t startjet;
	pthread_t setTarget;
	AirPort()
	{
		runway = true;
		for (int i = 0; i < 168; i++)
		{
			for (int j = 0; j < 50; j++)
			{
				unit_position[i][j] = 0;
			}
		}
		unit_position[0][0] = 0;
		x = 50; //Wspolrzedne startowe (lotniska)
		y = 80;
		how_many_targets_is = 0;
		for (int i = 0; i < 6; i++)
		{
			bombers[i] = new Bomber();
			bombers[i]->start();
			tab[i] = false;
		}
	};
	void setLand(int *tab)
	{
		land = tab;
	}
	static void* mechanicHelper(void* arg)
	{
		AirPort* Carrier = reinterpret_cast<AirPort*>(arg);
		Carrier->jetsMechanics();
		return 0;
	}
	static void* reactionHelper(void* arg)
	{
		AirPort* Carrier = reinterpret_cast<AirPort*>(arg);
		Carrier->jetStart();
		return 0;
	}
	static void* setTargetsHelper(void* arg)
	{
		AirPort* Carrier = reinterpret_cast<AirPort*>(arg);
		Carrier->setTargets();
		return 0;
	}
	void start()
	{
		pthread_create(&mechanic, 0, mechanicHelper, this);
		pthread_create(&startjet, 0, reactionHelper, this);
		pthread_create(&setTarget, 0, setTargetsHelper, this);
		for (int i = 0; i < 6; i++)
		{
			bombers[i]->setLand(land);
		}
	}
	void join()
	{
		for (int j = 0; j < 6; j++)
		{
			bombers[j]->join();																								// Zamkni�cie wszystkich w�tk�w samolot�w
		}
		pthread_join(mechanic, 0);
		pthread_join(startjet, 0);
		pthread_join(setTarget, 0);
	}
	int runJet(int samolot, int nr)
	{
		/* Funkcja startuj�ca wybrany mysliwiec */
		if (bombers[samolot]->getBoardStatus() && bombers[samolot]->health > 90 && bombers[samolot]->bombs > 10 && target_tab[nr].zlokalizowany && tab[nr])			// Je�eli samolot jest na pok�adzie i ma wiecej niz 90 hp oraz 10 bomb to mo�na go wystartowa�
		{
			if (runway)																										// Je�eli pas startowy jest wolny to mo�na wystartowa�
			{
				runway = false;																								// Pas startowy lotniska bazy jest zaj�ty przez startuj�cy samolot
				for (int j = 0; j < 6; j++)
				{
					bombers[j]->setairportRunway(false);														// Poinformowanie wszystkich bombowcow stacjonuj�cych w bazie lotniczej, �e pas startowy jest zaj�ty
				}
				bombers[samolot]->setBoardStatus();																			// Zmiana statusu samolotu z "na pok�adzie" na "w powietrzu"
				bombers[samolot]->setEnemyStatus();																			// Zmiana statusu wroga z "brak wroga" na "jest wr�g"
				bombers[samolot]->setEnNumber(nr);
				bombers[samolot]->setTarget(target_tab[nr].x, target_tab[nr].y);
				bombers[samolot]->setPosition(y, x);
				tab[nr] = false;
			}
			else
			{
				return 2;																									// Pas startowy jest zaj�ty, funkcja zwraca 2 symbolizuj�c problem z pasem
			}
		}
		else
		{
			return 1;																										// Samolot nie spe�nia warunk�w by wys�a� go na misje, funkcja zwraca 1 co wskazuje na problemy z samolotem
		}
		usleep(3000000);
		runway = true;																										// Samolot wystartowa�, nast�puje zwolnienie pasa
		for (int j = 0; j < 6; j++)
		{
			bombers[j]->setairportRunway(true);																			// Poinformowanie wszystkich samolot�w o tym �e pas jest wolny
		}
		return 0;																											// Wszystko przebieg�o pomy�lnie, funkcja zwraca 0 b��d�w
	}
	void setTargets()
	{
		int number = 0;
		do
		{
			for (int i = 0; i < 100; i++)
			{
				if (target_tab[i].visible && !target_tab[i].zlokalizowany && target_tab[i].health > 0)
				{
					target_tab[i].zlokalizowany = true;
					how_many_targets_is++;
					how_many_jets_need++;
					tab[i] = true;
				}
			}
			usleep(1000000);
		} while (true);
	}
	void jetStart()
	{
		int number;
		do
		{
			if (how_many_jets_need > 0)
			{
				number = runJet((rand() % 6), (rand() % 100));
				if (number == 0)
				{
					how_many_jets_need--;
				}
			}
		} while (true);
	}
	void jetsMechanics()
	{
		do
		{
			int jet = rand() % 6;
			if (bombers[jet]->getBoardStatus())
			{
				bombers[jet]->setbombs();															// Za�adunek bomb
				bombers[jet]->setHealth();																// Naprawa uszkodze� samolotu
			}
			usleep(100000);
		} while (true);
	}
};
class Reconnaissance_aircraft
{
public:
	int *land, targetX, targetY, health, bomb, x, y, oldX, oldY, targetDistance, baseX, baseY, enemyHealth, targetNumber;
	bool enemy, draw, on_the_board, carrierRunway;
	pthread_t mover;
	pthread_t spy;
	pthread_t localisation;
	Reconnaissance_aircraft()
	{
		x = 140;	//Wspólrzedne startowe
		y = 16;
	}
	void baseCurrentPosition(int xx, int yy)
	{
		baseX = xx;
		baseY = yy;
	}
	void setLand(int *tab)
	{
		land = tab;
	}
	void fly()
	{
		//Funkcja odpowiadająca za poruszanie się
		int number;
		do
		{
			oldX = x;
			oldY = y;
			number = rand() % 500;
			if (number % 5 == 2 && x >= 0 && x < 168)
			{
				x++;
			}
			else if (number % 5 == 3 && x >= 1 && x < 168)
			{
				x--;
			}
			else if (number % 5 == 4 && y >= 0 && y < 49)
			{
				y++;
			}
			else if (number % 5 == 5 && y >= 1 && y < 50)
			{
				y--;
			}
			targetDistance = sqrt((targetX - x)*(targetX - x) + (targetY - y)*(targetY - y));
			usleep(200000);
		} while (true);
	}
	void spyy()
	{
		do {
			for (int i = 0; i < 100; i++)
			{
				if (sqrt((target_tab[i].x - x)*(target_tab[i].x - x) + (target_tab[i].y - y)*(target_tab[i].y - y)) < 8) //Lokalizuje wrogie obiektu w promieniu 8 jednostek
				{
					target_tab[i].visible = true; //Zlokalizowanie wrogiego obiektu
				}
			}
			usleep(100000);
		} while (true);
	}
	void drawLocalisation()
	{
		do
		{
			if (on_the_board)
			{
			}
			else
			{
				errno = pthread_mutex_lock(&blokada);
				if (land[oldY] > oldX)
				{
					attrset(COLOR_PAIR(5));
					move(oldY, oldX);
					printw(" ");
				}
				else
				{
					attrset(COLOR_PAIR(3));
					move(oldY, oldX);
					printw(" ");

				}
				attrset(COLOR_PAIR(1));
				move(y, x);
				printw(" ");
				refresh();
				errno = pthread_mutex_unlock(&blokada);
			}
			usleep(200000);
		} while (true);
		attrset(COLOR_PAIR(5));
		move(oldX, oldY);
		printw(" ");
	}
	static void* moveHelper(void* arg)
	{
		Reconnaissance_aircraft* Reconnaissance = reinterpret_cast<Reconnaissance_aircraft*>(arg);
		Reconnaissance->fly();
		return 0;
	}
	static void* spyHelper(void* arg)
	{
		Reconnaissance_aircraft* Reconnaissance = reinterpret_cast<Reconnaissance_aircraft*>(arg);
		Reconnaissance->spyy();
		return 0;
	}
	static void* localisationHelper(void* arg)
	{
		Reconnaissance_aircraft* Reconnaissance = reinterpret_cast<Reconnaissance_aircraft*>(arg);
		Reconnaissance->drawLocalisation();
		return 0;
	}
	void start()
	{
		pthread_create(&mover, 0, moveHelper, this);
		pthread_create(&spy, 0, spyHelper, this);
		pthread_create(&localisation, 0, localisationHelper, this);
	}
	void join()
	{

		pthread_join(mover, 0);
		pthread_join(localisation, 0);
	}
};
class Jet
{
public:
	int *land, targetX, targetY, health, missile, x, y, oldX, oldY, targetDistance, carrierX, carrierY, enemyHealth, targetNumber;
	bool enemy, draw, on_the_board, carrierRunway;
	pthread_t mover;
	pthread_t localisation;
	Jet()
	{
		health = 100;
		missile = 12;
		enemy = false;
		on_the_board = true;
	}
	void setLand(int *tab)
	{
		land = tab;
	}
	void setEnNumber(int tab)
	{
		targetNumber = tab;
	}
	void setTarget(int xx, int yy)
	{
		targetX = xx;
		targetY = yy;
	}
	void carrierCurrentPosition(int xx, int yy)
	{
		carrierX = xx;
		carrierY = yy;
	}
	void setPosition(int xx, int yy)
	{
		x = xx;
		y = yy;
	}
	void fly()
	{
		// Funkcja odpowiadająca za poruszanie się
		int number;
		do
		{
			oldX = x;
			oldY = y;
			if (on_the_board)
			{
			}
			else
			{
				if (enemy)
				{
					targetDistance = sqrt((targetX - x)*(targetX - x) + (targetY - y)*(targetY - y));
					if (targetX > x)
					{
						x++;
					}
					else
					{
						if (targetX < x)x--;
					}
					if (targetY > y)
					{
						y++;
					}
					else
					{
						if (targetY < y)y--;
					}
					if (targetDistance < 10)
					{
						errno = pthread_mutex_lock(&blokada);
						attack();
						errno = pthread_mutex_unlock(&blokada);
					}
				}
				else
				{
					if (carrierY <= x)
					{
						x--;
					}
					else if (carrierY >= x)
					{
						x++;
					}
					if (carrierX <= y)
					{
						y--;
					}
					else if (carrierX >= y)
					{
						y++;
					}
					if (carrierY == x && carrierX == y && carrierRunway)
					{
						on_the_board = true;
					}
				}
			}
			usleep(200000);
		} while (true);
	}
	void drawLocalisation()
	{
		//Funkcja rysujaca lokalizacje obiektu na mapie
		do
		{
			if (on_the_board)
			{

			}
			else
			{
				errno = pthread_mutex_lock(&blokada);
				if (land[oldY] > oldX)
				{
					attrset(COLOR_PAIR(5));
					move(oldY, oldX);
					printw(" ");
				}
				else
				{
					attrset(COLOR_PAIR(3));
					move(oldY, oldX);
					printw(" ");
				}
				if (sqrt((carrierX - y)*(carrierX - y) + (carrierY - x)*(carrierY - x)) > 5)
				{
					attrset(COLOR_PAIR(8));
					move(y, x);
					printw(" ");
				}
				refresh();
				errno = pthread_mutex_unlock(&blokada);
			}
			usleep(100000);
		} while (true);
		attrset(COLOR_PAIR(5));
		move(oldX, oldY);
		printw(" ");
	}
	void attack()
	{
		target_tab[targetNumber].health = target_tab[targetNumber].health - (rand() % 100);
		missile--;
		if (target_tab[targetNumber].health < 0)
		{
			enemyHealth = 0;
			target_tab[targetNumber].visible = false;
			enemy = false;
		}
	}
	int getEnemyHealth()
	{
		return enemyHealth;
	}
	int getX()
	{
		return x;
	}
	int getY()
	{
		return y;
	}
	void setMissile()
	{
		missile++;
		if (missile > 12)
		{
			missile = 12;
		}
	}
	void setHealth()
	{
		health += 10;
		if (health > 100)
		{
			health = 100;
		}
	}
	static void* moveHelper(void* arg)
	{
		Jet* jets = reinterpret_cast<Jet*>(arg);
		jets->fly();
		return 0;
	}
	static void* localisationHelper(void* arg)
	{
		Jet* jets = reinterpret_cast<Jet*>(arg);
		jets->drawLocalisation();
		return 0;
	}
	bool getBoardStatus()
	{
		return on_the_board;
	}
	void setBoardStatus()
	{
		on_the_board = false;
	}
	bool getEnemyStatus()
	{
		return enemy;
	}
	void setEnemyStatus()
	{
		enemy = true;
	}
	void setCarrierRunway(bool cos)
	{
		carrierRunway = cos;
	}
	void start()
	{
		pthread_create(&mover, 0, moveHelper, this);
		pthread_create(&localisation, 0, localisationHelper, this);
	}
	void join()
	{

		pthread_join(mover, 0);
		pthread_join(localisation, 0);
	}
};
class Aircraft_Carrier
{
public:
	int unit_position[168][50];
	bool runway;
	int x, y, oldX, oldY, *land;
	int how_many_targets_is;
	int how_many_jets_need;
	Jet *fighters[20];
	bool tab[100];
	int tab2[100];
	pthread_t mover;
	pthread_t localisation;
	pthread_t mechanic;
	pthread_t startjet;
	pthread_t setTarget;
	Aircraft_Carrier()
	{
		runway = true;
		for (int i = 0; i < 168; i++)
		{
			for (int j = 0; j < 50; j++)
			{
				unit_position[i][j] = 0;
			}
		}
		unit_position[0][0] = 0;
		x = 3;
		y = 3;
		how_many_targets_is = 0;
		for (int i = 0; i < 20; i++)
		{
			fighters[i] = new Jet();
			fighters[i]->start();
			tab[i] = false;
		}
	};
	void setLand(int *tab)
	{
		land = tab;
	}
	static void* moveHelper(void* arg)
	{
		Aircraft_Carrier* Carrier = reinterpret_cast<Aircraft_Carrier*>(arg);
		Carrier->swim();
		return 0;
	}
	static void* localisationHelper(void* arg)
	{
		Aircraft_Carrier* Carrier = reinterpret_cast<Aircraft_Carrier*>(arg);
		Carrier->drawLocalisation();
		return 0;
	}
	static void* mechanicHelper(void* arg)
	{
		Aircraft_Carrier* Carrier = reinterpret_cast<Aircraft_Carrier*>(arg);
		Carrier->jetsMechanics();
		return 0;
	}
	static void* reactionHelper(void* arg)
	{
		Aircraft_Carrier* Carrier = reinterpret_cast<Aircraft_Carrier*>(arg);
		Carrier->jetStart();
		return 0;
	}
	static void* setTargetsHelper(void* arg)
	{
		Aircraft_Carrier* Carrier = reinterpret_cast<Aircraft_Carrier*>(arg);
		Carrier->setTargets();
		return 0;
	}
	void start()
	{

		pthread_create(&mover, 0, moveHelper, this);
		pthread_create(&localisation, 0, localisationHelper, this);
		pthread_create(&mechanic, 0, mechanicHelper, this);
		pthread_create(&startjet, 0, reactionHelper, this);
		pthread_create(&setTarget, 0, setTargetsHelper, this);
		for (int i = 0; i < 20; i++)
		{
			fighters[i]->setLand(land);
		}
	}
	void join()
	{
		for (int j = 0; j < 20; j++)
		{
			fighters[j]->join();																								// Zamkni�cie wszystkich w�tk�w samolot�w
		}
		pthread_join(mover, 0);
		pthread_join(localisation, 0);
		pthread_join(mechanic, 0);
		pthread_join(startjet, 0);
		pthread_join(setTarget, 0);
	}
	int runJet(int samolot, int nr)
	{
		/* Funkcja startuj�ca wybrany mysliwiec */
		if (fighters[samolot]->getBoardStatus() && fighters[samolot]->health > 90 && fighters[samolot]->missile > 10 && target_tab[nr].zlokalizowany && tab[nr])			// Je�eli samolot jest na pok�adzie i ma wiecej niz 90 hp oraz 10 rakiet to mo�na go wystartowa�
		{
			if (runway)																										// Je�eli pas startowy jest wolny to mo�na wystartowa�
			{
				runway = false;																								// Pas startowy lotniskowca jest zaj�ty przez startuj�cy samolot
				for (int j = 0; j < 20; j++)
				{
					fighters[j]->setCarrierRunway(false);																	// Poinformowanie wszystkich my�liwc�w stacjonuj�cych na lotniskowcu, �e pas startowy jest zaj�ty
				}
				fighters[samolot]->setBoardStatus();																			// Zmiana statusu samolotu z "na pok�adzie" na "w powietrzu"
				fighters[samolot]->setEnemyStatus();																			// Zmiana statusu wroga z "brak wroga" na "jest wr�g"
				fighters[samolot]->setEnNumber(nr);
				fighters[samolot]->setTarget(target_tab[nr].x, target_tab[nr].y);
				fighters[samolot]->setPosition(y, x);
				tab[nr] = false;
			}
			else
			{
				return 2;																									// Pas startowy jest zaj�ty, funkcja zwraca 2 symbolizuj�c problem z pasem
			}
		}
		else
		{
			return 1;																										// Samolot nie spe�nia warunk�w by wys�a� go na misje, funkcja zwraca 1 co wskazuje na problemy z samolotem
		}
		usleep(3000000);
		runway = true;																										// Samolot wystartowa�, nast�puje zwolnienie pasa
		for (int j = 0; j < 20; j++)
		{
			fighters[j]->setCarrierRunway(true);																			// Poinformowanie wszystkich samolot�w o tym �e pas jest wolny
		}
		return 0;																											// Wszystko przebieg�o pomy�lnie, funkcja zwraca 0 b��d�w
	}
	void setTargets()
	{
		int number = 0;
		do
		{
			for (int i = 0; i < 100; i++)
			{
				if (target_tab[i].visible && !target_tab[i].zlokalizowany && target_tab[i].health > 0)
				{
					target_tab[i].zlokalizowany = true;
					how_many_targets_is++;
					how_many_jets_need++;
					tab[i] = true;
				}
			}
			usleep(1000000);
		} while (true);
	}
	void jetStart()
	{
		int number;
		do
		{
			if (how_many_jets_need > 0)
			{
				number = runJet((rand() % 20), (rand() % 100));
				if (number == 0)
				{
					how_many_jets_need--;
				}
			}
		} while (true);
	}
	void swim()
	{
		int number = 0;
		do
		{
			unit_position[x][y] = 0;
			oldX = x;
			oldY = y;
			number = rand() % 500;
			if (number % 5 == 2 && x >= 3 && x < land[y])
			{
				x++;
			}
			else if (number % 5 == 3 && x >= 3 && x < land[y])
			{
				x--;
			}
			else if (number % 5 == 4 && y >= 3 && y < 49)
			{
				y++;
			}
			else if (number % 5 == 5 && y >= 3 && y < 50)
			{
				y--;
			}
			for (int j = 0; j < 20; j++)
			{
				fighters[j]->carrierCurrentPosition(x, y);																					// Przekazanie wsp��rz�dnych lotniskowca (bazy) do wszystkich my�liwc�w
			}
			unit_position[x][y] = 1;
			usleep(1000000);
		} while (true);
	}
	void drawLocalisation()
	{
		do
		{
			errno = pthread_mutex_lock(&blokada);
			attrset(COLOR_PAIR(5));
			move(oldX, oldY);
			printw(" ");
			attrset(COLOR_PAIR(2));
			move(x, y);
			printw(" ");
			refresh();
			errno = pthread_mutex_unlock(&blokada);
			usleep(1000000);
		} while (true);
	}
	void jetsMechanics()
	{
		do
		{
			int jet = rand() % 20;
			if (fighters[jet]->getBoardStatus())
			{
				fighters[jet]->setMissile();																// Za�adunek rakiet
				fighters[jet]->setHealth();																// Naprawa uszkodze� samolotu
			}
			usleep(100000);
		} while (true);
	}
};
void setWybrzez(int *tab)
{
	//Funkcja która tworzy zarys wybrzeza
	tab[0] = 80;
	tab[1] = 80;
	tab[2] = 81;
	tab[3] = 90;
	tab[4] = 96;
	tab[5] = 100;
	tab[6] = 103;
	tab[7] = 104;
	tab[8] = 107;
	tab[9] = 113;
	tab[10] = 118;
	tab[11] = 124;
	tab[12] = 126;
	tab[13] = 127;
	tab[14] = 128;
	tab[15] = 128;
	tab[16] = 128;
	tab[17] = 128;
	tab[18] = 129;
	tab[19] = 130;
	tab[20] = 131;
	tab[21] = 134;
	tab[22] = 135;
	tab[23] = 135;
	tab[24] = 136;
	tab[25] = 136;
	tab[26] = 135;
	tab[27] = 133;
	tab[28] = 131;
	tab[29] = 129;
	tab[30] = 128;
	tab[31] = 126;
	tab[32] = 124;
	tab[33] = 122;
	tab[34] = 120;
	tab[35] = 118;
	tab[36] = 116;
	tab[37] = 113;
	tab[38] = 110;
	tab[39] = 107;
	tab[40] = 104;
	tab[41] = 101;
	tab[42] = 98;
	tab[43] = 95;
	tab[44] = 93;
	tab[45] = 92;
	tab[46] = 91;
	tab[47] = 90;
	tab[48] = 89;
	tab[49] = 87;
	tab[50] = 86;
}
void drawScenery(int *tab)
{
	attrset(COLOR_PAIR(5));
	for (int r = 0; r <= 60; r++)
	{
		for (int c = 0; c <= 168; c++)
		{
			move(r, c);
			printw(" ");
		}
	}
	attrset(COLOR_PAIR(3));
	for (int r = 0; r <= 50; r++)
	{
		for (int c = tab[r]; c <= 168; c++)
		{
			move(r, c);
			printw(" ");
		}
	}
}
void initColor()
{
	init_pair(1, 1, 0); // Black
	init_pair(2, 1, 1); // Red
	init_pair(3, 1, 2); // Green
	init_pair(4, 1, 3); // Yellow
	init_pair(5, 1, 4); // Blue
	init_pair(6, 1, 5); // Magenta
	init_pair(7, 1, 6); // Cyan
	init_pair(8, 1, 7); // White
}
void initialized_enemy(int *tab)
{
	int number, liczba = 0;
	for (int r = 0; r <= 50; r++)
	{
		for (int c = tab[r]; c <= 168; c++)
		{
			number = rand() % 500;
			if (number % 17 == 2)
			{
				target_tab[liczba].x = c;
				target_tab[liczba].y = r;
				target_tab[liczba].health = 100;
				target_tab[liczba].name = "building";
				target_tab[liczba].visible = false;
				target_tab[liczba].narysowany = false;
				target_tab[liczba].zlokalizowany = false;
				liczba++;
			}
			else if (number % 23 == 3 && number % 13 == 2)
			{
				target_tab[liczba].x = c;
				target_tab[liczba].y = r;
				target_tab[liczba].health = 100;
				target_tab[liczba].name = "antiaicraftdefense";
				target_tab[liczba].visible = false;
				target_tab[liczba].narysowany = false;
				target_tab[liczba].zlokalizowany = false;
				liczba++;
			}
		}
		if (liczba > 98)
		{
			targets = liczba;
			return;
		}
	}
	targets = liczba;
}
void setNewEnemy()
{
	do {
		bool warunek = true;
		for (int i = 0; i < 100; i++)
		{
			if (target_tab[i].health > 0)
			{
				warunek = false;
			}
		}
		if (warunek)
		{
			initialized_enemy(tab);
		}
		usleep(1000000000);
	} while (true);
}
void drawEnemy()
{
	do
	{
		for (int i = 0; i < targets; i++)
		{
			if (target_tab[i].visible)
			{
				if (target_tab[i].name == "antiaicraftdefense")
				{
					errno = pthread_mutex_lock(&blokada);
					attrset(COLOR_PAIR(6));
					move(target_tab[i].x, target_tab[i].y);
					printw(" ");
					refresh();
					errno = pthread_mutex_unlock(&blokada);
				}
				else
				{
					errno = pthread_mutex_lock(&blokada);
					attrset(COLOR_PAIR(4));
					move(target_tab[i].y, target_tab[i].x);
					printw(" ");
					refresh();
					errno = pthread_mutex_unlock(&blokada);
				}
			}
			else
			{
				errno = pthread_mutex_lock(&blokada);
				attrset(COLOR_PAIR(3));
				move(target_tab[i].y, target_tab[i].x);
				printw(" ");
				refresh();
				errno = pthread_mutex_unlock(&blokada);
			}
		}
		usleep(1000000);
	} while (true);
}
static void* drawer(void* arg)
{
	drawEnemy();
	return 0;
}
static void* newEnemy(void* arg)
{
	setNewEnemy();
	return 0;
}
int main()
{
	pthread_t enemy;
	pthread_t newenemy;
	initscr();				//Inicjacja sceny
	start_color();			//Start kolor
	initColor();			//Inicjacja kolorów
	curs_set(0);			//Wyłączenie kursora

	setWybrzez(tab);		//Ustawienie lini wybrzeza
	drawScenery(tab);		//Wyrysowanie sceny
	initialized_enemy(tab);	//Zainicjowanie wrogó
	usleep(100000);

	Aircraft_Carrier *carrier;			//Stworzenie obiektu lotniskowiec
	carrier = new Aircraft_Carrier();
	carrier->setLand(tab);

	Reconnaissance_aircraft *recon;		//Stworzenie obiektu samolot rozpoznawczy(szpiegowski)
	recon = new Reconnaissance_aircraft();
	recon->setLand(tab);

	AirPort *port;						//Stworzenie obiektu lotnisko
	port = new AirPort();
	port->setLand(tab);

	/*Start wątków*/
	pthread_create(&enemy, 0, drawer, 0);
	pthread_create(&newenemy, 0, newEnemy, 0);
	carrier->start();
	recon->start();
	port->start();

	/*Zakończenie wątków*/
	port->join();
	recon->join();
	carrier->join();
	pthread_join(enemy, 0);
	pthread_join(newenemy, 0);

	/*KONIEC */
	getch();
	endwin();
	return 0;
}
