all: iosif

iosif_sources := $(wildcard *.c) $(wildcard *.h) archiver/archiver.c archiver/unarchiver.c archiver/byteswap.c
c_files := $(wildcard *.c) archiver/archiver.c archiver/unarchiver.c archiver/byteswap.c archiver/lz4.c

# nsutil.m -lobjc -framework Foundation
iosif: $(iosif_sources)
	gcc -DNNG -lnng -I /usr/local/include -o iosif $(c_files) -F ./ -framework CoreFoundation -framework MobileDevice
       
clean:
	$(RM) iosif