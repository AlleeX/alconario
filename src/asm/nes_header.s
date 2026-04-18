; nes_header.s — NES cartridge configuration symbols
; Required by neslib's crt0.s to build the iNES header.

.export NES_PRG_BANKS : absolute = 2
.export NES_CHR_BANKS : absolute = 1
.export NES_MAPPER    : absolute = 0
.export NES_MIRRORING : absolute = 1

.segment "ZEROPAGE"
.exportzp _oam_off
_oam_off: .res 1
