# Hand-written 6502 assembly

Drop `.s` files here for code that must be implemented in assembly
(custom IRQ handlers, fast inner loops, mapper-specific header overrides…).

The Makefile picks them up automatically via `wildcard src/asm/*.s`.

By default the iNES header and `crt0.s` come from **neslib** (`lib/neslib/`).
