// $Id$
/********************************************************************
 * VMU 2 GIF v1.0 (Dec/2005)
 * coded by El Bucanero
 *
 * Copyright (C) 2005 Damian Parrino <bucanero@fibertel.com.ar>
 * http://www.bucanero.com.ar/
 *
 * Greetz to:
 * Marcus Comstedt for his great info & resources					<http://mc.pp.se/dc/>
 * Dan Potter for KallistiOS and other great libs like Tsunami		<http://gamedev.allusion.net/>
 * AndrewK / Napalm 2001 for the lovely DC load-ip/tool				<http://adk.napalm-x.com/>
 * Lawrence Sebald for the MinGW/msys cross compiler tutorial		<http://ljsdcdev.sunsite.dk/>
 * Sverre H. Huseby for the GIF save routines					<sverrehu@ifi.uio.no>
 *
 * and last, but not least, thanks to SEGA for my ALWAYS LOVED Dreamcast! =)
 *
 ********************************************************************/

#include <string.h>
#include <stdio.h>

#include "gifsave.hpp"
#include "dcvmu.hpp"

//--------------------
//	Main functions
//--------------------

char* decodeMonth(uint8 m) {
	switch(m) {
		case 0x01: return("Jan");
		case 0x02: return("Feb");
		case 0x03: return("Mar");
		case 0x04: return("Apr");
		case 0x05: return("May");
		case 0x06: return("Jun");
		case 0x07: return("Jul");
		case 0x08: return("Aug");
		case 0x09: return("Sep");
		case 0x10: return("Oct");
		case 0x11: return("Nov");
		default: return("Dec");
	}
}

void printFileInfo(DreamcastFile *df) {
	vmu_timestamp_t ts;
	
	ts=df->getTime();
	printf(" [i] Name: %s\n", df->getName());
	printf(" [i] Size: %d bytes\n", df->getSize());
	printf(" [i] Type: %s File\n", ((df->isGameFile()) ? "Game" : "Data"));
	printf(" [i] Date: %02x %s %02x%02x\n", ts.day, decodeMonth(ts.month), ts.cent, ts.year);
	printf(" [i] Time: %02x:%02x:%02x\n", ts.hour, ts.min, ts.sec);
	printf(" [i] Copy Protected: %s\n", ((df->isCopyProtected()) ? "Yes" : "No"));
}

int writeIconGif(char *fn, VirtualFile *src) {
	uint8 hdr[13]={0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x20, 0x00, 0x20, 0x00, 0xB3, 0x00, 0x00};
	uint8 ani[19]={0x21, 0xF9, 0x04, 0x00, 0x33, 0x00, 0x07, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x04};
	uint8 web[19]={0x21, 0xFF, 0x0B, 0x4E, 0x45, 0x54, 0x53, 0x43, 0x41, 0x50, 0x45, 0x32, 0x2E, 0x30, 0x03, 0x01, 0x00, 0x00, 0x00};
	char fname[17];
	uint8 x, y, z;
	uint8 tmp[1024];
	uint8 *icon;
	uint16 *pal;
	file_t f;
	GifSave *gif;
	DreamcastFile *df;

	printf(" [+] Reading %s ... ", src->getFileName());
	if (src->readData() != 0) {
		printf(" [-] Source file %s could not be readed.\n", src->getFileName());
		return(-1);
	}
	printf("OK!\n");
	df=src->getDCFile();
	printFileInfo(df);
	if (fn == NULL) {
		sprintf(fname, "%s.GIF", df->getName());
	} else {
		strcpy(fname, fn);
	}
	printf(" [+] Writing %s ... ", fname);
	if (!(f=fs_open(fname, "wb"))) {
		printf("ERROR: Can't open %s!\n", fname);
		printf(" [-] Destination file %s could not be written.\n", fname);
		return(-2);
	}
	fs_write(f, hdr, 13);
	pal=df->getIconPalette();
	for (x=0; x<16; x++) {
		tmp[x*3]=((pal[x] & 0x0f00) >> 8) *17;
		tmp[x*3 + 1]=((pal[x] & 0x00f0) >> 4) *17;
		tmp[x*3 + 2]=(pal[x] & 0x000f) *17;
	}
	fs_write(f, tmp, 48);
	fs_write(f, web, 19);
	ani[4]=df->getAnimationSpeed()*4;
	gif=new GifSave();
	for (z=0; z<df->getIcons(); z++) {	
		icon=df->getIconBitmap(z);
		fs_write(f, ani, 19);
		for (y=0; y<32; y++) {
			for (x=0; x<32; x+=2) {
				tmp[y*32 + x]=(icon[y*16 + x/2] & 0xf0) >> 4;
				tmp[y*32 + x+1]=(icon[y*16 + x/2] & 0x0f);
			}
		}
		gif->LZW_Compress(4, tmp, 1024, f);
		fs_write(f, "\0\0", 2);
	}
	delete gif;
	fs_write(f, ";", 1);
	fs_close(f);
	printf("OK!\n");
	return(0);
}

int main(int argc, char **argv) {
	VirtualFile *src;
	
	printf("\n VMU 2 GIF Tool v1.0\n El Bucanero - www.bucanero.com.ar\n\n");
	argc--;
	if ((argc != 2) && (argc != 1)) {
		printf("usage: vmu2gif <source file> [destination file]\n\n");
		printf("about:\n This tool extracts icon images from VMU files.\n Supported source formats are .DCI and .VMI/VMS.\n\n");
		printf("NOTE:\n Destination file name is optional.\n\n");
		printf("examples:\n vmu2gif SONIC.VMI\n");
		printf(" vmu2gif SHENMUE.DCI SHENICON.GIF\n");
		return(0);
	}
	if (strstr(argv[1], ".DCI") || strstr(argv[1], ".dci")) {
		src=new DCIFile(argv[1]);
	} else if (strstr(argv[1], ".VMI") || strstr(argv[1], ".vmi")) {
		src=new VMIFile(argv[1], "", "", "");
	} else {
		printf(" [-] ERROR: Invalid source file: <%s>.\n", argv[1]);
		printf(" [-] File extension not recognized.\n");
		printf(" [-] Valid extensions are: .VMI .vmi .DCI .dci.\n");
		return(-1);
	}
	if (writeIconGif(argv[2], src) != 0) {
		printf(" [-] Failed!\n");
	} else {
		printf(" [+] All done!\n");
	}
	delete src;
	return(0);
}
