cc := gcc

D_SRC = src
D_INC = -I./inc
D_OBJ = obj
D_BIN = bin
D_INSTALL_G =/usr/bin
D_INSTALL_L =~/pcron/bin
TATGET = $(D_BIN)/pcron

SRC_C   = $(wildcard $(D_SRC)/*.c)

OBJ_C   = $(addprefix $(D_OBJ)/,$(patsubst %.c,%.o,$(notdir $(SRC_C))))


$(TATGET):$(OBJ_C)
	if [ ! -d $(D_BIN) ]; then mkdir -p $(D_BIN); fi;\
	gcc -o $@ $^

$(D_OBJ)/%.o:$(D_SRC)/%.c
	if [ ! -d $(D_OBJ) ]; then mkdir -p $(D_OBJ); fi;\
	gcc -c $< -o $@

.PHONY: clean

clean:
	rm -f $(D_OBJ)/* $(TATGET) 

install:
	if [ ! -d $(D_INSTALL_L) ]; then mkdir -p $(D_INSTALL_L); fi;\
	cp -f $(TATGET) $(D_INSTALL_L)/
	cp -f $(TATGET) $(D_INSTALL_G)/

uninstall:
	rm -f $(D_INSTALL)/pcron
