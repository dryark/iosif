all: iosif

iosif_sources := $(wildcard *.c) $(wildcard *.h) nsutil.m
c_files := $(wildcard *.c)

iosif: $(iosif_sources)
	gcc -o iosif $(c_files) nsutil.m -F ./ -lobjc -framework Foundation -framework CoreFoundation -framework MobileDevice
       
clean:
	$(RM) iosif