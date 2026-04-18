# NES Development Glossary 📖

Quick reference for terms you'll encounter in code, docs, and forums.

---

## Hardware

| Term | What it is |
|---|---|
| **CPU** | Ricoh 2A03 — 8-bit processor based on MOS 6502 (no decimal mode). Runs at ~1.79 MHz (NTSC). Executes your game logic. |
| **PPU** | Picture Processing Unit (Ricoh 2C02). Separate chip that handles all graphics: background tiles, sprites, scrolling, palettes. Your C code talks to it through memory-mapped registers (`$2000`–`$2007`). |
| **APU** | Audio Processing Unit. Built into the 2A03. Five channels: 2 pulse (square wave), 1 triangle, 1 noise, 1 DMC (sample playback). |
| **OAM** | Object Attribute Memory — 256 bytes inside the PPU that hold sprite data (64 sprites × 4 bytes each: Y, tile index, attributes, X). Typically filled via DMA from CPU RAM at `$0200`. |
| **DMA** | Direct Memory Access — hardware-accelerated copy of 256 bytes from CPU RAM to OAM. Triggered by writing to `$4014`. Takes ~513 CPU cycles. |
| **VRAM** | Video RAM — 2 KiB inside the PPU for nametables. Also called CIRAM. |

## Memory Map

| Term | What it is |
|---|---|
| **Zero Page** | CPU RAM `$0000`–`$00FF`. Fastest memory — 6502 has special addressing modes for it. cc65 maps register variables here. |
| **Stack** | CPU RAM `$0100`–`$01FF`. Hardware stack, grows downward. Also used by neslib for the PAL buffer (`$01C0`). |
| **PRG-ROM** | Program ROM — the cartridge memory that holds your compiled code and data. Mapped at `$8000`–`$FFFF` for NROM-256 (32 KiB). |
| **CHR-ROM** | Character ROM — 8 KiB of tile graphics on the cartridge, directly accessed by the PPU. Contains pattern tables. |

## Graphics Concepts

| Term | What it is |
|---|---|
| **Tile** | 8×8 pixel block — the basic building unit. Each tile is 16 bytes in CHR-ROM (2 bits per pixel = 4 colors). |
| **Pattern Table** | A set of 256 tiles (4 KiB). CHR-ROM has two: `$0000` (usually BG) and `$1000` (usually sprites). Also called "tile pages". |
| **Nametable** | A 32×30 grid of tile indices (960 bytes) that defines one screen of background. The PPU has room for two; mirroring fakes the other two. |
| **Attribute Table** | 64 bytes at the end of each nametable. Assigns one of four sub-palettes to each 16×16 pixel area (2×2 tiles). This is why NES backgrounds have that "blocky color" look. |
| **Palette** | 4 colors per sub-palette, 4 sub-palettes for BG + 4 for sprites = 32 bytes total at PPU `$3F00`. Color 0 of each group is shared (universal background color). |
| **Sprite** | A movable 8×8 (or 8×16) graphic. Max 64 on screen, max 8 per scanline (extras flicker). Defined in OAM. |
| **Mirroring** | How the four logical nametables map to physical VRAM. **Horizontal** = vertical scrolling, **Vertical** = horizontal scrolling. Set by cartridge wiring or mapper. |
| **Scanline** | One horizontal line of the 256×240 display. The PPU renders one per ~113 CPU cycles. |
| **VBlank** | Vertical Blank — the ~20-scanline gap after the visible frame (lines 241–260). **The only safe time** to update VRAM/palettes/scroll registers. |
| **NMI** | Non-Maskable Interrupt — fires at the start of every VBlank. Your NMI handler (in neslib's `crt0.s`) pushes OAM, palette updates, and scroll values to the PPU here. |
| **Sprite 0 Hit** | Hardware flag set when sprite 0's opaque pixel overlaps a BG opaque pixel. Used for split-screen scroll tricks (e.g., fixed status bar + scrolling playfield). |

## Mappers

| Term | What it is |
|---|---|
| **Mapper** | Cartridge hardware that extends the NES beyond its base addressing. Handles bank switching, extra RAM, IRQs, etc. |
| **NROM** | Mapper 0 — no bank switching. 16 or 32 KiB PRG + 8 KiB CHR. Simplest. Your current setup. |
| **UNROM** | Mapper 2 — switchable 16 KiB PRG banks (up to 256 KiB), fixed CHR-RAM. Good next step. |
| **MMC1** | Mapper 1 — switchable PRG + CHR banks, configurable mirroring. Used by Zelda, Metroid. |
| **MMC3** | Mapper 4 — fine-grained banking + scanline counter IRQ. Used by Super Mario Bros. 3, Mega Man 3–6. |

## iNES Header

The 16-byte header at the start of every `.nes` ROM file:

| Byte(s) | Meaning |
|---|---|
| 0–3 | Magic: `N E S $1A` |
| 4 | Number of 16 KiB PRG-ROM banks |
| 5 | Number of 8 KiB CHR-ROM banks (0 = CHR-RAM) |
| 6 | Flags 6: mapper low nibble, mirroring, battery, trainer |
| 7 | Flags 7: mapper high nibble, NES 2.0 indicator |
| 8–15 | Extended flags (usually zero for basic ROMs) |

## cc65 / Build Toolchain

| Term | What it is |
|---|---|
| **cc65** | C compiler — translates `.c` → 6502 assembly (`.s`). |
| **ca65** | Assembler — translates `.s` → object file (`.o`). |
| **ld65** | Linker — combines `.o` files using a config (`.cfg`) → final `.nes`. |
| **Segment** | A named block in the linker config (`CODE`, `RODATA`, `BSS`, `ZEROPAGE`, `CHARS`, etc.). You place code/data into segments; the linker maps them to physical memory. |
| **crt0.s** | C Runtime Zero — startup code that runs before `main()`. Initializes hardware, clears RAM, sets up the stack, copies DATA segment, calls `main()`. In NES projects, also contains the iNES header, NMI handler, and interrupt vectors. |

## neslib Specifics

| Term | What it is |
|---|---|
| **neslib** | Shiru's lightweight NES library for cc65. Provides: pad reading, OAM buffer management, palette updates, vblank synchronization, nametable helpers. |
| **famitone2** | Shiru's compact music/SFX engine. Plays FamiTracker-exported data. Runs inside the NMI handler (~3–5% CPU per frame). |
| **ppu_on_all()** | neslib function: enable BG + sprite rendering. |
| **ppu_off()** | neslib function: disable rendering (safe to bulk-write VRAM outside VBlank). |
| **oam_spr()** | neslib function: add a sprite to the OAM buffer. |
| **pad_poll()** | neslib function: read controller state. Returns bitmask of pressed buttons. |

## Common PPU Registers

| Address | Name | Purpose |
|---|---|---|
| `$2000` | PPUCTRL | NMI enable, sprite size, BG/sprite pattern table, nametable select, VRAM increment |
| `$2001` | PPUMASK | Color emphasis, sprite/BG enable, left-column clipping |
| `$2002` | PPUSTATUS | VBlank flag, sprite 0 hit, sprite overflow (read resets `$2005`/`$2006` latch) |
| `$2003` | OAMADDR | OAM address for `$2004` access (rarely used directly — prefer DMA) |
| `$2005` | PPUSCROLL | Set X/Y scroll (two writes) |
| `$2006` | PPUADDR | Set VRAM address (two writes, high byte first) |
| `$2007` | PPUDATA | Read/write VRAM data |
| `$4014` | OAMDMA | Trigger OAM DMA from CPU page (write `$02` → copies `$0200`–`$02FF`) |

## Controller Buttons

```
Bit   Button     neslib constant
─────────────────────────────────
 7    Right      PAD_RIGHT
 6    Left       PAD_LEFT
 5    Down       PAD_DOWN
 4    Up         PAD_UP
 3    Start      PAD_START
 2    Select     PAD_SELECT
 1    B          PAD_B
 0    A          PAD_A
```

---

*Add terms as you learn them — future you will thank you.* 🌻
