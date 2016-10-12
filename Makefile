CXX=g++
OUT_DIR=build
SRC_EXT=cpp
OBJ_EXT=o

CPPFLAGS= -std=c++11

LIBFLAGS=-lcurl -lssh2 -lcrypto -lzip -ljsoncpp
CPPFLAGS+= $(LIBFLAGS)

RM=rm -f
LDFLAGS=-g

SRC=$(wildcard *.$(SRC_EXT))
OBJS=$(patsubst %.cpp,$(OUT_DIR)/%.$(OBJ_EXT),$(SRC))

all: directories dboget

dboget: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUT_DIR)/dbo-get $(OBJS) $(LDLIBS) $(CPPFLAGS)

$(OUT_DIR)/%.$(OBJ_EXT): %.$(SRC_EXT)
	@$(CXX) $(CPPFLAGS) -MD -c -o $@ $<
	@cp $(OUT_DIR)/$*.d $(OUT_DIR)/$*.P; \
			sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $(OUT_DIR)/$*.d >> $(OUT_DIR)/$*.P; \
			rm -f $(OUT_DIR)/$*.d

-include *.P

clean:
	$(RM) $(OBJS)

MKDIR_P = mkdir -p

.PHONY: directories

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}
