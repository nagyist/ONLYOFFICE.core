//========================================================================
//
// Stream.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include "gmem.h"
#include "gmempp.h"
#include "gfile.h"
#if MULTITHREADED
#include "GMutex.h"
#endif
#include "config.h"
#include "Error.h"
#include "Object.h"
#include "Lexer.h"
#include "GfxState.h"
#include "Stream.h"
#include "JBIG2Stream.h"
#include "JPXStream.h"
#include "Stream-CCITT.h"

#ifdef USE_EXTERNAL_JPEG2000
#include "../../SrcReader/JPXStream2.h"
#endif

#ifdef __DJGPP__
static GBool setDJSYSFLAGS = gFalse;
#endif

#ifdef VMS
#ifdef __GNUC__
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
#endif

//------------------------------------------------------------------------
// Stream (base class)
//------------------------------------------------------------------------

Stream::Stream() {
}

Stream::~Stream() {
}

void Stream::close() {
}

int Stream::getRawChar() {
  error(errInternal, -1, "Called getRawChar() on non-predictor stream");
  return EOF;
}

int Stream::getBlock(char *buf, int size) {
  int n, c;

  n = 0;
  while (n < size) {
    if ((c = getChar()) == EOF) {
      break;
    }
    buf[n++] = (char)c;
  }
  return n;
}

char *Stream::getLine(char *buf, int size) {
  int i;
  int c;

  if (lookChar() == EOF || size < 0)
    return NULL;
  for (i = 0; i < size - 1; ++i) {
    c = getChar();
    if (c == EOF || c == '\n')
      break;
    if (c == '\r') {
      if ((c = lookChar()) == '\n')
	getChar();
      break;
    }
    buf[i] = (char)c;
  }
  buf[i] = '\0';
  return buf;
}

Guint Stream::discardChars(Guint n) {
  char buf[4096];
  Guint count, i, j;

  count = 0;
  while (count < n) {
    if ((i = n - count) > sizeof(buf)) {
      i = (Guint)sizeof(buf);
    }
    j = (Guint)getBlock(buf, (int)i);
    count += j;
    if (j != i) {
      break;
    }
  }
  return count;
}

GString *Stream::getPSFilter(int psLevel, const char *indent,
			     GBool okToReadStream) {
  return new GString();
}

Stream *Stream::addFilters(Object *dict, int recursion) {
  Object obj, obj2;
  Object params, params2;
  Stream *str;
  int i;

  str = this;
  dict->dictLookup("Filter", &obj, recursion);
  if (obj.isNull()) {
    obj.free();
    dict->dictLookup("F", &obj, recursion);
  }
  dict->dictLookup("DecodeParms", &params, recursion);
  if (params.isNull()) {
    params.free();
    dict->dictLookup("DP", &params, recursion);
  }
  if (obj.isName()) {
    str = makeFilter(obj.getName(), str, &params, recursion);
  } else if (obj.isArray()) {
    for (i = 0; i < obj.arrayGetLength(); ++i) {
      obj.arrayGet(i, &obj2, recursion);
      if (params.isArray())
	params.arrayGet(i, &params2, recursion);
      else
	params2.initNull();
      if (obj2.isName()) {
	str = makeFilter(obj2.getName(), str, &params2, recursion);
      } else {
	error(errSyntaxError, getPos(), "Bad filter name");
	str = new EOFStream(str);
      }
      obj2.free();
      params2.free();
    }
  } else if (!obj.isNull()) {
    error(errSyntaxError, getPos(), "Bad 'Filter' attribute in stream");
  }
  obj.free();
  params.free();

  return str;
}

Stream *Stream::makeFilter(char *name, Stream *str, Object *params,
			   int recursion) {
  int pred;			// parameters
  int colors;
  int bits;
  int early;
  int encoding;
  GBool endOfLine, byteAlign, endOfBlock, black;
  int columns, rows;
  int colorXform;
  Object globals, obj;

  if (!strcmp(name, "ASCIIHexDecode") || !strcmp(name, "AHx")) {
    str = new ASCIIHexStream(str);
  } else if (!strcmp(name, "ASCII85Decode") || !strcmp(name, "A85")) {
    str = new ASCII85Stream(str);
  } else if (!strcmp(name, "LZWDecode") || !strcmp(name, "LZW")) {
    pred = 1;
    columns = 1;
    colors = 1;
    bits = 8;
    early = 1;
    if (params->isDict()) {
      params->dictLookup("Predictor", &obj, recursion);
      if (obj.isInt())
	pred = obj.getInt();
      obj.free();
      params->dictLookup("Columns", &obj, recursion);
      if (obj.isInt())
	columns = obj.getInt();
      obj.free();
      params->dictLookup("Colors", &obj, recursion);
      if (obj.isInt())
	colors = obj.getInt();
      obj.free();
      params->dictLookup("BitsPerComponent", &obj, recursion);
      if (obj.isInt())
	bits = obj.getInt();
      obj.free();
      params->dictLookup("EarlyChange", &obj, recursion);
      if (obj.isInt())
	early = obj.getInt();
      obj.free();
    }
    str = new LZWStream(str, pred, columns, colors, bits, early);
  } else if (!strcmp(name, "RunLengthDecode") || !strcmp(name, "RL")) {
    str = new RunLengthStream(str);
  } else if (!strcmp(name, "CCITTFaxDecode") || !strcmp(name, "CCF")) {
    encoding = 0;
    endOfLine = gFalse;
    byteAlign = gFalse;
    columns = 1728;
    rows = 0;
    endOfBlock = gTrue;
    black = gFalse;
    if (params->isDict()) {
      params->dictLookup("K", &obj, recursion);
      if (obj.isInt()) {
	encoding = obj.getInt();
      }
      obj.free();
      params->dictLookup("EndOfLine", &obj, recursion);
      if (obj.isBool()) {
	endOfLine = obj.getBool();
      }
      obj.free();
      params->dictLookup("EncodedByteAlign", &obj, recursion);
      if (obj.isBool()) {
	byteAlign = obj.getBool();
      }
      obj.free();
      params->dictLookup("Columns", &obj, recursion);
      if (obj.isInt()) {
	columns = obj.getInt();
      }
      obj.free();
      params->dictLookup("Rows", &obj, recursion);
      if (obj.isInt()) {
	rows = obj.getInt();
      }
      obj.free();
      params->dictLookup("EndOfBlock", &obj, recursion);
      if (obj.isBool()) {
	endOfBlock = obj.getBool();
      }
      obj.free();
      params->dictLookup("BlackIs1", &obj, recursion);
      if (obj.isBool()) {
	black = obj.getBool();
      }
      obj.free();
    }
    str = new CCITTFaxStream(str, encoding, endOfLine, byteAlign,
			     columns, rows, endOfBlock, black);
  } else if (!strcmp(name, "DCTDecode") || !strcmp(name, "DCT")) {
    colorXform = -1;
    if (params->isDict()) {
      if (params->dictLookup("ColorTransform", &obj, recursion)->isInt()) {
	colorXform = obj.getInt();
      }
      obj.free();
    }
    str = new DCTStream(str, colorXform);
  } else if (!strcmp(name, "FlateDecode") || !strcmp(name, "Fl")) {
    pred = 1;
    columns = 1;
    colors = 1;
    bits = 8;
    if (params->isDict()) {
      params->dictLookup("Predictor", &obj, recursion);
      if (obj.isInt())
	pred = obj.getInt();
      obj.free();
      params->dictLookup("Columns", &obj, recursion);
      if (obj.isInt())
	columns = obj.getInt();
      obj.free();
      params->dictLookup("Colors", &obj, recursion);
      if (obj.isInt())
	colors = obj.getInt();
      obj.free();
      params->dictLookup("BitsPerComponent", &obj, recursion);
      if (obj.isInt())
	bits = obj.getInt();
      obj.free();
    }
    str = new FlateStream(str, pred, columns, colors, bits);
  } else if (!strcmp(name, "JBIG2Decode")) {
    if (params->isDict()) {
      params->dictLookup("JBIG2Globals", &globals, recursion);
    }
    str = new JBIG2Stream(str, &globals);
    globals.free();
  } else if (!strcmp(name, "JPXDecode")) {
#ifdef USE_EXTERNAL_JPEG2000
    // check_j2000_type, openjpeg support
    str->reset();
    bool bOpenJpegSupport = false;
    unsigned char pBuffer[32] = {0};
    for (int i = 0; i < 32; ++i)
    {
      int nCurrentChar = str->getChar();
      if (EOF == nCurrentChar)
        break;
      pBuffer[i] = nCurrentChar;
    }
    if (0x00 == pBuffer[0]  && 0x00 == pBuffer[1]  && 0x00 == pBuffer[2]  && 0x0c == pBuffer[3]
     && 0x6a == pBuffer[4]  && 0x50 == pBuffer[5]  && 0x20 == pBuffer[6]  && 0x20 == pBuffer[7]
     && 0x0d == pBuffer[8]  && 0x0a == pBuffer[9]  && 0x87 == pBuffer[10] && 0x0a == pBuffer[11]
     && 0x00 == pBuffer[12] && 0x00 == pBuffer[13] && 0x00 == pBuffer[14]
     && 0x66 == pBuffer[16] && 0x74 == pBuffer[17] && 0x79 == pBuffer[18] && 0x70 == pBuffer[19]
     && 0x6a == pBuffer[20] && 0x70 == pBuffer[21] && 0x32 == pBuffer[22] && 0x20 == pBuffer[23]
     && 0x00 == pBuffer[24] && 0x00 == pBuffer[25] && 0x00 == pBuffer[26] && 0x00 == pBuffer[27]) {
        bOpenJpegSupport = true; // JP2
    } else if (0xff == pBuffer[0] && 0x4f == pBuffer[1] && 0xff == pBuffer[2] && 0x51 == pBuffer[3]) {
        bOpenJpegSupport = true; // J2K
    } else if (0x00 == pBuffer[0]  && 0x00 == pBuffer[1]  && 0x00 == pBuffer[2]  && 0x0c == pBuffer[3]
            && 0x6a == pBuffer[4]  && 0x50 == pBuffer[5]  && 0x20 == pBuffer[6]  && 0x20 == pBuffer[7]
            && 0x0d == pBuffer[8]  && 0x0a == pBuffer[9]  && 0x87 == pBuffer[10] && 0x0a == pBuffer[11]
            && 0x00 == pBuffer[12] && 0x00 == pBuffer[13] && 0x00 == pBuffer[14] && 0x18 == pBuffer[15]
            && 0x66 == pBuffer[16] && 0x74 == pBuffer[17] && 0x79 == pBuffer[18] && 0x70 == pBuffer[19]
            && 0x6d == pBuffer[20] && 0x6a == pBuffer[21] && 0x70 == pBuffer[22] && 0x32 == pBuffer[23]
            && 0x00 == pBuffer[24] && 0x00 == pBuffer[25] && 0x00 == pBuffer[26] && 0x00 == pBuffer[27]
            && 0x6d == pBuffer[28] && 0x6a == pBuffer[29] && 0x70 == pBuffer[30] && 0x32 == pBuffer[31]) {
        bOpenJpegSupport = true; // JPT
    }
    str->reset();
    if (bOpenJpegSupport)
      str = new JPXStream2(str);
    else
      str = new JPXStream(str);
#else
    str = new JPXStream(str);
#endif
  } else {
    error(errSyntaxError, getPos(), "Unknown filter '{0:s}'", name);
    str = new EOFStream(str);
  }
  return str;
}

//------------------------------------------------------------------------
// BaseStream
//------------------------------------------------------------------------

BaseStream::BaseStream(Object *dictA) {
  dict = *dictA;
}

BaseStream::~BaseStream() {
  dict.free();
}

//------------------------------------------------------------------------
// FilterStream
//------------------------------------------------------------------------

FilterStream::FilterStream(Stream *strA) {
  str = strA;
}

FilterStream::~FilterStream() {
}

void FilterStream::close() {
  str->close();
}

void FilterStream::setPos(GFileOffset pos, int dir) {
  error(errInternal, -1, "Called setPos() on FilterStream");
}

//------------------------------------------------------------------------
// ImageStream
//------------------------------------------------------------------------

ImageStream::ImageStream(Stream *strA, int widthA, int nCompsA, int nBitsA) {
  int imgLineSize;

  str = strA;
  width = widthA;
  nComps = nCompsA;
  nBits = nBitsA;

  nVals = width * nComps;
  inputLineSize = (nVals * nBits + 7) >> 3;
  if (width > INT_MAX / nComps ||
      nVals > (INT_MAX - 7) / nBits) {
    // force a call to gmallocn(-1,...), which will throw an exception
    inputLineSize = -1;
  }
  inputLine = (char *)gmallocn(inputLineSize, sizeof(char));
  if (nBits == 8) {
    imgLine = (Guchar *)inputLine;
  } else {
    if (nBits == 1) {
      imgLineSize = (nVals + 7) & ~7;
    } else {
      imgLineSize = nVals;
    }
    imgLine = (Guchar *)gmallocn(imgLineSize, sizeof(Guchar));
  }
  imgIdx = nVals;
}

ImageStream::~ImageStream() {
  if (imgLine != (Guchar *)inputLine) {
    gfree(imgLine);
  }
  gfree(inputLine);
}

void ImageStream::reset() {
  str->reset();
}

void ImageStream::close() {
  str->close();
}

GBool ImageStream::getPixel(Guchar *pix) {
  int i;

  if (imgIdx >= nVals) {
    if (!getLine()) {
      return gFalse;
    }
    imgIdx = 0;
  }
  for (i = 0; i < nComps; ++i) {
    pix[i] = imgLine[imgIdx++];
  }
  return gTrue;
}

int ImageStream::getVals()
{
    return nVals;
}
int ImageStream::getComps()
{
    return nComps;
}

Guchar *ImageStream::getLine() {
  Gulong buf, bitMask;
  int bits;
  int c;
  int i;
  char *p;

  if (str->getBlock(inputLine, inputLineSize) != inputLineSize) {
    return NULL;
  }
  if (nBits == 1) {
    p = inputLine;
    for (i = 0; i < nVals; i += 8) {
      c = *p++;
      imgLine[i+0] = (Guchar)((c >> 7) & 1);
      imgLine[i+1] = (Guchar)((c >> 6) & 1);
      imgLine[i+2] = (Guchar)((c >> 5) & 1);
      imgLine[i+3] = (Guchar)((c >> 4) & 1);
      imgLine[i+4] = (Guchar)((c >> 3) & 1);
      imgLine[i+5] = (Guchar)((c >> 2) & 1);
      imgLine[i+6] = (Guchar)((c >> 1) & 1);
      imgLine[i+7] = (Guchar)(c & 1);
    }
  } else if (nBits == 8) {
    // special case: imgLine == inputLine
  } else if (nBits == 16) {
    for (i = 0; i < nVals; ++i) {
      imgLine[i] = (Guchar)inputLine[2*i];
    }
  } else {
    bitMask = (1 << nBits) - 1;
    buf = 0;
    bits = 0;
    p = inputLine;
    for (i = 0; i < nVals; ++i) {
      if (bits < nBits) {
	buf = (buf << 8) | (*p++ & 0xff);
	bits += 8;
      }
      imgLine[i] = (Guchar)((buf >> (bits - nBits)) & bitMask);
      bits -= nBits;
    }
  }
  return imgLine;
}

void ImageStream::skipLine() {
  str->getBlock(inputLine, inputLineSize);
}


//------------------------------------------------------------------------
// StreamPredictor
//------------------------------------------------------------------------

StreamPredictor::StreamPredictor(Stream *strA, int predictorA,
				 int widthA, int nCompsA, int nBitsA) {
  str = strA;
  predictor = predictorA;
  width = widthA;
  nComps = nCompsA;
  nBits = nBitsA;
  predLine = NULL;
  ok = gFalse;

  nVals = width * nComps;
  pixBytes = (nComps * nBits + 7) >> 3;
  rowBytes = ((nVals * nBits + 7) >> 3) + pixBytes;
  if (width <= 0 || nComps <= 0 || nBits <= 0 ||
      nComps > gfxColorMaxComps ||
      nBits > 16 ||
      width >= INT_MAX / nComps ||      // check for overflow in nVals 
      nVals >= (INT_MAX - 7) / nBits) { // check for overflow in rowBytes
    return;
  }
  predLine = (Guchar *)gmalloc(rowBytes);

  reset();

  ok = gTrue;
}

StreamPredictor::~StreamPredictor() {
  gfree(predLine);
}

void StreamPredictor::reset() {
  memset(predLine, 0, rowBytes);
  predIdx = rowBytes;
}

int StreamPredictor::lookChar() {
  if (predIdx >= rowBytes) {
    if (!getNextLine()) {
      return EOF;
    }
  }
  return predLine[predIdx];
}

int StreamPredictor::getChar() {
  if (predIdx >= rowBytes) {
    if (!getNextLine()) {
      return EOF;
    }
  }
  return predLine[predIdx++];
}

int StreamPredictor::getBlock(char *blk, int size) {
  int n, m;

  n = 0;
  while (n < size) {
    if (predIdx >= rowBytes) {
      if (!getNextLine()) {
	break;
      }
    }
    m = rowBytes - predIdx;
    if (m > size - n) {
      m = size - n;
    }
    memcpy(blk + n, predLine + predIdx, m);
    predIdx += m;
    n += m;
  }
  return n;
}

GBool StreamPredictor::getNextLine() {
  int curPred;
  Guchar upLeftBuf[gfxColorMaxComps * 2 + 1];
  int left, up, upLeft, p, pa, pb, pc;
  int c;
  Gulong inBuf, outBuf, bitMask;
  int inBits, outBits;
  int i, j, k, kk;

  // get PNG optimum predictor number
  if (predictor >= 10) {
    if ((curPred = str->getRawChar()) == EOF) {
      return gFalse;
    }
    curPred += 10;
  } else {
    curPred = predictor;
  }

  // read the raw line, apply PNG (byte) predictor
  memset(upLeftBuf, 0, pixBytes + 1);
  for (i = pixBytes; i < rowBytes; ++i) {
    for (j = pixBytes; j > 0; --j) {
      upLeftBuf[j] = upLeftBuf[j-1];
    }
    upLeftBuf[0] = predLine[i];
    if ((c = str->getRawChar()) == EOF) {
      if (i > pixBytes) {
	// this ought to return false, but some (broken) PDF files
	// contain truncated image data, and Adobe apparently reads the
	// last partial line
	break;
      }
      return gFalse;
    }
    switch (curPred) {
    case 11:			// PNG sub
      predLine[i] = (Guchar)(predLine[i - pixBytes] + c);
      break;
    case 12:			// PNG up
      predLine[i] = (Guchar)(predLine[i] + c);
      break;
    case 13:			// PNG average
      predLine[i] = (Guchar)(((predLine[i - pixBytes] + predLine[i]) >> 1) + c);
      break;
    case 14:			// PNG Paeth
      left = predLine[i - pixBytes];
      up = predLine[i];
      upLeft = upLeftBuf[pixBytes];
      p = left + up - upLeft;
      if ((pa = p - left) < 0)
	pa = -pa;
      if ((pb = p - up) < 0)
	pb = -pb;
      if ((pc = p - upLeft) < 0)
	pc = -pc;
      if (pa <= pb && pa <= pc)
	predLine[i] = (Guchar)(left + c);
      else if (pb <= pc)
	predLine[i] = (Guchar)(up + c);
      else
	predLine[i] = (Guchar)(upLeft + c);
      break;
    case 10:			// PNG none
    default:			// no predictor or TIFF predictor
      predLine[i] = (Guchar)c;
      break;
    }
  }

  // apply TIFF (component) predictor
  if (predictor == 2) {
    if (nBits == 8) {
      for (i = pixBytes; i < rowBytes; ++i) {
	predLine[i] = (Guchar)(predLine[i] + predLine[i - nComps]);
      }
    } else if (nBits == 16) {
      for (i = pixBytes; i < rowBytes; i += 2) {
	c = ((predLine[i] + predLine[i - 2*nComps]) << 8) +
	    predLine[i + 1] + predLine[i + 1 - 2*nComps];
	predLine[i] = (Guchar)(c >> 8);
	predLine[i+1] = (Guchar)(c & 0xff);
      }
    } else {
      memset(upLeftBuf, 0, nComps);
      bitMask = (1 << nBits) - 1;
      inBuf = outBuf = 0;
      inBits = outBits = 0;
      j = k = pixBytes;
      for (i = 0; i < width; ++i) {
	for (kk = 0; kk < nComps; ++kk) {
	  if (inBits < nBits) {
	    inBuf = (inBuf << 8) | (predLine[j++] & 0xff);
	    inBits += 8;
	  }
	  upLeftBuf[kk] = (Guchar)((upLeftBuf[kk] +
				    (inBuf >> (inBits - nBits))) & bitMask);
	  inBits -= nBits;
	  outBuf = (outBuf << nBits) | upLeftBuf[kk];
	  outBits += nBits;
	  if (outBits >= 8) {
	    predLine[k++] = (Guchar)(outBuf >> (outBits - 8));
	    outBits -= 8;
	  }
	}
      }
      if (outBits > 0) {
	predLine[k++] = (Guchar)((outBuf << (8 - outBits)) +
				 (inBuf & ((1 << (8 - outBits)) - 1)));
      }
    }
  }

  // reset to start of line
  predIdx = pixBytes;

  return gTrue;
}

//------------------------------------------------------------------------
// SharedFile
//------------------------------------------------------------------------

class SharedFile {
public:

  SharedFile(FILE *fA);
  SharedFile *copy();
  void free();
  int readBlock(char *buf, GFileOffset pos, int size);
  GFileOffset getSize();

private:

  ~SharedFile();

  FILE *f;
  int refCnt;
#if MULTITHREADED
  GMutex mutex;
#endif
};

SharedFile::SharedFile(FILE *fA) {
  f = fA;
  refCnt = 1;
#if MULTITHREADED
  gInitMutex(&mutex);
#endif
}

SharedFile::~SharedFile() {
#if MULTITHREADED
  gDestroyMutex(&mutex);
#endif
}

SharedFile *SharedFile::copy() {
#if MULTITHREADED
  gLockMutex(&mutex);
#endif
  ++refCnt;
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
  return this;
}

void SharedFile::free() {
  int newCount;

#if MULTITHREADED
  gLockMutex(&mutex);
#endif
  newCount = --refCnt;
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
  if (newCount == 0) {
    delete this;
  }
}

int SharedFile::readBlock(char *buf, GFileOffset pos, int size) {
  int n;

#if MULTITHREADED
  gLockMutex(&mutex);
#endif
  gfseek(f, pos, SEEK_SET);
  n = (int)fread(buf, 1, size, f);
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
  return n;
}

GFileOffset SharedFile::getSize() {
  GFileOffset size;

#if MULTITHREADED
  gLockMutex(&mutex);
#endif
  gfseek(f, 0, SEEK_END);
  size = gftell(f);
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
  return size;
}

//------------------------------------------------------------------------
// FileStream
//------------------------------------------------------------------------

FileStream::FileStream(FILE *fA, GFileOffset startA, GBool limitedA,
		       GFileOffset lengthA, Object *dictA):
    BaseStream(dictA) {
  f = new SharedFile(fA);
  start = startA;
  limited = limitedA;
  length = lengthA;
  bufPtr = bufEnd = buf;
  bufPos = start;
}

FileStream::FileStream(SharedFile *fA, GFileOffset startA, GBool limitedA,
		       GFileOffset lengthA, Object *dictA):
    BaseStream(dictA) {
  f = fA->copy();
  start = startA;
  limited = limitedA;
  length = lengthA;
  bufPtr = bufEnd = buf;
  bufPos = start;
}

FileStream::~FileStream() {
  f->free();
}

Stream *FileStream::copy() {
  Object dictA;

  dict.copy(&dictA);
  return new FileStream(f, start, limited, length, &dictA);
}

Stream *FileStream::makeSubStream(GFileOffset startA, GBool limitedA,
				  GFileOffset lengthA, Object *dictA) {
  return new FileStream(f, startA, limitedA, lengthA, dictA);
}

void FileStream::reset() {
  bufPtr = bufEnd = buf;
  bufPos = start;
}

int FileStream::getBlock(char *blk, int size) {
  int n, m;

  n = 0;
  while (n < size) {
    if (bufPtr >= bufEnd) {
      if (!fillBuf()) {
	break;
      }
    }
    m = (int)(bufEnd - bufPtr);
    if (m > size - n) {
      m = size - n;
    }
    memcpy(blk + n, bufPtr, m);
    bufPtr += m;
    n += m;
  }
  return n;
}

GBool FileStream::fillBuf() {
  int n;

  bufPos += (int)(bufEnd - buf);
  bufPtr = bufEnd = buf;
  if (limited && bufPos >= start + length) {
    return gFalse;
  }
  if (limited && bufPos + fileStreamBufSize > start + length) {
    n = (int)(start + length - bufPos);
  } else {
    n = fileStreamBufSize;
  }
  n = f->readBlock(buf, bufPos, n);
  bufEnd = buf + n;
  if (bufPtr >= bufEnd) {
    return gFalse;
  }
  return gTrue;
}

void FileStream::setPos(GFileOffset pos, int dir) {
  GFileOffset size;

  if (dir >= 0) {
    bufPos = pos;
  } else {
    size = f->getSize();
    if (pos <= size) {
      bufPos = size - pos;
    } else {
      bufPos = 0;
    }
  }
  bufPtr = bufEnd = buf;
}

void FileStream::moveStart(int delta) {
  start += delta;
  bufPtr = bufEnd = buf;
  bufPos = start;
}

//------------------------------------------------------------------------
// MemStream
//------------------------------------------------------------------------

MemStream::MemStream(char *bufA, Guint startA, Guint lengthA, Object *dictA, GBool needFreeA):
    BaseStream(dictA) {
  buf = bufA;
  start = startA;
  length = lengthA;
  bufEnd = buf + start + length;
  bufPtr = buf + start;
  needFree = needFreeA;
}

MemStream::~MemStream() {
  if (needFree) {
    gfree(buf);
  }
}

Stream *MemStream::copy() {
  Object dictA;

  dict.copy(&dictA);
  return new MemStream(buf, start, length, &dictA);
}

Stream *MemStream::makeSubStream(GFileOffset startA, GBool limited,
				 GFileOffset lengthA, Object *dictA) {
  MemStream *subStr;
  Guint newStart, newLength;

  if (startA < start) {
    newStart = start;
  } else if (startA > start + length) {
    newStart = start + (int)length;
  } else {
    newStart = (int)startA;
  }
  if (!limited || newStart + lengthA > start + length) {
    newLength = start + length - newStart;
  } else {
    newLength = (Guint)lengthA;
  }
  subStr = new MemStream(buf, newStart, newLength, dictA);
  return subStr;
}

void MemStream::reset() {
  bufPtr = buf + start;
}

void MemStream::close() {
}

int MemStream::getBlock(char *blk, int size) {
  int n;

  if (size <= 0) {
    return 0;
  }
  if (bufEnd - bufPtr < size) {
    n = (int)(bufEnd - bufPtr);
  } else {
    n = size;
  }
  memcpy(blk, bufPtr, n);
  bufPtr += n;
  return n;
}

void MemStream::setPos(GFileOffset pos, int dir) {
  Guint i;

  if (dir >= 0) {
    i = (Guint)pos;
  } else {
    i = (Guint)(start + length - pos);
  }
  if (dir < 0 && start + length < pos) {
    i = 0;
  } else if (i < start) {
    i = start;
  } else if (i > start + length) {
    i = start + length;
  }
  bufPtr = buf + i;
}

void MemStream::moveStart(int delta) {
  start += delta;
  length -= delta;
  bufPtr = buf + start;
}

//------------------------------------------------------------------------
// EmbedStream
//------------------------------------------------------------------------

EmbedStream::EmbedStream(Stream *strA, Object *dictA,
			 GBool limitedA, GFileOffset lengthA):
    BaseStream(dictA) {
  str = strA;
  limited = limitedA;
  length = lengthA;
}

EmbedStream::~EmbedStream() {
}

Stream *EmbedStream::copy() {
  Object dictA;

  dict.copy(&dictA);
  return new EmbedStream(str, &dictA, limited, length);
}

Stream *EmbedStream::makeSubStream(GFileOffset start, GBool limitedA,
				   GFileOffset lengthA, Object *dictA) {
  error(errInternal, -1, "Called makeSubStream() on EmbedStream");
  return NULL;
}

int EmbedStream::getChar() {
  if (limited && !length) {
    return EOF;
  }
  --length;
  return str->getChar();
}

int EmbedStream::lookChar() {
  if (limited && !length) {
    return EOF;
  }
  return str->lookChar();
}

int EmbedStream::getBlock(char *blk, int size) {
  if (size <= 0) {
    return 0;
  }
  if (limited && length < (Guint)size) {
    size = (int)length;
  }
  length -= size;
  return str->getBlock(blk, size);
}

void EmbedStream::setPos(GFileOffset pos, int dir) {
  error(errInternal, -1, "Called setPos() on EmbedStream");
}

GFileOffset EmbedStream::getStart() {
  error(errInternal, -1, "Called getStart() on EmbedStream");
  return 0;
}

void EmbedStream::moveStart(int delta) {
  error(errInternal, -1, "Called moveStart() on EmbedStream");
}

//------------------------------------------------------------------------
// ASCIIHexStream
//------------------------------------------------------------------------

ASCIIHexStream::ASCIIHexStream(Stream *strA):
    FilterStream(strA) {
  buf = EOF;
  eof = gFalse;
}

ASCIIHexStream::~ASCIIHexStream() {
  delete str;
}

Stream *ASCIIHexStream::copy() {
  return new ASCIIHexStream(str->copy());
}

void ASCIIHexStream::reset() {
  str->reset();
  buf = EOF;
  eof = gFalse;
}

int ASCIIHexStream::lookChar() {
  int c1, c2, x;

  if (buf != EOF)
    return buf;
  if (eof) {
    buf = EOF;
    return EOF;
  }
  do {
    c1 = str->getChar();
  } while (isspace(c1));
  if (c1 == '>') {
    eof = gTrue;
    buf = EOF;
    return buf;
  }
  do {
    c2 = str->getChar();
  } while (isspace(c2));
  if (c2 == '>') {
    eof = gTrue;
    c2 = '0';
  }
  if (c1 >= '0' && c1 <= '9') {
    x = (c1 - '0') << 4;
  } else if (c1 >= 'A' && c1 <= 'F') {
    x = (c1 - 'A' + 10) << 4;
  } else if (c1 >= 'a' && c1 <= 'f') {
    x = (c1 - 'a' + 10) << 4;
  } else if (c1 == EOF) {
    eof = gTrue;
    x = 0;
  } else {
    error(errSyntaxError, getPos(),
	  "Illegal character <{0:02x}> in ASCIIHex stream", c1);
    x = 0;
  }
  if (c2 >= '0' && c2 <= '9') {
    x += c2 - '0';
  } else if (c2 >= 'A' && c2 <= 'F') {
    x += c2 - 'A' + 10;
  } else if (c2 >= 'a' && c2 <= 'f') {
    x += c2 - 'a' + 10;
  } else if (c2 == EOF) {
    eof = gTrue;
    x = 0;
  } else {
    error(errSyntaxError, getPos(),
	  "Illegal character <{0:02x}> in ASCIIHex stream", c2);
  }
  buf = x & 0xff;
  return buf;
}

GString *ASCIIHexStream::getPSFilter(int psLevel, const char *indent,
				     GBool okToReadStream) {
  GString *s;

  if (psLevel < 2) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent, okToReadStream))) {
    return NULL;
  }
  s->append(indent)->append("/ASCIIHexDecode filter\n");
  return s;
}

GBool ASCIIHexStream::isBinary(GBool last) {
  return str->isBinary(gFalse);
}

//------------------------------------------------------------------------
// ASCII85Stream
//------------------------------------------------------------------------

ASCII85Stream::ASCII85Stream(Stream *strA):
    FilterStream(strA) {
  index = n = 0;
  eof = gFalse;
}

ASCII85Stream::~ASCII85Stream() {
  delete str;
}

Stream *ASCII85Stream::copy() {
  return new ASCII85Stream(str->copy());
}

void ASCII85Stream::reset() {
  str->reset();
  index = n = 0;
  eof = gFalse;
}

int ASCII85Stream::lookChar() {
  int k;
  Gulong t;

  if (index >= n) {
    if (eof)
      return EOF;
    index = 0;
    do {
      c[0] = str->getChar();
    } while (Lexer::isSpace(c[0]));
    if (c[0] == '~' || c[0] == EOF) {
      eof = gTrue;
      n = 0;
      return EOF;
    } else if (c[0] == 'z') {
      b[0] = b[1] = b[2] = b[3] = 0;
      n = 4;
    } else {
      for (k = 1; k < 5; ++k) {
	do {
	  c[k] = str->getChar();
	} while (Lexer::isSpace(c[k]));
	if (c[k] == '~' || c[k] == EOF)
	  break;
      }
      n = k - 1;
      if (k < 5 && (c[k] == '~' || c[k] == EOF)) {
	for (++k; k < 5; ++k)
	  c[k] = 0x21 + 84;
	eof = gTrue;
      }
      t = 0;
      for (k = 0; k < 5; ++k)
	t = t * 85 + (c[k] - 0x21);
      for (k = 3; k >= 0; --k) {
	b[k] = (int)(t & 0xff);
	t >>= 8;
      }
    }
  }
  return b[index];
}

GString *ASCII85Stream::getPSFilter(int psLevel, const char *indent,
				    GBool okToReadStream) {
  GString *s;

  if (psLevel < 2) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent, okToReadStream))) {
    return NULL;
  }
  s->append(indent)->append("/ASCII85Decode filter\n");
  return s;
}

GBool ASCII85Stream::isBinary(GBool last) {
  return str->isBinary(gFalse);
}

//------------------------------------------------------------------------
// LZWStream
//------------------------------------------------------------------------

LZWStream::LZWStream(Stream *strA, int predictor, int columns, int colors,
		     int bits, int earlyA):
    FilterStream(strA) {
  if (predictor != 1) {
    pred = new StreamPredictor(this, predictor, columns, colors, bits);
    if (!pred->isOk()) {
      delete pred;
      pred = NULL;
    }
  } else {
    pred = NULL;
  }
  early = earlyA;
  eof = gFalse;
  inputBits = 0;
  clearTable();
}

LZWStream::~LZWStream() {
  if (pred) {
    delete pred;
  }
  delete str;
}

Stream *LZWStream::copy() {
  if (pred) {
    return new LZWStream(str->copy(), pred->getPredictor(),
			 pred->getWidth(), pred->getNComps(),
			 pred->getNBits(), early);
  } else {
    return new LZWStream(str->copy(), 1, 0, 0, 0, early);
  }
}

int LZWStream::getChar() {
  if (pred) {
    return pred->getChar();
  }
  if (eof) {
    return EOF;
  }
  if (seqIndex >= seqLength) {
    if (!processNextCode()) {
      return EOF;
    }
  }
  return seqBuf[seqIndex++];
}

int LZWStream::lookChar() {
  if (pred) {
    return pred->lookChar();
  }
  if (eof) {
    return EOF;
  }
  if (seqIndex >= seqLength) {
    if (!processNextCode()) {
      return EOF;
    }
  }
  return seqBuf[seqIndex];
}

int LZWStream::getRawChar() {
  if (eof) {
    return EOF;
  }
  if (seqIndex >= seqLength) {
    if (!processNextCode()) {
      return EOF;
    }
  }
  return seqBuf[seqIndex++];
}

int LZWStream::getBlock(char *blk, int size) {
  int n, m;

  if (pred) {
    return pred->getBlock(blk, size);
  }
  if (eof) {
    return 0;
  }
  n = 0;
  while (n < size) {
    if (seqIndex >= seqLength) {
      if (!processNextCode()) {
	break;
      }
    }
    m = seqLength - seqIndex;
    if (m > size - n) {
      m = size - n;
    }
    memcpy(blk + n, seqBuf + seqIndex, m);
    seqIndex += m;
    n += m;
  }
  return n;
}

void LZWStream::reset() {
  str->reset();
  if (pred) {
    pred->reset();
  }
  eof = gFalse;
  inputBits = 0;
  clearTable();
  totalIn = totalOut = 0;
}

GBool LZWStream::processNextCode() {
  int code;
  int nextLength;
  int i, j;

  // check for EOF
  if (eof) {
    return gFalse;
  }

  // check for eod and clear-table codes
 start:
  code = getCode();
  if (code == EOF || code == 257) {
    eof = gTrue;
    return gFalse;
  }
  if (code == 256) {
    clearTable();
    goto start;
  }
  if (nextCode >= 4097) {
    error(errSyntaxError, getPos(),
	  "Bad LZW stream - expected clear-table code");
    clearTable();
  }

  // process the next code
  nextLength = seqLength + 1;
  if (code < 256) {
    seqBuf[0] = (Guchar)code;
    seqLength = 1;
  } else if (code < nextCode) {
    seqLength = table[code].length;
    for (i = seqLength - 1, j = code; i > 0; --i) {
      seqBuf[i] = table[j].tail;
      j = table[j].head;
    }
    seqBuf[0] = (Guchar)j;
  } else if (code == nextCode) {
    seqBuf[seqLength] = (Guchar)newChar;
    ++seqLength;
  } else {
    error(errSyntaxError, getPos(), "Bad LZW stream - unexpected code");
    eof = gTrue;
    return gFalse;
  }
  newChar = seqBuf[0];
  if (first) {
    first = gFalse;
  } else {
    table[nextCode].length = nextLength;
    table[nextCode].head = prevCode;
    table[nextCode].tail = (Guchar)newChar;
    ++nextCode;
    if (nextCode + early == 512)
      nextBits = 10;
    else if (nextCode + early == 1024)
      nextBits = 11;
    else if (nextCode + early == 2048)
      nextBits = 12;
  }
  prevCode = code;
  totalOut += seqLength;

  // check for a 'decompression bomb'
  if (totalOut > 500000000 && totalIn < totalOut / 250) {
    error(errSyntaxError, getPos(), "Decompression bomb in flate stream");
    eof = gTrue;
    return gFalse;
  }

  // reset buffer
  seqIndex = 0;

  return gTrue;
}

void LZWStream::clearTable() {
  nextCode = 258;
  nextBits = 9;
  seqIndex = seqLength = 0;
  first = gTrue;
}

int LZWStream::getCode() {
  int c;
  int code;

  while (inputBits < nextBits) {
    if ((c = str->getChar()) == EOF)
      return EOF;
    inputBuf = (inputBuf << 8) | (c & 0xff);
    inputBits += 8;
    ++totalIn;
  }
  code = (inputBuf >> (inputBits - nextBits)) & ((1 << nextBits) - 1);
  inputBits -= nextBits;
  return code;
}

GString *LZWStream::getPSFilter(int psLevel, const char *indent,
				GBool okToReadStream) {
  GString *s;

  if (psLevel < 2 || pred) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent, okToReadStream))) {
    return NULL;
  }
  s->append(indent)->append("<< ");
  if (!early) {
    s->append("/EarlyChange 0 ");
  }
  s->append(">> /LZWDecode filter\n");
  return s;
}

GBool LZWStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

//------------------------------------------------------------------------
// RunLengthStream
//------------------------------------------------------------------------

RunLengthStream::RunLengthStream(Stream *strA):
    FilterStream(strA) {
  bufPtr = bufEnd = buf;
  eof = gFalse;
}

RunLengthStream::~RunLengthStream() {
  delete str;
}

Stream *RunLengthStream::copy() {
  return new RunLengthStream(str->copy());
}

void RunLengthStream::reset() {
  str->reset();
  bufPtr = bufEnd = buf;
  eof = gFalse;
}

int RunLengthStream::getBlock(char *blk, int size) {
  int n, m;

  n = 0;
  while (n < size) {
    if (bufPtr >= bufEnd) {
      if (!fillBuf()) {
	break;
      }
    }
    m = (int)(bufEnd - bufPtr);
    if (m > size - n) {
      m = size - n;
    }
    memcpy(blk + n, bufPtr, m);
    bufPtr += m;
    n += m;
  }
  return n;
}

GString *RunLengthStream::getPSFilter(int psLevel, const char *indent,
				      GBool okToReadStream) {
  GString *s;

  if (psLevel < 2) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent, okToReadStream))) {
    return NULL;
  }
  s->append(indent)->append("/RunLengthDecode filter\n");
  return s;
}

GBool RunLengthStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

GBool RunLengthStream::fillBuf() {
  int c;
  int n, i;

  if (eof)
    return gFalse;
  c = str->getChar();
  if (c == 0x80 || c == EOF) {
    eof = gTrue;
    return gFalse;
  }
  if (c < 0x80) {
    n = c + 1;
    for (i = 0; i < n; ++i)
      buf[i] = (char)str->getChar();
  } else {
    n = 0x101 - c;
    c = str->getChar();
    for (i = 0; i < n; ++i)
      buf[i] = (char)c;
  }
  bufPtr = buf;
  bufEnd = buf + n;
  return gTrue;
}

//------------------------------------------------------------------------
// CCITTFaxStream
//------------------------------------------------------------------------

CCITTFaxStream::CCITTFaxStream(Stream *strA, int encodingA, GBool endOfLineA,
			       GBool byteAlignA, int columnsA, int rowsA,
			       GBool endOfBlockA, GBool blackA):
    FilterStream(strA) {
  encoding = encodingA;
  endOfLine = endOfLineA;
  byteAlign = byteAlignA;
  columns = columnsA;
  if (columns < 1) {
    columns = 1;
  } else if (columns > INT_MAX - 3) {
    columns = INT_MAX - 3;
  }
  rows = rowsA;
  endOfBlock = endOfBlockA;
  black = blackA;
  blackXOR = black ? 0xff : 0x00;
  // 0 <= codingLine[0] < codingLine[1] < ... < codingLine[n] = columns
  // ---> max codingLine size = columns + 1
  // refLine has two extra guard entries at the end
  // ---> max refLine size = columns + 3
  codingLine = (int *)gmallocn(columns + 1, sizeof(int));
  refLine = (int *)gmallocn(columns + 3, sizeof(int));

  eof = gFalse;
  row = 0;
  nextLine2D = encoding < 0;
  inputBits = 0;
  codingLine[0] = columns;
  nextCol = columns;
  a0i = 0;
  err = gFalse;
  nErrors = 0;
}

CCITTFaxStream::~CCITTFaxStream() {
  delete str;
  gfree(refLine);
  gfree(codingLine);
}

Stream *CCITTFaxStream::copy() {
  return new CCITTFaxStream(str->copy(), encoding, endOfLine,
			    byteAlign, columns, rows, endOfBlock, black);
}

void CCITTFaxStream::reset() {
  int code1;

  str->reset();
  eof = gFalse;
  row = 0;
  nextLine2D = encoding < 0;
  inputBits = 0;
  codingLine[0] = columns;
  nextCol = columns;
  a0i = 0;

  // skip any initial zero bits and end-of-line marker, and get the 2D
  // encoding tag
  while ((code1 = lookBits(12)) == 0) {
    eatBits(1);
  }
  if (code1 == 0x001) {
    eatBits(12);
    endOfLine = gTrue;
  }
  if (encoding > 0) {
    nextLine2D = !lookBits(1);
    eatBits(1);
  }
}

int CCITTFaxStream::getChar() {
  int c, bitsNeeded, bitsAvail, bitsUsed;

  if (nextCol >= columns) {
    if (eof) {
      return EOF;
    }
    if (!readRow()) {
      return EOF;
    }
  }
  bitsAvail = codingLine[a0i] - nextCol;
  if (bitsAvail > 8) {
    c = (a0i & 1) ? 0x00 : 0xff;
  } else {
    c = 0;
    bitsNeeded = 8;
    do {
      bitsUsed = (bitsAvail < bitsNeeded) ? bitsAvail : bitsNeeded;
      c <<= bitsUsed;
      if (!(a0i & 1)) {
	c |= 0xff >> (8 - bitsUsed);
      }
      bitsAvail -= bitsUsed;
      bitsNeeded -= bitsUsed;
      if (bitsAvail == 0) {
	if (codingLine[a0i] >= columns) {
	  c <<= bitsNeeded;
	  break;
	}
	++a0i;
	bitsAvail = codingLine[a0i] - codingLine[a0i - 1];
      }
    } while (bitsNeeded > 0);
  }
  nextCol += 8;
  c ^= blackXOR;
  return c;
}

int CCITTFaxStream::lookChar() {
  int c, bitsNeeded, bitsAvail, bitsUsed, i;

  if (nextCol >= columns) {
    if (eof) {
      return EOF;
    }
    if (!readRow()) {
      return EOF;
    }
  }
  bitsAvail = codingLine[a0i] - nextCol;
  if (bitsAvail >= 8) {
    c = (a0i & 1) ? 0x00 : 0xff;
  } else {
    i = a0i;
    c = 0;
    bitsNeeded = 8;
    do {
      bitsUsed = (bitsAvail < bitsNeeded) ? bitsAvail : bitsNeeded;
      c <<= bitsUsed;
      if (!(i & 1)) {
	c |= 0xff >> (8 - bitsUsed);
      }
      bitsAvail -= bitsUsed;
      bitsNeeded -= bitsUsed;
      if (bitsAvail == 0) {
	if (codingLine[i] >= columns) {
	  c <<= bitsNeeded;
	  break;
	}
	++i;
	bitsAvail = codingLine[i] - codingLine[i - 1];
      }
    } while (bitsNeeded > 0);
  }
  c ^= blackXOR;
  return c;
}

int CCITTFaxStream::getBlock(char *blk, int size) {
  int bytesRead, bitsAvail, bitsNeeded, bitsUsed, byte, c;

  bytesRead = 0;
  while (bytesRead < size) {
    if (nextCol >= columns) {
      if (eof) {
	break;
      }
      if (!readRow()) {
	break;
      }
    }
    bitsAvail = codingLine[a0i] - nextCol;
    byte = (a0i & 1) ? 0x00 : 0xff;
    if (bitsAvail > 8) {
      c = byte;
      bitsAvail -= 8;
    } else {
      c = 0;
      bitsNeeded = 8;
      do {
	bitsUsed = (bitsAvail < bitsNeeded) ? bitsAvail : bitsNeeded;
	c <<= bitsUsed;
	c |= byte >> (8 - bitsUsed);
	bitsAvail -= bitsUsed;
	bitsNeeded -= bitsUsed;
	if (bitsAvail == 0) {
	  if (codingLine[a0i] >= columns) {
	    c <<= bitsNeeded;
	    break;
	  }
	  ++a0i;
	  bitsAvail = codingLine[a0i] - codingLine[a0i - 1];
	  byte ^= 0xff;
	}
      } while (bitsNeeded > 0);
    }
    nextCol += 8;
    blk[bytesRead++] = (char)(c ^ blackXOR);
  }
  return bytesRead;
}

inline void CCITTFaxStream::addPixels(int a1, int blackPixels) {
  if (a1 > codingLine[a0i]) {
    if (a1 > columns) {
      error(errSyntaxError, getPos(),
	    "CCITTFax row is wrong length ({0:d})", a1);
      err = gTrue;
      ++nErrors;
      a1 = columns;
    }
    if ((a0i & 1) ^ blackPixels) {
      ++a0i;
    }
    codingLine[a0i] = a1;
  }
}

inline void CCITTFaxStream::addPixelsNeg(int a1, int blackPixels) {
  if (a1 > codingLine[a0i]) {
    if (a1 > columns) {
      error(errSyntaxError, getPos(),
	    "CCITTFax row is wrong length ({0:d})", a1);
      err = gTrue;
      ++nErrors;
      a1 = columns;
    }
    if ((a0i & 1) ^ blackPixels) {
      ++a0i;
    }
    codingLine[a0i] = a1;
  } else if (a1 < codingLine[a0i]) {
    if (a1 < 0) {
      error(errSyntaxError, getPos(), "Invalid CCITTFax code");
      err = gTrue;
      ++nErrors;
      a1 = 0;
    }
    while (a0i > 0 && a1 <= codingLine[a0i - 1]) {
      --a0i;
    }
    codingLine[a0i] = a1;
  }
}

GBool CCITTFaxStream::readRow() {
  int code1, code2, code3;
  int b1i, blackPixels, i;
  GBool gotEOL;

  // if at eof just return EOF
  if (eof) {
    return gFalse;
  }

  err = gFalse;

  // 2-D encoding
  if (nextLine2D) {
    for (i = 0; codingLine[i] < columns; ++i) {
      refLine[i] = codingLine[i];
    }
    refLine[i++] = columns;
    refLine[i++] = columns;
    refLine[i] = columns;
    codingLine[0] = 0;
    a0i = 0;
    b1i = 0;
    blackPixels = 0;
    // invariant:
    // refLine[b1i-1] <= codingLine[a0i] < refLine[b1i] < refLine[b1i+1]
    //                                                             <= columns
    // exception at left edge:
    //   codingLine[a0i = 0] = refLine[b1i = 0] = 0 is possible
    // exception at right edge:
    //   refLine[b1i] = refLine[b1i+1] = columns is possible
    while (codingLine[a0i] < columns) {
      code1 = getTwoDimCode();
      switch (code1) {
      case twoDimPass:
	addPixels(refLine[b1i + 1], blackPixels);
	if (refLine[b1i + 1] < columns) {
	  b1i += 2;
	}
	break;
      case twoDimHoriz:
	code1 = code2 = 0;
	if (blackPixels) {
	  do {
	    code1 += code3 = getBlackCode();
	  } while (code3 >= 64);
	  do {
	    code2 += code3 = getWhiteCode();
	  } while (code3 >= 64);
	} else {
	  do {
	    code1 += code3 = getWhiteCode();
	  } while (code3 >= 64);
	  do {
	    code2 += code3 = getBlackCode();
	  } while (code3 >= 64);
	}
	addPixels(codingLine[a0i] + code1, blackPixels);
	if (codingLine[a0i] < columns) {
	  addPixels(codingLine[a0i] + code2, blackPixels ^ 1);
	}
	while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	  b1i += 2;
	}
	break;
      case twoDimVertR3:
	addPixels(refLine[b1i] + 3, blackPixels);
	blackPixels ^= 1;
	if (codingLine[a0i] < columns) {
	  ++b1i;
	  while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	    b1i += 2;
	  }
	}
	break;
      case twoDimVertR2:
	addPixels(refLine[b1i] + 2, blackPixels);
	blackPixels ^= 1;
	if (codingLine[a0i] < columns) {
	  ++b1i;
	  while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	    b1i += 2;
	  }
	}
	break;
      case twoDimVertR1:
	addPixels(refLine[b1i] + 1, blackPixels);
	blackPixels ^= 1;
	if (codingLine[a0i] < columns) {
	  ++b1i;
	  while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	    b1i += 2;
	  }
	}
	break;
      case twoDimVert0:
	addPixels(refLine[b1i], blackPixels);
	blackPixels ^= 1;
	if (codingLine[a0i] < columns) {
	  ++b1i;
	  while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	    b1i += 2;
	  }
	}
	break;
      case twoDimVertL3:
	addPixelsNeg(refLine[b1i] - 3, blackPixels);
	blackPixels ^= 1;
	if (codingLine[a0i] < columns) {
	  if (b1i > 0) {
	    --b1i;
	  } else {
	    ++b1i;
	  }
	  while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	    b1i += 2;
	  }
	}
	break;
      case twoDimVertL2:
	addPixelsNeg(refLine[b1i] - 2, blackPixels);
	blackPixels ^= 1;
	if (codingLine[a0i] < columns) {
	  if (b1i > 0) {
	    --b1i;
	  } else {
	    ++b1i;
	  }
	  while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	    b1i += 2;
	  }
	}
	break;
      case twoDimVertL1:
	addPixelsNeg(refLine[b1i] - 1, blackPixels);
	blackPixels ^= 1;
	if (codingLine[a0i] < columns) {
	  if (b1i > 0) {
	    --b1i;
	  } else {
	    ++b1i;
	  }
	  while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < columns) {
	    b1i += 2;
	  }
	}
	break;
      case EOF:
	addPixels(columns, 0);
	err = gTrue;
	break;
      default:
	error(errSyntaxError, getPos(),
	      "Bad 2D code {0:04x} in CCITTFax stream", code1);
	addPixels(columns, 0);
	err = gTrue;
	++nErrors;
	break;
      }
    }

  // 1-D encoding
  } else {
    codingLine[0] = 0;
    a0i = 0;
    blackPixels = 0;
    while (codingLine[a0i] < columns) {
      code1 = 0;
      if (blackPixels) {
	do {
	  code1 += code3 = getBlackCode();
	} while (code3 >= 64);
      } else {
	do {
	  code1 += code3 = getWhiteCode();
	} while (code3 >= 64);
      }
      addPixels(codingLine[a0i] + code1, blackPixels);
      blackPixels ^= 1;
    }
  }

  // check for end-of-line marker, skipping over any extra zero bits
  // (if EncodedByteAlign is true and EndOfLine is false, there can
  // be "false" EOL markers -- i.e., if the last n unused bits in
  // row i are set to zero, and the first 11-n bits in row i+1
  // happen to be zero -- so we don't look for EOL markers in this
  // case)
  gotEOL = gFalse;
  if (!endOfBlock && row == rows - 1) {
    eof = gTrue;
  } else if (endOfLine || !byteAlign) {
    code1 = lookBits(12);
    if (endOfLine) {
      while (code1 != EOF && code1 != 0x001) {
	eatBits(1);
	code1 = lookBits(12);
      }
    } else {
      while (code1 == 0) {
	eatBits(1);
	code1 = lookBits(12);
      }
    }
    if (code1 == 0x001) {
      eatBits(12);
      gotEOL = gTrue;
    }
  }

  // byte-align the row
  // (Adobe apparently doesn't do byte alignment after EOL markers
  // -- I've seen CCITT image data streams in two different formats,
  // both with the byteAlign flag set:
  //   1. xx:x0:01:yy:yy
  //   2. xx:00:1y:yy:yy
  // where xx is the previous line, yy is the next line, and colons
  // separate bytes.)
  if (byteAlign && !gotEOL) {
    inputBits &= ~7;
  }

  // check for end of stream
  if (lookBits(1) == EOF) {
    eof = gTrue;
  }

  // get 2D encoding tag
  if (!eof && encoding > 0) {
    nextLine2D = !lookBits(1);
    eatBits(1);
  }

  // check for end-of-block marker
  if (endOfBlock && !endOfLine && byteAlign) {
    // in this case, we didn't check for an EOL code above, so we
    // need to check here
    code1 = lookBits(24);
    if (code1 == 0x001001) {
      eatBits(12);
      gotEOL = gTrue;
    }
  }
  if (endOfBlock && gotEOL) {
    code1 = lookBits(12);
    if (code1 == 0x001) {
      eatBits(12);
      if (encoding > 0) {
	lookBits(1);
	eatBits(1);
      }
      if (encoding > 0) {
	for (i = 0; i < 4; ++i) {
	  code1 = lookBits(12);
	  if (code1 != 0x001) {
	    error(errSyntaxError, getPos(),
		  "Bad RTC code in CCITTFax stream");
	    ++nErrors;
	  }
	  eatBits(12);
	  if (encoding > 0) {
	    lookBits(1);
	    eatBits(1);
	  }
	}
      }
      eof = gTrue;
    }

  // look for an end-of-line marker after an error -- we only do
  // this if we know the stream contains end-of-line markers because
  // the "just plow on" technique tends to work better otherwise
  } else if (err && endOfLine) {
    while (1) {
      code1 = lookBits(13);
      if (code1 == EOF) {
	eof = gTrue;
	return gFalse;
      }
      if ((code1 >> 1) == 0x001) {
	break;
      }
      eatBits(1);
    }
    eatBits(12); 
    if (encoding > 0) {
      eatBits(1);
      nextLine2D = !(code1 & 1);
    }
  }

  // corrupt CCITTFax streams can generate huge data expansion -- we
  // avoid that case by aborting decode after 1000 errors
  if (nErrors > 1000) {
    error(errSyntaxError, getPos(), "Too many errors in CCITTFaxStream - aborting decode");
    eof = gTrue;
    return gFalse;
  }

  // set up for output
  nextCol = 0;
  a0i = (codingLine[0] > 0) ? 0 : 1;

  ++row;

  return gTrue;
}

short CCITTFaxStream::getTwoDimCode() {
  int code;
  CCITTCode *p;
  int n;

  code = 0; // make gcc happy
  if (endOfBlock) {
    if ((code = lookBits(7)) != EOF) {
      p = &twoDimTab1[code];
      if (p->bits > 0) {
	eatBits(p->bits);
	return p->n;
      }
    }
  } else {
    for (n = 1; n <= 7; ++n) {
      if ((code = lookBits(n)) == EOF) {
	break;
      }
      if (n < 7) {
	code <<= 7 - n;
      }
      p = &twoDimTab1[code];
      if (p->bits == n) {
	eatBits(n);
	return p->n;
      }
    }
  }
  error(errSyntaxError, getPos(),
	"Bad two dim code ({0:04x}) in CCITTFax stream", code);
  ++nErrors;
  return EOF;
}

short CCITTFaxStream::getWhiteCode() {
  short code;
  CCITTCode *p;
  int n;

  code = 0; // make gcc happy
  if (endOfBlock) {
    code = lookBits(12);
    if (code == EOF) {
      return 1;
    }
    if ((code >> 5) == 0) {
      p = &whiteTab1[code];
    } else {
      p = &whiteTab2[code >> 3];
    }
    if (p->bits > 0) {
      eatBits(p->bits);
      return p->n;
    }
  } else {
    for (n = 1; n <= 9; ++n) {
      code = lookBits(n);
      if (code == EOF) {
	return 1;
      }
      if (n < 9) {
	code = (short)(code << (9 - n));
      }
      p = &whiteTab2[code];
      if (p->bits == n) {
	eatBits(n);
	return p->n;
      }
    }
    for (n = 11; n <= 12; ++n) {
      code = lookBits(n);
      if (code == EOF) {
	return 1;
      }
      if (n < 12) {
	code = (short)(code << (12 - n));
      }
      p = &whiteTab1[code];
      if (p->bits == n) {
	eatBits(n);
	return p->n;
      }
    }
  }
  error(errSyntaxError, getPos(),
	"Bad white code ({0:04x}) in CCITTFax stream", code);
  ++nErrors;
  // eat a bit and return a positive number so that the caller doesn't
  // go into an infinite loop
  eatBits(1);
  return 1;
}

short CCITTFaxStream::getBlackCode() {
  short code;
  CCITTCode *p;
  int n;

  code = 0; // make gcc happy
  if (endOfBlock) {
    code = lookBits(13);
    if (code == EOF) {
      return 1;
    }
    if ((code >> 7) == 0) {
      p = &blackTab1[code];
    } else if ((code >> 9) == 0 && (code >> 7) != 0) {
      p = &blackTab2[(code >> 1) - 64];
    } else {
      p = &blackTab3[code >> 7];
    }
    if (p->bits > 0) {
      eatBits(p->bits);
      return p->n;
    }
  } else {
    for (n = 2; n <= 6; ++n) {
      code = lookBits(n);
      if (code == EOF) {
	return 1;
      }
      if (n < 6) {
	code = (short)(code << (6 - n));
      }
      p = &blackTab3[code];
      if (p->bits == n) {
	eatBits(n);
	return p->n;
      }
    }
    for (n = 7; n <= 12; ++n) {
      code = lookBits(n);
      if (code == EOF) {
	return 1;
      }
      if (n < 12) {
	code = (short)(code << (12 - n));
      }
      if (code >= 64) {
	p = &blackTab2[code - 64];
	if (p->bits == n) {
	  eatBits(n);
	  return p->n;
	}
      }
    }
    for (n = 10; n <= 13; ++n) {
      code = lookBits(n);
      if (code == EOF) {
	return 1;
      }
      if (n < 13) {
	code = (short)(code << (13 - n));
      }
      p = &blackTab1[code];
      if (p->bits == n) {
	eatBits(n);
	return p->n;
      }
    }
  }
  error(errSyntaxError, getPos(),
	"Bad black code ({0:04x}) in CCITTFax stream", code);
  ++nErrors;
  // eat a bit and return a positive number so that the caller doesn't
  // go into an infinite loop
  eatBits(1);
  return 1;
}

short CCITTFaxStream::lookBits(int n) {
  int c;

  while (inputBits < n) {
    if ((c = str->getChar()) == EOF) {
      if (inputBits == 0) {
	return EOF;
      }
      // near the end of the stream, the caller may ask for more bits
      // than are available, but there may still be a valid code in
      // however many bits are available -- we need to return correct
      // data in this case
      return (short)((inputBuf << (n - inputBits)) & (0xffffffff >> (32 - n)));
    }
    inputBuf = (inputBuf << 8) + c;
    inputBits += 8;
  }
  return (short)((inputBuf >> (inputBits - n)) & (0xffffffff >> (32 - n)));
}

GString *CCITTFaxStream::getPSFilter(int psLevel, const char *indent,
				     GBool okToReadStream) {
  GString *s;

  if (psLevel < 2) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent, okToReadStream))) {
    return NULL;
  }
  s->append(indent)->append("<< ");
  if (encoding != 0) {
    s->appendf("/K {0:d} ", encoding);
  }
  if (endOfLine) {
    s->append("/EndOfLine true ");
  }
  if (byteAlign) {
    s->append("/EncodedByteAlign true ");
  }
  s->appendf("/Columns {0:d} ", columns);
  if (rows != 0) {
    s->appendf("/Rows {0:d} ", rows);
  }
  if (!endOfBlock) {
    s->append("/EndOfBlock false ");
  }
  if (black) {
    s->append("/BlackIs1 true ");
  }
  s->append(">> /CCITTFaxDecode filter\n");
  return s;
}

GBool CCITTFaxStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

//------------------------------------------------------------------------
// DCTStream
//------------------------------------------------------------------------

#if HAVE_JPEGLIB

DCTStream::DCTStream(Stream *strA, GBool colorXformA):
    FilterStream(strA) {
  colorXform = colorXformA;
  lineBuf = NULL;
  inlineImage = str->isEmbedStream();
}

DCTStream::~DCTStream() {
  delete str;
}

Stream *DCTStream::copy() {
  return new DCTStream(str->copy(), colorXform);
}

void DCTStream::reset() {
  int i;

  lineBuf = NULL;
  error = gFalse;

  str->reset();

  // initialize the libjpeg decompression object
  decomp.err = jpeg_std_error(&errorMgr.err);
  errorMgr.err.error_exit = &errorExit;
  errorMgr.err.output_message = &errorMessage;
  if (setjmp(errorMgr.setjmpBuf)) {
    error = gTrue;
    return;
  }
  jpeg_create_decompress(&decomp);

  // set up the data source manager
  sourceMgr.src.next_input_byte = NULL;
  sourceMgr.src.bytes_in_buffer = 0;
  sourceMgr.src.init_source = &initSourceCbk;
  sourceMgr.src.fill_input_buffer = &fillInputBufferCbk;
  sourceMgr.src.skip_input_data = &skipInputDataCbk;
  sourceMgr.src.resync_to_restart = &jpeg_resync_to_restart;
  sourceMgr.src.term_source = &termSourceCbk;
  sourceMgr.str = this;
  decomp.src = &sourceMgr.src;

  // read the header
  jpeg_read_header(&decomp, TRUE);
  jpeg_calc_output_dimensions(&decomp);

  // set up the color transform
  if (!decomp.saw_Adobe_marker && colorXform >= 0) {
    if (decomp.num_components == 3) {
      decomp.jpeg_color_space = colorXform ? JCS_YCbCr : JCS_RGB;
      decomp.out_color_space = JCS_RGB;
      decomp.out_color_components = 3;
    } else if (decomp.num_components == 4) {
      decomp.jpeg_color_space = colorXform ? JCS_YCCK : JCS_CMYK;
      decomp.out_color_space = JCS_CMYK;
      decomp.out_color_components = 4;
    }
  }

  // allocate a line buffer
  if ((lineBufHeight = decomp.rec_outbuf_height) > 4) {
    lineBufHeight = 4;
  }
  lineBuf = (char *)gmallocn(lineBufHeight * decomp.out_color_components,
			     decomp.output_width);
  for (i = 0; i < lineBufHeight; ++i) {
    lineBufRows[i] = lineBuf +
                     i * decomp.out_color_components * decomp.output_width;
  }
  bufPtr = bufEnd = lineBuf;

  // start up the decompression process
  jpeg_start_decompress(&decomp);
}

GBool DCTStream::checkSequentialInterleaved() {
  //~ this is unimplemented
  return gTrue;
}

void DCTStream::close() {
  // we don't call jpeg_finish_decompress() here because it will report
  // an error if the full image wasn't read
  if (setjmp(errorMgr.setjmpBuf)) {
    goto skip;
  }
  jpeg_destroy_decompress(&decomp);
 skip:
  gfree(lineBuf);
  FilterStream::close();
}

int DCTStream::getChar() {
  if (error) {
    return EOF;
  }
  if (bufPtr == bufEnd) {
    if (!fillBuf()) {
      return EOF;
    }
  }
  return *bufPtr++ & 0xff;
}

int DCTStream::lookChar() {
  if (error) {
    return EOF;
  }
  if (bufPtr == bufEnd) {
    if (!fillBuf()) {
      return EOF;
    }
  }
  return *bufPtr & 0xff;
}

int DCTStream::getBlock(char *blk, int size) {
  int nRead, nAvail, n;

  if (error) {
    return 0;
  }
  nRead = 0;
  while (nRead < size) {
    if (bufPtr == bufEnd) {
      if (!fillBuf()) {
	break;
      }
    }
    nAvail = bufEnd - bufPtr;
    n = (nAvail < size - nRead) ? nAvail : size - nRead;
    memcpy(blk + nRead, bufPtr, n);
    bufPtr += n;
    nRead += n;
  }
  return nRead;
}

GBool DCTStream::fillBuf() {
  int nLines;

  if (setjmp(errorMgr.setjmpBuf)) {
    error = gTrue;
    return gFalse;
  }
  nLines = jpeg_read_scanlines(&decomp, (JSAMPARRAY)lineBufRows,
			       lineBufHeight);
  bufPtr = lineBuf;
  bufEnd = lineBuf +
           nLines * decomp.out_color_components * decomp.output_width;
  return nLines > 0;
}

void DCTStream::errorExit(j_common_ptr d) {
  DCTErrorMgr *errMgr = (DCTErrorMgr *)d->err;
  longjmp(errMgr->setjmpBuf, 1);
}

void DCTStream::errorMessage(j_common_ptr d) {
#if 0 // for debugging
  char buf[JMSG_LENGTH_MAX];

  (*d->err->format_message)(d, buf);
  fprintf(stderr, "%s\n", buf);
#endif
}

void DCTStream::initSourceCbk(j_decompress_ptr d) {
  DCTSourceMgr *sourceMgr = (DCTSourceMgr *)d->src;

  sourceMgr->src.next_input_byte = NULL;
  sourceMgr->src.bytes_in_buffer = 0;
}

boolean DCTStream::fillInputBufferCbk(j_decompress_ptr d) {
  DCTSourceMgr *sourceMgr = (DCTSourceMgr *)d->src;
  int c, n;

  // for inline images, we need to read one byte at a time so we don't
  // read past the end of the input data
  if (sourceMgr->str->inlineImage) {
    c = sourceMgr->str->str->getChar();
    if (c == EOF) {
      sourceMgr->buf[0] = (char)0xff;
      sourceMgr->buf[1] = (char)JPEG_EOI;
      sourceMgr->src.bytes_in_buffer = 2;
    } else {
      sourceMgr->buf[0] = (char)c;
      sourceMgr->src.bytes_in_buffer = 1;
    }
  } else {
    n = sourceMgr->str->str->getBlock(sourceMgr->buf, dctStreamBufSize);
    if (n > 0) {
      sourceMgr->src.bytes_in_buffer = (size_t)n;
    } else {
      sourceMgr->buf[0] = (char)0xff;
      sourceMgr->buf[1] = (char)JPEG_EOI;
      sourceMgr->src.bytes_in_buffer = 2;
    }
  }
  sourceMgr->src.next_input_byte = (JOCTET *)sourceMgr->buf;
  return TRUE;
}

void DCTStream::skipInputDataCbk(j_decompress_ptr d, long numBytes) {
  DCTSourceMgr *sourceMgr = (DCTSourceMgr *)d->src;

  if (numBytes > 0) {
    if ((long)sourceMgr->src.bytes_in_buffer < numBytes) {
      sourceMgr->str->str->discardChars(
		   (Guint)(numBytes - sourceMgr->src.bytes_in_buffer));
      sourceMgr->src.bytes_in_buffer = 0;
    } else {
      sourceMgr->src.bytes_in_buffer -= numBytes;
      sourceMgr->src.next_input_byte += numBytes;
    }
  }
}

void DCTStream::termSourceCbk(j_decompress_ptr d) {
}

#else // HAVE_JPEGLIB

#define idctScaleA 1024
#define idctScaleB 1138
#define idctScaleC 1730
#define idctScaleD 1609
#define idctScaleE 1264
#define idctScaleF 1922
#define idctScaleG 1788
#define idctScaleH 2923
#define idctScaleI 2718
#define idctScaleJ 2528

static int idctScaleMat[64] = {
  idctScaleA, idctScaleB, idctScaleC, idctScaleD, idctScaleA, idctScaleD, idctScaleC, idctScaleB,
  idctScaleB, idctScaleE, idctScaleF, idctScaleG, idctScaleB, idctScaleG, idctScaleF, idctScaleE,
  idctScaleC, idctScaleF, idctScaleH, idctScaleI, idctScaleC, idctScaleI, idctScaleH, idctScaleF,
  idctScaleD, idctScaleG, idctScaleI, idctScaleJ, idctScaleD, idctScaleJ, idctScaleI, idctScaleG,
  idctScaleA, idctScaleB, idctScaleC, idctScaleD, idctScaleA, idctScaleD, idctScaleC, idctScaleB,
  idctScaleD, idctScaleG, idctScaleI, idctScaleJ, idctScaleD, idctScaleJ, idctScaleI, idctScaleG,
  idctScaleC, idctScaleF, idctScaleH, idctScaleI, idctScaleC, idctScaleI, idctScaleH, idctScaleF,
  idctScaleB, idctScaleE, idctScaleF, idctScaleG, idctScaleB, idctScaleG, idctScaleF, idctScaleE
};

// color conversion parameters (16.16 fixed point format)
#define dctCrToR   91881	//  1.4020
#define dctCbToG  -22553	// -0.3441363
#define dctCrToG  -46802	// -0.71413636
#define dctCbToB  116130	//  1.772

// The dctClip function clips signed integers to the [0,255] range.
// To handle valid DCT inputs, this must support an input range of at
// least [-256,511].  Invalid DCT inputs (e.g., from damaged PDF
// files) can result in arbitrary values, so we want to mask those
// out.  We round the input range size up to a power of 2 (so we can
// use a bit mask), which gives us an input range of [-384,639].  The
// end result is:
//     input       output
//     ----------  ------
//     <-384       X        invalid inputs -> output is "don't care"
//     -384..-257  0        invalid inputs, clipped
//     -256..-1    0        valid inputs, need to be clipped
//     0..255      0..255
//     256..511    255      valid inputs, need to be clipped
//     512..639    255      invalid inputs, clipped
//     >=512       X        invalid inputs -> output is "don't care"

#define dctClipOffset  384
#define dctClipMask   1023
static Guchar dctClipData[1024];

static inline void dctClipInit() {
  static int initDone = 0;
  int i;
  if (!initDone) {
    for (i = -384; i < 0; ++i) {
      dctClipData[dctClipOffset + i] = 0;
    }
    for (i = 0; i < 256; ++i) {
      dctClipData[dctClipOffset + i] = (Guchar)i;
    }
    for (i = 256; i < 639; ++i) {
      dctClipData[dctClipOffset + i] = 255;
    }
    initDone = 1;
  }
}

static inline Guchar dctClip(int x) {
  return dctClipData[(dctClipOffset + x) & dctClipMask];
}

// zig zag decode map
static int dctZigZag[64] = {
   0,
   1,  8,
  16,  9,  2,
   3, 10, 17, 24,
  32, 25, 18, 11, 4,
   5, 12, 19, 26, 33, 40,
  48, 41, 34, 27, 20, 13,  6,
   7, 14, 21, 28, 35, 42, 49, 56,
  57, 50, 43, 36, 29, 22, 15,
  23, 30, 37, 44, 51, 58,
  59, 52, 45, 38, 31,
  39, 46, 53, 60,
  61, 54, 47,
  55, 62,
  63
};

DCTStream::DCTStream(Stream *strA, GBool colorXformA):
    FilterStream(strA) {
  int i;

  colorXform = colorXformA;
  progressive = interleaved = gFalse;
  width = height = 0;
  mcuWidth = mcuHeight = 0;
  numComps = 0;
  comp = 0;
  x = y = 0;
  for (i = 0; i < 4; ++i) {
    frameBuf[i] = NULL;
  }
  rowBuf = NULL;
  memset(dcHuffTables, 0, sizeof(dcHuffTables));
  memset(acHuffTables, 0, sizeof(acHuffTables));

  dctClipInit();
}

DCTStream::~DCTStream() {
  close();
  delete str;
}

Stream *DCTStream::copy() {
  return new DCTStream(str->copy(), colorXform);
}

void DCTStream::reset() {
  int i;

  str->reset();

  progressive = interleaved = gFalse;
  width = height = 0;
  numComps = 0;
  numQuantTables = 0;
  numDCHuffTables = 0;
  numACHuffTables = 0;
  gotJFIFMarker = gFalse;
  gotAdobeMarker = gFalse;
  restartInterval = 0;

  if (!readHeader(gTrue)) {
    // force an EOF condition
    progressive = gTrue;
    y = height;
    return;
  }

  // compute MCU size
  if (numComps == 1) {
    compInfo[0].hSample = compInfo[0].vSample = 1;
  }
  mcuWidth = compInfo[0].hSample;
  mcuHeight = compInfo[0].vSample;
  for (i = 1; i < numComps; ++i) {
    if (compInfo[i].hSample > mcuWidth) {
      mcuWidth = compInfo[i].hSample;
    }
    if (compInfo[i].vSample > mcuHeight) {
      mcuHeight = compInfo[i].vSample;
    }
  }
  mcuWidth *= 8;
  mcuHeight *= 8;

  // figure out color transform
  if (colorXform == -1) {
    if (numComps == 3) {
      if (gotJFIFMarker) {
	colorXform = 1;
      } else if (compInfo[0].id == 82 && compInfo[1].id == 71 &&
		 compInfo[2].id == 66) { // ASCII "RGB"
	colorXform = 0;
      } else {
	colorXform = 1;
      }
    } else {
      colorXform = 0;
    }
  }

  if (progressive || !interleaved) {

    // allocate a buffer for the whole image
    bufWidth = ((width + mcuWidth - 1) / mcuWidth) * mcuWidth;
    bufHeight = ((height + mcuHeight - 1) / mcuHeight) * mcuHeight;
    if (bufWidth <= 0 || bufHeight <= 0 ||
	bufWidth > INT_MAX / bufWidth / (int)sizeof(int)) {
      error(errSyntaxError, getPos(), "Invalid image size in DCT stream");
      y = height;
      return;
    }
    for (i = 0; i < numComps; ++i) {
      frameBuf[i] = (int *)gmallocn(bufWidth * bufHeight, sizeof(int));
      memset(frameBuf[i], 0, bufWidth * bufHeight * sizeof(int));
    }

    // read the image data
    do {
      restartMarker = 0xd0;
      restart();
      readScan();
    } while (readHeader(gFalse));

    // decode
    decodeImage();

    // initialize counters
    comp = 0;
    x = 0;
    y = 0;

  } else {

    if (scanInfo.numComps != numComps) {
      error(errSyntaxError, getPos(), "Invalid scan in sequential DCT stream");
      y = height;
      return;
    }

    // allocate a buffer for one row of MCUs
    bufWidth = ((width + mcuWidth - 1) / mcuWidth) * mcuWidth;
    rowBuf = (Guchar *)gmallocn(numComps * mcuHeight, bufWidth);
    rowBufPtr = rowBufEnd = rowBuf;

    // initialize counters
    y = -mcuHeight;

    restartMarker = 0xd0;
    restart();
  }
}

GBool DCTStream::checkSequentialInterleaved() {
  GBool headerOk;

  str->reset();

  progressive = interleaved = gFalse;
  width = height = 0;
  numComps = 0;
  numQuantTables = 0;
  numDCHuffTables = 0;
  numACHuffTables = 0;
  gotJFIFMarker = gFalse;
  gotAdobeMarker = gFalse;
  restartInterval = 0;

  headerOk = readHeader(gTrue);

  FilterStream::close();

  return headerOk && !progressive && interleaved;
}

void DCTStream::close() {
  int i;

  for (i = 0; i < 4; ++i) {
    gfree(frameBuf[i]);
    frameBuf[i] = NULL;
  }
  gfree(rowBuf);
  rowBuf = NULL;
  FilterStream::close();
}

int DCTStream::getChar() {
  int c;

  if (progressive || !interleaved) {
    if (y >= height) {
      return EOF;
    }
    c = frameBuf[comp][y * bufWidth + x];
    if (++comp == numComps) {
      comp = 0;
      if (++x == width) {
	x = 0;
	++y;
      }
    }
  } else {
    if (rowBufPtr == rowBufEnd) {
      if (y + mcuHeight >= height) {
	return EOF;
      }
      y += mcuHeight;
      if (!readMCURow()) {
	y = height;
	return EOF;
      }
    }
    c = *rowBufPtr++;
  }
  return c;
}

int DCTStream::lookChar() {
  if (progressive || !interleaved) {
    if (y >= height) {
      return EOF;
    }
    return frameBuf[comp][y * bufWidth + x];
  } else {
    if (rowBufPtr == rowBufEnd) {
      if (y + mcuHeight >= height) {
	return EOF;
      }
      if (!readMCURow()) {
	y = height;
	return EOF;
      }
    }
    return *rowBufPtr;
  }
}

int DCTStream::getBlock(char *blk, int size) {
  int nRead, nAvail, n;

  if (progressive || !interleaved) {
    if (y >= height) {
      return 0;
    }
    for (nRead = 0; nRead < size; ++nRead) {
      blk[nRead] = (char)frameBuf[comp][y * bufWidth + x];
      if (++comp == numComps) {
	comp = 0;
	if (++x == width) {
	  x = 0;
	  ++y;
	  if (y >= height) {
	    ++nRead;
	    break;
	  }
	}
      }
    }
  } else {
    nRead = 0;
    while (nRead < size) {
      if (rowBufPtr == rowBufEnd) {
	if (y + mcuHeight >= height) {
	  break;
	}
	y += mcuHeight;
	if (!readMCURow()) {
	  y = height;
	  break;
	}
      }
      nAvail = (int)(rowBufEnd - rowBufPtr);
      n = (nAvail < size - nRead) ? nAvail : size - nRead;
      memcpy(blk + nRead, rowBufPtr, n);
      rowBufPtr += n;
      nRead += n;
    }
  }
  return nRead;
}

void DCTStream::restart() {
  int i;

  inputBits = 0;
  restartCtr = restartInterval;
  for (i = 0; i < numComps; ++i) {
    compInfo[i].prevDC = 0;
  }
  eobRun = 0;
}

// Read one row of MCUs from a sequential JPEG stream.
GBool DCTStream::readMCURow() {
  int data1[64];
  Guchar data2[64];
  Guchar *p1, *p2;
  int pY, pCb, pCr, pR, pG, pB;
  int h, v, horiz, vert, hSub, vSub;
  int x1, x2, y2, x3, y3, x4, y4, x5, y5, cc, i;
  int c;

  for (cc = 0; cc < numComps; ++cc) {
    if (scanInfo.dcHuffTable[cc] >= numDCHuffTables ||
	scanInfo.acHuffTable[cc] >= numACHuffTables) {
      error(errSyntaxError, getPos(),
	    "Bad DCT data: invalid Huffman table index");
      return gFalse;
    }
    if (compInfo[cc].quantTable > numQuantTables) {
      error(errSyntaxError, getPos(),
	    "Bad DCT data: invalid quant table index");
      return gFalse;
    }
  }

  for (x1 = 0; x1 < width; x1 += mcuWidth) {

    // deal with restart marker
    if (restartInterval > 0 && restartCtr == 0) {
      c = readMarker();
      if (c != restartMarker) {
	error(errSyntaxError, getPos(),
	      "Bad DCT data: incorrect restart marker");
	return gFalse;
      }
      if (++restartMarker == 0xd8)
	restartMarker = 0xd0;
      restart();
    }

    // read one MCU
    for (cc = 0; cc < numComps; ++cc) {
      h = compInfo[cc].hSample;
      v = compInfo[cc].vSample;
      horiz = mcuWidth / h;
      vert = mcuHeight / v;
      hSub = horiz / 8;
      vSub = vert / 8;
      for (y2 = 0; y2 < mcuHeight; y2 += vert) {
	for (x2 = 0; x2 < mcuWidth; x2 += horiz) {
	  if (!readDataUnit(&dcHuffTables[scanInfo.dcHuffTable[cc]],
			    &acHuffTables[scanInfo.acHuffTable[cc]],
			    &compInfo[cc].prevDC,
			    data1)) {
	    return gFalse;
	  }
	  transformDataUnit(quantTables[compInfo[cc].quantTable],
			    data1, data2);
	  if (hSub == 1 && vSub == 1 && x1+x2+8 <= width) {
	    for (y3 = 0, i = 0; y3 < 8; ++y3, i += 8) {
	      p1 = &rowBuf[((y2+y3) * width + (x1+x2)) * numComps + cc];
	      p1[0]          = data2[i];
	      p1[  numComps] = data2[i+1];
	      p1[2*numComps] = data2[i+2];
	      p1[3*numComps] = data2[i+3];
	      p1[4*numComps] = data2[i+4];
	      p1[5*numComps] = data2[i+5];
	      p1[6*numComps] = data2[i+6];
	      p1[7*numComps] = data2[i+7];
	    }
	  } else if (hSub == 2 && vSub == 2 && x1+x2+16 <= width) {
	    for (y3 = 0, i = 0; y3 < 16; y3 += 2, i += 8) {
	      p1 = &rowBuf[((y2+y3) * width + (x1+x2)) * numComps + cc];
	      p2 = p1 + width * numComps;
	      p1[0] = p1[numComps] =
		p2[0] = p2[numComps] = data2[i];
	      p1[2*numComps] = p1[3*numComps] =
		p2[2*numComps] = p2[3*numComps] = data2[i+1];
	      p1[4*numComps] = p1[5*numComps] =
		p2[4*numComps] = p2[5*numComps] = data2[i+2];
	      p1[6*numComps] = p1[7*numComps] =
		p2[6*numComps] = p2[7*numComps] = data2[i+3];
	      p1[8*numComps] = p1[9*numComps] =
		p2[8*numComps] = p2[9*numComps] = data2[i+4];
	      p1[10*numComps] = p1[11*numComps] =
		p2[10*numComps] = p2[11*numComps] = data2[i+5];
	      p1[12*numComps] = p1[13*numComps] =
		p2[12*numComps] = p2[13*numComps] = data2[i+6];
	      p1[14*numComps] = p1[15*numComps] =
		p2[14*numComps] = p2[15*numComps] = data2[i+7];
	    }
	  } else {
	    p1 = &rowBuf[(y2 * width + (x1+x2)) * numComps + cc];
	    i = 0;
	    for (y3 = 0, y4 = 0; y3 < 8; ++y3, y4 += vSub) {
	      for (x3 = 0, x4 = 0; x3 < 8; ++x3, x4 += hSub) {
		for (y5 = 0; y5 < vSub; ++y5) {
		  for (x5 = 0; x5 < hSub && x1+x2+x4+x5 < width; ++x5) {
		    p1[((y4+y5) * width + (x4+x5)) * numComps] = data2[i];
		  }
		}
		++i;
	      }
	    }
	  }
	}
      }
    }
    --restartCtr;
  }

  // color space conversion
  if (colorXform) {
    // convert YCbCr to RGB
    if (numComps == 3) {
      for (i = 0, p1 = rowBuf; i < width * mcuHeight; ++i, p1 += 3) {
	pY = p1[0];
	pCb = p1[1] - 128;
	pCr = p1[2] - 128;
	pR = ((pY << 16) + dctCrToR * pCr + 32768) >> 16;
	p1[0] = dctClip(pR);
	pG = ((pY << 16) + dctCbToG * pCb + dctCrToG * pCr + 32768) >> 16;
	p1[1] = dctClip(pG);
	pB = ((pY << 16) + dctCbToB * pCb + 32768) >> 16;
	p1[2] = dctClip(pB);
      }
    // convert YCbCrK to CMYK (K is passed through unchanged)
    } else if (numComps == 4) {
      for (i = 0, p1 = rowBuf; i < width * mcuHeight; ++i, p1 += 4) {
	pY = p1[0];
	pCb = p1[1] - 128;
	pCr = p1[2] - 128;
	pR = ((pY << 16) + dctCrToR * pCr + 32768) >> 16;
	p1[0] = (Guchar)(255 - dctClip(pR));
	pG = ((pY << 16) + dctCbToG * pCb + dctCrToG * pCr + 32768) >> 16;
	p1[1] = (Guchar)(255 - dctClip(pG));
	pB = ((pY << 16) + dctCbToB * pCb + 32768) >> 16;
	p1[2] = (Guchar)(255 - dctClip(pB));
      }
    }
  }

  rowBufPtr = rowBuf;
  if (y + mcuHeight <= height) {
    rowBufEnd = rowBuf + numComps * width * mcuHeight;
  } else {
    rowBufEnd = rowBuf + numComps * width * (height - y);
  }

  return gTrue;
}

// Read one scan from a progressive or non-interleaved JPEG stream.
void DCTStream::readScan() {
  int data[64];
  int x1, y1, dx1, dy1, x2, y2, y3, cc, i;
  int h, v, horiz, vert, vSub;
  int *p1;
  int c;

  for (cc = 0; cc < numComps; ++cc) {
    if (scanInfo.comp[cc] &&
	(scanInfo.dcHuffTable[cc] >= numDCHuffTables ||
	 ((!progressive || scanInfo.lastCoeff > 0) &&
	  scanInfo.acHuffTable[cc] >= numACHuffTables))) {
      error(errSyntaxError, getPos(),
	    "Bad DCT data: invalid Huffman table index");
      return;
    }
    if (compInfo[cc].quantTable > numQuantTables) {
      error(errSyntaxError, getPos(),
	    "Bad DCT data: invalid quant table index");
      return;
    }
  }

  if (scanInfo.numComps == 1) {
    for (cc = 0; cc < numComps; ++cc) {
      if (scanInfo.comp[cc]) {
	break;
      }
    }
    dx1 = mcuWidth / compInfo[cc].hSample;
    dy1 = mcuHeight / compInfo[cc].vSample;
  } else {
    dx1 = mcuWidth;
    dy1 = mcuHeight;
  }

  for (y1 = 0; y1 < height; y1 += dy1) {
    for (x1 = 0; x1 < width; x1 += dx1) {

      // deal with restart marker
      if (restartInterval > 0 && restartCtr == 0) {
	c = readMarker();
	if (c != restartMarker) {
	  error(errSyntaxError, getPos(),
		"Bad DCT data: incorrect restart marker");
	  return;
	}
	if (++restartMarker == 0xd8) {
	  restartMarker = 0xd0;
	}
	restart();
      }

      // read one MCU
      for (cc = 0; cc < numComps; ++cc) {
	if (!scanInfo.comp[cc]) {
	  continue;
	}

	h = compInfo[cc].hSample;
	v = compInfo[cc].vSample;
	horiz = mcuWidth / h;
	vert = mcuHeight / v;
	vSub = vert / 8;
	for (y2 = 0; y2 < dy1; y2 += vert) {
	  for (x2 = 0; x2 < dx1; x2 += horiz) {

	    // pull out the current values
	    p1 = &frameBuf[cc][(y1+y2) * bufWidth + (x1+x2)];
	    for (y3 = 0, i = 0; y3 < 8; ++y3, i += 8) {
	      data[i] = p1[0];
	      data[i+1] = p1[1];
	      data[i+2] = p1[2];
	      data[i+3] = p1[3];
	      data[i+4] = p1[4];
	      data[i+5] = p1[5];
	      data[i+6] = p1[6];
	      data[i+7] = p1[7];
	      p1 += bufWidth * vSub;
	    }

	    // read one data unit
	    if (progressive) {
	      if (!readProgressiveDataUnit(
		       &dcHuffTables[scanInfo.dcHuffTable[cc]],
		       &acHuffTables[scanInfo.acHuffTable[cc]],
		       &compInfo[cc].prevDC,
		       data)) {
		return;
	      }
	    } else {
	      if (!readDataUnit(&dcHuffTables[scanInfo.dcHuffTable[cc]],
				&acHuffTables[scanInfo.acHuffTable[cc]],
				&compInfo[cc].prevDC,
				data)) {
		return;
	      }
	    }

	    // add the data unit into frameBuf
	    p1 = &frameBuf[cc][(y1+y2) * bufWidth + (x1+x2)];
	    for (y3 = 0, i = 0; y3 < 8; ++y3, i += 8) {
	      p1[0] = data[i];
	      p1[1] = data[i+1];
	      p1[2] = data[i+2];
	      p1[3] = data[i+3];
	      p1[4] = data[i+4];
	      p1[5] = data[i+5];
	      p1[6] = data[i+6];
	      p1[7] = data[i+7];
	      p1 += bufWidth * vSub;
	    }
	  }
	}
      }
      --restartCtr;
    }
  }
}

// Read one data unit from a sequential JPEG stream.
GBool DCTStream::readDataUnit(DCTHuffTable *dcHuffTable,
			      DCTHuffTable *acHuffTable,
			      int *prevDC, int data[64]) {
  int run, size, amp;
  int c;
  int i, j;

  if ((size = readHuffSym(dcHuffTable)) == 9999) {
    return gFalse;
  }
  if (size > 0) {
    if ((amp = readAmp(size)) == 9999) {
      return gFalse;
    }
  } else {
    amp = 0;
  }
  data[0] = *prevDC += amp;
  for (i = 1; i < 64; ++i) {
    data[i] = 0;
  }
  i = 1;
  while (i < 64) {
    run = 0;
    while ((c = readHuffSym(acHuffTable)) == 0xf0 && run < 0x30) {
      run += 0x10;
    }
    if (c == 9999) {
      return gFalse;
    }
    if (c == 0x00) {
      break;
    } else {
      run += (c >> 4) & 0x0f;
      size = c & 0x0f;
      amp = readAmp(size);
      if (amp == 9999) {
	return gFalse;
      }
      i += run;
      if (i < 64) {
	j = dctZigZag[i++];
	data[j] = amp;
      }
    }
  }
  return gTrue;
}

// Read one data unit from a progressive JPEG stream.
GBool DCTStream::readProgressiveDataUnit(DCTHuffTable *dcHuffTable,
					 DCTHuffTable *acHuffTable,
					 int *prevDC, int data[64]) {
  int run, size, amp, bit, c;
  int i, j, k;

  // get the DC coefficient
  i = scanInfo.firstCoeff;
  if (i == 0) {
    if (scanInfo.ah == 0) {
      if ((size = readHuffSym(dcHuffTable)) == 9999) {
	return gFalse;
      }
      if (size > 0) {
	if ((amp = readAmp(size)) == 9999) {
	  return gFalse;
	}
      } else {
	amp = 0;
      }
      data[0] += (*prevDC += amp) << scanInfo.al;
    } else {
      if ((bit = readBit()) == 9999) {
	return gFalse;
      }
      if (bit) {
	data[0] += 1 << scanInfo.al;
      }
    }
    ++i;
  }
  if (scanInfo.lastCoeff == 0) {
    return gTrue;
  }

  // check for an EOB run
  if (eobRun > 0) {
    while (i <= scanInfo.lastCoeff) {
      j = dctZigZag[i++];
      if (data[j] != 0) {
	if ((bit = readBit()) == EOF) {
	  return gFalse;
	}
	if (bit) {
	  if (data[j] >= 0) {
	    data[j] += 1 << scanInfo.al;
	  } else {
	    data[j] -= 1 << scanInfo.al;
	  }
	}
      }
    }
    --eobRun;
    return gTrue;
  }

  // read the AC coefficients
  while (i <= scanInfo.lastCoeff) {
    if ((c = readHuffSym(acHuffTable)) == 9999) {
      return gFalse;
    }

    // ZRL
    if (c == 0xf0) {
      k = 0;
      while (k < 16 && i <= scanInfo.lastCoeff) {
	j = dctZigZag[i++];
	if (data[j] == 0) {
	  ++k;
	} else {
	  if ((bit = readBit()) == EOF) {
	    return gFalse;
	  }
	  if (bit) {
	    if (data[j] >= 0) {
	      data[j] += 1 << scanInfo.al;
	    } else {
	      data[j] -= 1 << scanInfo.al;
	    }
	  }
	}
      }

    // EOB run
    } else if ((c & 0x0f) == 0x00) {
      j = c >> 4;
      eobRun = 0;
      for (k = 0; k < j; ++k) {
	if ((bit = readBit()) == EOF) {
	  return gFalse;
	}
	eobRun = (eobRun << 1) | bit;
      }
      eobRun += 1 << j;
      while (i <= scanInfo.lastCoeff) {
	j = dctZigZag[i++];
	if (data[j] != 0) {
	  if ((bit = readBit()) == EOF) {
	    return gFalse;
	  }
	  if (bit) {
	    if (data[j] >= 0) {
	      data[j] += 1 << scanInfo.al;
	    } else {
	      data[j] -= 1 << scanInfo.al;
	    }
	  }
	}
      }
      --eobRun;
      break;

    // zero run and one AC coefficient
    } else {
      run = (c >> 4) & 0x0f;
      size = c & 0x0f;
      if ((amp = readAmp(size)) == 9999) {
	return gFalse;
      }
      j = 0; // make gcc happy
      for (k = 0; k <= run && i <= scanInfo.lastCoeff; ++k) {
	j = dctZigZag[i++];
	while (data[j] != 0 && i <= scanInfo.lastCoeff) {
	  if ((bit = readBit()) == EOF) {
	    return gFalse;
	  }
	  if (bit) {
	    if (data[j] >= 0) {
	      data[j] += 1 << scanInfo.al;
	    } else {
	      data[j] -= 1 << scanInfo.al;
	    }
	  }
	  j = dctZigZag[i++];
	}
      }
      data[j] = amp << scanInfo.al;
    }
  }

  return gTrue;
}

// Decode a progressive JPEG image.
void DCTStream::decodeImage() {
  int dataIn[64];
  Guchar dataOut[64];
  Gushort *quantTable;
  int pY, pCb, pCr, pR, pG, pB;
  int x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, cc, i;
  int h, v, horiz, vert, hSub, vSub;
  int *p0, *p1, *p2;

  for (y1 = 0; y1 < bufHeight; y1 += mcuHeight) {
    for (x1 = 0; x1 < bufWidth; x1 += mcuWidth) {
      for (cc = 0; cc < numComps; ++cc) {
	quantTable = quantTables[compInfo[cc].quantTable];
	h = compInfo[cc].hSample;
	v = compInfo[cc].vSample;
	horiz = mcuWidth / h;
	vert = mcuHeight / v;
	hSub = horiz / 8;
	vSub = vert / 8;
	for (y2 = 0; y2 < mcuHeight; y2 += vert) {
	  for (x2 = 0; x2 < mcuWidth; x2 += horiz) {

	    // pull out the coded data unit
	    p1 = &frameBuf[cc][(y1+y2) * bufWidth + (x1+x2)];
	    for (y3 = 0, i = 0; y3 < 8; ++y3, i += 8) {
	      dataIn[i]   = p1[0];
	      dataIn[i+1] = p1[1];
	      dataIn[i+2] = p1[2];
	      dataIn[i+3] = p1[3];
	      dataIn[i+4] = p1[4];
	      dataIn[i+5] = p1[5];
	      dataIn[i+6] = p1[6];
	      dataIn[i+7] = p1[7];
	      p1 += bufWidth * vSub;
	    }

	    // transform
	    transformDataUnit(quantTable, dataIn, dataOut);

	    // store back into frameBuf, doing replication for
	    // subsampled components
	    p1 = &frameBuf[cc][(y1+y2) * bufWidth + (x1+x2)];
	    if (hSub == 1 && vSub == 1) {
	      for (y3 = 0, i = 0; y3 < 8; ++y3, i += 8) {
		p1[0] = dataOut[i] & 0xff;
		p1[1] = dataOut[i+1] & 0xff;
		p1[2] = dataOut[i+2] & 0xff;
		p1[3] = dataOut[i+3] & 0xff;
		p1[4] = dataOut[i+4] & 0xff;
		p1[5] = dataOut[i+5] & 0xff;
		p1[6] = dataOut[i+6] & 0xff;
		p1[7] = dataOut[i+7] & 0xff;
		p1 += bufWidth;
	      }
	    } else if (hSub == 2 && vSub == 2) {
	      p2 = p1 + bufWidth;
	      for (y3 = 0, i = 0; y3 < 16; y3 += 2, i += 8) {
		p1[0] = p1[1] = p2[0] = p2[1] = dataOut[i] & 0xff;
		p1[2] = p1[3] = p2[2] = p2[3] = dataOut[i+1] & 0xff;
		p1[4] = p1[5] = p2[4] = p2[5] = dataOut[i+2] & 0xff;
		p1[6] = p1[7] = p2[6] = p2[7] = dataOut[i+3] & 0xff;
		p1[8] = p1[9] = p2[8] = p2[9] = dataOut[i+4] & 0xff;
		p1[10] = p1[11] = p2[10] = p2[11] = dataOut[i+5] & 0xff;
		p1[12] = p1[13] = p2[12] = p2[13] = dataOut[i+6] & 0xff;
		p1[14] = p1[15] = p2[14] = p2[15] = dataOut[i+7] & 0xff;
		p1 += bufWidth * 2;
		p2 += bufWidth * 2;
	      }
	    } else {
	      i = 0;
	      for (y3 = 0, y4 = 0; y3 < 8; ++y3, y4 += vSub) {
		for (x3 = 0, x4 = 0; x3 < 8; ++x3, x4 += hSub) {
		  p2 = p1 + x4;
		  for (y5 = 0; y5 < vSub; ++y5) {
		    for (x5 = 0; x5 < hSub; ++x5) {
		      p2[x5] = dataOut[i] & 0xff;
		    }
		    p2 += bufWidth;
		  }
		  ++i;
		}
		p1 += bufWidth * vSub;
	      }
	    }
	  }
	}
      }

      // color space conversion
      if (colorXform) {
	// convert YCbCr to RGB
	if (numComps == 3) {
	  for (y2 = 0; y2 < mcuHeight; ++y2) {
	    p0 = &frameBuf[0][(y1+y2) * bufWidth + x1];
	    p1 = &frameBuf[1][(y1+y2) * bufWidth + x1];
	    p2 = &frameBuf[2][(y1+y2) * bufWidth + x1];
	    for (x2 = 0; x2 < mcuWidth; ++x2) {
	      pY = *p0;
	      pCb = *p1 - 128;
	      pCr = *p2 - 128;
	      pR = ((pY << 16) + dctCrToR * pCr + 32768) >> 16;
	      *p0++ = dctClip(pR);
	      pG = ((pY << 16) + dctCbToG * pCb + dctCrToG * pCr +
		    32768) >> 16;
	      *p1++ = dctClip(pG);
	      pB = ((pY << 16) + dctCbToB * pCb + 32768) >> 16;
	      *p2++ = dctClip(pB);
	    }
	  }
	// convert YCbCrK to CMYK (K is passed through unchanged)
	} else if (numComps == 4) {
	  for (y2 = 0; y2 < mcuHeight; ++y2) {
	    p0 = &frameBuf[0][(y1+y2) * bufWidth + x1];
	    p1 = &frameBuf[1][(y1+y2) * bufWidth + x1];
	    p2 = &frameBuf[2][(y1+y2) * bufWidth + x1];
	    for (x2 = 0; x2 < mcuWidth; ++x2) {
	      pY = *p0;
	      pCb = *p1 - 128;
	      pCr = *p2 - 128;
	      pR = ((pY << 16) + dctCrToR * pCr + 32768) >> 16;
	      *p0++ = 255 - dctClip(pR);
	      pG = ((pY << 16) + dctCbToG * pCb + dctCrToG * pCr +
		    32768) >> 16;
	      *p1++ = 255 - dctClip(pG);
	      pB = ((pY << 16) + dctCbToB * pCb + 32768) >> 16;
	      *p2++ = 255 - dctClip(pB);
	    }
	  }
	}
      }
    }
  }
}

// Transform one data unit -- this performs the dequantization and
// IDCT steps.  This IDCT algorithm is taken from:
//   Y. A. Reznik, A. T. Hinds, L. Yu, Z. Ni, and C-X. Zhang,
//   "Efficient fixed-point approximations of the 8x8 inverse discrete
//   cosine transform" (invited paper), Proc. SPIE Vol. 6696, Sep. 24,
//   2007.
// which is based on:
//   Christoph Loeffler, Adriaan Ligtenberg, George S. Moschytz,
//   "Practical Fast 1-D DCT Algorithms with 11 Multiplications",
//   IEEE Intl. Conf. on Acoustics, Speech & Signal Processing, 1989,
//   988-991.
// The stage numbers mentioned in the comments refer to Figure 1 in the
// Loeffler paper.
void DCTStream::transformDataUnit(Gushort *quantTable,
				  int dataIn[64], Guchar dataOut[64]) {
  int v0, v1, v2, v3, v4, v5, v6, v7;
  int t0, t1, t2, t3, t4, t5, t6, t7;
  int *p, *scale;
  Gushort *q;
  int i;

  // dequant; inverse DCT on rows
  for (i = 0; i < 64; i += 8) {
    p = dataIn + i;
    q = quantTable + i;
    scale = idctScaleMat + i;

    // check for all-zero AC coefficients
    if (p[1] == 0 && p[2] == 0 && p[3] == 0 &&
	p[4] == 0 && p[5] == 0 && p[6] == 0 && p[7] == 0) {
      t0 = p[0] * q[0] * scale[0];
      if (i == 0) {
	t0 += 1 << 12;		// rounding bias
      }
      p[0] = t0;
      p[1] = t0;
      p[2] = t0;
      p[3] = t0;
      p[4] = t0;
      p[5] = t0;
      p[6] = t0;
      p[7] = t0;
      continue;
    }

    // stage 4
    v0 = p[0] * q[0] * scale[0];
    if (i == 0) {
      v0 += 1 << 12;		// rounding bias
    }
    v1 = p[4] * q[4] * scale[4];
    v2 = p[2] * q[2] * scale[2];
    v3 = p[6] * q[6] * scale[6];
    t0 = p[1] * q[1] * scale[1];
    t1 = p[7] * q[7] * scale[7];
    v4 = t0 - t1;
    v7 = t0 + t1;
    v5 = p[3] * q[3] * scale[3];
    v6 = p[5] * q[5] * scale[5];

    // stage 3
    t0 = v0 - v1;
    v0 = v0 + v1;
    v1 = t0;
    t0 = v2 + (v2 >> 5);
    t1 = t0 >> 2;
    t2 = t1 + (v2 >> 4);	// 41/128 * v2
    t3 = t0 - t1;		// 99/128 * v2
    t4 = v3 + (v3 >> 5);
    t5 = t4 >> 2;
    t6 = t5 + (v3 >> 4);	// 41/128 * v3
    t7 = t4 - t5;		// 99/128 * v3
    v2 = t2 - t7;
    v3 = t3 + t6;
    t0 = v4 - v6;
    v4 = v4 + v6;
    v6 = t0;
    t0 = v7 + v5;
    v5 = v7 - v5;
    v7 = t0;

    // stage 2
    t0 = v0 - v3;
    v0 = v0 + v3;
    v3 = t0;
    t0 = v1 - v2;
    v1 = v1 + v2;
    v2 = t0;
    t0 = (v4 >> 9) - v4;
    t1 = v4 >> 1;		// 1/2 * v4
    t2 = (t0 >> 2) - t0;	// 1533/2048 * v4
    t3 = (v7 >> 9) - v7;
    t4 = v7 >> 1;		// 1/2 * v7
    t5 = (t3 >> 2) - t3;	// 1533/2048 * v7
    v4 = t2 - t4;
    v7 = t1 + t5;
    t0 = (v5 >> 3) - (v5 >> 7);
    t1 = t0 - (v5 >> 11);
    t2 = t0 + (t1 >> 1);	// 719/4096 * v5
    t3 = v5 - t0;		// 113/256 * v5
    t4 = (v6 >> 3) - (v6 >> 7);
    t5 = t4 - (v6 >> 11);
    t6 = t4 + (t5 >> 1);	// 719/4096 * v6
    t7 = v6 - t4;		// 113/256 * v6
    v5 = t3 - t6;
    v6 = t2 + t7;

    // stage 1
    p[0] = v0 + v7;
    p[7] = v0 - v7;
    p[1] = v1 + v6;
    p[6] = v1 - v6;
    p[2] = v2 + v5;
    p[5] = v2 - v5;
    p[3] = v3 + v4;
    p[4] = v3 - v4;
  }

  // inverse DCT on columns
  for (i = 0; i < 8; ++i) {
    p = dataIn + i;

    // check for all-zero AC coefficients
    if (p[1*8] == 0 && p[2*8] == 0 && p[3*8] == 0 &&
	p[4*8] == 0 && p[5*8] == 0 && p[6*8] == 0 && p[7*8] == 0) {
      t0 = p[0*8];
      p[1*8] = t0;
      p[2*8] = t0;
      p[3*8] = t0;
      p[4*8] = t0;
      p[5*8] = t0;
      p[6*8] = t0;
      p[7*8] = t0;
      continue;
    }

    // stage 4
    v0 = p[0*8];
    v1 = p[4*8];
    v2 = p[2*8];
    v3 = p[6*8];
    t0 = p[1*8];
    t1 = p[7*8];
    v4 = t0 - t1;
    v7 = t0 + t1;
    v5 = p[3*8];
    v6 = p[5*8];

    // stage 3
    t0 = v0 - v1;
    v0 = v0 + v1;
    v1 = t0;
    t0 = v2 + (v2 >> 5);
    t1 = t0 >> 2;
    t2 = t1 + (v2 >> 4);	// 41/128 * v2
    t3 = t0 - t1;		// 99/128 * v2
    t4 = v3 + (v3 >> 5);
    t5 = t4 >> 2;
    t6 = t5 + (v3 >> 4);	// 41/128 * v3
    t7 = t4 - t5;		// 99/128 * v3
    v2 = t2 - t7;
    v3 = t3 + t6;
    t0 = v4 - v6;
    v4 = v4 + v6;
    v6 = t0;
    t0 = v7 + v5;
    v5 = v7 - v5;
    v7 = t0;

    // stage 2
    t0 = v0 - v3;
    v0 = v0 + v3;
    v3 = t0;
    t0 = v1 - v2;
    v1 = v1 + v2;
    v2 = t0;
    t0 = (v4 >> 9) - v4;
    t1 = v4 >> 1;		// 1/2 * v4
    t2 = (t0 >> 2) - t0;	// 1533/2048 * v4
    t3 = (v7 >> 9) - v7;
    t4 = v7 >> 1;		// 1/2 * v7
    t5 = (t3 >> 2) - t3;	// 1533/2048 * v7
    v4 = t2 - t4;
    v7 = t1 + t5;
    t0 = (v5 >> 3) - (v5 >> 7);
    t1 = t0 - (v5 >> 11);
    t2 = t0 + (t1 >> 1);	// 719/4096 * v5
    t3 = v5 - t0;		// 113/256 * v5
    t4 = (v6 >> 3) - (v6 >> 7);
    t5 = t4 - (v6 >> 11);
    t6 = t4 + (t5 >> 1);	// 719/4096 * v6
    t7 = v6 - t4;		// 113/256 * v6
    v5 = t3 - t6;
    v6 = t2 + t7;

    // stage 1
    p[0*8] = v0 + v7;
    p[7*8] = v0 - v7;
    p[1*8] = v1 + v6;
    p[6*8] = v1 - v6;
    p[2*8] = v2 + v5;
    p[5*8] = v2 - v5;
    p[3*8] = v3 + v4;
    p[4*8] = v3 - v4;
  }

  // convert to 8-bit integers
  for (i = 0; i < 64; ++i) {
    dataOut[i] = dctClip(128 + (dataIn[i] >> 13));
  }
}

int DCTStream::readHuffSym(DCTHuffTable *table) {
  Gushort code;
  int bit;
  int codeBits;

  code = 0;
  codeBits = 0;
  do {
    // add a bit to the code
    if ((bit = readBit()) == EOF) {
      return 9999;
    }
    code = (Gushort)((code << 1) + bit);
    ++codeBits;

    // look up code
    if (code < table->firstCode[codeBits]) {
      break;
    }
    if (code - table->firstCode[codeBits] < table->numCodes[codeBits]) {
      code = (Gushort)(code - table->firstCode[codeBits]);
      return table->sym[table->firstSym[codeBits] + code];
    }
  } while (codeBits < 16);

  error(errSyntaxError, getPos(), "Bad Huffman code in DCT stream");
  return 9999;
}

int DCTStream::readAmp(int size) {
  int amp, bit;
  int bits;

  amp = 0;
  for (bits = 0; bits < size; ++bits) {
    if ((bit = readBit()) == EOF)
      return 9999;
    amp = (amp << 1) + bit;
  }
  if (amp < (1 << (size - 1)))
    amp -= (1 << size) - 1;
  return amp;
}

int DCTStream::readBit() {
  int bit;
  int c, c2;

  if (inputBits == 0) {
    if ((c = str->getChar()) == EOF)
      return EOF;
    if (c == 0xff) {
      do {
	c2 = str->getChar();
      } while (c2 == 0xff);
      if (c2 != 0x00) {
	error(errSyntaxError, getPos(), "Bad DCT data: missing 00 after ff");
	return EOF;
      }
    }
    inputBuf = c;
    inputBits = 8;
  }
  bit = (inputBuf >> (inputBits - 1)) & 1;
  --inputBits;
  return bit;
}

GBool DCTStream::readHeader(GBool frame) {
  GBool doScan;
  int n, i;
  int c = 0;

  // read headers
  doScan = gFalse;
  while (!doScan) {
    c = readMarker();
    switch (c) {
    case 0xc0:			// SOF0 (sequential)
    case 0xc1:			// SOF1 (extended sequential)
      if (!frame) {
	error(errSyntaxError, getPos(),
	      "Invalid DCT marker in scan <{0:02x}>", c);
	return gFalse;
      }
      if (!readBaselineSOF()) {
	return gFalse;
      }
      break;
    case 0xc2:			// SOF2 (progressive)
      if (!frame) {
	error(errSyntaxError, getPos(),
	      "Invalid DCT marker in scan <{0:02x}>", c);
	return gFalse;
      }
      if (!readProgressiveSOF()) {
	return gFalse;
      }
      break;
    case 0xc4:			// DHT
      if (!readHuffmanTables()) {
	return gFalse;
      }
      break;
    case 0xd8:			// SOI
      if (!frame) {
	error(errSyntaxError, getPos(),
	      "Invalid DCT marker in scan <{0:02x}>", c);
	return gFalse;
      }
      break;
    case 0xd9:			// EOI
      return gFalse;
    case 0xda:			// SOS
      if (!readScanInfo()) {
	return gFalse;
      }
      doScan = gTrue;
      break;
    case 0xdb:			// DQT
      if (!readQuantTables()) {
	return gFalse;
      }
      break;
    case 0xdd:			// DRI
      if (!readRestartInterval()) {
	return gFalse;
      }
      break;
    case 0xe0:			// APP0
      if (!frame) {
	error(errSyntaxError, getPos(),
	      "Invalid DCT marker in scan <{0:02x}>", c);
	return gFalse;
      }
      if (!readJFIFMarker()) {
	return gFalse;
      }
      break;
    case 0xee:			// APP14
      if (!frame) {
	error(errSyntaxError, getPos(),
	      "Invalid DCT marker in scan <{0:02x}>", c);
	return gFalse;
      }
      if (!readAdobeMarker()) {
	return gFalse;
      }
      break;
    case EOF:
      error(errSyntaxError, getPos(), "Bad DCT header");
      return gFalse;
    default:
      // skip APPn / COM / etc.
      if (c >= 0xe0) {
	n = read16() - 2;
	str->discardChars(n);
      } else {
	error(errSyntaxError, getPos(), "Unknown DCT marker <{0:02x}>", c);
	return gFalse;
      }
      break;
    }
  }

  for (i = 0; i < numComps; ++i) {
    if (compInfo[i].quantTable >= numQuantTables) {
      error(errSyntaxError, getPos(), "Invalid DCT quant table selector");
      return gFalse;
    }
  }

  return gTrue;
}

GBool DCTStream::readBaselineSOF() {
  int prec;
  int i;
  int c;

  read16(); // length
  prec = str->getChar();
  height = read16();
  width = read16();
  numComps = str->getChar();
  if (numComps <= 0 || numComps > 4) {
    error(errSyntaxError, getPos(), "Bad number of components in DCT stream");
    numComps = 0;
    return gFalse;
  }
  if (prec != 8) {
    error(errSyntaxError, getPos(), "Bad DCT precision {0:d}", prec);
    return gFalse;
  }
  for (i = 0; i < numComps; ++i) {
    compInfo[i].id = str->getChar();
    c = str->getChar();
    compInfo[i].hSample = (c >> 4) & 0x0f;
    compInfo[i].vSample = c & 0x0f;
    compInfo[i].quantTable = str->getChar();
    // a sampling factor of 3 is allowed by the spec, but requires
    // messy upsampling, and appears not to be used in practice
    if (!(compInfo[i].hSample == 1 ||
	  compInfo[i].hSample == 2 ||
	  compInfo[i].hSample == 4) ||
	!(compInfo[i].vSample == 1 ||
	  compInfo[i].vSample == 2 ||
	  compInfo[i].vSample == 4)) {
      error(errSyntaxError, getPos(), "Bad DCT sampling factor");
      return gFalse;
    }
    if (compInfo[i].quantTable < 0 || compInfo[i].quantTable > 3) {
      error(errSyntaxError, getPos(), "Bad DCT quant table selector");
      return gFalse;
    }
  }
  progressive = gFalse;
  return gTrue;
}

GBool DCTStream::readProgressiveSOF() {
  int prec;
  int i;
  int c;

  read16(); // length
  prec = str->getChar();
  height = read16();
  width = read16();
  numComps = str->getChar();
  if (numComps <= 0 || numComps > 4) {
    error(errSyntaxError, getPos(), "Bad number of components in DCT stream");
    numComps = 0;
    return gFalse;
  }
  if (prec != 8) {
    error(errSyntaxError, getPos(), "Bad DCT precision {0:d}", prec);
    return gFalse;
  }
  for (i = 0; i < numComps; ++i) {
    compInfo[i].id = str->getChar();
    c = str->getChar();
    compInfo[i].hSample = (c >> 4) & 0x0f;
    compInfo[i].vSample = c & 0x0f;
    compInfo[i].quantTable = str->getChar();
    // a sampling factor of 3 is allowed by the spec, but requires
    // messy upsampling, and appears not to be used in practice
    if (!(compInfo[i].hSample == 1 ||
	  compInfo[i].hSample == 2 ||
	  compInfo[i].hSample == 4) ||
	!(compInfo[i].vSample == 1 ||
	  compInfo[i].vSample == 2 ||
	  compInfo[i].vSample == 4)) {
      error(errSyntaxError, getPos(), "Bad DCT sampling factor");
      return gFalse;
    }
    if (compInfo[i].quantTable < 0 || compInfo[i].quantTable > 3) {
      error(errSyntaxError, getPos(), "Bad DCT quant table selector");
      return gFalse;
    }
  }
  progressive = gTrue;
  return gTrue;
}

GBool DCTStream::readScanInfo() {
  int length;
  int id, c;
  int i, j;

  length = read16() - 2;
  scanInfo.numComps = str->getChar();
  if (scanInfo.numComps <= 0 || scanInfo.numComps > 4) {
    error(errSyntaxError, getPos(), "Bad number of components in DCT stream");
    scanInfo.numComps = 0;
    return gFalse;
  }
  --length;
  if (length != 2 * scanInfo.numComps + 3) {
    error(errSyntaxError, getPos(), "Bad DCT scan info block");
    return gFalse;
  }
  interleaved = scanInfo.numComps == numComps;
  for (j = 0; j < numComps; ++j) {
    scanInfo.comp[j] = gFalse;
  }
  for (i = 0; i < scanInfo.numComps; ++i) {
    id = str->getChar();
    // some (broken) DCT streams reuse ID numbers, but at least they
    // keep the components in order, so we check compInfo[i] first to
    // work around the problem
    if (id == compInfo[i].id) {
      j = i;
    } else {
      for (j = 0; j < numComps; ++j) {
	if (id == compInfo[j].id) {
	  break;
	}
      }
      if (j == numComps) {
	error(errSyntaxError, getPos(),
	      "Bad DCT component ID in scan info block");
	return gFalse;
      }
    }
    if (scanInfo.comp[j]) {
      error(errSyntaxError, getPos(),
	    "Invalid DCT component ID in scan info block");
      return gFalse;
    }
    scanInfo.comp[j] = gTrue;
    c = str->getChar();
    scanInfo.dcHuffTable[j] = (c >> 4) & 0x0f;
    scanInfo.acHuffTable[j] = c & 0x0f;
  }
  scanInfo.firstCoeff = str->getChar();
  scanInfo.lastCoeff = str->getChar();
  if (scanInfo.firstCoeff < 0 || scanInfo.lastCoeff > 63 ||
      scanInfo.firstCoeff > scanInfo.lastCoeff) {
    error(errSyntaxError, getPos(),
	  "Bad DCT coefficient numbers in scan info block");
    return gFalse;
  }
  c = str->getChar();
  scanInfo.ah = (c >> 4) & 0x0f;
  scanInfo.al = c & 0x0f;
  return gTrue;
}

GBool DCTStream::readQuantTables() {
  int length, prec, i, index;

  length = read16() - 2;
  while (length > 0) {
    index = str->getChar();
    prec = (index >> 4) & 0x0f;
    index &= 0x0f;
    if (prec > 1 || index >= 4) {
      error(errSyntaxError, getPos(), "Bad DCT quantization table");
      return gFalse;
    }
    if (index >= numQuantTables) {
      numQuantTables = index + 1;
    }
    for (i = 0; i < 64; ++i) {
      if (prec) {
	quantTables[index][dctZigZag[i]] = (Gushort)read16();
      } else {
	quantTables[index][dctZigZag[i]] = (Gushort)str->getChar();
      }
    }
    if (prec) {
      length -= 129;
    } else {
      length -= 65;
    }
  }
  return gTrue;
}

GBool DCTStream::readHuffmanTables() {
  DCTHuffTable *tbl;
  int length;
  int index;
  Gushort code;
  Guchar sym;
  int i;
  int c;

  length = read16() - 2;
  while (length > 0) {
    index = str->getChar();
    --length;
    if ((index & 0x0f) >= 4) {
      error(errSyntaxError, getPos(), "Bad DCT Huffman table");
      return gFalse;
    }
    if (index & 0x10) {
      index &= 0x0f;
      if (index >= numACHuffTables)
	numACHuffTables = index+1;
      tbl = &acHuffTables[index];
    } else {
      index &= 0x0f;
      if (index >= numDCHuffTables)
	numDCHuffTables = index+1;
      tbl = &dcHuffTables[index];
    }
    sym = 0;
    code = 0;
    for (i = 1; i <= 16; ++i) {
      c = str->getChar();
      tbl->firstSym[i] = sym;
      tbl->firstCode[i] = code;
      tbl->numCodes[i] = (Gushort)c;
      sym = (Guchar)(sym + c);
      code = (Gushort)((code + c) << 1);
    }
    length -= 16;
    for (i = 0; i < sym; ++i)
      tbl->sym[i] = (Guchar)str->getChar();
    length -= sym;
  }
  return gTrue;
}

GBool DCTStream::readRestartInterval() {
  int length;

  length = read16();
  if (length != 4) {
    error(errSyntaxError, getPos(), "Bad DCT restart interval");
    return gFalse;
  }
  restartInterval = read16();
  return gTrue;
}

GBool DCTStream::readJFIFMarker() {
  int length, i;
  char buf[5];
  int c;

  length = read16();
  length -= 2;
  if (length >= 5) {
    for (i = 0; i < 5; ++i) {
      if ((c = str->getChar()) == EOF) {
	error(errSyntaxError, getPos(), "Bad DCT APP0 marker");
	return gFalse;
      }
      buf[i] = (char)c;
    }
    length -= 5;
    if (!memcmp(buf, "JFIF\0", 5)) {
      gotJFIFMarker = gTrue;
    }
  }
  while (length > 0) {
    if (str->getChar() == EOF) {
      error(errSyntaxError, getPos(), "Bad DCT APP0 marker");
      return gFalse;
    }
    --length;
  }
  return gTrue;
}

GBool DCTStream::readAdobeMarker() {
  int length, i;
  char buf[12];
  int c;

  length = read16();
  if (length < 14) {
    goto err;
  }
  for (i = 0; i < 12; ++i) {
    if ((c = str->getChar()) == EOF) {
      goto err;
    }
    buf[i] = (char)c;
  }
  if (!strncmp(buf, "Adobe", 5)) {
    colorXform = buf[11];
    gotAdobeMarker = gTrue;
  }
  for (i = 14; i < length; ++i) {
    if (str->getChar() == EOF) {
      goto err;
    }
  }
  return gTrue;

 err:
  error(errSyntaxError, getPos(), "Bad DCT Adobe APP14 marker");
  return gFalse;
}

GBool DCTStream::readTrailer() {
  int c;

  c = readMarker();
  if (c != 0xd9) {		// EOI
    error(errSyntaxError, getPos(), "Bad DCT trailer");
    return gFalse;
  }
  return gTrue;
}

int DCTStream::readMarker() {
  int c;

  do {
    do {
      c = str->getChar();
    } while (c != 0xff && c != EOF);
    do {
      c = str->getChar();
    } while (c == 0xff);
  } while (c == 0x00);
  return c;
}

int DCTStream::read16() {
  int c1, c2;

  if ((c1 = str->getChar()) == EOF)
    return EOF;
  if ((c2 = str->getChar()) == EOF)
    return EOF;
  return (c1 << 8) + c2;
}

#endif // HAVE_JPEGLIB

GString *DCTStream::getPSFilter(int psLevel, const char *indent,
				GBool okToReadStream) {
  GString *s;

  if (psLevel < 2) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent, okToReadStream))) {
    return NULL;
  }
  if (okToReadStream && !checkSequentialInterleaved()) {
    // PostScript does not allow progressive or interleaved JPEG
    delete s;
    return NULL;
  }
  s->append(indent)->append("<< >> /DCTDecode filter\n");
  return s;
}

GBool DCTStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

//------------------------------------------------------------------------
// FlateStream
//------------------------------------------------------------------------

int FlateStream::codeLenCodeMap[flateMaxCodeLenCodes] = {
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

FlateDecode FlateStream::lengthDecode[flateMaxLitCodes-257] = {
  {0,   3},
  {0,   4},
  {0,   5},
  {0,   6},
  {0,   7},
  {0,   8},
  {0,   9},
  {0,  10},
  {1,  11},
  {1,  13},
  {1,  15},
  {1,  17},
  {2,  19},
  {2,  23},
  {2,  27},
  {2,  31},
  {3,  35},
  {3,  43},
  {3,  51},
  {3,  59},
  {4,  67},
  {4,  83},
  {4,  99},
  {4, 115},
  {5, 131},
  {5, 163},
  {5, 195},
  {5, 227},
  {0, 258},
  {0, 258},
  {0, 258}
};

FlateDecode FlateStream::distDecode[flateMaxDistCodes] = {
  { 0,     1},
  { 0,     2},
  { 0,     3},
  { 0,     4},
  { 1,     5},
  { 1,     7},
  { 2,     9},
  { 2,    13},
  { 3,    17},
  { 3,    25},
  { 4,    33},
  { 4,    49},
  { 5,    65},
  { 5,    97},
  { 6,   129},
  { 6,   193},
  { 7,   257},
  { 7,   385},
  { 8,   513},
  { 8,   769},
  { 9,  1025},
  { 9,  1537},
  {10,  2049},
  {10,  3073},
  {11,  4097},
  {11,  6145},
  {12,  8193},
  {12, 12289},
  {13, 16385},
  {13, 24577}
};

static FlateCode flateFixedLitCodeTabCodes[512] = {
  {7, 0x0100},
  {8, 0x0050},
  {8, 0x0010},
  {8, 0x0118},
  {7, 0x0110},
  {8, 0x0070},
  {8, 0x0030},
  {9, 0x00c0},
  {7, 0x0108},
  {8, 0x0060},
  {8, 0x0020},
  {9, 0x00a0},
  {8, 0x0000},
  {8, 0x0080},
  {8, 0x0040},
  {9, 0x00e0},
  {7, 0x0104},
  {8, 0x0058},
  {8, 0x0018},
  {9, 0x0090},
  {7, 0x0114},
  {8, 0x0078},
  {8, 0x0038},
  {9, 0x00d0},
  {7, 0x010c},
  {8, 0x0068},
  {8, 0x0028},
  {9, 0x00b0},
  {8, 0x0008},
  {8, 0x0088},
  {8, 0x0048},
  {9, 0x00f0},
  {7, 0x0102},
  {8, 0x0054},
  {8, 0x0014},
  {8, 0x011c},
  {7, 0x0112},
  {8, 0x0074},
  {8, 0x0034},
  {9, 0x00c8},
  {7, 0x010a},
  {8, 0x0064},
  {8, 0x0024},
  {9, 0x00a8},
  {8, 0x0004},
  {8, 0x0084},
  {8, 0x0044},
  {9, 0x00e8},
  {7, 0x0106},
  {8, 0x005c},
  {8, 0x001c},
  {9, 0x0098},
  {7, 0x0116},
  {8, 0x007c},
  {8, 0x003c},
  {9, 0x00d8},
  {7, 0x010e},
  {8, 0x006c},
  {8, 0x002c},
  {9, 0x00b8},
  {8, 0x000c},
  {8, 0x008c},
  {8, 0x004c},
  {9, 0x00f8},
  {7, 0x0101},
  {8, 0x0052},
  {8, 0x0012},
  {8, 0x011a},
  {7, 0x0111},
  {8, 0x0072},
  {8, 0x0032},
  {9, 0x00c4},
  {7, 0x0109},
  {8, 0x0062},
  {8, 0x0022},
  {9, 0x00a4},
  {8, 0x0002},
  {8, 0x0082},
  {8, 0x0042},
  {9, 0x00e4},
  {7, 0x0105},
  {8, 0x005a},
  {8, 0x001a},
  {9, 0x0094},
  {7, 0x0115},
  {8, 0x007a},
  {8, 0x003a},
  {9, 0x00d4},
  {7, 0x010d},
  {8, 0x006a},
  {8, 0x002a},
  {9, 0x00b4},
  {8, 0x000a},
  {8, 0x008a},
  {8, 0x004a},
  {9, 0x00f4},
  {7, 0x0103},
  {8, 0x0056},
  {8, 0x0016},
  {8, 0x011e},
  {7, 0x0113},
  {8, 0x0076},
  {8, 0x0036},
  {9, 0x00cc},
  {7, 0x010b},
  {8, 0x0066},
  {8, 0x0026},
  {9, 0x00ac},
  {8, 0x0006},
  {8, 0x0086},
  {8, 0x0046},
  {9, 0x00ec},
  {7, 0x0107},
  {8, 0x005e},
  {8, 0x001e},
  {9, 0x009c},
  {7, 0x0117},
  {8, 0x007e},
  {8, 0x003e},
  {9, 0x00dc},
  {7, 0x010f},
  {8, 0x006e},
  {8, 0x002e},
  {9, 0x00bc},
  {8, 0x000e},
  {8, 0x008e},
  {8, 0x004e},
  {9, 0x00fc},
  {7, 0x0100},
  {8, 0x0051},
  {8, 0x0011},
  {8, 0x0119},
  {7, 0x0110},
  {8, 0x0071},
  {8, 0x0031},
  {9, 0x00c2},
  {7, 0x0108},
  {8, 0x0061},
  {8, 0x0021},
  {9, 0x00a2},
  {8, 0x0001},
  {8, 0x0081},
  {8, 0x0041},
  {9, 0x00e2},
  {7, 0x0104},
  {8, 0x0059},
  {8, 0x0019},
  {9, 0x0092},
  {7, 0x0114},
  {8, 0x0079},
  {8, 0x0039},
  {9, 0x00d2},
  {7, 0x010c},
  {8, 0x0069},
  {8, 0x0029},
  {9, 0x00b2},
  {8, 0x0009},
  {8, 0x0089},
  {8, 0x0049},
  {9, 0x00f2},
  {7, 0x0102},
  {8, 0x0055},
  {8, 0x0015},
  {8, 0x011d},
  {7, 0x0112},
  {8, 0x0075},
  {8, 0x0035},
  {9, 0x00ca},
  {7, 0x010a},
  {8, 0x0065},
  {8, 0x0025},
  {9, 0x00aa},
  {8, 0x0005},
  {8, 0x0085},
  {8, 0x0045},
  {9, 0x00ea},
  {7, 0x0106},
  {8, 0x005d},
  {8, 0x001d},
  {9, 0x009a},
  {7, 0x0116},
  {8, 0x007d},
  {8, 0x003d},
  {9, 0x00da},
  {7, 0x010e},
  {8, 0x006d},
  {8, 0x002d},
  {9, 0x00ba},
  {8, 0x000d},
  {8, 0x008d},
  {8, 0x004d},
  {9, 0x00fa},
  {7, 0x0101},
  {8, 0x0053},
  {8, 0x0013},
  {8, 0x011b},
  {7, 0x0111},
  {8, 0x0073},
  {8, 0x0033},
  {9, 0x00c6},
  {7, 0x0109},
  {8, 0x0063},
  {8, 0x0023},
  {9, 0x00a6},
  {8, 0x0003},
  {8, 0x0083},
  {8, 0x0043},
  {9, 0x00e6},
  {7, 0x0105},
  {8, 0x005b},
  {8, 0x001b},
  {9, 0x0096},
  {7, 0x0115},
  {8, 0x007b},
  {8, 0x003b},
  {9, 0x00d6},
  {7, 0x010d},
  {8, 0x006b},
  {8, 0x002b},
  {9, 0x00b6},
  {8, 0x000b},
  {8, 0x008b},
  {8, 0x004b},
  {9, 0x00f6},
  {7, 0x0103},
  {8, 0x0057},
  {8, 0x0017},
  {8, 0x011f},
  {7, 0x0113},
  {8, 0x0077},
  {8, 0x0037},
  {9, 0x00ce},
  {7, 0x010b},
  {8, 0x0067},
  {8, 0x0027},
  {9, 0x00ae},
  {8, 0x0007},
  {8, 0x0087},
  {8, 0x0047},
  {9, 0x00ee},
  {7, 0x0107},
  {8, 0x005f},
  {8, 0x001f},
  {9, 0x009e},
  {7, 0x0117},
  {8, 0x007f},
  {8, 0x003f},
  {9, 0x00de},
  {7, 0x010f},
  {8, 0x006f},
  {8, 0x002f},
  {9, 0x00be},
  {8, 0x000f},
  {8, 0x008f},
  {8, 0x004f},
  {9, 0x00fe},
  {7, 0x0100},
  {8, 0x0050},
  {8, 0x0010},
  {8, 0x0118},
  {7, 0x0110},
  {8, 0x0070},
  {8, 0x0030},
  {9, 0x00c1},
  {7, 0x0108},
  {8, 0x0060},
  {8, 0x0020},
  {9, 0x00a1},
  {8, 0x0000},
  {8, 0x0080},
  {8, 0x0040},
  {9, 0x00e1},
  {7, 0x0104},
  {8, 0x0058},
  {8, 0x0018},
  {9, 0x0091},
  {7, 0x0114},
  {8, 0x0078},
  {8, 0x0038},
  {9, 0x00d1},
  {7, 0x010c},
  {8, 0x0068},
  {8, 0x0028},
  {9, 0x00b1},
  {8, 0x0008},
  {8, 0x0088},
  {8, 0x0048},
  {9, 0x00f1},
  {7, 0x0102},
  {8, 0x0054},
  {8, 0x0014},
  {8, 0x011c},
  {7, 0x0112},
  {8, 0x0074},
  {8, 0x0034},
  {9, 0x00c9},
  {7, 0x010a},
  {8, 0x0064},
  {8, 0x0024},
  {9, 0x00a9},
  {8, 0x0004},
  {8, 0x0084},
  {8, 0x0044},
  {9, 0x00e9},
  {7, 0x0106},
  {8, 0x005c},
  {8, 0x001c},
  {9, 0x0099},
  {7, 0x0116},
  {8, 0x007c},
  {8, 0x003c},
  {9, 0x00d9},
  {7, 0x010e},
  {8, 0x006c},
  {8, 0x002c},
  {9, 0x00b9},
  {8, 0x000c},
  {8, 0x008c},
  {8, 0x004c},
  {9, 0x00f9},
  {7, 0x0101},
  {8, 0x0052},
  {8, 0x0012},
  {8, 0x011a},
  {7, 0x0111},
  {8, 0x0072},
  {8, 0x0032},
  {9, 0x00c5},
  {7, 0x0109},
  {8, 0x0062},
  {8, 0x0022},
  {9, 0x00a5},
  {8, 0x0002},
  {8, 0x0082},
  {8, 0x0042},
  {9, 0x00e5},
  {7, 0x0105},
  {8, 0x005a},
  {8, 0x001a},
  {9, 0x0095},
  {7, 0x0115},
  {8, 0x007a},
  {8, 0x003a},
  {9, 0x00d5},
  {7, 0x010d},
  {8, 0x006a},
  {8, 0x002a},
  {9, 0x00b5},
  {8, 0x000a},
  {8, 0x008a},
  {8, 0x004a},
  {9, 0x00f5},
  {7, 0x0103},
  {8, 0x0056},
  {8, 0x0016},
  {8, 0x011e},
  {7, 0x0113},
  {8, 0x0076},
  {8, 0x0036},
  {9, 0x00cd},
  {7, 0x010b},
  {8, 0x0066},
  {8, 0x0026},
  {9, 0x00ad},
  {8, 0x0006},
  {8, 0x0086},
  {8, 0x0046},
  {9, 0x00ed},
  {7, 0x0107},
  {8, 0x005e},
  {8, 0x001e},
  {9, 0x009d},
  {7, 0x0117},
  {8, 0x007e},
  {8, 0x003e},
  {9, 0x00dd},
  {7, 0x010f},
  {8, 0x006e},
  {8, 0x002e},
  {9, 0x00bd},
  {8, 0x000e},
  {8, 0x008e},
  {8, 0x004e},
  {9, 0x00fd},
  {7, 0x0100},
  {8, 0x0051},
  {8, 0x0011},
  {8, 0x0119},
  {7, 0x0110},
  {8, 0x0071},
  {8, 0x0031},
  {9, 0x00c3},
  {7, 0x0108},
  {8, 0x0061},
  {8, 0x0021},
  {9, 0x00a3},
  {8, 0x0001},
  {8, 0x0081},
  {8, 0x0041},
  {9, 0x00e3},
  {7, 0x0104},
  {8, 0x0059},
  {8, 0x0019},
  {9, 0x0093},
  {7, 0x0114},
  {8, 0x0079},
  {8, 0x0039},
  {9, 0x00d3},
  {7, 0x010c},
  {8, 0x0069},
  {8, 0x0029},
  {9, 0x00b3},
  {8, 0x0009},
  {8, 0x0089},
  {8, 0x0049},
  {9, 0x00f3},
  {7, 0x0102},
  {8, 0x0055},
  {8, 0x0015},
  {8, 0x011d},
  {7, 0x0112},
  {8, 0x0075},
  {8, 0x0035},
  {9, 0x00cb},
  {7, 0x010a},
  {8, 0x0065},
  {8, 0x0025},
  {9, 0x00ab},
  {8, 0x0005},
  {8, 0x0085},
  {8, 0x0045},
  {9, 0x00eb},
  {7, 0x0106},
  {8, 0x005d},
  {8, 0x001d},
  {9, 0x009b},
  {7, 0x0116},
  {8, 0x007d},
  {8, 0x003d},
  {9, 0x00db},
  {7, 0x010e},
  {8, 0x006d},
  {8, 0x002d},
  {9, 0x00bb},
  {8, 0x000d},
  {8, 0x008d},
  {8, 0x004d},
  {9, 0x00fb},
  {7, 0x0101},
  {8, 0x0053},
  {8, 0x0013},
  {8, 0x011b},
  {7, 0x0111},
  {8, 0x0073},
  {8, 0x0033},
  {9, 0x00c7},
  {7, 0x0109},
  {8, 0x0063},
  {8, 0x0023},
  {9, 0x00a7},
  {8, 0x0003},
  {8, 0x0083},
  {8, 0x0043},
  {9, 0x00e7},
  {7, 0x0105},
  {8, 0x005b},
  {8, 0x001b},
  {9, 0x0097},
  {7, 0x0115},
  {8, 0x007b},
  {8, 0x003b},
  {9, 0x00d7},
  {7, 0x010d},
  {8, 0x006b},
  {8, 0x002b},
  {9, 0x00b7},
  {8, 0x000b},
  {8, 0x008b},
  {8, 0x004b},
  {9, 0x00f7},
  {7, 0x0103},
  {8, 0x0057},
  {8, 0x0017},
  {8, 0x011f},
  {7, 0x0113},
  {8, 0x0077},
  {8, 0x0037},
  {9, 0x00cf},
  {7, 0x010b},
  {8, 0x0067},
  {8, 0x0027},
  {9, 0x00af},
  {8, 0x0007},
  {8, 0x0087},
  {8, 0x0047},
  {9, 0x00ef},
  {7, 0x0107},
  {8, 0x005f},
  {8, 0x001f},
  {9, 0x009f},
  {7, 0x0117},
  {8, 0x007f},
  {8, 0x003f},
  {9, 0x00df},
  {7, 0x010f},
  {8, 0x006f},
  {8, 0x002f},
  {9, 0x00bf},
  {8, 0x000f},
  {8, 0x008f},
  {8, 0x004f},
  {9, 0x00ff}
};

FlateHuffmanTab FlateStream::fixedLitCodeTab = {
  flateFixedLitCodeTabCodes, 9
};

static FlateCode flateFixedDistCodeTabCodes[32] = {
  {5, 0x0000},
  {5, 0x0010},
  {5, 0x0008},
  {5, 0x0018},
  {5, 0x0004},
  {5, 0x0014},
  {5, 0x000c},
  {5, 0x001c},
  {5, 0x0002},
  {5, 0x0012},
  {5, 0x000a},
  {5, 0x001a},
  {5, 0x0006},
  {5, 0x0016},
  {5, 0x000e},
  {0, 0x0000},
  {5, 0x0001},
  {5, 0x0011},
  {5, 0x0009},
  {5, 0x0019},
  {5, 0x0005},
  {5, 0x0015},
  {5, 0x000d},
  {5, 0x001d},
  {5, 0x0003},
  {5, 0x0013},
  {5, 0x000b},
  {5, 0x001b},
  {5, 0x0007},
  {5, 0x0017},
  {5, 0x000f},
  {0, 0x0000}
};

FlateHuffmanTab FlateStream::fixedDistCodeTab = {
  flateFixedDistCodeTabCodes, 5
};

FlateStream::FlateStream(Stream *strA, int predictor, int columns,
			 int colors, int bits):
    FilterStream(strA) {
  if (predictor != 1) {
    pred = new StreamPredictor(this, predictor, columns, colors, bits);
    if (!pred->isOk()) {
      delete pred;
      pred = NULL;
    }
  } else {
    pred = NULL;
  }
  litCodeTab.codes = NULL;
  distCodeTab.codes = NULL;
  memset(buf, 0, flateWindow);
}

FlateStream::~FlateStream() {
  if (litCodeTab.codes != fixedLitCodeTab.codes) {
    gfree(litCodeTab.codes);
  }
  if (distCodeTab.codes != fixedDistCodeTab.codes) {
    gfree(distCodeTab.codes);
  }
  if (pred) {
    delete pred;
  }
  delete str;
}

Stream *FlateStream::copy() {
  if (pred) {
    return new FlateStream(str->copy(), pred->getPredictor(),
			   pred->getWidth(), pred->getNComps(),
			   pred->getNBits());
  } else {
    return new FlateStream(str->copy(), 1, 0, 0, 0);
  }
}

void FlateStream::reset() {
  int cmf, flg;

  index = 0;
  remain = 0;
  codeBuf = 0;
  codeSize = 0;
  compressedBlock = gFalse;
  endOfBlock = gTrue;
  eof = gTrue;

  str->reset();
  if (pred) {
    pred->reset();
  }

  // read header
  //~ need to look at window size?
  endOfBlock = eof = gTrue;
  cmf = str->getChar();
  flg = str->getChar();
  totalIn = 2;
  totalOut = 0;
  if (cmf == EOF || flg == EOF)
    return;
  if ((cmf & 0x0f) != 0x08) {
    error(errSyntaxError, getPos(),
	  "Unknown compression method in flate stream");
    return;
  }
  if ((((cmf << 8) + flg) % 31) != 0) {
    error(errSyntaxError, getPos(), "Bad FCHECK in flate stream");
    return;
  }
  if (flg & 0x20) {
    error(errSyntaxError, getPos(), "FDICT bit set in flate stream");
    return;
  }

  eof = gFalse;
}

int FlateStream::getChar() {
  int c;

  if (pred) {
    return pred->getChar();
  }
  while (remain == 0) {
    if (endOfBlock && eof)
      return EOF;
    readSome();
  }
  c = buf[index];
  index = (index + 1) & flateMask;
  --remain;
  return c;
}

int FlateStream::lookChar() {
  int c;

  if (pred) {
    return pred->lookChar();
  }
  while (remain == 0) {
    if (endOfBlock && eof)
      return EOF;
    readSome();
  }
  c = buf[index];
  return c;
}

int FlateStream::getRawChar() {
  int c;

  while (remain == 0) {
    if (endOfBlock && eof)
      return EOF;
    readSome();
  }
  c = buf[index];
  index = (index + 1) & flateMask;
  --remain;
  return c;
}

int FlateStream::getBlock(char *blk, int size) {
  int n, k;

  if (pred) {
    return pred->getBlock(blk, size);
  }

  n = 0;
  while (n < size) {
    if (remain == 0) {
      if (endOfBlock && eof) {
	break;
      }
      readSome();
    }
    k = remain;
    if (size - n < k) {
      k = size - n;
    }
    if (flateWindow - index < k) {
      k = flateWindow - index;
    }
    memcpy(blk + n, buf + index, k);
    n += k;
    index = (index + k) & flateMask;
    remain -= k;
  }
  return n;
}

GString *FlateStream::getPSFilter(int psLevel, const char *indent,
				  GBool okToReadStream) {
  GString *s;

  if (psLevel < 3 || pred) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent, okToReadStream))) {
    return NULL;
  }
  s->append(indent)->append("<< >> /FlateDecode filter\n");
  return s;
}

GBool FlateStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

void FlateStream::readSome() {
  int code1, code2;
  int len, dist;
  int src, dest, n1, n2, n3, i, j, k;
  int c;

  if (endOfBlock) {
    if (!startBlock())
      return;
  }

  if (compressedBlock) {
    if ((code1 = getHuffmanCodeWord(&litCodeTab)) == EOF)
      goto err;
    if (code1 < 256) {
      buf[index] = (Guchar)code1;
      remain = 1;
    } else if (code1 == 256) {
      endOfBlock = gTrue;
      remain = 0;
    } else {
      code1 -= 257;
      code2 = lengthDecode[code1].bits;
      if (code2 > 0 && (code2 = getCodeWord(code2)) == EOF)
	goto err;
      len = lengthDecode[code1].first + code2;
      if ((code1 = getHuffmanCodeWord(&distCodeTab)) == EOF)
	goto err;
      code2 = distDecode[code1].bits;
      if (code2 > 0 && (code2 = getCodeWord(code2)) == EOF)
	goto err;
      dist = distDecode[code1].first + code2;
      dest = index;
      src = (index - dist) & flateMask;
      // the following is an optimized version of:
      // for (k = 0; k < len; ++k) {
      //   buf[dest] = buf[src];
      //   dest = (dest + 1) & flateMask;
      //   src = (src + 1) & flateMask;
      // }
      if (dest + len <= flateWindow) {
	if (src + len <= flateWindow) {
	  for (k = 0; k < len; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	} else {
	  n1 = flateWindow - src;
	  n2 = len - n1;
	  for (k = 0; k < n1; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	  dest = dest + n1;
	  src = 0;
	  for (k = 0; k < n2; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	}
      } else {
	if (src + len <= flateWindow) {
	  n1 = flateWindow - dest;
	  n2 = len - n1;
	  for (k = 0; k < n1; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	  dest = 0;
	  src = src + n1;
	  for (k = 0; k < n2; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	} else if (src < dest) {
	  n1 = flateWindow - dest;
	  n2 = dest - src;
	  n3 = len - n1 - n2;
	  for (k = 0; k < n1; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	  dest = 0;
	  src = src + n1;
	  for (k = 0; k < n2; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	  dest = n2;
	  src = 0;
	  for (k = 0; k < n3; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	} else {
	  n1 = flateWindow - src;
	  n2 = src - dest;
	  n3 = len - n1 - n2;
	  for (k = 0; k < n1; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	  dest = dest + n1;
	  src = 0;
	  for (k = 0; k < n2; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	  dest = 0;
	  src = n2;
	  for (k = 0; k < n3; ++k) {
	    buf[dest + k] = buf[src + k];
	  }
	}
      }
      remain = len;
    }

  } else {
    len = (blockLen < flateWindow) ? blockLen : flateWindow;
    for (i = 0, j = index; i < len; ++i, j = (j + 1) & flateMask) {
      if ((c = str->getChar()) == EOF) {
	endOfBlock = eof = gTrue;
	break;
      }
      buf[j] = (Guchar)c;
    }
    remain = i;
    blockLen -= len;
    if (blockLen == 0)
      endOfBlock = gTrue;
    totalIn += remain;
  }
  totalOut += remain;

  // check for a 'decompression bomb'
  if (totalOut > 500000000 && totalIn < totalOut / 250) {
    error(errSyntaxError, getPos(), "Decompression bomb in flate stream");
    endOfBlock = eof = gTrue;
    remain = 0;
  }

  return;

err:
  error(errSyntaxError, getPos(), "Unexpected end of file in flate stream");
  endOfBlock = eof = gTrue;
  remain = 0;
}

GBool FlateStream::startBlock() {
  int blockHdr;
  int c;
  int check;

  // free the code tables from the previous block
  if (litCodeTab.codes != fixedLitCodeTab.codes) {
    gfree(litCodeTab.codes);
  }
  litCodeTab.codes = NULL;
  if (distCodeTab.codes != fixedDistCodeTab.codes) {
    gfree(distCodeTab.codes);
  }
  distCodeTab.codes = NULL;

  // read block header
  blockHdr = getCodeWord(3);
  if (blockHdr & 1)
    eof = gTrue;
  blockHdr >>= 1;

  // uncompressed block
  if (blockHdr == 0) {
    compressedBlock = gFalse;
    if ((c = str->getChar()) == EOF)
      goto err;
    blockLen = c & 0xff;
    if ((c = str->getChar()) == EOF)
      goto err;
    blockLen |= (c & 0xff) << 8;
    if ((c = str->getChar()) == EOF)
      goto err;
    check = c & 0xff;
    if ((c = str->getChar()) == EOF)
      goto err;
    check |= (c & 0xff) << 8;
    if (check != (~blockLen & 0xffff))
      error(errSyntaxError, getPos(),
	    "Bad uncompressed block length in flate stream");
    codeBuf = 0;
    codeSize = 0;
    totalIn += 4;

  // compressed block with fixed codes
  } else if (blockHdr == 1) {
    compressedBlock = gTrue;
    loadFixedCodes();

  // compressed block with dynamic codes
  } else if (blockHdr == 2) {
    compressedBlock = gTrue;
    if (!readDynamicCodes()) {
      goto err;
    }

  // unknown block type
  } else {
    goto err;
  }

  endOfBlock = gFalse;
  return gTrue;

err:
  error(errSyntaxError, getPos(), "Bad block header in flate stream");
  endOfBlock = eof = gTrue;
  return gFalse;
}

void FlateStream::loadFixedCodes() {
  litCodeTab.codes = fixedLitCodeTab.codes;
  litCodeTab.maxLen = fixedLitCodeTab.maxLen;
  distCodeTab.codes = fixedDistCodeTab.codes;
  distCodeTab.maxLen = fixedDistCodeTab.maxLen;
}

GBool FlateStream::readDynamicCodes() {
  int numCodeLenCodes;
  int numLitCodes;
  int numDistCodes;
  int codeLenCodeLengths[flateMaxCodeLenCodes];
  FlateHuffmanTab codeLenCodeTab;
  int len, repeat, code;
  int i;

  codeLenCodeTab.codes = NULL;

  // read lengths
  if ((numLitCodes = getCodeWord(5)) == EOF) {
    goto err;
  }
  numLitCodes += 257;
  if ((numDistCodes = getCodeWord(5)) == EOF) {
    goto err;
  }
  numDistCodes += 1;
  if ((numCodeLenCodes = getCodeWord(4)) == EOF) {
    goto err;
  }
  numCodeLenCodes += 4;
  if (numLitCodes > flateMaxLitCodes ||
      numDistCodes > flateMaxDistCodes ||
      numCodeLenCodes > flateMaxCodeLenCodes) {
    goto err;
  }

  // build the code length code table
  for (i = 0; i < flateMaxCodeLenCodes; ++i) {
    codeLenCodeLengths[i] = 0;
  }
  for (i = 0; i < numCodeLenCodes; ++i) {
    if ((codeLenCodeLengths[codeLenCodeMap[i]] = getCodeWord(3)) == -1) {
      goto err;
    }
  }
  compHuffmanCodes(codeLenCodeLengths, flateMaxCodeLenCodes, &codeLenCodeTab);

  // build the literal and distance code tables
  len = 0;
  repeat = 0;
  i = 0;
  while (i < numLitCodes + numDistCodes) {
    if ((code = getHuffmanCodeWord(&codeLenCodeTab)) == EOF) {
      goto err;
    }
    if (code == 16) {
      if ((repeat = getCodeWord(2)) == EOF) {
	goto err;
      }
      repeat += 3;
      if (i + repeat > numLitCodes + numDistCodes) {
	goto err;
      }
      for (; repeat > 0; --repeat) {
	codeLengths[i++] = len;
      }
    } else if (code == 17) {
      if ((repeat = getCodeWord(3)) == EOF) {
	goto err;
      }
      repeat += 3;
      if (i + repeat > numLitCodes + numDistCodes) {
	goto err;
      }
      len = 0;
      for (; repeat > 0; --repeat) {
	codeLengths[i++] = 0;
      }
    } else if (code == 18) {
      if ((repeat = getCodeWord(7)) == EOF) {
	goto err;
      }
      repeat += 11;
      if (i + repeat > numLitCodes + numDistCodes) {
	goto err;
      }
      len = 0;
      for (; repeat > 0; --repeat) {
	codeLengths[i++] = 0;
      }
    } else {
      codeLengths[i++] = len = code;
    }
  }
  compHuffmanCodes(codeLengths, numLitCodes, &litCodeTab);
  compHuffmanCodes(codeLengths + numLitCodes, numDistCodes, &distCodeTab);

  gfree(codeLenCodeTab.codes);
  return gTrue;

err:
  error(errSyntaxError, getPos(), "Bad dynamic code table in flate stream");
  gfree(codeLenCodeTab.codes);
  return gFalse;
}

// Convert an array <lengths> of <n> lengths, in value order, into a
// Huffman code lookup table.
void FlateStream::compHuffmanCodes(int *lengths, int n, FlateHuffmanTab *tab) {
  int tabSize, len, code, code2, skip, val, i, t;

  // find max code length
  tab->maxLen = 0;
  for (val = 0; val < n; ++val) {
    if (lengths[val] > tab->maxLen) {
      tab->maxLen = lengths[val];
    }
  }

  // allocate the table
  tabSize = 1 << tab->maxLen;
  tab->codes = (FlateCode *)gmallocn(tabSize, sizeof(FlateCode));

  // clear the table
  for (i = 0; i < tabSize; ++i) {
    tab->codes[i].len = 0;
    tab->codes[i].val = 0;
  }

  // build the table
  for (len = 1, code = 0, skip = 2;
       len <= tab->maxLen;
       ++len, code <<= 1, skip <<= 1) {
    for (val = 0; val < n; ++val) {
      if (lengths[val] == len) {

	// bit-reverse the code
	code2 = 0;
	t = code;
	for (i = 0; i < len; ++i) {
	  code2 = (code2 << 1) | (t & 1);
	  t >>= 1;
	}

	// fill in the table entries
	for (i = code2; i < tabSize; i += skip) {
	  tab->codes[i].len = (Gushort)len;
	  tab->codes[i].val = (Gushort)val;
	}

	++code;
      }
    }
  }
}

int FlateStream::getHuffmanCodeWord(FlateHuffmanTab *tab) {
  FlateCode *code;
  int c;

  while (codeSize < tab->maxLen) {
    if ((c = str->getChar()) == EOF) {
      break;
    }
    codeBuf |= (c & 0xff) << codeSize;
    codeSize += 8;
    ++totalIn;
  }
  code = &tab->codes[codeBuf & ((1 << tab->maxLen) - 1)];
  if (codeSize == 0 || codeSize < code->len || code->len == 0) {
    return EOF;
  }
  codeBuf >>= code->len;
  codeSize -= code->len;
  return (int)code->val;
}

int FlateStream::getCodeWord(int bits) {
  int c;

  while (codeSize < bits) {
    if ((c = str->getChar()) == EOF)
      return EOF;
    codeBuf |= (c & 0xff) << codeSize;
    codeSize += 8;
    ++totalIn;
  }
  c = codeBuf & ((1 << bits) - 1);
  codeBuf >>= bits;
  codeSize -= bits;
  return c;
}

//------------------------------------------------------------------------
// EOFStream
//------------------------------------------------------------------------

EOFStream::EOFStream(Stream *strA):
    FilterStream(strA) {
}

EOFStream::~EOFStream() {
  delete str;
}

Stream *EOFStream::copy() {
  return new EOFStream(str->copy());
}

//------------------------------------------------------------------------
// BufStream
//------------------------------------------------------------------------

BufStream::BufStream(Stream *strA, int bufSizeA): FilterStream(strA) {
  bufSize = bufSizeA;
  buf = (int *)gmallocn(bufSize, sizeof(int));
}

BufStream::~BufStream() {
  gfree(buf);
  delete str;
}

Stream *BufStream::copy() {
  return new BufStream(str->copy(), bufSize);
}

void BufStream::reset() {
  int i;

  str->reset();
  for (i = 0; i < bufSize; ++i) {
    buf[i] = str->getChar();
  }
}

int BufStream::getChar() {
  int c, i;

  c = buf[0];
  for (i = 1; i < bufSize; ++i) {
    buf[i-1] = buf[i];
  }
  buf[bufSize - 1] = str->getChar();
  return c;
}

int BufStream::lookChar() {
  return buf[0];
}

int BufStream::lookChar(int idx) {
  return buf[idx];
}

GBool BufStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

//------------------------------------------------------------------------
// FixedLengthEncoder
//------------------------------------------------------------------------

FixedLengthEncoder::FixedLengthEncoder(Stream *strA, int lengthA):
    FilterStream(strA) {
  length = lengthA;
  count = 0;
}

FixedLengthEncoder::~FixedLengthEncoder() {
  if (str->isEncoder())
    delete str;
}

Stream *FixedLengthEncoder::copy() {
  error(errInternal, -1, "Called copy() on FixedLengthEncoder");
  return NULL;
}

void FixedLengthEncoder::reset() {
  str->reset();
  count = 0;
}

int FixedLengthEncoder::getChar() {
  if (length >= 0 && count >= length)
    return EOF;
  ++count;
  return str->getChar();
}

int FixedLengthEncoder::lookChar() {
  if (length >= 0 && count >= length)
    return EOF;
  return str->getChar();
}

GBool FixedLengthEncoder::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

//------------------------------------------------------------------------
// ASCIIHexEncoder
//------------------------------------------------------------------------

ASCIIHexEncoder::ASCIIHexEncoder(Stream *strA):
    FilterStream(strA) {
  bufPtr = bufEnd = buf;
  lineLen = 0;
  eof = gFalse;
}

ASCIIHexEncoder::~ASCIIHexEncoder() {
  if (str->isEncoder()) {
    delete str;
  }
}

Stream *ASCIIHexEncoder::copy() {
  error(errInternal, -1, "Called copy() on ASCIIHexEncoder");
  return NULL;
}

void ASCIIHexEncoder::reset() {
  str->reset();
  bufPtr = bufEnd = buf;
  lineLen = 0;
  eof = gFalse;
}

GBool ASCIIHexEncoder::fillBuf() {
  static const char *hex = "0123456789abcdef";
  int c;

  if (eof) {
    return gFalse;
  }
  bufPtr = bufEnd = buf;
  if ((c = str->getChar()) == EOF) {
    *bufEnd++ = '>';
    eof = gTrue;
  } else {
    if (lineLen >= 64) {
      *bufEnd++ = '\n';
      lineLen = 0;
    }
    *bufEnd++ = hex[(c >> 4) & 0x0f];
    *bufEnd++ = hex[c & 0x0f];
    lineLen += 2;
  }
  return gTrue;
}

//------------------------------------------------------------------------
// ASCII85Encoder
//------------------------------------------------------------------------

ASCII85Encoder::ASCII85Encoder(Stream *strA):
    FilterStream(strA) {
  bufPtr = bufEnd = buf;
  lineLen = 0;
  eof = gFalse;
}

ASCII85Encoder::~ASCII85Encoder() {
  if (str->isEncoder())
    delete str;
}

Stream *ASCII85Encoder::copy() {
  error(errInternal, -1, "Called copy() on ASCII85Encoder");
  return NULL;
}

void ASCII85Encoder::reset() {
  str->reset();
  bufPtr = bufEnd = buf;
  lineLen = 0;
  eof = gFalse;
}

GBool ASCII85Encoder::fillBuf() {
  Guint t;
  char buf1[5];
  int c0, c1, c2, c3;
  int n, i;

  if (eof) {
    return gFalse;
  }
  c0 = str->getChar();
  c1 = str->getChar();
  c2 = str->getChar();
  c3 = str->getChar();
  bufPtr = bufEnd = buf;
  if (c3 == EOF) {
    if (c0 == EOF) {
      n = 0;
      t = 0;
    } else {
      if (c1 == EOF) {
	n = 1;
	t = c0 << 24;
      } else if (c2 == EOF) {
	n = 2;
	t = (c0 << 24) | (c1 << 16);
      } else {
	n = 3;
	t = (c0 << 24) | (c1 << 16) | (c2 << 8);
      }
      for (i = 4; i >= 0; --i) {
	buf1[i] = (char)(t % 85 + 0x21);
	t /= 85;
      }
      for (i = 0; i <= n; ++i) {
	*bufEnd++ = buf1[i];
	if (++lineLen == 65) {
	  *bufEnd++ = '\n';
	  lineLen = 0;
	}
      }
    }
    *bufEnd++ = '~';
    *bufEnd++ = '>';
    eof = gTrue;
  } else {
    t = (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
    if (t == 0) {
      *bufEnd++ = 'z';
      if (++lineLen == 65) {
	*bufEnd++ = '\n';
	lineLen = 0;
      }
    } else {
      for (i = 4; i >= 0; --i) {
	buf1[i] = (char)(t % 85 + 0x21);
	t /= 85;
      }
      for (i = 0; i <= 4; ++i) {
	*bufEnd++ = buf1[i];
	if (++lineLen == 65) {
	  *bufEnd++ = '\n';
	  lineLen = 0;
	}
      }
    }
  }
  return gTrue;
}

//------------------------------------------------------------------------
// RunLengthEncoder
//------------------------------------------------------------------------

RunLengthEncoder::RunLengthEncoder(Stream *strA):
    FilterStream(strA) {
  bufPtr = bufEnd = nextEnd = buf;
  eof = gFalse;
}

RunLengthEncoder::~RunLengthEncoder() {
  if (str->isEncoder())
    delete str;
}

Stream *RunLengthEncoder::copy() {
  error(errInternal, -1, "Called copy() on RunLengthEncoder");
  return NULL;
}

void RunLengthEncoder::reset() {
  str->reset();
  bufPtr = bufEnd = nextEnd = buf;
  eof = gFalse;
}

//
// When fillBuf finishes, buf[] looks like this:
//   +-----+--------------+-----------------+--
//   + tag | ... data ... | next 0, 1, or 2 |
//   +-----+--------------+-----------------+--
//    ^                    ^                 ^
//    bufPtr               bufEnd            nextEnd
//
GBool RunLengthEncoder::fillBuf() {
  int c, c1, c2;
  int n;

  // already hit EOF?
  if (eof)
    return gFalse;

  // grab two bytes
  if (nextEnd < bufEnd + 1) {
    if ((c1 = str->getChar()) == EOF) {
      eof = gTrue;
      return gFalse;
    }
  } else {
    c1 = bufEnd[0] & 0xff;
  }
  if (nextEnd < bufEnd + 2) {
    if ((c2 = str->getChar()) == EOF) {
      eof = gTrue;
      buf[0] = 0;
      buf[1] = (char)c1;
      bufPtr = buf;
      bufEnd = &buf[2];
      return gTrue;
    }
  } else {
    c2 = bufEnd[1] & 0xff;
  }

  // check for repeat
  c = 0; // make gcc happy
  if (c1 == c2) {
    n = 2;
    while (n < 128 && (c = str->getChar()) == c1)
      ++n;
    buf[0] = (char)(257 - n);
    buf[1] = (char)c1;
    bufEnd = &buf[2];
    if (c == EOF) {
      eof = gTrue;
    } else if (n < 128) {
      buf[2] = (char)c;
      nextEnd = &buf[3];
    } else {
      nextEnd = bufEnd;
    }

  // get up to 128 chars
  } else {
    buf[1] = (char)c1;
    buf[2] = (char)c2;
    n = 2;
    while (n < 128) {
      if ((c = str->getChar()) == EOF) {
	eof = gTrue;
	break;
      }
      ++n;
      buf[n] = (char)c;
      if (buf[n] == buf[n-1])
	break;
    }
    if (buf[n] == buf[n-1]) {
      buf[0] = (char)(n-2-1);
      bufEnd = &buf[n-1];
      nextEnd = &buf[n+1];
    } else {
      buf[0] = (char)(n-1);
      bufEnd = nextEnd = &buf[n+1];
    }
  }
  bufPtr = buf;
  return gTrue;
}

//------------------------------------------------------------------------
// LZWEncoder
//------------------------------------------------------------------------

LZWEncoder::LZWEncoder(Stream *strA):
  FilterStream(strA)
{
  inBufStart = 0;
  inBufLen = 0;
  outBufLen = 0;
}

LZWEncoder::~LZWEncoder() {
  if (str->isEncoder()) {
    delete str;
  }
}

Stream *LZWEncoder::copy() {
  error(errInternal, -1, "Called copy() on LZWEncoder");
  return NULL;
}

void LZWEncoder::reset() {
  int i;

  str->reset();

  // initialize code table
  for (i = 0; i < 256; ++i) {
    table[i].byte = i;
    table[i].next = NULL;
    table[i].children = NULL;
  }
  nextSeq = 258;
  codeLen = 9;

  // initialize input buffer
  inBufLen = str->getBlock((char *)inBuf, sizeof(inBuf));
  inBufStart = 0;

  // initialize output buffer with a clear-table code
  outBuf = 256;
  outBufLen = 9;
  needEOD = gFalse;
}

int LZWEncoder::getChar() {
  int ret;

  if (inBufLen == 0 && !needEOD && outBufLen == 0) {
    return EOF;
  }
  if (outBufLen < 8 && (inBufLen > 0 || needEOD)) {
    fillBuf();
  }
  if (outBufLen >= 8) {
    ret = (outBuf >> (outBufLen - 8)) & 0xff;
    outBufLen -= 8;
  } else {
    ret = (outBuf << (8 - outBufLen)) & 0xff;
    outBufLen = 0;
  }
  return ret;
}

int LZWEncoder::lookChar() {
  if (inBufLen == 0 && !needEOD && outBufLen == 0) {
    return EOF;
  }
  if (outBufLen < 8 && (inBufLen > 0 || needEOD)) {
    fillBuf();
  }
  if (outBufLen >= 8) {
    return (outBuf >> (outBufLen - 8)) & 0xff;
  } else {
    return (outBuf << (8 - outBufLen)) & 0xff;
  }
}

// On input, outBufLen < 8.
// This function generates, at most, 2 12-bit codes
//   --> outBufLen < 8 + 12 + 12 = 32
void LZWEncoder::fillBuf() {
  LZWEncoderNode *p0, *p1;
  int seqLen, code, i;

  if (needEOD) {
    outBuf = (outBuf << codeLen) | 257;
    outBufLen += codeLen;
    needEOD = gFalse;
    return;
  }

  // find longest matching sequence (if any)
  p0 = table + inBuf[inBufStart];
  seqLen = 1;
  while (inBufLen > seqLen) {
    for (p1 = p0->children; p1; p1 = p1->next) {
      if (p1->byte == inBuf[inBufStart + seqLen]) {
	break;
      }
    }
    if (!p1) {
      break;
    }
    p0 = p1;
    ++seqLen;
  }
  code = (int)(p0 - table);

  // generate an output code
  outBuf = (outBuf << codeLen) | code;
  outBufLen += codeLen;

  // update the table
  table[nextSeq].byte = seqLen < inBufLen ? inBuf[inBufStart + seqLen] : 0;
  table[nextSeq].children = NULL;
  if (table[code].children) {
    table[nextSeq].next = table[code].children;
  } else {
    table[nextSeq].next = NULL;
  }
  table[code].children = table + nextSeq;
  ++nextSeq;

  // update the input buffer
  inBufStart += seqLen;
  inBufLen -= seqLen;
  if (inBufStart >= 4096 && inBufStart + inBufLen == sizeof(inBuf)) {
    memcpy(inBuf, inBuf + inBufStart, inBufLen);
    inBufStart = 0;
    inBufLen += str->getBlock((char *)inBuf + inBufLen,
			      (int)sizeof(inBuf) - inBufLen);
  }

  // increment codeLen; generate clear-table code
  if (nextSeq == (1 << codeLen)) {
    ++codeLen;
    if (codeLen == 13) {
      outBuf = (outBuf << 12) | 256;
      outBufLen += 12;
      for (i = 0; i < 256; ++i) {
	table[i].next = NULL;
	table[i].children = NULL;
      }
      nextSeq = 258;
      codeLen = 9;
    }
  }

  // generate EOD next time
  if (inBufLen == 0) {
    needEOD = gTrue;
  }
}
