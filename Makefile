#/*
# * This file is part of sparsefileutils.
# *
# * Copyright (C) 2012  D.Herrendoerfer
# *
# *   sparsefileutils is free software: you can redistribute it and/or modify
# *   it under the terms of the GNU General Public License as published by
# *   the Free Software Foundation, either version 3 of the License, or
# *   (at your option) any later version.
# *
# *   sparsefileutils is distributed in the hope that it will be useful,
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *   GNU General Public License for more details.
# *
# *   You should have received a copy of the GNU General Public License
# *   along with this file.  If not, see <http://www.gnu.org/licenses/>.
# */
  
CFLAGS = -O2 -Wall
CC = gcc
INSTALL = install
TARGET = /usr/sbin


all: sbcp sbcat sbdiff sbpatch sbmapcat sbmapmerge sbinfo

sbcp:
	$(CC) $(CFLAGS) sbcp.c -o sbcp

sbcat:
	$(CC) $(CFLAGS) sbcat.c -o sbcat

sbdiff:
	$(CC) $(CFLAGS) sbdiff.c -o sbdiff

sbpatch:
	$(CC) $(CFLAGS) sbpatch.c -o sbpatch

sbmapcat:
	$(CC) $(CFLAGS) sbmapcat.c -o sbmapcat

sbmapmerge:
	$(CC) $(CFLAGS) sbmapmerge.c -o sbmapmerge

sbsparsify:
	$(CC) $(CFLAGS) sbsparsify.c -o sbsparsify

sbinfo:
	$(CC) $(CFLAGS) sbinfo.c -o sbinfo

install: sbcp sbdiff sbpatch sbmapcat sbmapmerge
	$(INSTALL) sbcp $(TARGET)/sbcp
	$(INSTALL) sbcat $(TARGET)/sbcat
	$(INSTALL) sbdiff $(TARGET)/sbdiff
	$(INSTALL) sbpatch $(TARGET)/sbpatch
	$(INSTALL) sbmapcat $(TARGET)/sbmapcat
	$(INSTALL) sbmapmerge $(TARGET)/sbmapmerge
# Dont install sbsparsify, it's broken
#	$(INSTALL) sbsparsify $(TARGET)/sbsparsify
	$(INSTALL) sbinfo $(TARGET)/sbinfo

clean:
	rm -f sbcp sbcat sbdiff sbpatch sbmapcat sbmapmerge sbsparsify sbinfo
