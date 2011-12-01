/**
 * "forfax", a sample Stratego AI for the UCC Programming Competition 2012
 * The main function for the "forfax" AI program
 * @author Sam Moore (matches) [SZM]
 * @website http://matches.ucc.asn.au/stratego
 * @email progcomp@ucc.asn.au or matches@ucc.asn.au
 * @git git.ucc.asn.au/progcomp2012.git
 */

#include <cstdlib>
#include <iostream>

#include "forfax.h"

using namespace std;

#include <stdio.h>

/**
 * The AI
 */
Forfax forfax;

/**
 * The main function
 * @param argc the number of arguments
 * @param argv the arguments
 * @returns exit code 0 for success, something else for error
 * Do I really need to tell you this?
 */
int main(int argc, char ** argv)
{
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);


	Forfax::Status move = forfax.Setup();

	
	while (move == Forfax::OK)
	{
		move = forfax.MakeMove();
	}

	switch (move)
	{
		case Forfax::OK:
			cerr << argv[0] << " Error - Should never see this!\n";
			break;
		case Forfax::NO_NEWLINE:
			cerr << argv[0] << " Error - Expected a new line!\n";
			break;
		case Forfax::EXPECTED_ATTACKER:
			cerr << argv[0] << " Error - Attacking piece does not exist on board!\n";
			break;
		case Forfax::UNEXPECTED_DEFENDER:
			cerr << argv[0] << " Error - Unexpected defending piece on board!\n";
			break;
		case Forfax::NO_ATTACKER:
			cerr << argv[0] << " Error - Couldn't find attacker in list of pieces!\n";
			break;
		case Forfax::NO_DEFENDER:
			cerr << argv[0] << " Error - Couldn't find defender in list of pieces!\n";
			break;

		case Forfax::COLOUR_MISMATCH:
			cerr << argv[0] << " Error - Colour of attacker and defender are the same!\n";
			break;
		case Forfax::INVALID_QUERY:
			cerr << argv[0] << " Error - Query did not make sense\n";
			break;
		case Forfax::VICTORY:
			cerr << argv[0] << " Game end - VICTORY!\n";
			break;		
		case Forfax::BOARD_ERROR:
			cerr << argv[0] << " Error - An error occurred with the board!\n";
			break;
	}

	cerr << "Final board state:\n";
	forfax.PrintBoard(cerr);

	cerr << "Forfax is now exiting!\n";
	
	
	exit(EXIT_SUCCESS);
	return 0;

	
}


