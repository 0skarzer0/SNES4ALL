.text
.little
.section        .bss
#ifndef USE_MMU
	.align 5
#else
	.align 12
#endif
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
.global _APURegisters
_APURegisters:
	.rept (8/4)
	.long 0
	.endr
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
.global _videogl_cache_reseted
_videogl_cache_reseted: .long 1
.global _videogl_cache_to_reseted
_videogl_cache_to_reseted: .long 0
.global _videogl_cache
_videogl_cache: .long 0
.global _videogl_tiles
_videogl_tiles: .long 0
.global _videogl_first_brightness
_videogl_first_brightness: .byte 0
.global _videogl_brightness
_videogl_brightness: .byte 0
.global _videogl_min_z
_videogl_min_z: .byte 255
.global _videogl_max_z
_videogl_max_z: .byte 0
.global _videogl_hw_render
_videogl_hw_render: .long -1
.global _videogl_changed_colors
_videogl_changed_colors: .long 0
.global _videogl_changed_colors_back
_videogl_changed_colors_back: .long 0
.global _videogl_clip
_videogl_clip: .long 0
.global _videogl_clip_enabled
_videogl_clip_enabled: .long 0
.global _videogl_used_screen
_videogl_used_screen: .long 0
.global _videogl_used_addfix
_videogl_used_addfix: .long 0
.global _videogl_addfix_x0
_videogl_addfix_x0: .long 0
.global _videogl_addfix_y0
_videogl_addfix_y0: .long 256
.global _videogl_addfix_x1
_videogl_addfix_x1: .long 256
.global _videogl_addfix_y1
_videogl_addfix_y1: .long 0
#if 0
.global _videogl_used_sub
_videogl_used_sub: .long 0
.global _videogl_sub_color
_videogl_sub_color: .short 0x8000, 0
.global _videogl_sub_x0
_videogl_sub_x0: .long 0
.global _videogl_sub_y0
_videogl_sub_y0: .long 256
.global _videogl_sub_x1
_videogl_sub_x1: .long 256
.global _videogl_sub_y1
_videogl_sub_y1: .long 0
#endif
.global _videogl_8bit_tiles
_videogl_8bit_tiles: .long 0
.global _videogl_screen_endY
_videogl_screen_endY: .long 0
.global _videogl_screen_buffer
_videogl_screen_buffer: .long 0
.global _videogl_screen_texture
_videogl_screen_texture: .long 0
.global _videogl_screen
_videogl_screen: .long 0
.global _videogl_z
_videogl_z: .float 0
.global _videogl_fps_enabled
_videogl_fps_enabled: .long 0
.global _videogl_fps_buffer
_videogl_fps_buffer: .long 0
.global _videogl_info_enabled
_videogl_info_enabled: .long 0
.global _videogl_info_buffer
_videogl_info_buffer: .long 0
.global _videogl_black_buffer
_videogl_black_buffer: .long 0
.global _videogl_background_buffer
_videogl_background_buffer: .long 0
.global _videogl_back_no_tiles
_videogl_back_no_tiles: .long 0
.global _videogl_addfix_buffer
_videogl_addfix_buffer: .long 0
.global _videogl_sub_buffer
_videogl_sub_buffer: .long 0
.global _videogl_changed_color
_videogl_changed_color: .zero 256
.global _videogl_actual_color
_videogl_actual_color: .zero 512
.global _videogl_tiles_z
_videogl_tiles_z: .zero 512
.global _videogl_cached
_videogl_cached: .zero (VIDEO_GL_MAX_TEX * 2)
.global _videogl_background_color
_videogl_background_color: .zero (VIDEO_GL_RASTER * 2)
.global _videogl_addfix_color
_videogl_addfix_color: .zero (VIDEO_GL_RASTER * 2)
#ifndef USE_VIDEOGL_DIRECT_SLOT
.global _videogl_cache_slot_last
_videogl_cache_slot_last: .long 0
.global _videogl_cache_slot
_videogl_cache_slot: .zero (VIDEO_GL_MAX_TEX * 2)
.global _videogl_cache_tile_slot
_videogl_cache_tile_slot: .zero (VIDEO_GL_MAX_TEX * 2)
#endif
#ifdef USE_VIDEOGL_SELECTIVE_CACHE_RESET
.global _videogl_selective
_videogl_selective: .zero 512
.global _videogl_selective_cache
_videogl_selective_cache: .zero ((VIDEO_GL_MAX_TEX / 16) * 512)
#endif
.global _videogl_tile
_videogl_tile: .zero (VIDEO_GL_MAX_TEX * 12)

	.align 5
