all: in out

in_c := $(wildcard *.c) ../cfutil.c ../bytearr.c nsutil.m
in_c := $(filter-out out.c, $(in_c))
in_all := $(in_c) $(wildcard *.h)

out_c := $(wildcard *.c) ../bytearr.c
out_c := $(filter-out in.c, $(out_c))
out_all := $(out_c) $(wildcard *.h)

in: $(in_all)
	gcc -o in $(in_c) -F ./ -lobjc -framework Foundation -framework CoreFoundation

out: $(out_all)
	gcc -o out $(out_c) -F ./

clean:
	$(RM) test