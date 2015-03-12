MAKE 		= make
MINGW 		= x86_64-w64-mingw32-gcc
MAKEFILE 	= Makefile.std

rogue:
	$(MAKE) -C ./src -f $(MAKEFILE) MINGW=$(MINGW) MAKEFILE="-f $(MAKEFILE)" dist.mingw32
	cp ./src/rogue54.exe ./
	cp ./src/rogue54.doc ./
	cp ./src/rogue54.html ./