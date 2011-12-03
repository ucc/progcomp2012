#include "human_controller.h"

#include "game.h"

#include <iostream> //Really I can't be bothered with fscanf any more

using namespace std;

MovementResult Human_Controller::QuerySetup(const char * opponentName, string setup[])
{
	
	static bool shownMessage = false;
	if (!shownMessage)
	{
		if (graphicsEnabled)
			fprintf(stderr, "GUI not yet supported! Please use CLI\n");
		fprintf(stdout,"Enter %d x %d Setup grid\n", Game::theGame->theBoard.Width(), 4);
		fprintf(stdout,"Please enter one line at a time, using the following allowed characters:\n");
		for (Piece::Type rank = Piece::FLAG; rank <= Piece::BOMB; rank = Piece::Type((int)(rank) + 1))
		{
			fprintf(stdout,"%c x %d\n", Piece::tokens[(int)rank], Piece::maxUnits[(int)rank]);
		}
		fprintf(stdout, "You must place at least the Flag (%c). Use '%c' for empty squares.\n", Piece::tokens[(int)Piece::FLAG], Piece::tokens[(int)Piece::NOTHING]);
		fprintf(stdout, "NOTE: Player RED occupies the top four rows, and BLUE occupies the bottom four rows.\n");
		
		shownMessage = true;
	}
		
	

	for (int y = 0; y < 4; ++y)
		cin >> setup[y];
	assert(cin.get() == '\n');
	
	return MovementResult::OK;
}

MovementResult Human_Controller::QueryMove(string & buffer)
{
	static bool shownMessage = false;
	if (!shownMessage)
	{
		if (graphicsEnabled)
			fprintf(stderr, "GUI not yet supported! Please use the CLI\n");
		fprintf(stdout, "Please enter your move in the format:\n X Y DIRECTION [MULTIPLIER=1]\n");
		fprintf(stdout, "Where X and Y indicate the coordinates of the piece to move;\n DIRECTION is one of UP, DOWN, LEFT or RIGHT\n and MULTIPLIER is optional (and only valid for scouts (%c))\n", Piece::tokens[(int)(Piece::SCOUT)]);
		shownMessage = true;
	}

	


	buffer.clear();
	for (char in = fgetc(stdin); in != '\n'; in = fgetc(stdin))
	{
		buffer += in;
	}
	
	

	return MovementResult::OK;
	
}
