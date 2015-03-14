MAKE 		= make
MINGW 		= x86_64-w64-mingw32-gcc
MAKEFILE 	= Makefile.std

rogue:
	$(MAKE) -C ./src -f $(MAKEFILE) MINGW=$(MINGW) MAKEFILE="-f $(MAKEFILE)" dist.mingw32
	mv ./src/rogue54.exe ./
	mv ./src/rogue54.doc ./
	mv ./src/rogue54.html ./