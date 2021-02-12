all: iosif

iosif_sources := $(wildcard *.c) $(wildcard *.h) nsutil.m

iosif: $(iosif_sources)
	gcc main.c cfutil.c nsutil.m bytearr.c -o iosif -F ./ -lobjc -framework Foundation -framework CoreFoundation -framework MobileDevice

clean:
	$(RM) iosif