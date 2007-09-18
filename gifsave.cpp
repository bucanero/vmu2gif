/**************************************************************************
 *
 *  FILE:           GIFSAVE.C
 *
 *  MODULE OF:      GIFSAVE
 *
 *  DESCRIPTION:    Routines to create a GIF-file. See GIFSAVE.DOC for
 *                  a description . . .
 *
 *                  The functions were originally written using Borland's
 *                  C-compiler on an IBM PC -compatible computer, but they
 *                  are compiled and tested on SunOS (Unix) as well.
 *
 *  WRITTEN BY:     Sverre H. Huseby
 *                  Bjoelsengt. 17
 *                  N-0468 Oslo
 *                  Norway
 *
 *                  sverrehu@ifi.uio.no
 *
 *  LAST MODIFIED:  26/9-1992, v1.0, Sverre H. Huseby
 *                    * Version 1.0, no modifications
 *
 **************************************************************************/





#include <stdlib.h>
#include <stdio.h>

#include "gifsave.hpp"




/*-------------------------------------------------------------------------
 *
 *  NAME:           Write()
 *
 *  DESCRIPTION:    Output bytes to the current OutFile.
 *
 *  PARAMETERS:     buf - Pointer to buffer to write
 *                  len - Number of bytes to write
 *
 *  RETURNS:        GIF_OK       - OK
 *                  GIF_ERRWRITE - Error writing to the file
 *
 */
int GifSave::Write(void *buf, uint16 len)
{
    if (fwrite(buf, sizeof(uint8), len, OutFile) < len)
        return GIF_ERRWRITE;

    return GIF_OK;
}





/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteByte()
 *
 *  DESCRIPTION:    Output one byte to the current OutFile.
 *
 *  PARAMETERS:     b - Byte to write
 *
 *  RETURNS:        GIF_OK       - OK
 *                  GIF_ERRWRITE - Error writing to the file
 *
 */
int GifSave::WriteByte(uint8 b)
{
    if (putc(b, OutFile) == EOF)
        return GIF_ERRWRITE;

    return GIF_OK;
}


/*========================================================================*
 =                                                                        =
 =                      Routines to write a bit-file                      =
 =                                                                        =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME:           InitBitFile()
 *
 *  DESCRIPTION:    Initiate for using a bitfile. All output is sent to
 *                  the current OutFile using the I/O-routines above.
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        Nothing
 *
 */
void GifSave::InitBitFile(void)
{
    Buffer[Index = 0] = 0;
    BitsLeft = 8;
}





/*-------------------------------------------------------------------------
 *
 *  NAME:           ResetOutBitFile()
 *
 *  DESCRIPTION:    Tidy up after using a bitfile
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        0 - OK, -1 - error
 *
 */
int GifSave::ResetOutBitFile(void)
{
    uint8 numbytes;


    /*
     *  Find out how much is in the buffer
     */
    numbytes = Index + (BitsLeft == 8 ? 0 : 1);

    /*
     *  Write whatever is in the buffer to the file
     */
    if (numbytes) {
        if (WriteByte(numbytes+1) != GIF_OK)
            return -1;

        if (Write(Buffer, numbytes) != GIF_OK)
            return -1;

        Buffer[Index = 0] = 0;
        BitsLeft = 8;
    }

    return 0;
}





/*-------------------------------------------------------------------------
 *
 *  NAME:           WriteBits()
 *
 *  DESCRIPTION:    Put the given number of bits to the outfile.
 *
 *  PARAMETERS:     bits    - bits to write from (right justified)
 *                  numbits - number of bits to write
 *
 *  RETURNS:        bits written, or -1 on error.
 *
 */
int GifSave::WriteBits(int bits, int numbits)
{
    int  bitswritten = 0;
    uint8 numbytes = 255;


    do {
        /*
         *  If the buffer is full, write it.
         */
        if ((Index == 254 && !BitsLeft) || Index > 254) {
            if (WriteByte(numbytes) != GIF_OK)
                return -1;

            if (Write(Buffer, numbytes) != GIF_OK)
                return -1;

            Buffer[Index = 0] = 0;
            BitsLeft = 8;
        }

        /*
         *  Now take care of the two specialcases
         */
        if (numbits <= BitsLeft) {
            Buffer[Index] |= (bits & ((1 << numbits) - 1)) << (8 - BitsLeft);
            bitswritten += numbits;
            BitsLeft -= numbits;
            numbits = 0;
        } else {
            Buffer[Index] |= (bits & ((1 << BitsLeft) - 1)) << (8 - BitsLeft);
            bitswritten += BitsLeft;
            bits >>= BitsLeft;
            numbits -= BitsLeft;

            Buffer[++Index] = 0;
            BitsLeft = 8;
        }
    } while (numbits);

    return bitswritten;
}





/*========================================================================*
 =                                                                        =
 =                Routines to maintain an LZW-string table                =
 =                                                                        =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME:           FreeStrtab()
 *
 *  DESCRIPTION:    Free arrays used in string table routines
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        Nothing
 *
 */
void GifSave::FreeStrtab(void)
{
    if (StrHsh) {
        free(StrHsh);
        StrHsh = NULL;
    }

    if (StrNxt) {
        free(StrNxt);
        StrNxt = NULL;
    }

    if (StrChr) {
        free(StrChr);
        StrChr = NULL;
    }
}





/*-------------------------------------------------------------------------
 *
 *  NAME:           AllocStrtab()
 *
 *  DESCRIPTION:    Allocate arrays used in string table routines
 *
 *  PARAMETERS:     None
 *
 *  RETURNS:        GIF_OK     - OK
 *                  GIF_OUTMEM - Out of memory
 *
 */
int GifSave::AllocStrtab(void)
{
    /*
     *  Just in case . . .
     */
    FreeStrtab();

    if ((StrChr = (uint8 *) malloc(MAXSTR * sizeof(uint8))) == 0) {
        FreeStrtab();
        return GIF_OUTMEM;
    }

    if ((StrNxt = (uint16 *) malloc(MAXSTR * sizeof(uint16))) == 0) {
        FreeStrtab();
        return GIF_OUTMEM;
    }

    if ((StrHsh = (uint16 *) malloc(HASHSIZE * sizeof(uint16))) == 0) {
        FreeStrtab();
        return GIF_OUTMEM;
    }

    return GIF_OK;
}





/*-------------------------------------------------------------------------
 *
 *  NAME:           AddCharString()
 *
 *  DESCRIPTION:    Add a string consisting of the string of index plus
 *                  the byte b.
 *
 *                  If a string of length 1 is wanted, the index should
 *                  be 0xFFFF.
 *
 *  PARAMETERS:     index - Index to first part of string, or 0xFFFF is
 *                          only 1 byte is wanted
 *                  b     - Last byte in new string
 *
 *  RETURNS:        Index to new string, or 0xFFFF if no more room
 *
 */
uint16 GifSave::AddCharString(uint16 index, uint8 b)
{
    uint16 hshidx;


    /*
     *  Check if there is more room
     */
    if (NumStrings >= MAXSTR)
        return 0xFFFF;

    /*
     *  Search the string table until a free position is found
     */
    hshidx = HASH(index, b);
    while (StrHsh[hshidx] != 0xFFFF)
        hshidx = (hshidx + HASHSTEP) % HASHSIZE;

    /*
     *  Insert new string
     */
    StrHsh[hshidx] = NumStrings;
    StrChr[NumStrings] = b;
    StrNxt[NumStrings] = (index != 0xFFFF) ? index : NEXT_FIRST;

    return NumStrings++;
}





/*-------------------------------------------------------------------------
 *
 *  NAME:           FindCharString()
 *
 *  DESCRIPTION:    Find index of string consisting of the string of index
 *                  plus the byte b.
 *
 *                  If a string of length 1 is wanted, the index should
 *                  be 0xFFFF.
 *
 *  PARAMETERS:     index - Index to first part of string, or 0xFFFF is
 *                          only 1 byte is wanted
 *                  b     - Last byte in string
 *
 *  RETURNS:        Index to string, or 0xFFFF if not found
 *
 */
uint16 GifSave::FindCharString(uint16 index, uint8 b)
{
    uint16 hshidx, nxtidx;


    /*
     *  Check if index is 0xFFFF. In that case we need only
     *  return b, since all one-character strings has their
     *  bytevalue as their index
     */
    if (index == 0xFFFF)
        return b;

    /*
     *  Search the string table until the string is found, or
     *  we find HASH_FREE. In that case the string does not
     *  exist.
     */
    hshidx = HASH(index, b);
    while ((nxtidx = StrHsh[hshidx]) != 0xFFFF) {
        if (StrNxt[nxtidx] == index && StrChr[nxtidx] == b)
            return nxtidx;
        hshidx = (hshidx + HASHSTEP) % HASHSIZE;
    }

    /*
     *  No match is found
     */
    return 0xFFFF;
}





/*-------------------------------------------------------------------------
 *
 *  NAME:           ClearStrtab()
 *
 *  DESCRIPTION:    Mark the entire table as free, enter the 2**codesize
 *                  one-byte strings, and reserve the RES_CODES reserved
 *                  codes.
 *
 *  PARAMETERS:     codesize - Number of bits to encode one pixel
 *
 *  RETURNS:        Nothing
 *
 */
void GifSave::ClearStrtab(int codesize)
{
    int q, w;
    uint16 *wp;


    /*
     *  No strings currently in the table
     */
    NumStrings = 0;

    /*
     *  Mark entire hashtable as free
     */
    wp = StrHsh;
    for (q = 0; q < HASHSIZE; q++)
        *wp++ = HASH_FREE;

    /*
     *  Insert 2**codesize one-character strings, and reserved codes
     */
    w = (1 << codesize) + RES_CODES;
    for (q = 0; q < w; q++)
        AddCharString(0xFFFF, q);
}





/*========================================================================*
 =                                                                        =
 =                        LZW compression routine                         =
 =                                                                        =
 *========================================================================*/

/*-------------------------------------------------------------------------
 *
 *  NAME:           LZW_Compress()
 *
 *  DESCRIPTION:    Perform LZW compression as specified in the
 *                  GIF-standard.
 *
 *  PARAMETERS:     codesize  - Number of bits needed to represent
 *                              one pixelvalue.
 *                  inputbyte - Function that fetches each byte to compress.
 *                              Must return -1 when no more bytes.
 *
 *  RETURNS:        GIF_OK     - OK
 *                  GIF_OUTMEM - Out of memory
 *
 */
int GifSave::LZW_Compress(int codesize, uint8* inbytes, int inlength, FILE* outf)
{
    register int c;
    register uint16 index;
    int  clearcode, endofinfo, numbits, limit, errcode;
    uint16 prefix = 0xFFFF;

	OutFile=outf;
    /*
     *  Set up the given outfile
     */
    InitBitFile();

    /*
     *  Set up variables and tables
     */
    clearcode = 1 << codesize;
    endofinfo = clearcode + 1;

    numbits = codesize + 1;
    limit = (1 << numbits) - 1;

    if ((errcode = AllocStrtab()) != GIF_OK)
        return errcode;
    ClearStrtab(codesize);

    /*
     *  First send a code telling the unpacker to clear the stringtable.
     */
    WriteBits(clearcode, numbits);

    /*
     *  Pack image
     */
	for (int k=0; k<inlength; k++) {
		c=inbytes[k];
//	 while ((c = inputbyte()) != -1) {
        /*
         *  Now perform the packing.
         *  Check if the prefix + the new character is a string that
         *  exists in the table
         */
        if ((index = FindCharString(prefix, c)) != 0xFFFF) {
            /*
             *  The string exists in the table.
             *  Make this string the new prefix.
             */
            prefix = index;

        } else {
            /*
             *  The string does not exist in the table.
             *  First write code of the old prefix to the file.
             */
            WriteBits(prefix, numbits);

            /*
             *  Add the new string (the prefix + the new character)
             *  to the stringtable.
             */
            if (AddCharString(prefix, c) > limit) {
                if (++numbits > 12) {
                    WriteBits(clearcode, numbits - 1);
                    ClearStrtab(codesize);
                    numbits = codesize + 1;
                }
                limit = (1 << numbits) - 1;
            }

            /*
             *  Set prefix to a string containing only the character
             *  read. Since all possible one-character strings exists
             *  int the table, there's no need to check if it is found.
             */
            prefix = c;
        }
    }

    /*
     *  End of info is reached. Write last prefix.
     */
    if (prefix != 0xFFFF)
        WriteBits(prefix, numbits);

    /*
     *  Write end of info -mark.
     */
    WriteBits(endofinfo, numbits);

    /*
     *  Flush the buffer
     */
    ResetOutBitFile();

    /*
     *  Tidy up
     */
    FreeStrtab();

    return GIF_OK;
}

GifSave::GifSave() {
	StrChr = NULL;
	StrNxt = NULL;
	StrHsh = NULL;
}
