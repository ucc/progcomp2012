#Makefile for the sample AI programs for UCC progcomp 2012

CPP = g++ -Wall -pedantic -lSDL -lGL  -g

dummy : dummy.o
	$(CPP) -o dummy dummy.o



clean :
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~



