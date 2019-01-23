# Documentation of monitor program mon24.s19
# ==========================================
# Create a documented assembler source file and 
# source listing from the Motorola S-Record file.
#
# The MC6809 disassembler f9dasm is needed.
# Download: https://github.com/Arakula/f9dasm
#
DISASM  = f9dasm
SRCFILE = mon24.s19
SRC     = ../$(SRCFILE)
TARGET1 = $(patsubst %.s19,%.asm,$(SRCFILE))
TARGET2 = $(patsubst %.s19,%.lst,$(SRCFILE))
INFO    = $(patsubst %.s19,%.inf,$(SRCFILE))
HAS_DISASM := $(shell command -v $(DISASM) 2> /dev/null)

all: $(TARGET1) $(TARGET2)
ifndef HAS_DISASM
	$(error "$(DISASM) not found. \
		Download from: https://github.com/Arakula/f9dasm")
endif

$(TARGET1): $(SRC) $(INFO)
	$(DISASM) -info $(INFO) -out $(TARGET1) -noaddr -noasc -nohex \
	       -nofcc -cchar \; $(SRC)

$(TARGET2): $(SRC) $(INFO)
	$(DISASM) -info $(INFO) -out $(TARGET2) -nofcc -cchar \; $(SRC)

clean:
	rm -rf $(TARGET1) $(TARGET2)

PHONY: clean all

