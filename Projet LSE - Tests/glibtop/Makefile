CC=gcc
CFLAGS=-Wall -g -I
CLIBS=-lgtop-2.0 -lgtop_sysdeps-2.0 -lgtop_common-2.0

cpuinfo: 
	$(CC) $(CFLAGS) systeminfo.c -o systeminfo $(CLIBS)
clean:
	rm -f systeminfo
