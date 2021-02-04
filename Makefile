all: iosif

iosif_sources := $(wildcard *.c) $(wildcard *.h)

iosif: $(iosif_sources)
	gcc main.c -o iosif -F ./ -framework CoreFoundation -framework MobileDevice

clean:
	$(RM) iosif