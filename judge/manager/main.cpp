#include <stdlib.h>
#include <stdio.h>






#include "game.h"


using namespace std;

Piece::Colour SetupGame(int argc, char ** argv);
void DestroyGame();
void PrintResults(const MovementResult & result, string & buffer);

int main(int argc, char ** argv)
{
	


	if (argc == 1)
	{
		fprintf(stderr, "Usage: stratego [options] red blue\n");
		fprintf(stderr, "       stratego --help\n");
		exit(EXIT_SUCCESS);
		
	}
	

	Piece::Colour setupError = SetupGame(argc, argv);
	MovementResult result = MovementResult::OK;
	if (setupError == Piece::NONE)
	{
		result = Game::theGame->Play();
	}
	else
	{
		result = MovementResult::BAD_SETUP;
		Game::theGame->ForceTurn(setupError);
	}
	
	Game::theGame->PrintEndMessage(result);

	string buffer = "";
	PrintResults(result, buffer);

	//Message the AI's the quit message
	Game::theGame->red->Message("QUIT " + buffer);
	Game::theGame->blue->Message("QUIT " + buffer);

	//Log the message
	if (Game::theGame->GetLogFile() != stdout)
		Game::theGame->logMessage("%s\n", buffer.c_str());

	fprintf(stdout, "%s\n", buffer.c_str());

	exit(EXIT_SUCCESS);
	return 0;
}

Piece::Colour SetupGame(int argc, char ** argv)
{
	char * red = NULL; char * blue = NULL; double stallTime = 0.0; bool graphics = false; bool allowIllegal = false; FILE * log = NULL;
	Piece::Colour reveal = Piece::BOTH; char * inputFile = NULL; int maxTurns = 5000; bool printBoard = false; double timeoutTime = 2.0;
	bool server = false; bool client = false;
	for (int ii=1; ii < argc; ++ii)
	{
		if (argv[ii][0] == '-')
		{
			switch (argv[ii][1])
			{
				case 't':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected stall time value after -t switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "inf") == 0)
						stallTime = -1;
					else
						stallTime = atof(argv[ii+1]);
					++ii;
					break;

				case 'T':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected timeout value after -T switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "inf") == 0)
						timeoutTime = -1;
					else
						timeoutTime = atof(argv[ii+1]);
					++ii;
					break;

				case 'g':
					#ifdef BUILD_GRAPHICS
					graphics = !graphics;
					#else
					fprintf(stderr, "ERROR: -g switch supplied, but the program was not built with graphics.\n Please do not use the -g switch.");
					exit(EXIT_FAILURE);
					#endif //BUILD_GRAPHICS

					break;
				case 'p':
					printBoard = !printBoard;
					break;
				case 'i':
					allowIllegal = true;
					break;

				case 'o':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected filename or \"stdout\" after -o switch!\n");
						exit(EXIT_FAILURE);
					}
					if (log != NULL)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected at most ONE -o switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "stdout") == 0)
						log = stdout;
					else
						log = fopen(argv[ii+1], "w");
					setbuf(log, NULL);
				
					++ii;
					break;	

				case 'r':
					if (reveal == Piece::BOTH)
						reveal = Piece::BLUE;
					else
						reveal = Piece::NONE;
					break;			
				case 'b':
					if (reveal == Piece::BOTH)
						reveal = Piece::RED;
					else
						reveal = Piece::NONE;
					break;
				case 'm':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected max_turns value after -m switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "inf") == 0)
						maxTurns = -1;
					else
						maxTurns = atoi(argv[ii+1]);
					++ii;
					break;
				case 'f':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected filename after -f switch!\n");
						exit(EXIT_FAILURE);
					}
					if (log != NULL)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected at most ONE -f switch!\n");
						exit(EXIT_FAILURE);
					}
					red = (char*)("file");
					blue = (char*)("file");
					inputFile = argv[ii+1];
					++ii;
					break;
				case 'h':
					system("clear");	
					system("less manual.txt");
					exit(EXIT_SUCCESS);
					break;
				case '-':
					if (strcmp(argv[ii]+2, "help") == 0)
					{
						system("clear");	
						system("less manual.txt");
						exit(EXIT_SUCCESS);
					}
					else if (strcmp(argv[ii]+2, "server") == 0)
					{
						if (client == true)
						{
							fprintf(stderr, "ARGUMENT_ERROR - Can't be both a server and a client!\n");
							exit(EXIT_FAILURE);
						}
						server = true;
						
					}
					else if (strcmp(argv[ii]+2, "client") == 0)
					{
						if (server == true)
						{
							fprintf(stderr, "ARGUMENT_ERROR - Can't be both a server and a client!\n");
							exit(EXIT_FAILURE);
						}
						client = true;	
						
					}
					else
					{
						fprintf(stderr, "ARGUMENT_ERROR - Unrecognised switch \"%s\"...\n", argv[ii]);
						exit(EXIT_FAILURE);
					}
			}
			
		}
		else
		{
			if (red == NULL)
				red = argv[ii];
			else if (blue == NULL)
				blue = argv[ii];
			else
			{
				fprintf(stderr, "ARGUMENT_ERROR - Unexpected argument \"%s\"...\n", argv[ii]);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (graphics && stallTime == 0.0)
		stallTime = 0.00001; //Hack so that SDL events (ie SDL_QUIT) will have time to be captured when graphics are enabled

	if (inputFile == NULL)
	{
		if (server)  
		{
			if (red != NULL && blue != NULL)
			{
				fprintf(stderr, "ARGUMENT_ERROR - When using the --server switch, only supply ONE (1) player.\n");
				exit(EXIT_FAILURE);
			}
		}
		else if (red == NULL || blue == NULL) //Not enough players
		{
			if (client)
				fprintf(stderr, "ARGUMENT_ERROR - When using the --client switch, supply an IP for the Red player.\n");
			else			
				fprintf(stderr, "ARGUMENT_ERROR - Did not recieve enough players (did you mean to use the -f switch?)\n");	
			exit(EXIT_FAILURE);	
		}
		
		if (client)
		{
			blue = red; red = NULL;
		}

		Game::theGame = new Game(red,blue, graphics, stallTime, allowIllegal,log, reveal,maxTurns, printBoard, timeoutTime, server, client);
	}
	else
	{
		if (server || client)
		{
			fprintf(stderr, "ARGUMENT_ERROR - The -f switch is incompatable with the --server or --client switches!\n");
			exit(EXIT_FAILURE);
		}
		Game::theGame = new Game(inputFile, graphics, stallTime, allowIllegal,log, reveal,maxTurns, printBoard, timeoutTime);
	}

	if (Game::theGame == NULL)
	{
		fprintf(stderr,"INTERNAL_ERROR - Error creating Game!\n");
		exit(EXIT_FAILURE);
	}
	atexit(DestroyGame);
	
	return Game::theGame->Setup(red, blue);
	

}

void PrintResults(const MovementResult & result, string & buffer)
{
	stringstream s("");
	switch (Game::theGame->Turn())
	{
		case Piece::RED:
			s << Game::theGame->red->name << " RED ";
			break;
		case Piece::BLUE:
			s << Game::theGame->blue->name << " BLUE ";
			break;
		case Piece::BOTH:
			s << "neither BOTH ";
			break;
		case Piece::NONE:
			s << "neither NONE ";
			break;
	}

	if (!Board::LegalResult(result) && result != MovementResult::BAD_SETUP)
		s << "ILLEGAL ";
	else if (!Board::HaltResult(result))
		s << "INTERNAL_ERROR ";
	else
	{
		switch (result.type)
		{
			case MovementResult::VICTORY_FLAG:
			case MovementResult::VICTORY_ATTRITION: //It does not matter how you win, it just matters that you won!
				s <<  "VICTORY ";
				break;
			case MovementResult::SURRENDER:
				s << "SURRENDER ";
				break;
			case MovementResult::DRAW:
				s << "DRAW ";
				break;
			case MovementResult::DRAW_DEFAULT:
				s << "DRAW_DEFAULT ";
				break;
			case MovementResult::BAD_SETUP:
				s << "BAD_SETUP ";
				break;	
			default:
				s << "INTERNAL_ERROR ";
				break;	
		}
	}
	
	s << Game::theGame->TurnCount() << " " << Game::theGame->theBoard.TotalPieceValue(Piece::RED) << " " << Game::theGame->theBoard.TotalPieceValue(Piece::BLUE);

	buffer = s.str();
	

}

void DestroyGame()
{
	delete Game::theGame;
	Game::theGame = NULL;
}
