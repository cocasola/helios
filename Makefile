# *-* Makefile *-*

default:
	make -C core
	make -C ui
	make -C graphics

clean:
	rm obj/*