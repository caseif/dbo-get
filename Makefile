# The Makefile language is probably the most confusing one I've ever used. This
# file is a mosaic of probably 15 different StackOverflow posts, plus a couple
# articles from other sites for good measure.

CC = gcc
CXX = g++

VPATH = src
SRC_EXT_C = c
SRC_EXT_CXX = cpp
OUT_DIR = build
OBJ_EXT = o

CFLAGS = -Wall -pedantic-errors -std=c11
CXXFLAGS = -Wall -pedantic-errors -std=c++11 -I/usr/include/jsoncpp

LIBFLAGS = -lcurl -lssh2 -lcrypto -lzip -ljsoncpp
CFLAGS +=  $(LIBFLAGS)
CXXFLAGS += $(LIBFLAGS)

RM = rm -f
LDFLAGS =

SRC_C = $(wildcard $(VPATH)/*.$(SRC_EXT_C))
SRC_CXX = $(wildcard $(VPATH)/*.$(SRC_EXT_CXX))

OBJS = $(patsubst %, $(OUT_DIR)/%, $(patsubst $(VPATH)/%.$(SRC_EXT_C), %.$(OBJ_EXT), $(SRC_C)) $(patsubst $(VPATH)/%.$(SRC_EXT_CXX), %.$(OBJ_EXT), $(SRC_CXX)))

all: directories dboget

dboget: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUT_DIR)/dbo-get $(OBJS) $(LDLIBS) $(CXXFLAGS)

.SECONDEXPANSION:
$(OUT_DIR)/%.$(OBJ_EXT): $$(wildcard $(VPATH)/%.$(SRC_EXT_C)) $$(wildcard $(VPATH)/%.$(SRC_EXT_CXX))
	$(CXX) $(CXXFLAGS) -MD -c -o $@ $<
	@cp $(OUT_DIR)/$*.d $(OUT_DIR)/$*.P; \
			sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $(OUT_DIR)/$*.d >> $(OUT_DIR)/$*.P; \
			rm -f $(OUT_DIR)/$*.d

-include *.P

clean:
	$(RM -r $(OUT_DIR))

MKDIR_P = @mkdir -p

.PHONY: directories

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}
