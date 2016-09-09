CXX=g++
OUT_DIR=build
SRC_EXT=cpp
OBJ_EXT=o

CPPFLAGS= --std=c++11

LIBFLAGS=-lcurl -lssh2
CPPFLAGS+= $(LIBFLAGS)

RM=rm -f
LDFLAGS=-g

SRC=$(wildcard *.$(SRC_EXT))
OBJS=$(patsubst %.cpp,$(OUT_DIR)/%.$(OBJ_EXT),$(SRC))

all: dboget

dboget: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUT_DIR)/dbo-get $(OBJS) $(LDLIBS) $(CPPFLAGS)

$(OUT_DIR)/%.$(OBJ_EXT): %.$(SRC_EXT)
	@g++ -MD -c -o $@ $<
	@cp $(OUT_DIR)/$*.d $(OUT_DIR)/$*.P; \
			sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $(OUT_DIR)/$*.d >> $(OUT_DIR)/$*.P; \
			rm -f $(OUT_DIR)/$*.d

-include *.P

clean:
	$(RM) $(OBJS)
