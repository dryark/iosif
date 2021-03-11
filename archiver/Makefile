all: test

test_sources := $(wildcard *.c) $(wildcard *.h) ../cfutil.c ../bytearr.c nsutil.m 
c_files := $(wildcard *.c) ../cfutil.c ../bytearr.c nsutil.m 

test: $(test_sources)
	gcc -o test $(c_files) -F ./ -lobjc -framework Foundation -framework CoreFoundation
       
clean:
	$(RM) test