# Assets

| Folder       | Format            | Notes                                       |
| ------------ | ----------------- | ------------------------------------------- |
| `chr/`       | `.chr` (8 KiB)    | Pattern tables. Built from `.png` via tools |
| `palettes/`  | `.pal` (16 bytes) | NES palette indexes                         |
| `music/`     | `.ftm`, `.s`      | FamiTracker source + famitone2 export       |
| `levels/`    | binary or `.s`    | Nametables, collision maps                  |

CHR data is included into the ROM via the `CHARS` segment.  The simplest
workflow:

1. Edit tiles in **NEXXT** / **YY-CHR**.
2. Export `tiles.chr` (8 KiB).
3. Reference it from an asm file:

   ```asm
   .segment "CHARS"
   .incbin "../assets/chr/tiles.chr"
   ```
