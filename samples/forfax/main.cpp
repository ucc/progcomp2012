#include <cstdlib>
#include <iostream>

#include "forfax.h"

using namespace std;

#include <stdio.h>
void quit();
Forfax forfax;
int main(int argc, char ** argv)
{
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	atexit(&quit);

	if (!forfax.Setup())
		exit(EXIT_SUCCESS);

	while (forfax.MakeMove());

	cerr << "Forfax threw a hissy fit, and exited!\n";
	
	exit(EXIT_SUCCESS);
	return 0;

	
}

void quit()
{
	cerr << " Forfax quit\n";
}
