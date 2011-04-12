# Meta-makefile

.PHONY: all client server clean clean-client clean-server

all: client server

client:
	$(MAKE) -f Makefile.client

server:
	$(MAKE) -f Makefile.server

clean: clean-client clean-server

clean-client:
	$(MAKE) -f Makefile.client clean

clean-server:
	$(MAKE) -f Makefile.server clean
