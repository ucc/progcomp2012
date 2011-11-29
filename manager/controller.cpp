#include <sstream>

#include "stratego.h"

#include "controller.h"

using namespace std;

/**
 * Queries the AI program to setup its pieces
 * @returns the result of the response
 */
Board::MovementResult Controller::Setup(const char * opponentName)
{
	int y;
	switch (colour)
	{
		case Piece::RED:
			assert(SendMessage("RED %s %d %d", opponentName, Board::theBoard.Width(), Board::theBoard.Height()));
			y = 0;
			
			break;
		case Piece::BLUE:
			assert(SendMessage("BLUE %s %d %d", opponentName, Board::theBoard.Width(), Board::theBoard.Height()));
			y = Board::theBoard.Height()-4;
			
			break;
		case Piece::NONE:
		case Piece::BOTH:
			//Should never see this;
			assert(false);
			break;
	}


	int usedUnits[(int)(Piece::BOMB)];
	for (int ii = 0; ii <= (int)(Piece::BOMB); ++ii)
		usedUnits[ii] = 0;

	//The setup is spread across 4 lines of the board - blue at the top, red at the bottom. AI has 2.5s for each line.
	



	for (int ii=0; ii < 4; ++ii)
	{
		string line="";
		if (!GetMessage(line, 2.5))
		{
			fprintf(stderr, "Timeout on setup\n");
			return Board::BAD_RESPONSE;
		}
		if ((int)(line.size()) != Board::theBoard.Width())
		{
			fprintf(stderr, "Bad length of \"%s\" on setup\n", line.c_str());
			return Board::BAD_RESPONSE;
		}
	
		for (int x = 0; x < (int)(line.size()); ++x)
		{
			Piece::Type type = Piece::GetType(line[x]);
			if (type != Piece::NOTHING)
			{
//fprintf(stderr, "x y %d %d\n", x, y+ii);
//					fprintf(stderr, "Found unit of type '%c' (%d '%c') %d vs %d\n", line[x], (int)(type), Piece::tokens[(int)(type)], usedUnits[(int)(type)], Piece::maxUnits[(int)type]);
		///			fprintf(stderr, "Marshal is %d '%c', flag is %d '%c'\n", (int)Piece::MARSHAL, Piece::tokens[(int)(Piece::MARSHAL)], (int)Piece::FLAG, Piece::tokens[(int)(Piece::FLAG)]);

				usedUnits[(int)(type)] += 1;
				if (usedUnits[type] > Piece::maxUnits[(int)type])
				{
					fprintf(stderr, "Too many units of type %c\n", Piece::tokens[(int)(type)]);
					return Board::BAD_RESPONSE;
				}
	
				Board::theBoard.AddPiece(x, y+ii, type, colour);
			}
		}	
	}

	if (usedUnits[(int)Piece::FLAG] <= 0)
	{
		return Board::BAD_RESPONSE; //You need to include a flag!
	}

	return Board::OK;
}


/**
 * Queries the AI program to respond to a state of Board::theBoard
 * @returns The result of the response and/or move if made
 */
Board::MovementResult Controller::MakeMove(string & buffer)
{
	
	if (!Running())
		return Board::NO_MOVE; //AI has quit
	Board::theBoard.Print(output, colour);

	

	
	buffer.clear();
	if (!GetMessage(buffer,2))
	{
		return Board::NO_MOVE; //AI did not respond. It will lose by default.
	}

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
		fprintf(stderr, "BAD_RESPONSE \"%s\"\n", buffer.c_str());
		return Board::BAD_RESPONSE; //AI gave bogus direction - it will lose by default.	
	}

	int multiplier = 1;
	if (s.peek() != EOF)
		s >> multiplier;
	Board::MovementResult moveResult = Board::theBoard.MovePiece(x, y, dir, multiplier, colour);
	switch (moveResult)
	{
		case Board::OK:
			buffer += " OK";
			break;
		case Board::VICTORY:
			buffer += " FLAG";
			break;
		case Board::KILLS:
			buffer += " KILLS";
			break;
		case Board::DIES:
			buffer += " DIES";
			break;
		case Board::BOTH_DIE:
			buffer += " BOTHDIE";
			break;	
		default:
			buffer += " ILLEGAL";
			break;			
		
	}

	if (!Board::LegalResult(moveResult))
		return Board::OK; //HACK - Legal results returned!
	else
		return moveResult; 	

}

