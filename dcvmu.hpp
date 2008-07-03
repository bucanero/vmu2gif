// $Id$

#define ssize_t unsigned int
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned long
#define file_t FILE*
#define O_RDONLY "rb"
#define fs_open fopen
#define fs_seek fseek
#define fs_close fclose
#define fs_read(arc, var, len) fread(var, len, 1, arc)
#define fs_write(arc, var, len) fwrite(var, len, 1, arc)

#include "vmufs.h"

typedef struct vmi_hdr {
	char	checksum[4];
	char	description[32];
	char	copyright[32];
	vmu_timestamp_t timestamp;
	uint16	version;
	uint16	filenumber;
	char	resource_name[8];
	char	filename[12];
	uint16	filemode;
	uint16	padding;
	uint32	filesize;
} vmi_hdr_t;

#define VMI_NOCOPY 1
#define VMI_VMUGAME 2

class DreamcastFile {

private:
	char name[13];
	char path[64];
	ssize_t size;
	vmu_timestamp_t time;
	uint8 *data;
	bool game;			// 0: DATA				- 1: GAME
	bool copyProtect;	// 0: NOT READ ONLY		- 1: READ ONLY
	uint8 dec_to_bcd(uint8 dec);
	uint8 bcd_to_dec(uint8 bcd);

public:
	DreamcastFile();
	~DreamcastFile();
	void setName(char *n);
	char* getName();
	void setSize(ssize_t s);
	ssize_t getSize();
	void setTime(vmu_timestamp_t t);
	vmu_timestamp_t getTime();
	void setData(uint8 *d);
	uint8* getData();
	void setGameFile(bool t);
	bool isGameFile();
	void setCopyProtected(bool cp);
	bool isCopyProtected();
	void setPath(char *p);
	char* getPath();
	int readFile();
	int writeFile();
	void timeToBCD(vmu_timestamp_t *tdec);
	void timeToDec(vmu_timestamp_t *tbcd);
	uint16* getIconPalette();
	uint8* getIconBitmap(uint8 i);
	uint8 getIcons();
	uint8 getAnimationSpeed();
	void loadHeader(vmu_dir_t *header);
	void buildHeader(vmu_dir_t *header);
};

class VirtualFile {

private:
	DreamcastFile *dcf;
	char filename[256];

public:
	VirtualFile();
	~VirtualFile();
	void setFileName(char *fn);
	char* getFileName();
	void setDCFile(DreamcastFile *df);
	DreamcastFile* getDCFile();
	int readFile(char *fn);
	int writeFile(char *fn);
	virtual int readData() =0;
	virtual int writeData() =0;
};

class DCIFile : public VirtualFile {

private:
	uint8* swapData(uint8 *raw);
	
public:
	DCIFile();
	DCIFile(char *fn);
	~DCIFile();
	int readData();
	int writeData();
};

class VMIFile : public VirtualFile {

private:
	char description[33];
	char copyright[33];
	char resource_name[9];
	
public:
	VMIFile();
	VMIFile(char *fn, char *ds, char *cr, char *rn);
	~VMIFile();
	void setResourceName(char *rn);
	char* getResourceName();
	void setDescription(char *d);
	char* getDescription();
	void setCopyright(char *c);
	char* getCopyright();
	int readData();
	int writeData();
	void loadHeader(vmi_hdr_t *header);
	void buildHeader(vmi_hdr_t *header);
};
