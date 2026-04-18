; -----------------------------------------------------------------------------
; chr.s — embeds CHR-ROM into the cartridge image.
;
; Replace ../assets/chr/tiles.chr with your own 8 KiB CHR file.
; While you don't have one yet, this stub fills the bank with zeros so the
; ROM still builds.
; -----------------------------------------------------------------------------

.segment "CHARS"

.ifdef HAS_CHR_FILE
        .incbin "../assets/chr/tiles.chr"
.else
        .res 8192, $00
.endif
