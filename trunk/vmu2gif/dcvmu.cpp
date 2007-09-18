/********************************************************************
 * Dreamcast VMU File Handling foundation classes v0.2 (Dec/2005)
 * dcvmu.cpp - coded by El Bucanero
 *
 * Copyright (C) 2005 Damian Parrino <bucanero@fibertel.com.ar>
 * http://www.bucanero.com.ar/
 *
 * Greetz to:
 * Marcus Comstedt for his great info & resources					<http://mc.pp.se/dc/>
 * Dan Potter for KallistiOS and other great libs like Tsunami		<http://gamedev.allusion.net/>
 * AndrewK / Napalm 2001 for the lovely DC load-ip/tool				<http://adk.napalm-x.com/>
 * Lawrence Sebald for the MinGW/msys cross compiler tutorial		<http://ljsdcdev.sunsite.dk/>
 *
 * and last, but not least, thanks to SEGA for my ALWAYS LOVED Dreamcast! =)
 *
 ********************************************************************/

#include "dcvmu.hpp"

#include <string.h>
#include <stdio.h>

//--------------------
//	Class DreamcastFile
//--------------------

DreamcastFile::DreamcastFile() {
	strcpy(name, "");
	strcpy(path, "");
	size=0;
	data=NULL;
	game=false;
	copyProtect=false;
	memset(&time, 0x00, 0x08);
}

DreamcastFile::~DreamcastFile() {
	if(data != NULL) {
		delete data;
	}
}

void DreamcastFile::setName(char *n) {
//	pName=new char[strlen(name)];
	strncpy(name, n, 12);
}

char* DreamcastFile::getName() {
	return(name);
};

void DreamcastFile::setSize(ssize_t s) {
	size=s;
};

ssize_t DreamcastFile::getSize() {
	return(size);
};

void DreamcastFile::setTime(vmu_timestamp_t t) {
	time=t;
};

vmu_timestamp_t DreamcastFile::getTime() {
	return(time);
};

void DreamcastFile::timeToBCD(vmu_timestamp_t *tdec) {
	tdec->cent=dec_to_bcd(tdec->cent);
	tdec->year=dec_to_bcd(tdec->year);
	tdec->month=dec_to_bcd(tdec->month);
	tdec->day=dec_to_bcd(tdec->day);
	tdec->hour=dec_to_bcd(tdec->hour);
	tdec->min=dec_to_bcd(tdec->min);
	tdec->sec=dec_to_bcd(tdec->sec);
	tdec->dow=dec_to_bcd(tdec->dow);
}

void DreamcastFile::timeToDec(vmu_timestamp_t *tbcd) {
	tbcd->cent=bcd_to_dec(tbcd->cent);
	tbcd->year=bcd_to_dec(tbcd->year);
	tbcd->month=bcd_to_dec(tbcd->month);
	tbcd->day=bcd_to_dec(tbcd->day);
	tbcd->hour=bcd_to_dec(tbcd->hour);
	tbcd->min=bcd_to_dec(tbcd->min);
	tbcd->sec=bcd_to_dec(tbcd->sec);
	tbcd->dow=bcd_to_dec(tbcd->dow);
}

void DreamcastFile::setData(uint8 *d) {
/*	if(data != NULL) {
		delete data;
	}
	data=new uint8[size];
	memcpy(data, d, size);
*/
	data=d;
}

uint8* DreamcastFile::getData() {
	return(data);
}

void DreamcastFile::setGameFile(bool t) {
	game=t;
}

bool DreamcastFile::isGameFile() {
	return(game);
}

void DreamcastFile::setCopyProtected(bool cp) {
	copyProtect=cp;
}

bool DreamcastFile::isCopyProtected() {
	return(copyProtect);
}

void DreamcastFile::setPath(char *p) {
	strncpy(path, p, 63);
}

char* DreamcastFile::getPath() {
	return(path);
};

uint8 DreamcastFile::dec_to_bcd(uint8 dec) {
	uint8 rv = 0;

	rv = dec % 10;
	rv |= ((dec / 10) % 10) << 4;
	return rv;
}

uint8 DreamcastFile::bcd_to_dec(uint8 bcd) {
	uint8 rv = 0;

	rv=0x0f & bcd;
	rv+=((0xf0 & bcd) >> 4) * 10;
	return rv;
}

int DreamcastFile::readFile() {
//vmufs_write(*maple, name, data, size, flags)
	return(0);
}

int DreamcastFile::writeFile() {
//vmufs_write(*maple, name, data, size, flags)
	return(0);
}

uint16* DreamcastFile::getIconPalette() {
	if (game) {
		return((uint16*)(data+0x260));
	} else {
		return((uint16*)(data+0x60));
	}
}

uint8* DreamcastFile::getIconBitmap(uint8 i) {
	if (game) {
		return(data+0x280+(i*512));
	} else {
		return(data+0x80+(i*512));
	}
}

uint8 DreamcastFile::getIcons() {
	if (game) {
		return(*(data+0x240));
	} else {
		return(*(data+0x40));
	}
}

uint8 DreamcastFile::getAnimationSpeed() {
	return(*(data+0x42));
}

void DreamcastFile::loadHeader(vmu_dir_t *header) {
	game=(header->filetype == 0xCC);
	copyProtect=(header->copyprotect == 0xFF);
	setName(header->filename);
	time=header->timestamp;
	size=header->filesize*512;
}

void DreamcastFile::buildHeader(vmu_dir_t *header) {
	memset(header, 0x00, 0x20);
	header->filetype=game ? 0xCC : 0x33;
	header->copyprotect=copyProtect ? 0xFF : 0x00;
	strcpy(header->filename, name);
	header->timestamp=time;
	header->filesize=size/512;
}

//--------------------
//	Class VirtualFile
//--------------------

VirtualFile::VirtualFile() {
	dcf=NULL;
	strcpy(filename, "");
}

VirtualFile::~VirtualFile() {
	if(dcf != NULL) {
		delete dcf;
	}
}

void VirtualFile::setFileName(char *fn) {
	strcpy(filename, fn);
}

char* VirtualFile::getFileName() {
	return(filename);
};

DreamcastFile* VirtualFile::getDCFile() {
	return(dcf);
};

void VirtualFile::setDCFile(DreamcastFile *df) {
	dcf=df;
};

int VirtualFile::readFile(char *fn) {
	setFileName(fn);
	return(readData());
};

int VirtualFile::writeFile(char *fn) {
	setFileName(fn);
	return(writeData());
};

//--------------------
//	Class DCIFile
//--------------------

DCIFile::DCIFile() {
}

DCIFile::DCIFile(char *fn) {
	setFileName(fn);
}

DCIFile::~DCIFile() {
//	this->VirtualFile::~VirtualFile();
}

int DCIFile::readData() {
	file_t f;
	uint8 *raw;
	vmu_dir_t header;
	DreamcastFile *df;
	
	df=getDCFile();
	if(df == NULL) {
		df=new DreamcastFile();
		setDCFile(df);
	}
	if (!(f=fs_open(getFileName(), O_RDONLY))) {
		printf("ERROR: Can't open %s!\n", getFileName());
		return(-1);
	}
	fs_read(f, &header, 0x20);
	df->loadHeader(&header);
//	raw=(uint8*)malloc(df->getSize());
	raw=new uint8[df->getSize()];
	fs_read(f, raw, df->getSize());
	df->setData(swapData(raw));
//	free(raw);
	fs_close(f);
	return(0);
}

int DCIFile::writeData() {
	file_t f;
	vmu_dir_t header;
	DreamcastFile *df;

	df=getDCFile();
	if (!(f=fs_open(getFileName(), "wb"))) {
		printf("ERROR: Can't open %s!\n", getFileName());
		return(-1);
	}
	df->buildHeader(&header);
	fs_write(f, &header, 0x20);
	fs_write(f, swapData(df->getData()), df->getSize());
	fs_close(f);
	swapData(df->getData());
	return(0);
}

uint8* DCIFile::swapData(uint8 *raw) {
	uint8 tmp[4];

	for (int i=0; i<getDCFile()->getSize(); i+=4) {
		tmp[3]=raw[i];
		tmp[2]=raw[i+1];
		tmp[1]=raw[i+2];
		tmp[0]=raw[i+3];
		memcpy(raw+i, tmp, 4);
	}
	return(raw);
}

//--------------------
//	Class VMIFile
//--------------------

VMIFile::VMIFile() {
	strcpy(description, "");
	strcpy(copyright, "");
	strcpy(resource_name, "");
}

VMIFile::VMIFile(char *fn, char *ds, char *cr, char *rn) {
	setFileName(fn);
	setDescription(ds);
	setCopyright(cr);
	setResourceName(rn);
}

VMIFile::~VMIFile() {
//	this->VirtualFile::~VirtualFile();
}

void VMIFile::setResourceName(char *rn) {
	strncpy(resource_name, rn, 8);
}

char* VMIFile::getResourceName() {
	return(resource_name);
};

void VMIFile::setCopyright(char *c) {
	strncpy(copyright, c, 32);
}

char* VMIFile::getCopyright() {
	return(copyright);
};

void VMIFile::setDescription(char *d) {
	strncpy(description, d, 32);
}

char* VMIFile::getDescription() {
	return(description);
};

void VMIFile::loadHeader(vmi_hdr_t *header) {
	DreamcastFile *df;
	uint16 year;

	df=getDCFile();
	setDescription(header->description);
	setCopyright(header->copyright);
	setResourceName(header->resource_name);
	df->setSize(header->filesize);
	df->setName(header->filename);
	df->setGameFile(header->filemode & VMI_VMUGAME);
	df->setCopyProtected(header->filemode & VMI_NOCOPY);
	memcpy(&year, &header->timestamp.cent, 2);
	header->timestamp.cent=year / 100;
	header->timestamp.year=year % 100;
	header->timestamp.dow=(header->timestamp.dow == 0) ? 6 : header->timestamp.dow-1;
	df->timeToBCD(&header->timestamp);
	df->setTime(header->timestamp);
}

int VMIFile::readData() {
	file_t f;
	vmi_hdr_t header;
	uint8 *raw;
	char tmp[13];
	DreamcastFile *df;
	
	df=getDCFile();
	if(df == NULL) {
		df=new DreamcastFile();
		setDCFile(df);
	}
	if (!(f=fs_open(getFileName(), O_RDONLY))) {
		printf("ERROR: Can't open %s!\n", getFileName());
		return(-1);
	}
	fs_read(f, &header, 0x6C);
	fs_close(f);
	loadHeader(&header);
	sprintf(tmp, "%s.VMS", getResourceName());
	if (!(f=fs_open(tmp, O_RDONLY))) {
		printf("ERROR: Can't open %s!\n", tmp);
		return(-2);
	}
//	raw=(uint8*)malloc(df->getSize());
	raw=new uint8[df->getSize()];
	fs_read(f, raw, df->getSize());
	df->setData(raw);
//	free(raw);
	fs_close(f);
	return(0);
}

void VMIFile::buildHeader(vmi_hdr_t *header) {
	DreamcastFile *df;
	uint16 year;
	
	df=getDCFile();
	memset(header, 0x00, 0x6C);
	strcpy(header->description, getDescription());
	strcpy(header->copyright, getCopyright());
	strcpy(header->resource_name, getResourceName());
	header->checksum[0]=header->resource_name[0] & 'S';
	header->checksum[1]=header->resource_name[1] & 'E';
	header->checksum[2]=header->resource_name[2] & 'G';
	header->checksum[3]=header->resource_name[3] & 'A';
	header->timestamp=df->getTime();
	df->timeToDec(&header->timestamp);
	year=header->timestamp.cent*100 + header->timestamp.year;
	memcpy(&header->timestamp.cent, &year, 2);
	header->timestamp.dow=(header->timestamp.dow == 6) ? 0 : header->timestamp.dow+1;
	header->filenumber=0x01;
	strcpy(header->filename, df->getName());
	header->filemode=((df->isGameFile()) ? VMI_VMUGAME : 0x00) | ((df->isCopyProtected()) ? VMI_NOCOPY : 0x00);
	header->filesize=df->getSize();
}

int VMIFile::writeData() {
	file_t f;
	vmi_hdr_t header;
	DreamcastFile *df;
	char tmp[13];
	
	df=getDCFile();
	if (!(f=fs_open(getFileName(), "wb"))) {
		printf("ERROR: Can't open %s!\n", getFileName());
		return(-1);
	}
	buildHeader(&header);
	fs_write(f, &header, 0x6C);
	fs_close(f);
	sprintf(tmp, "%s.VMS", getResourceName());
	if (!(f=fs_open(tmp, "wb"))) {
		printf("ERROR: Can't open %s!\n", tmp);
		return(-2);
	}
	fs_write(f, df->getData(), df->getSize());
	fs_close(f);
	return(0);
}
