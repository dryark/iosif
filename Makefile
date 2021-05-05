all: iosif

iosif_sources := $(wildcard *.c) $(wildcard *.h) $(wildcard archiver/*.c) $(wildcard archiver/*.h) $(wildcard ujsonin/*.c) $(wildcard ujsonin/*.h)
c_files := $(wildcard *.c) $(wildcard archiver/*.c) $(wildcard ujsonin/*.c)
c_files := $(filter-out archiver/in.c archiver/out.c, $(c_files))

# nsutil.m -lobjc -framework Foundation
# -L /usr/local/opt/openssl/lib -lssl -I /usr/local/opt/openssl/include
# -DDEBUG -DDEBUG2
iosif: $(iosif_sources)
	gcc -g -DNNG -lnng -I /usr/local/include -o iosif $(c_files) -F ./ -framework CoreFoundation -framework MobileDevice
       
clean:
	$(RM) iosif