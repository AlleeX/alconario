; -----------------------------------------------------------------------------
; chr.s — embeds CHR-ROM into the cartridge image.
;
; Replace ../assets/chr/tiles.chr with your own 8 KiB CHR file.
; While you don't have one yet, this stub fills the bank with zeros so the
; ROM still builds.
; -----------------------------------------------------------------------------

.segment "CHARS"

        .incbin "../../assets/chr/tiles.chr"
