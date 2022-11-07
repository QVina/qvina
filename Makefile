BASE=/usr/local
BOOST_VERSION=1_59
BOOST_INCLUDE = $(BASE)/include
C_PLATFORM=-static -pthread
GPP=g++
#C_OPTIONS= -g
C_OPTIONS= -O3 -DNDEBUG
BOOST_LIB_VERSION=

#include makefile_common


LIBOBJ = visited.o cache.o coords.o current_weights.o everything.o grid.o szv_grid.o manifold.o model.o monte_carlo.o mutate.o my_pid.o naive_non_cache.o non_cache.o parallel_mc.o parse_pdbqt.o pdb.o quasi_newton.o quaternion.o random.o ssd.o terms.o weighted_terms.o
MAINOBJ = main.o
SPLITOBJ = split.o

INCFLAGS = -I $(BOOST_INCLUDE)

# -pedantic fails on Mac with Boost 1.41 (syntax problems in their headers)
#CC = ${GPP} ${C_PLATFORM} -ansi -pedantic -Wno-long-long ${C_OPTIONS} $(INCFLAGS)
CC = ${GPP} ${C_PLATFORM} -ansi -Wno-long-long ${C_OPTIONS} $(INCFLAGS) -std=c++0x
exe_name = qvina-w

LDFLAGS = -L$(BASE)/lib -L.

LIBS = -l boost_system${BOOST_LIB_VERSION} -l boost_thread${BOOST_LIB_VERSION} -l boost_serialization${BOOST_LIB_VERSION} -l boost_filesystem${BOOST_LIB_VERSION} -l boost_program_options${BOOST_LIB_VERSION}#-l pthread

.SUFFIXES: .cpp .o

%.o : src/lib/%.cpp 
	$(CC) $(CFLAGS) -o $@ -c $< 

%.o : src/design/%.cpp 
	$(CC) $(CFLAGS) -I src/lib -o $@ -c $< 
	
%.o : src/main/%.cpp 
	$(CC) $(CFLAGS) -I src/lib -o $@ -c $< 

%.o : src/split/%.cpp 
	$(CC) $(CFLAGS) -I src/lib -o $@ -c $< 

all: depend vina vina_split
	mv vina $(exe_name)

set_serial:
	$(eval CC := $(CC) -Dserial=TRUE)
	$(eval exe_name := $(exe_name)_serial)
	
serial: set_serial all

include dependencies

vina: $(MAINOBJ) $(LIBOBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

vina_split: $(SPLITOBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o

bfgs:
	rm -f visited.o quasi_newton.o #main.o monte_carlo.o parallel_mc.o manifold.o
increment: 	bfgs all
#increment: 	all

depend:
	ln -sf `${GPP} -print-file-name=libstdc++.a`
	rm -f dependencies_tmp dependencies_tmp.bak
	touch dependencies_tmp
	makedepend -f dependencies_tmp -Y -I src/lib src/lib/*.cpp src/tests/*.cpp src/design/*.cpp src/main/*.cpp src/split/*.cpp  src/tune/*.cpp
	sed -e "s/^\/src\/[a-z]*\//.\//" dependencies_tmp > dependencies
	rm -f dependencies_tmp dependencies_tmp.bak
