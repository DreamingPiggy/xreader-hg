	.set noreorder

#include "pspstub.s"

	STUB_START "xrAudioCore",0x00010011,0x00060005
	STUB_FUNC  0x129AA4DF, register_musicdrv
	STUB_FUNC  0x129AA4DF, unregister_musicdrv
	STUB_FUNC  0x481C9ADA, malloc 
	STUB_FUNC  0xAD8AF84F, free
	STUB_FUNC  0x7EF3FBEC, realloc
	STUB_FUNC  0xBA64636D, calloc
	STUB_END
