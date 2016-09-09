CXX=g++

CPPFLAGS=-v --std=c++11

LIBFLAGS=-lcurl -lssh2
CPPFLAGS+= $(LIBFLAGS)

RM=rm -f
LDFLAGS=-g

SRC=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRC))
$(info src is $(SRC))
$(info objs is $(OBJS))

all: dboget

dboget: $(OBJS)
	$(CXX) $(LDFLAGS) -o dbo-get $(OBJS) $(LDLIBS) $(CPPFLAGS)

%.o : %.cpp
	@g++ -MD -c -o $@ $<
	@cp $*.d $*.P; \
			sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
			rm -f $*.d

-include *.P

clean:
	$(RM) $(OBJS)
