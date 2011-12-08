#!/usr/bin/python -u

from asmodeus import *

asmodeus = Asmodeus()
if asmodeus.Setup():
	while asmodeus.MoveCycle():
		pass
