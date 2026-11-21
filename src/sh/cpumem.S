.text
.little
.section        .bss
	.align 5
.global _CMemory_RAM, CMemory_RAM
CMemory_RAM: 
_CMemory_RAM: 
        .zero   0x20000
.global _CMemory_SRAM, CMemory_SRAM
CMemory_SRAM: 
_CMemory_SRAM: 
        .zero   0x20000
.global _CMemory_VRAM, CMemory_VRAM
CMemory_VRAM: 
_CMemory_VRAM: 
        .zero   0x10000
.global _CMemory_BSRAM, CMemory_BSRAM
CMemory_BSRAM:
_CMemory_BSRAM:
	.zero	0x80000
.global _CMemory_FillRAM, CMemory_FillRAM
CMemory_FillRAM: 
_CMemory_FillRAM: 
        .zero	0x600000+0x200+0x8000
.global __IPPU_TileCache_2_
__IPPU_TileCache_2_:
	.zero	4096*128
.global __IPPU_TileCache_4_
__IPPU_TileCache_4_:
	.zero	2048*128
.global __IPPU_TileCache_8_
__IPPU_TileCache_8_:
	.zero	1024*128
.global __IPPU_TileCached_2_
__IPPU_TileCached_2_:
	.zero	4096
.global __IPPU_TileCached_4_
__IPPU_TileCached_4_:
	.zero	2048
.global __IPPU_TileCached_8_
__IPPU_TileCached_8_:
	.zero	1024
.global _S9xAPUCycles
_S9xAPUCycles:
	.zero 1024
.global _Registers
_Registers:
	.zero 16
.global _CPU
_CPU:
	.zero 76
	.align 2
.global _ICPU
_ICPU:
	.zero 36
	.align 2
.global _Settings
_Settings:
	.zero	148
	.align 2
.global _IAPU
_IAPU:
	.zero	68
	.align 2
.global _APU
_APU:
	.zero	224
	.align 2
.global _IPPU
_IPPU:
	.zero	4340
	.align 2
.global _PPU
_PPU:
	.zero	2776
	.align 2
.global _SA1
_SA1:
	.zero	32848
	.align 2
.global _snes4all_vcounter
_snes4all_vcounter:
	.zero  1024
	.align 5
#if 0
.data
.global _EnvRateSteps
_EnvRateSteps:
	.long 0x00000000
	.long 0x00000000
	.long 0xFA000000
	.long 0x00000000
	.long 0x71F80000
	.long 0x00000009
	.long 0x71F80000
	.long 0x00000009
	.long 0xF4000000
	.long 0x00000001
	.long 0x03E80000
	.long 0x00000000
	.long 0xFA000000
	.long 0x00000000
	.long 0xD6D80000
	.long 0x00000000
	.long 0xFA000000
	.long 0x00000000
	.long 0x71F80000
	.long 0x00000009
	.align 5
#endif
