#!/usr/bin/python -u

from basic_python import *

basicAI = BasicAI()
if basicAI.Setup():
	while basicAI.MoveCycle():
		pass
