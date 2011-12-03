#include "controller.h"

#include <sstream>
#include "game.h"

using namespace std;

/**
 * Queries the player to setup their pieces
 *
 */

MovementResult Controller::Setup(const char * opponentName)
{
	string setup[4] = {"","","",""};
	MovementResult query = this->QuerySetup(opponentName, setup);
	if (query != MovementResult::OK)
		return query;

	

	int usedUnits[(int)(Piece::BOMB)];
	for (int ii = 0; ii <= (int)(Piece::BOMB); ++ii)
		usedUnits[ii] = 0;

	int yStart = 0;
	switch (colour)
	{
		case Piece::RED:
			yStart = 0;
			break;
		case Piece::BLUE:
			yStart = Game::theGame->theBoard.Height()-4;
			break;
		default:
			return MovementResult::COLOUR_ERROR; 
			break;
	}


	for (int y = 0; y < 4; ++y)
	{
		if ((int)setup[y].length() != Game::theGame->theBoard.Width())
			return MovementResult::BAD_RESPONSE;

		for (int x = 0; x < Game::theGame->theBoard.Width(); ++x)
		{
			Piece::Type type = Piece::GetType(setup[y][x]);
			if (type != Piece::NOTHING)
			{
				usedUnits[(int)(type)]++;
				if (usedUnits[type] > Piece::maxUnits[(int)type])
				{
					//fprintf(stderr, "Too many units of type %c\n", Piece::tokens[(int)(type)]);
					return MovementResult::BAD_RESPONSE;
				}
				Game::theGame->theBoard.AddPiece(x, yStart+y, type, colour);
			}
		}
	}
	if (usedUnits[(int)Piece::FLAG] <= 0)
	{
		return MovementResult::BAD_RESPONSE; //You need to include a flag!
	}

	return MovementResult::OK;

}


/**
 * Queries the player to respond to a state of Game::theGame->theBoard
 * @param buffer String which is used to store the player's responses
 * @returns The result of the response and/or move if made
 */
MovementResult Controller::MakeMove(string & buffer)
{
	buffer.clear();
	MovementResult query = this->QueryMove(buffer);
	if (query != MovementResult::OK)
		return query;

	
	int x; int y; string direction="";
	stringstream s(buffer);
	s >> x;
	s >> y;
	

	s >> direction;
	Board::Direction dir;
	if (direction == "UP")
	{
		dir = Board::UP;
	}
	else if (direction == "DOWN")
	{
		dir = Board::DOWN;
	}
	else if (direction == "LEFT")
	{
		dir = Board::LEFT;
	}
	else if (direction == "RIGHT")
	{
		dir = Board::RIGHT;
	}	
	else
	{
		//fprintf(stderr, "BAD_RESPONSE \"%s\"\n", buffer.c_str());
		return MovementResult::BAD_RESPONSE; //Player gave bogus direction - it will lose by default.	
	}

	int multiplier = 1;
	if (s.peek() != EOF)
		s >> multiplier;
	MovementResult moveResult = Game::theGame->theBoard.MovePiece(x, y, dir, multiplier, colour);

	s.clear(); 	s.str("");

	//I stored the ranks in the wrong order; rank 1 is the marshal, 2 is the general etc...
	//So I am reversing them in the output... great work
	s << Piece::tokens[(int)(moveResult.attackerRank)] << " " << Piece::tokens[(int)(moveResult.defenderRank)];	
	switch (moveResult.type)
	{
		case MovementResult::OK:
			buffer += " OK";
			break;
		case MovementResult::VICTORY:
			buffer += " FLAG";
			break;
		case MovementResult::KILLS:
			buffer += " KILLS ";
			buffer += s.str();

			break;
		case MovementResult::DIES:
			buffer += " DIES ";
			buffer += s.str();
			break;
		case MovementResult::BOTH_DIE:
			buffer += " BOTHDIE ";
			buffer += s.str();
			break;	
		default:
			buffer += " ILLEGAL";
			break;			
		
	}

	if (Game::theGame->allowIllegalMoves && !Board::LegalResult(moveResult))
		return MovementResult::OK; //HACK - Legal results returned!
	else
		return moveResult; 	

}
