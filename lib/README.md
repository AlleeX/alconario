# Third-party libraries (vendored)

This folder holds external libraries used by the build.  None are committed by
default — fetch them yourself:

## neslib (Shiru)

```bash
git clone https://github.com/clbr/neslib.git lib/neslib
```

Provides:
- `neslib.h` / `neslib.s` — sprite, PPU, pad, VRAM helpers
- `crt0.s` — startup, NMI handler, iNES header
- `nesdoug.s` (optional extras)

After cloning, `make` will pick up `lib/neslib/crt0.s` automatically.

## famitone2 (optional, for music)

```bash
# Look for "famitone2" by Shiru — distributed with FamiTracker exports.
```

Place it under `lib/famitone2/` and add the `.s` files to the build.
