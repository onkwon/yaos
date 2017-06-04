MEMORY
{
	/*
	ram_fb (rwx)	: ORIGIN = 0x20028000, LENGTH = 160K
	ram (rwx)	: ORIGIN = 0x20000000, LENGTH = 160K
	*/
	ram (rwx)	: ORIGIN = 0x20000000, LENGTH = 320K
	rom (rx)	: ORIGIN = 0x08000000, LENGTH = 2048K
}
