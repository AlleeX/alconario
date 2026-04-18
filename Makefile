# =============================================================================
# Alconario — NES game Makefile  (cc65 + neslib)
# =============================================================================
#
# WHAT THIS FILE DOES
# -------------------
# A Makefile describes HOW to build a project.  Running `make` reads this
# file and executes only the commands whose outputs are out-of-date.
#
# THE cc65 TOOLCHAIN (three-step compile for 6502)
# ------------------------------------------------
#   Step 1:  cc65  — C compiler  — turns .c  files into 6502 assembly (.s)
#   Step 2:  ca65  — assembler   — turns .s  files into object files  (.o)
#   Step 3:  ld65  — linker      — combines all .o files + libraries into
#                                  the final .nes ROM.
#
# WHY NOT A SINGLE STEP?
# The 6502 is an 8-bit CPU with very limited addressing modes.  Splitting
# compile → assemble → link lets us inspect the generated assembly, hand-
# write hot inner loops in .s, and link them seamlessly with C code.
#
# USEFUL MAKE COMMANDS
# --------------------
#   make           — build build/alconario.nes
#   make run       — build and open in FCEUX (or override: make run EMU=mesen)
#   make clean     — delete the build/ directory
#   make help      — show this summary
# =============================================================================

# ---- Project identity -------------------------------------------------------
PROJECT   := alconario        # output filename (without extension)
TARGET    := nes              # cc65 target platform — tells the compiler/linker
                              # which runtime library and calling conventions to use
MAPPER    := nrom             # documentation only — the actual mapper is set
                              # in cfg/nes.cfg (NROM = mapper 0, simplest)

# ---- Toolchain binaries -----------------------------------------------------
CC        := cc65             # C → 6502 assembly
AS        := ca65             # 6502 assembly → object file
LD        := ld65             # object files + libs → .nes ROM

# ---- Directory layout -------------------------------------------------------
SRC_DIR   := src              # C source files  (*.c)
ASM_DIR   := src/asm          # Assembly files  (*.s) — custom 6502 stubs
INC_DIR   := include          # C header files  (*.h)
LIB_DIR   := lib              # Vendored third-party libraries
CFG_DIR   := cfg              # Linker configuration  (nes.cfg)
ASSET_DIR := assets           # Game assets  (CHR, palettes, music, levels)
BUILD_DIR := build            # All build outputs land here  (gitignored)
OBJ_DIR   := $(BUILD_DIR)/obj # Intermediate .o and generated .s files

NESLIB_DIR := $(LIB_DIR)/neslib  # Path to the cloned neslib repo

# ---- Compiler flags  (cc65) -------------------------------------------------
# -t $(TARGET)  : target = nes — selects nes.lib, correct startup code
# -O            : basic optimisation (dead code removal, const folding)
# -i            : inline some stdlib calls (saves JSR/RTS overhead)
# -r            : use register variables (mapped to zero-page for speed)
# -s            : optimise for size (important — only 32 KB PRG ROM!)
# -Cl           : make local variables static (no runtime stack allocation;
#                 functions are NOT re-entrant, but that's fine for NES C)
# -I $(INC_DIR) : search our include/ for headers
# -I $(NESLIB_DIR): search lib/neslib/ for neslib.h
CFLAGS  := -t $(TARGET) -Oirs -Cl \
           -I $(INC_DIR) -I $(NESLIB_DIR)

# ---- Assembler flags  (ca65) ------------------------------------------------
# -t $(TARGET)    : same NES target
# -I $(NESLIB_DIR): find neslib's .inc include files (macros, zero-page defs)
ASFLAGS := -t $(TARGET) -I $(NESLIB_DIR)

# ---- Linker flags  (ld65) ---------------------------------------------------
# -C cfg/nes.cfg  : memory map — tells ld65 where PRG, CHR, ZP, RAM live
# -m *.map        : write a map file — useful to inspect memory usage
# --dbgfile *.dbg : write a debug file — used by emulator debuggers (FCEUX, Mesen)
LDFLAGS := -C $(CFG_DIR)/nes.cfg \
           -m $(BUILD_DIR)/$(PROJECT).map \
           --dbgfile $(BUILD_DIR)/$(PROJECT).dbg

# nes.lib — the neslib C runtime for the NES target (provided by cc65 install)
LIBS    := nes.lib

# ---- Source file discovery --------------------------------------------------
# $(wildcard pattern) expands to all matching files.
C_SRCS   := $(wildcard $(SRC_DIR)/*.c)       # all .c files in src/
ASM_SRCS := $(wildcard $(ASM_DIR)/*.s)       # all .s files in src/asm/

# If neslib ships its own crt0.s (startup + NMI handler + iNES header),
# add it to the assembly source list automatically.
ifneq ($(wildcard $(NESLIB_DIR)/crt0.s),)
ASM_SRCS += $(NESLIB_DIR)/crt0.s
endif

# ---- Object file names  (derived from source lists) ------------------------
# patsubst replaces path prefix and extension: src/foo.c → build/obj/foo.o
C_OBJS   := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SRCS))

# For .s files we strip the directory (notdir) then replace .s with .o.
# This works for both src/asm/*.s AND lib/neslib/crt0.s.
ASM_OBJS := $(patsubst %.s,$(OBJ_DIR)/%.o,$(notdir $(ASM_SRCS)))

OBJS     := $(C_OBJS) $(ASM_OBJS)    # combined list passed to the linker

OUTPUT   := $(BUILD_DIR)/$(PROJECT).nes   # final ROM file

# ---- Default emulator -------------------------------------------------------
# Override on the command line:  make run EMU=mesen
EMU ?= fceux

# =============================================================================
# BUILD TARGETS
# =============================================================================

.PHONY: all run clean dirs help

# Default target — build the ROM.
all: dirs $(OUTPUT)

# Create the output directories before anything else.
dirs:
	@mkdir -p $(OBJ_DIR)

# ---- C source → object file  (two steps: cc65 → ca65) ----------------------
# $< = the .c prerequisite    $* = the stem (filename without extension)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "  CC  $<"
	@$(CC) $(CFLAGS) -o $(OBJ_DIR)/$*.s $<   # 1. compile to .s
	@$(AS) $(ASFLAGS) -o $@ $(OBJ_DIR)/$*.s  # 2. assemble to .o

# ---- Project assembly (src/asm/*.s) → object file ---------------------------
$(OBJ_DIR)/%.o: $(ASM_DIR)/%.s
	@echo "  AS  $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# ---- neslib assembly (lib/neslib/crt0.s etc.) → object file -----------------
$(OBJ_DIR)/%.o: $(NESLIB_DIR)/%.s
	@echo "  AS  $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# ---- Link all objects into the final .nes ROM --------------------------------
# $^ = all prerequisites (the full object list)
$(OUTPUT): $(OBJS)
	@echo "  LD  $@"
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "  ==> $@"

# ---- Run in emulator --------------------------------------------------------
run: all
	$(EMU) $(OUTPUT)

# ---- Remove all build artefacts ---------------------------------------------
clean:
	@rm -rf $(BUILD_DIR)
	@echo "  CLEAN"

# ---- Quick usage reminder ---------------------------------------------------
help:
	@echo "Targets:"
	@echo "  make           Build $(OUTPUT)"
	@echo "  make run       Build and run in \$$EMU (default: $(EMU))"
	@echo "  make clean     Remove build artifacts"
