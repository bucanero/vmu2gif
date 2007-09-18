#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned long

#define RES_CODES 2

#define HASH_FREE 0xFFFF
#define NEXT_FIRST 0xFFFF

#define MAXBITS 12
#define MAXSTR (1 << MAXBITS)

#define HASHSIZE 9973
#define HASHSTEP 2039

#define HASH(index, lastbyte) (((lastbyte << 8) ^ index) % HASHSIZE)

enum GIF_Code {
    GIF_OK,
    GIF_ERRCREATE,
    GIF_ERRWRITE,
    GIF_OUTMEM
};

class GifSave {

private:
	uint8 Buffer[256];
	int Index, BitsLeft;
	uint8 *StrChr;
	uint16 *StrNxt, *StrHsh, NumStrings;
	FILE *OutFile;

	int Write(void *buf, uint16 len);
	int WriteByte(uint8 b);
	void InitBitFile(void);
	int ResetOutBitFile(void);
	int WriteBits(int bits, int numbits);
	void FreeStrtab(void);
	int AllocStrtab(void);
	uint16 AddCharString(uint16 index, uint8 b);
	uint16 FindCharString(uint16 index, uint8 b);
	void ClearStrtab(int codesize);
	
public:
	GifSave();
	int LZW_Compress(int codesize, uint8 *inbytes, int inlength, FILE *outf);
};
