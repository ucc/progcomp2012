#include "common.h"

#include "stratego.h"

using namespace std;

/**
 * Static variables
 */
Board Board::theBoard(10,10);
//nothing, boulder, flag, spy, scout, miner, sergeant, lietenant, captain, major, colonel, general, marshal, bomb, error
char  Piece::tokens[] = {'.','+','F','y','s','n','S','L','c','m','C','G','M','B','?'};
int Piece::maxUnits[] = {0,0,1,1,8,5,4,4,4,3,2,1,1,6,0};



#ifdef GRAPHICS
	Piece::TextureManager Piece::textures;
#endif //GRAPHICS


#ifdef GRAPHICS

Piece::TextureManager::~TextureManager()
{
	Array<Texture*>::Iterator i(*this);
	while (i.Good())
	{
		delete (*i);
		++i;
	}
}

Texture & Piece::TextureManager::operator[](const LUint & at)
{
	while (Array<Texture*>::Size() <= at)
	{
		char buffer[BUFSIZ];
		sprintf(buffer, "images/piece%lu.bmp", Array<Texture*>::Size());
		Array<Texture*>::Add(new Texture(buffer, false));
		
	}
	return *(Array<Texture*>::operator[](at));
}
#endif //GRAPHICS

/**
 * Gets the type of a piece, based off a character token
 * @param fromToken - character identifying the piece
 * @returns The type of the piece
 */
Piece::Type Piece::GetType(char fromToken)
{
	for (int ii=0; ii <= (int)(Piece::BOMB); ++ii)
	{
		if (tokens[ii] == fromToken)
		{
			return Type(Piece::NOTHING + ii);
		}
	}
	return Piece::BOULDER;
}

/**
 * Construct a new, empty board
 * @param newWidth - the width of the board
 * @param newHeight - the height of the board
 */
Board::Board(int newWidth, int newHeight) : winner(Piece::NONE), width(newWidth), height(newHeight), board(NULL)
{
	board = new Piece**[width];
	for (int x=0; x < width; ++x)
	{
		board[x] = new Piece*[height];
		for (int y=0; y < height; ++y)
			board[x][y] = NULL;
	}
}

/**
 * Cleanup a board
 */
Board::~Board()
{
	for (int x=0; x < width; ++x)
	{
		for (int y=0; y < height; ++y)
			delete board[x][y];
		delete [] board[x];
	}
}

/**
 * Print textual representation of the board to a stream
 * @param stream - the stream to print information to
 * @param reveal - Pieces matching this colour will have their identify revealed, other pieces will be shown as '#' or '*' for RED or BLUE respectively.
 */
void Board::Print(FILE * stream, const Piece::Colour & reveal)
{
	for (int y=0; y < height; ++y)
	{
		for (int x=0; x < width; ++x)
		{
			Piece * piece = board[x][y];
			if (piece == NULL)
			{
				fprintf(stream, ".");
			}
			else if (piece->colour != Piece::NONE && (piece->colour == reveal || reveal == Piece::BOTH))
			{
				fprintf(stream, "%c", Piece::tokens[piece->type]);
			}
			else
			{
				switch (piece->colour)
				{
					case Piece::RED:
						fprintf(stream, "#");
						break;
					case Piece::BLUE:
						fprintf(stream, "*");
						break;
					case Piece::NONE:
						fprintf(stream, "+"); 
						break;
					case Piece::BOTH:
						fprintf(stream, "$"); //Should never see these!
						break;
				}
			}
		}
		fprintf(stream, "\n");
	}
	
}


#ifdef GRAPHICS
/**
 * Draw the board state to graphics
 * @param reveal - Pieces matching this colour will be revealed. All others will be shown as blank coloured squares.
 */
void Board::Draw(const Piece::Colour & reveal)
{
	if (!Graphics::Initialised())
	{
		Graphics::Initialise("Stratego", width*32, height*32);
	}

	Graphics::ClearScreen();
	
	for (int y=0; y < height; ++y)
	{
		for (int x=0; x < width; ++x)
		{
			Piece * piece = board[x][y];
			if (piece == NULL)
			{
				//Don't display anything

			}
			else if (piece->colour != Piece::NONE && (piece->colour == reveal || reveal == Piece::BOTH))
			{
				//Display the piece
				Piece::textures[(int)(piece->type)].DrawColour(x*32,y*32,0,1, Piece::GetGraphicsColour(piece->colour));
				
			}
			else
			{
				switch (piece->colour)
				{
					case Piece::RED:
						Piece::textures[(int)(Piece::BOULDER)].DrawColour(x*32,y*32,0,1, Piece::GetGraphicsColour(piece->colour));
						break;
					case Piece::BLUE:
						Piece::textures[(int)(Piece::BOULDER)].DrawColour(x*32,y*32,0,1, Piece::GetGraphicsColour(piece->colour));
						break;
					case Piece::NONE:
						Piece::textures[(int)(Piece::BOULDER)].DrawColour(x*32,y*32,0,1, Piece::GetGraphicsColour(piece->colour));
						break;
					case Piece::BOTH:
						Piece::textures[(int)(Piece::BOULDER)].DrawColour(x*32,y*32,0,1, Piece::GetGraphicsColour(piece->colour));
						break;
				}
			}
		}
		
	}
	Graphics::UpdateScreen();
	
}
#endif //GRAPHICS

/**
 * Adds a piece to the board
 * @param x - x-coord to place the piece at, starting at zero, must be less than board width
 * @param y - y-coord to place the piece at, starting at zero, must be less than board height
 * @param newType - the Type of the piece
 * @param newColour - the Colour of the piece
 * @returns true if and only if the piece could be successfully added.
 */
bool Board::AddPiece(int x, int y, const Piece::Type & newType, const Piece::Colour & newColour)
{
	if (board == NULL || x < 0 || y < 0 || x >= width || y >= width || board[x][y] != NULL)
		return false;

	Piece * piece = new Piece(newType, newColour);
	board[x][y] = piece;
	return true;
}

/**
 * Gets a pointer to a piece at a board location
 * UNUSED
 * @param x - x-coord of the piece
 * @param y - y-coord of the piece
 * @returns pointer to the piece, or NULL if the board location was empty
 * @throws error if board is null or coords are invalid
 */
Piece * Board::GetPiece(int x, int y)
{
	assert(board != NULL);
	assert(x >= 0 && x < width && y >= 0 && y < height);
	return board[x][y];
}

/**
 * Moves a piece at a specified position in the specified direction, handles combat if necessary
 * @param x - x-coord of the piece
 * @param y - y-coord of the piece
 * @param direction - Direction in which to move (UP, DOWN, LEFT or RIGHT)
 * @param colour - Colour which the piece must match for the move to be valid
 * @returns A MovementResult which indicates the result of the move - OK is good, VICTORY means that a flag was captured, anything else is an error
 */
MovementResult Board::MovePiece(int x, int y, const Direction & direction, int multiplier,const Piece::Colour & colour)
{
	if (board == NULL) 
	{
		return MovementResult(MovementResult::NO_BOARD);
	}
	if (!(x >= 0 && x < width && y >= 0 && y < height)) 
	{
		return MovementResult(MovementResult::INVALID_POSITION);
	}
	Piece * target = board[x][y];
	if (target == NULL) 
	{
		return MovementResult(MovementResult::NO_SELECTION);
	}
	if (!(colour == Piece::NONE || target->colour == colour)) 
	{
		return MovementResult(MovementResult::NOT_YOUR_UNIT);
	}
	if (target->type == Piece::FLAG || target->type == Piece::BOMB || target->type == Piece::BOULDER) 
	{
		return MovementResult(MovementResult::IMMOBILE_UNIT);
	}
	if (multiplier > 1 && target->type != Piece::SCOUT)
	{
		return MovementResult(MovementResult::INVALID_DIRECTION); //Can only move a scout multiple times.
	}
	int x2 = x; int y2 = y;

	for (int ii=0; ii < multiplier; ++ii)
	{
		switch (direction)
		{
			case UP:
				--y2;
				break;
			case DOWN:
				++y2;
				break;
			case LEFT:
				--x2;
				break;
			case RIGHT:
				++x2;
				break;
		}
		if (!(x2 >= 0 && x2 < width && y2 >= 0 && y2 < height)) 
		{
			return MovementResult(MovementResult::INVALID_DIRECTION);
		}
		if (ii < multiplier-1 && board[x2][y2] != NULL)
		{
			return MovementResult(MovementResult::POSITION_FULL);
		}
	}
	Piece * defender = board[x2][y2];
	if (defender == NULL)
	{
		board[x][y] = NULL;
		board[x2][y2] = target;
	}
	else if (defender->colour != target->colour)
	{
		Piece::Type defenderType = defender->type;
		Piece::Type attackerType = target->type;

		if (defender->colour == Piece::NONE) 
		{
			return MovementResult(MovementResult::POSITION_FULL);
		}
		if (defender->type == Piece::FLAG)
		{
			winner = target->colour;
			return MovementResult(MovementResult::VICTORY);
		}
		else if (defender->type == Piece::BOMB)
		{
			if (target->type == Piece::MINER)
			{

				delete defender;
				board[x][y] = NULL;
				board[x2][y2] = target;
				return MovementResult(MovementResult::KILLS, attackerType, defenderType);
			}
			else
			{
				delete defender;
				delete target;
				board[x][y] = NULL;
				board[x2][y2] = NULL;
				return MovementResult(MovementResult::BOTH_DIE, attackerType, defenderType);
			}
		}
		else if (defender->type == Piece::MARSHAL && target->type == Piece::SPY)
		{
			delete defender;
			board[x][y] = NULL;
			board[x2][y2] = target;
			return MovementResult(MovementResult::KILLS, attackerType, defenderType);
		}
		else if (target->operator > (*defender))
		{
			delete defender;
			board[x][y] = NULL;
			board[x2][y2] = target;
			return MovementResult(MovementResult::KILLS, attackerType, defenderType);
		}
		else if (target->operator==(*defender) && rand() % 2 == 0)
		{
			delete defender;
			board[x][y] = NULL;
			board[x2][y2] = target;	
			return MovementResult(MovementResult::KILLS, attackerType, defenderType);
		}
		else
		{
			delete target;
			board[x][y] = NULL;
			return MovementResult(MovementResult::DIES, attackerType, defenderType);
		}
	}
	else
	{
		return MovementResult(MovementResult::POSITION_FULL);
	}
	return MovementResult(MovementResult::OK);
}	



