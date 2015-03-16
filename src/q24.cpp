#include <iostream>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>
#include "q24.h"

using namespace std;

/**
 * A structure representing a die object with eight sides, useful
 * for generating random digits between 1 and 8, based off of the 
 * hash input.
 */
struct d8 {

	/**
	 * Constructor.
	 */
	d8(const uint8_t *input, int input_length) {

		this->input = input;
		this->input_length = input_length;
		pointer = 0;
		numRolls = 0;

		// Allocate space for 6 32-bit banks. Each bank is a mersenne
		// twister, and we will rotate through them to simulate an
		// entropy space of 192 bits total.
		twister = new uint32_t[NUM_BANKS];
		memset(twister, 0xFF, sizeof(*twister) * NUM_BANKS);
		currentBank = 0;

		// We need to start with some random entropic bitstring, so
		// run through every byte in the input and mess it up a lot.
		for (int i = 0; i < input_length; i++) {
			// Mangle up each twister bank seperately by XORing each
			// with the incoming byte and twisting.
			for (int j = 0; j < NUM_BANKS; j++) {

				twister[j] ^= input[i];

				// Crunch up 3 number of bytes by rotating to the RIGHT
				// 3 bits, and then performing a mersenne twist a couple
				// times.
				twister[j] = ((twister[j] & 0x7) << 30) | (twister[j] >> 3);
				for (int k = 0; k < 3 + (j * 3); k++) twist(j);
			}
		}
	}

	/**
	 * Free twister banks.
	 */
	~d8() {
		delete[] twister;
	}

	/**
	 * Returns a random digit between 1 and 8 inclusive.
	 * @return
	 */
	int roll() {

		numRolls++;

		// Twist 8 bits so we have a fresh byte at the LSB end.
		for (int i = 0; i < 8; i++) twist(currentBank);

		uint8_t bitscape = twister[currentBank];

		// XOR with the original input, just to keep things more entropic,
		// but make sure we account for the empty string.
		if (input_length > 0) {
			pointer = (pointer + 1) % input_length;
			bitscape ^= (uint8_t)input[pointer];
		}

		// The next time you roll, use the next twister bank.
		currentBank = (currentBank + 1) % NUM_BANKS;

		// Return the freshly twisted byte XORd with our pointer. We XOR with the
		// pointer just to make sure the entropy of the output hash does not depend
		// completely on the state of twister at the end of the constructor.
		int roll = 1 + (bitscape % 8);
		return roll;
	}

	/**
	 * Returns the number of rolls that the die has made.
	 * @return int
	 */
	int getNumRolls() {
		return numRolls;
	}

private:

	const uint8_t *input;
	int input_length;
	int pointer;
	int numRolls;
	uint32_t *twister;

	static const int NUM_BANKS = 6;
	int currentBank;

	/**
	 * Performs one iteration of the twister to scramble up the
	 * entropy.
	 */
	void twist(int i) {
		twister[i] ^= (twister[i] >> 11);
		twister[i] ^= (twister[i] <<  7) & 0x19871218U; // ff
		twister[i] ^= (twister[i] << 15) & 0x19870729U; // loz
		twister[i] = twister[i] ^ (twister[i] >> 18);
	}
};

/**
 * A structure containing stats for all game entities. Players and
 * monsters both have stats.
 */
struct stats_t {

	// Strength, agility, defense, and max hp.
	int str, agi, def;
	int maxhp;

	/**
	 * Default constructor.
	 */
	stats_t() {
		str = agi = def = 2;
		maxhp = 100;
	}

	/**
	 * Constructor.
	 */
	stats_t(int str, int agi, int def, int maxhp) {
		this->str = str;
		this->agi = agi;
		this->def = def;
		this->maxhp = maxhp;
	}

};

/**
 * An entity prototype, which represents both player and enemies,
 * is just a wrapper around a stat set, that internally maintains
 * its own hp.
 */
struct entity {

	/**
	 * Current value of the hp.
	 */
	int hp;

	/**
	 * Stats for this entity.
	 */
	stats_t stats;

	/**
	 * Constructor.
	 */
	entity(stats_t stats) {
		this->hp = stats.maxhp;
		this->stats = stats;
	}


	/**
	 * Heals the entity.
	 */
	void heal() {
		hp = stats.maxhp;
	}
};

/**
 * An enemy is just an entity that has a defined amount of
 * XP to reward after killing it.
 */
struct enemy_t : entity {

	// The amount of XP killing this enemy gives to the player.
	int xp;

	/**
	 * Constructor.
	 */
	enemy_t(stats_t stats, int xp) : entity(stats) {
		this->xp = xp;
	}

};

/**
 * The character is represented as an entity that can also 
 * level up.
 */
struct character : entity {

	// The character has the ability to gain experience and
	// level up.
	int xp;
	int nextlevel;
	int level;

	/**
	 * Constructor.
	 */
	character(stats_t stats) : entity(stats) {

		this->xp = 0;
		this->nextlevel = 100;
		this->level = 1;
	}

	/**
	 * Gains XP for the player.
	 * @param xp   amount
	 * @param die
	 */
	void gain(int xp, d8 & die, int verbose = 0) {

		this->xp += xp;

		while (this->xp >= nextlevel) {

			// Increment level and raise
			this->level++;
			nextlevel += 100;

			if (verbose > 1)
				cout << "Hero grew to level " << this->level << "! Nice!" << endl;

			// The hero gets some stat points to allocate into his stats. We run
			// into some issues if bad rolls of the dice put all the points into
			// dumb stats. If STR or AGI are neglected, there is no way the player
			// will ever finish the quests.
			int statPoints = 1 + level / 4;

			// Force the player to upgrade his/her lowest stat first. If they are all
			// equal, then that stat point will just continue on to the random allocation.
			statPoints--;
			if (stats.str < stats.agi && stats.str < stats.def) stats.str++;
			else if (stats.agi < stats.def) stats.agi++;
			else if (stats.def < stats.agi) stats.def++;
			else stats.agi++;

			// Allocate the remainder of the skill points randomly.
			while (statPoints-- > 0) {
				// Upgrade stats by rolling die and allocating skill points.
				int roll = die.roll();

				if (roll <= 2) stats.str++;
				else if (roll <= 4) stats.agi++;
				else if (roll <= 6) stats.def++;
				else if (roll <= 8) stats.maxhp += 21;
			}

			heal();
		}
	}

	/**
	 * Helpful for debugging, prints out player state.
	 */
	void print() {
		printf("[%d/%d] LVL%d str: %d, agi: %d, def: %d\n", hp, stats.maxhp, level, stats.str, stats.agi, stats.def);
	}
};

/**
 * Rotates the entire hash to the left one byte.
 */
void rotate(uint8_t *hash) {
	uint8_t first = hash[0];
	for (int i = 0; i < HASH_LENGTH - 1; i++) hash[i] = hash[i + 1];
	hash[HASH_LENGTH - 1] = first;
}

/**
 * Simulates a fight between the hero and the enemy. Returns 0 on
 * victory and -1 on death.
 * @param  player
 * @param  enemy   
 * @param  dice  
 * @return       
 */
int fight(character & hero, enemy_t & enemy, d8 & die, int verbose = 0) {

	if (hero.hp < 0) return -1;

	if (verbose > 1)
		cout << "Encounter! Enemy has " << enemy.hp << " HP." << endl;

	while (true) {

		char roll1 = die.roll();
		char roll2 = die.roll();
		char roll3 = die.roll();
		char roll4 = die.roll();

		int playerDamage = (hero.stats.str * 2 + 20) + (hero.stats.str * roll1 / 8);
		if (roll1 == 8) playerDamage *= 1.5;
		playerDamage -= enemy.stats.def * roll2 / 16;

		int enemyDamage = (enemy.stats.str * 2 + 20) + (enemy.stats.str * roll3 / 8);
		if (roll2 == 8) enemyDamage *= 1.5;
		enemyDamage -= hero.stats.def * roll4 / 32;

		if (playerDamage < 0) playerDamage = 0;
		if (enemyDamage < 0) enemyDamage = 0;

		if (verbose > 2) {
			cout << "Enemy dealt " << enemyDamage << " and hero dealt " << playerDamage << "." << endl;
			cout << "Enemy HP: " << enemy.hp << "\t\tHero HP: " << hero.hp << endl;
		}

		if (enemy.stats.agi > hero.stats.agi) {
			hero.hp -= enemyDamage;
			if (hero.hp < 1) break;
			enemy.hp -= playerDamage;
			if (enemy.hp < 1) break;
		}
		else {
			enemy.hp -= playerDamage;
			if (enemy.hp < 1) break;
			hero.hp -= enemyDamage;
			if (hero.hp < 1) break;
		}

	}

	if (hero.hp < 1) return -1;

	hero.gain(enemy.xp, die, verbose);
	return 1;
}

/**
 * This array contains all of the stats for the various enemies in the game.
 */
static int enemy_stats[][5] = 
{
	{ 10, 10, 12,   200, 50},
	{ 10, 10, 14,   300, 50},
	{ 16, 10, 16,   400, 50},
	{  8, 30, 18,   500, 50},

	{ 24, 15, 20,   600, 50},
	{ 24, 15, 22,   700, 50},
	{ 30, 15, 24,   800, 50},
	{ 40, 05, 26,   900, 50},

	{ 32, 20, 28,  1050, 50},
	{ 32, 20, 31,  1200, 50},
	{ 32, 20, 37,  1450, 50},
	{ 20, 40, 43,  1600, 50},

	{ 40, 25, 49,  1850, 50},
	{ 40, 25, 55,  2000, 50},
	{ 40, 25, 65,  2250, 50},
	{ 50, 10, 75,  2400, 50},

	{ 54, 40, 77,  2650, 50},
	{ 54, 40, 79,  2800, 50},
	{ 54, 40, 81,  3050, 50},
	{ 70, 10, 83,  3200, 50},

	{ 72, 60, 85,  4050, 50},
	{ 72, 60, 87,  4800, 50},
	{ 72, 60, 89,  6450, 50},
	{ 90, 10, 99, 12800, 50}
};

/**
 * Runs the numbered quest. If the player dies, he'll have to start
 * all over, but keeps XP gained and level.
 * @param  num    [description]
 * @param  player [description]
 * @param  dice   [description]
 * @return        [description]
 */
int quest(int num, character & hero, d8 & die, int verbose = 0) {

	hero.heal();

	// Load the quest data.
	int *a = enemy_stats[num - 1];

	// How many tiles the hero must travel, from 20 to 172.
	int numberTiles = (int)(20 + num * log(pow(num, 2)));

	for (int i = 0; i < numberTiles; i++) {

		int speed = hero.stats.agi / 8;
		i += speed;

		if (verbose > 2)
			cout << "Moving " << speed << " spaces..." << endl;

		int encounterRoll = die.roll();

		if (encounterRoll < 7) {

			stats_t gruntStats(a[0], a[1], a[2], a[3]);
			enemy_t grunt(gruntStats, a[4]);
			int result = fight(hero, grunt, die, verbose);

			if (result < 1) {

				hero.gain(grunt.xp / 10, die, verbose);

				if (verbose > 1)
					cout << "Hero died!" << endl;
				return -1;
			}
		}
		else if (encounterRoll == 8) {
			// Found a chest or secret dungeon! The hero can choose
			// between free 200 XP or a full heal potion. If the hero has
			// less than 25% HP, pick the potion.
			if (hero.hp < hero.stats.maxhp / 4) {
				hero.heal();

				if (verbose > 1)
					cout << "Found a chest! Inside was a potion!" << endl;
			}
			else {

				if (verbose > 1)
					cout << "Found a chest! Inside was 200 XP!" << endl;

				hero.gain(200, die, verbose);
			}
		}
	}

	return 1;
}

/**
 * Computes the 24 quests of the hash. Returns a heap-allocated array, so
 * remember to delete the result when done.
 * @param  input        input bytes
 * @param  input_length length of input
 * @return              byte array output
 */
uint8_t *compute(const uint8_t *input, int input_length, int verbose) {

	// Seed a RNG die with the input bytes. Every byte will
	// influence the rolls of the dice in some way.
	d8 die(input, input_length);

	// Allocate memory for the computed hash and initialize it with
	// dice rolls. 
	uint8_t *hash = new uint8_t[HASH_LENGTH];

	for (int i = 0; i < HASH_LENGTH; i++) 
		for (int j = 0; j < 8; j++)
			hash[i] = die.roll();

	// Initialize our player and roll for all his stats.
	character hero(stats_t(0, 0, 0, 255));
	hero.stats.str = 6 + die.roll() * 2;
	hero.stats.agi = 6 + die.roll() * 2;
	hero.stats.def = 6 + die.roll() * 2;

	if (verbose > 0) hero.print();

	// Put our daring adventurer through a harrowing set of 24 quests,
	// in which he will conquer the forces of evil and grow as a
	// character. At the end of each trial, influence that position in the
	// output hash with the player's leftover health, which should be
	// random, or at least fairly unpredictable.
	for (int i = 0; i < HASH_LENGTH; i++) {

		// Keep trying until you succeed!!
		while (quest(i + 1, hero, die, verbose) < 1) ;

		if (verbose > 0)
			printf("%02x............%02x....%04x..%02x\n", i + 1, hash[i], hero.hp, (uint8_t)(hash[i] ^ hero.hp));

		// Influence the output after the quest is over.
		hash[i] ^= hero.hp;
	}

	if (verbose > 0) {
		hero.print();
		cout << "Rolled the dice " << die.getNumRolls() << " times total." << endl;
	}

	return hash;
}
