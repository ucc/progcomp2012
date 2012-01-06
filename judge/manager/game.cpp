#include "game.h"
#include <stdarg.h>
using namespace std;



Game* Game::theGame = NULL;
bool Game::gameCreated = false;

Game::Game(const char * redPath, const char * bluePath, const bool enableGraphics, double newStallTime, const bool allowIllegal, FILE * newLog, const  Piece::Colour & newReveal, int newMaxTurns, bool newPrintBoard, double newTimeoutTime) : red(NULL), blue(NULL), turn(Piece::RED), theBoard(10,10), graphicsEnabled(enableGraphics), stallTime(newStallTime), allowIllegalMoves(allowIllegal), log(newLog), reveal(newReveal), turnCount(0), input(NULL), maxTurns(newMaxTurns), printBoard(newPrintBoard), timeoutTime(newTimeoutTime)
{
	gameCreated = false;
	if (gameCreated)
	{
		fprintf(stderr, "Game::Game - Error - Tried to create more than one Game!\n");
		exit(EXIT_FAILURE);
	}
	gameCreated = true;
	Game::theGame = this;
	signal(SIGPIPE, Game::HandleBrokenPipe);


	#ifdef BUILD_GRAPHICS
	if (graphicsEnabled && (!Graphics::Initialised()))
			Graphics::Initialise("Stratego", theBoard.Width()*32, theBoard.Height()*32);
	#endif //BUILD_GRAPHICS

	if (strcmp(redPath, "human") == 0)
		red = new Human_Controller(Piece::RED, graphicsEnabled);
	else
		red = new AI_Controller(Piece::RED, redPath, timeoutTime);
	
	
	if (strcmp(bluePath, "human") == 0)
		blue = new Human_Controller(Piece::BLUE, graphicsEnabled);
	else
		blue = new AI_Controller(Piece::BLUE, bluePath, timeoutTime);


}

Game::Game(const char * fromFile, const bool enableGraphics, double newStallTime, const bool allowIllegal, FILE * newLog, const  Piece::Colour & newReveal, int newMaxTurns, bool newPrintBoard, double newTimeoutTime) : red(NULL), blue(NULL), turn(Piece::RED), theBoard(10,10), graphicsEnabled(enableGraphics), stallTime(newStallTime), allowIllegalMoves(allowIllegal), log(newLog), reveal(newReveal), turnCount(0), input(NULL), maxTurns(newMaxTurns), printBoard(newPrintBoard), timeoutTime(newTimeoutTime)
{
	gameCreated = false;
	if (gameCreated)
	{
		fprintf(stderr, "Game::Game - Error - Tried to create more than one Game!\n");
		exit(EXIT_FAILURE);
	}
	gameCreated = true;
	Game::theGame = this;
	signal(SIGPIPE, Game::HandleBrokenPipe);

	#ifdef BUILD_GRAPHICS
	if (graphicsEnabled && (!Graphics::Initialised()))
			Graphics::Initialise("Stratego", theBoard.Width()*32, theBoard.Height()*32);
	#endif //BUILD_GRAPHICS

	input = fopen(fromFile, "r");

	red = new FileController(Piece::RED, input);
	blue = new FileController(Piece::BLUE, input);


}

Game::~Game()
{
	
	delete red;
	delete blue;

	if (log != NULL && log != stdout && log != stderr)
		fclose(log);

	if (input != NULL && input != stdin)
		fclose(input);
}

/**
 * Attempts to setup the board and controllers
 * @param redName the name of the red AI
 * @param blueName the name of the blue AI
 * @returns A colour, indicating if there were any errors
	Piece::NONE indicates no errors
	Piece::BOTH indicates errors with both AI
	Piece::RED / Piece::BLUE indicates an error with only one of the two AI
 */
Piece::Colour Game::Setup(const char * redName, const char * blueName)
{

	if (!red->Valid())
	{
		logMessage("Controller for Player RED is invalid!\n");
		if (!red->HumanController())
			logMessage("Check that executable \"%s\" exists and has executable permissions set.\n", redName);
	}
	if (!blue->Valid())
	{
		logMessage("Controller for Player BLUE is invalid!\n");
		if (!blue->HumanController())
			logMessage("Check that executable \"%s\" exists and has executable permissions set.\n", blueName);
	}
	if (!red->Valid())
	{
		if (!blue->Valid())
			return Piece::BOTH;
		return Piece::RED;
	}
	else if (!blue->Valid())
	{
		return Piece::BLUE;
	}

	for (int y = 4; y < 6; ++y)
	{
		for (int x = 2; x < 4; ++x)
		{
			theBoard.AddPiece(x,y,Piece::BOULDER, Piece::NONE);
		}
		for (int x = 6; x < 8; ++x)
		{
			theBoard.AddPiece(x,y,Piece::BOULDER, Piece::NONE);
		}
	}


	MovementResult redSetup = red->Setup(blueName);
	MovementResult blueSetup = blue->Setup(redName);


	Piece::Colour result = Piece::NONE;
	if (redSetup != MovementResult::OK)
	{	
		if (blueSetup != MovementResult::OK)
		{
			logMessage("BOTH players give invalid setup!\n");
			result = Piece::BOTH;
		}
		else
		{
			//logMessage("Player RED gave an invalid setup!\n");
			result = Piece::RED;
		}
		
	}
	else if (blueSetup != MovementResult::OK)
	{
		//logMessage("Player BLUE gave an invalid setup!\n");
		result = Piece::BLUE;
	}


	logMessage("%s RED SETUP\n", red->name.c_str());
	if (redSetup == MovementResult::OK)
	{
		for (int y=0; y < 4; ++y)
		{
			for (int x=0; x < theBoard.Width(); ++x)
			{
				if (theBoard.GetPiece(x, y) != NULL)
					logMessage("%c", Piece::tokens[(int)(theBoard.GetPiece(x, y)->type)]);
				else
					logMessage(".");
			}
			logMessage("\n");
		}	
	}
	else
	{
		logMessage("INVALID!\n");
	}

	logMessage("%s BLUE SETUP\n", blue->name.c_str());
	if (blueSetup == MovementResult::OK)
	{
		for (int y=0; y < 4; ++y)
		{
			for (int x=0; x < theBoard.Width(); ++x)
			{
				if (theBoard.GetPiece(x, theBoard.Height()-4+y) != NULL)
					logMessage("%c", Piece::tokens[(int)(theBoard.GetPiece(x, theBoard.Height()-4+y)->type)]);
				else
					logMessage(".");
			}
			logMessage("\n");
		}	
	}
	else
	{
		logMessage("INVALID!\n");
	}

	
	return result;

}

void Game::Wait(double wait)
{
	if (wait <= 0)
		return;

	TimerThread timer(wait*1000000); //Wait in seconds
	timer.Start();

	#ifdef BUILD_GRAPHICS
	if (!graphicsEnabled)
	{
		while (!timer.Finished());
		timer.Stop();
		return;
	}
	#endif //BUILD_GRAPHICS

	while (!timer.Finished())
	{
		#ifdef BUILD_GRAPHICS
		SDL_Event  event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					timer.Stop();
					exit(EXIT_SUCCESS);
					break;
			}
		}
		#endif //BUILD_GRAPHICS
	}
	timer.Stop();
	
}

void Game::HandleBrokenPipe(int sig)
{
	if (theGame == NULL)
	{
		fprintf(stderr, "ERROR - Recieved SIGPIPE during game exit!\n");
		exit(EXIT_FAILURE);
	}
	if (theGame->turn == Piece::RED)
	{
		theGame->logMessage("Game ends on RED's turn - REASON: ");
		if (theGame->blue->Valid()) //Should probably check this
			theGame->blue->Message("DEFAULT");	
	}
	else if (theGame->turn == Piece::BLUE)
	{
	
		theGame->logMessage("Game ends on BLUE's turn - REASON: ");
		if (theGame->red->Valid()) //Should probably check this
			theGame->red->Message("DEFAULT");
	}
	else
	{
		theGame->logMessage("Game ends on ERROR's turn - REASON: ");
			
	}
	
	theGame->logMessage("SIGPIPE - Broken pipe (AI program no longer running)\n");

	if (Game::theGame->printBoard)
		Game::theGame->theBoard.PrintPretty(stdout, Piece::BOTH);

	
	#ifdef BUILD_GRAPHICS
	if (Game::theGame->graphicsEnabled && theGame->log == stdout)
	{
		theGame->logMessage("CLOSE WINDOW TO EXIT\n");
		Game::theGame->theBoard.Draw(Piece::BOTH);
		while (true)
		{
			SDL_Event  event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_QUIT:
						exit(EXIT_SUCCESS);
						break;
				}
			}			
		}
	}
	else
	#endif //BUILD_GRAPHICS
	{
		if (theGame->log == stdout || theGame->log == stderr)
		{
			theGame->logMessage( "PRESS ENTER TO EXIT\n");
			theGame->theBoard.Print(theGame->log);
			while (fgetc(stdin) != '\n');
		}
	}
	

	exit(EXIT_SUCCESS);
}

void Game::PrintEndMessage(const MovementResult & result)
{
	if (turnCount == 0)
	{
		logMessage("Game ends in the SETUP phase - REASON: ");
	}
	else
	{
		if (turn == Piece::RED)
		{
			logMessage("Game ends on RED's turn - REASON: ");	
		}
		else if (turn == Piece::BLUE)
		{
			logMessage("Game ends on BLUE's turn - REASON: ");
		}
		else
		{
			logMessage("Game ends on ERROR's turn - REASON: ");
			
		}
	}
	switch (result.type)
	{
		case MovementResult::OK:
			logMessage("Status returned OK, unsure why game halted...\n");
			break;
		case MovementResult::DIES:
			logMessage("Status returned DIES, unsure why game halted...\n");
			break;
		case MovementResult::KILLS:
			logMessage("Status returned KILLS, unsure why game halted...\n");
			break;
		case MovementResult::BOTH_DIE:
			logMessage("Status returned BOTH_DIE, unsure why game halted...\n");
			break;
		case MovementResult::NO_BOARD:
			logMessage("Board does not exit?!\n");
			break;
		case MovementResult::INVALID_POSITION:
			logMessage("Coords outside board\n");
			break;
		case MovementResult::NO_SELECTION:
			logMessage("Move does not select a piece\n");
			break;
		case MovementResult::NOT_YOUR_UNIT:
			logMessage("Selected piece belongs to other player\n");
			break;
		case MovementResult::IMMOBILE_UNIT:
			logMessage("Selected piece is not mobile (FLAG or BOMB)\n");
			break;
		case MovementResult::INVALID_DIRECTION:
			logMessage("Selected unit cannot move that way\n");
			break;
		case MovementResult::POSITION_FULL:
			logMessage("Attempted move into square occupied by neutral or allied piece\n");
			break;
		case MovementResult::VICTORY_FLAG:
			logMessage("Captured the flag\n");
			break;
		case MovementResult::VICTORY_ATTRITION:
			logMessage("Destroyed all mobile enemy pieces\n");
			break;
		case MovementResult::BAD_RESPONSE:
			logMessage("Unintelligable response\n");
			break;
		case MovementResult::NO_MOVE:
			logMessage("Response timeout after %2f seconds.\n", timeoutTime);
			break;
		case MovementResult::COLOUR_ERROR:
			logMessage("Internal controller error - COLOUR_ERROR\n");
			break;
		case MovementResult::ERROR:
			logMessage("Internal controller error - Unspecified ERROR\n");
			break;
		case MovementResult::DRAW_DEFAULT:
			logMessage("Game declared a draw after %d turns\n", turnCount);
			break;
		case MovementResult::DRAW:
			logMessage("Game declared a draw because neither player has mobile pieces\n");
			break;
		case MovementResult::SURRENDER:
			logMessage("This player has surrendered!\n");
			break;
		case MovementResult::BAD_SETUP:
			switch (turn)
			{
				case Piece::RED:
					logMessage("An illegal setup was made by RED\n");
					break;
				case Piece::BLUE:
					logMessage("An illegal setup was made by BLUE\n");
					break;
				case Piece::BOTH:
					logMessage("An illegal setup was made by BOTH players\n");
					break;
				case Piece::NONE:
					logMessage("Unknown internal error.\n");
					break;
			}
			break;

	}

	if (printBoard)
	{
		system("clear");
		fprintf(stdout, "%d Final State\n", turnCount);
		theBoard.PrintPretty(stdout, Piece::BOTH);
		fprintf(stdout, "\n");
	}

	#ifdef BUILD_GRAPHICS
	if (graphicsEnabled && log == stdout)
	{
		logMessage("CLOSE WINDOW TO EXIT\n");
		theBoard.Draw(Piece::BOTH);
		while (true)
		{
			SDL_Event  event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_QUIT:
						exit(EXIT_SUCCESS);
						break;
				}
			}			
		}
	}
	else
	#endif //BUILD_GRAPHICS
	{
		if (log == stdout)
		{
			logMessage("PRESS ENTER TO EXIT\n");
			while (fgetc(stdin) != '\n');
			exit(EXIT_SUCCESS); //Might want to actually exit, you foolish fool
		}
	}

}
/** Checks for victory by attrition (destroying all mobile pieces)
 *
 *  @returns OK for no victory, 
 *	DRAW if both players have no pieces, or 
 *	VICTORY_ATTRITION  if the current player has won by attrition
 */
MovementResult Game::CheckVictoryAttrition()
{
        if (theBoard.MobilePieces(Piece::OppositeColour(turn)) == 0)
	{
		if (theBoard.MobilePieces(turn) == 0)
	                return MovementResult::DRAW;
	        else
		        return MovementResult::VICTORY_ATTRITION;
	}
	return MovementResult::OK;

}
MovementResult Game::Play()
{

	MovementResult result = MovementResult::OK;
	turnCount = 1;
	string buffer;

	Piece::Colour toReveal = reveal;
	
	
	


	red->Message("START");
	


	while (!Board::HaltResult(result) && (turnCount < maxTurns || maxTurns < 0))
	{
		if (red->HumanController() && blue->HumanController())
			toReveal = Piece::RED;
		if (printBoard)
		{
			system("clear");
			if (turnCount == 0)
				fprintf(stdout, "START:\n");
			else
				fprintf(stdout, "%d BLUE:\n", turnCount);
			theBoard.PrintPretty(stdout, toReveal);
			fprintf(stdout, "\n\n");
		}

		#ifdef BUILD_GRAPHICS
		if (graphicsEnabled)
			theBoard.Draw(toReveal);
		#endif //BUILD_GRAPHICS
		
		turn = Piece::RED;

		if (!Board::HaltResult(result))
		{
			result = CheckVictoryAttrition();
		}
		if (Board::HaltResult(result))
			break;

		logMessage( "%d RED: ", turnCount);
		result = red->MakeMove(buffer);
		red->Message(buffer);
		blue->Message(buffer);
		logMessage( "%s\n", buffer.c_str());

		if (!Board::HaltResult(result))
		{
			result = CheckVictoryAttrition();
		}
		if (Board::HaltResult(result))
			break;

		if (stallTime >= 0)
			Wait(stallTime);
		else
			ReadUserCommand();

		if (blue->HumanController() && red->HumanController())
			toReveal = Piece::BLUE;
		if (printBoard)
		{
			system("clear");
			fprintf(stdout, "%d RED:\n", turnCount);
			theBoard.PrintPretty(stdout, toReveal);
			fprintf(stdout, "\n\n");
		}
		
		#ifdef BUILD_GRAPHICS
		if (graphicsEnabled)
			theBoard.Draw(toReveal);
		#endif //BUILD_GRAPHICS

		
		
		turn = Piece::BLUE;

		if (!Board::HaltResult(result))
		{
			result = CheckVictoryAttrition();
		}
		if (Board::HaltResult(result))
			break;

		logMessage( "%d BLU: ", turnCount);
		result = blue->MakeMove(buffer);
		blue->Message(buffer);
		red->Message(buffer);
		logMessage( "%s\n", buffer.c_str());

		if (!Board::HaltResult(result))
		{
			result = CheckVictoryAttrition();
		}
		if (Board::HaltResult(result))
			break;

		if (theBoard.MobilePieces(Piece::RED) == 0)
			result = MovementResult::DRAW;

		if (theBoard.MobilePieces(Piece::RED) == 0)
		{
			if (theBoard.MobilePieces(Piece::BLUE) == 0)
				result = MovementResult::DRAW;
			else
				result = MovementResult::VICTORY_ATTRITION;
			break;			
		}

		if (stallTime >= 0)
			Wait(stallTime);
		else
			ReadUserCommand();
	
		

		++turnCount;
	}

	if ((maxTurns >= 0 && turnCount >= maxTurns) && result == MovementResult::OK)
	{
		result = MovementResult::DRAW_DEFAULT;
	}

	
	return result;

		

}

/**
 * Logs a message to the game's log file if it exists
 * @param format the format string
 * @param additional parameters - printed using va_args
 * @returns the result of vfprintf or a negative number if the log file does not exist
 */
int Game::logMessage(const char * format, ...)
{
	if (log == NULL)
		return -666;
		va_list ap;
	va_start(ap, format);

	int result = vfprintf(log, format, ap);
	va_end(ap);

	return result;
}

/**
 * Waits for a user command
 * Currently ignores the command.
 */
void Game::ReadUserCommand()
{
	fprintf(stdout, "Waiting for user to press enter... (type QUIT to exit)\n");
	string command("");
	for (char c = fgetc(stdin); c != '\n' && (int)(c) != EOF; c = fgetc(stdin))
	{
		command += c;
	}

	if (command == "QUIT")
	{
		fprintf(stdout, "Ordered to quit... exiting...\n");
		exit(EXIT_SUCCESS);
	}
}

MovementResult FileController::QuerySetup(const char * opponentName, std::string setup[])
{

	char c = fgetc(file);
	name = "";
	while (c != ' ')
	{
		name += c;
		c = fgetc(file);
	}

	while (fgetc(file) != '\n');

	for (int y = 0; y < 4; ++y)
	{
		setup[y] = "";
		for (int x = 0; x < Game::theGame->theBoard.Width(); ++x)
		{
			setup[y] += fgetc(file);
		}

		if (fgetc(file) != '\n')
		{
			return MovementResult::BAD_RESPONSE;
		}
	}
	return MovementResult::OK;

	
}

MovementResult FileController::QueryMove(std::string & buffer)
{
	//This bit is kind of hacky and terrible, and yes I am mixing C with C++
	//Yes I should have used fstream for the whole thing and it would be much easier.
	//Oh well.

	char buf[BUFSIZ];

	fgets(buf, sizeof(buf), file);
	char * s = (char*)(buf);
	while (*s != ':' && *s != '\0')
		++s;
	
	//Move forward to the start of the move information
	for (int i=0; i < 2; ++i)
	{
		if (*s != '\0' && *s != '\n')
			++s;
	}
	
	//Unfortunately we can't just copy the whole line
	buffer = string(s);
	//We have to remove the movement result tokens
	

	vector<string> tokens;
	Game::Tokenise(tokens, buffer, ' ');
	buffer.clear();

	if (tokens.size() < 1)
		return MovementResult::BAD_RESPONSE;
	buffer += tokens[0];

	
	if (tokens[0] == "NO_MOVE") //tokens[0] is either the x coordinate, or "NO_MOVE"
		return MovementResult::OK;
	if (tokens.size() < 2)
		return MovementResult::BAD_RESPONSE;
	buffer += " ";
	buffer += tokens[1]; //The y coordinate
	buffer += " ";
	buffer += tokens[2]; //The direction
	
	//Check for a possible multiplier. If tokens[3] is an integer it will be the multiplier, otherwise it won't be.
	if (tokens.size() > 3 && atoi(tokens[3].c_str()) != 0)
	{
		buffer += " ";
		buffer += tokens[3];
	}
	else
	{
		//(tokens[3] should include a new line)
		//buffer += "\n";
	}

	

	
	
	
	return MovementResult::OK;
}

/**
 * Tokenise a string
 */
int Game::Tokenise(std::vector<string> & buffer, std::string & str, char split)
{
	string token = "";
	for (unsigned int x = 0; x < str.size(); ++x)
	{
		if (str[x] == split && token.size() > 0)
		{
			buffer.push_back(token);
			token = "";
		}
		if (str[x] != split)
			token += str[x];
	}
	if (token.size() > 0)
		buffer.push_back(token);
	return buffer.size();
}


