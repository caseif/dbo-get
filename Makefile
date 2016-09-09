CC=gcc
CXX=g++

CFLAGS=-m32
CXXFLAGS=-m32

LIBFLAGS=-lcurl -lssh2 -L"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin""
CSSFLAGS+= $(LIBFLAGS)

RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)

SRCS=DboProject.cpp jsoncpp.cpp main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: dboget

dboget: $(OBJS)
	$(CXX) $(LDFLAGS) -o dbo-get $(OBJS) $(LDLIBS) $(CXXFLAGS)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend

include .depend
