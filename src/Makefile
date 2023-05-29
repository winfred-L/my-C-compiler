all: mygcc

OBJS = parser.o  \
       codegen.o \
       main.o    \
       tokens.o  \
	   native.o  \
	   utils.o   \

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++14 #-DDEBUG
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -lncurses -rdynamic
LIBS = `$(LLVMCONFIG) --system-libs --libs all`

parser.cpp: parser.y
	bison -d -o $@ $^
	
parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	flex -o $@ $^

%.o: %.cpp
	clang++ -gfull -c $(CPPFLAGS) -o $@ $<

mygcc: $(OBJS)
	clang++ -gfull -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	@echo -e Build Finished


clean:
	$(RM) -rf parser.cpp parser.hpp tokens.cpp *.o *.ll mygcc target
	@echo -e Clean Finished

# make test FILE=test.txt
test: mygcc
	@echo -e TEST $(FILE)
	@./mygcc ../test/$(FILE)
	@clang++ -o target a.o native.cpp
	@./target

# default object file
OBJFILE = a.o

link: native.cpp
	clang++ -o target $(OBJFILE) native.cpp