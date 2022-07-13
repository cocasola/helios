# *-* Makefile *-*

default:
	make -C core
	make -C ui
	make -C graphics
	make -C math

clean:
	rm obj/*