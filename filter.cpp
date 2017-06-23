#include <windows.h>
#include <math.h>
#include <iostream>
#include "omp.h"

#define PI 3.14159265

using namespace std;

BYTE* LoadBMP(int* width, int* height, long* size, LPCTSTR bmpfile)
{
	// declare bitmap structures
	BITMAPFILEHEADER bmpheader;
	BITMAPINFOHEADER bmpinfo;
	// value to be used in ReadFile funcs
	DWORD bytesread;
	// open file to read from
	HANDLE file = CreateFile(bmpfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (NULL == file)
		return NULL; // coudn't open file

	// read file header
	if (ReadFile(file, &bmpheader, sizeof (BITMAPFILEHEADER), &bytesread, NULL) == false)
	{
		CloseHandle(file);
		return NULL;
	}

	//read bitmap info

	if (ReadFile(file, &bmpinfo, sizeof (BITMAPINFOHEADER), &bytesread, NULL) == false)
	{
		CloseHandle(file);
		return NULL;
	}

	// check if file is actually a bmp
	if (bmpheader.bfType != 'MB')
	{
		CloseHandle(file);
		return NULL;
	}

	// get image measurements
	*width = bmpinfo.biWidth;
	*height = abs(bmpinfo.biHeight);

	// check if bmp is uncompressed
	if (bmpinfo.biCompression != BI_RGB)
	{
		CloseHandle(file);
		return NULL;
	}

	// check if we have 24 bit bmp
	if (bmpinfo.biBitCount != 24)
	{
		CloseHandle(file);
		return NULL;
	}


	// create buffer to hold the data
	*size = bmpheader.bfSize - bmpheader.bfOffBits;
	BYTE* Buffer = new BYTE[*size];
	// move file pointer to start of bitmap data
	SetFilePointer(file, bmpheader.bfOffBits, NULL, FILE_BEGIN);
	// read bmp data
	if (ReadFile(file, Buffer, *size, &bytesread, NULL) == false)
	{
		delete[] Buffer;
		CloseHandle(file);
		return NULL;
	}

	// everything successful here: close file and return buffer

	CloseHandle(file);

	return Buffer;
}

bool SaveBMP(BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile)
{
	// declare bmp structures 
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;

	// andinitialize them to zero
	memset(&bmfh, 0, sizeof (BITMAPFILEHEADER));
	memset(&info, 0, sizeof (BITMAPINFOHEADER));

	// fill the fileheader with data
	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+paddedsize;
	bmfh.bfOffBits = 0x36;		// number of bytes to start of bitmap bits

	// fill the infoheader

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;			// we only have one bitplane
	info.biBitCount = 24;		// RGB mode is 24 bits
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;		// can be 0 for 24 bit images
	info.biXPelsPerMeter = 0x0ec4;     // paint and PSP use this values
	info.biYPelsPerMeter = 0x0ec4;
	info.biClrUsed = 0;			// we are in RGB mode and have no palette
	info.biClrImportant = 0;    // all colors are important

	// now we open the file to write to
	HANDLE file = CreateFile(bmpfile, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == NULL)
	{
		CloseHandle(file);
		return false;
	}

	// write file header
	unsigned long bwritten;
	if (WriteFile(file, &bmfh, sizeof (BITMAPFILEHEADER), &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}
	// write infoheader
	if (WriteFile(file, &info, sizeof (BITMAPINFOHEADER), &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}
	// write image data
	if (WriteFile(file, Buffer, paddedsize, &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	// and close file
	CloseHandle(file);

	return true;
}

BYTE* ConvertBMPToIntensity(const BYTE* Buffer, int width, int height)
{
	// first make sure the parameters are valid
	if ((NULL == Buffer) || (width == 0) || (height == 0))
		return NULL;

	// find the number of padding bytes

	int padding = 0;
	int scanlinebytes = width * 3;
	while ((scanlinebytes + padding) % 4 != 0)     // DWORD = 4 bytes
		padding++;
	// get the padded scanline width
	int psw = scanlinebytes + padding;

	// create new buffer
	BYTE* newbuf = new BYTE[width*height];

	// now we loop trough all bytes of the original buffer, 
	// swap the R and B bytes and the scanlines
	long bufpos = 0;
	long newpos = 0;
	for (int row = 0; row < height; row++)
	for (int column = 0; column < width; column++)
	{
		newpos = row * width + column;
		bufpos = (height - row - 1) * psw + column * 3;
		newbuf[newpos] = BYTE(0.11*Buffer[bufpos + 2] + 0.59*Buffer[bufpos + 1] + 0.3*Buffer[bufpos]);
	}

	return newbuf;
}

BYTE* ConvertIntensityToBMP(const BYTE* Buffer, int width, int height, long* newsize)
{
	// first make sure the parameters are valid
	if ((NULL == Buffer) || (width == 0) || (height == 0))
		return NULL;

	// now we have to find with how many bytes
	// we have to pad for the next DWORD boundary	

	int padding = 0;
	int scanlinebytes = width * 3;
	while ((scanlinebytes + padding) % 4 != 0)     // DWORD = 4 bytes
		padding++;
	// get the padded scanline width
	int psw = scanlinebytes + padding;
	// we can already store the size of the new padded buffer
	*newsize = height * psw;

	// and create new buffer
	BYTE* newbuf = new BYTE[*newsize];

	// fill the buffer with zero bytes then we dont have to add
	// extra padding zero bytes later on
	memset(newbuf, 0, *newsize);

	// now we loop trough all bytes of the original buffer, 
	// swap the R and B bytes and the scanlines
	long bufpos = 0;
	long newpos = 0;
	for (int row = 0; row < height; row++)
	for (int column = 0; column < width; column++)
	{
		bufpos = row * width + column;     // position in original buffer
		newpos = (height - row - 1) * psw + column * 3;           // position in padded buffer

		newbuf[newpos] = Buffer[bufpos];       //  blue
		newbuf[newpos + 1] = Buffer[bufpos];   //  green
		newbuf[newpos + 2] = Buffer[bufpos];   //  red
	}

	return newbuf;
}

static BYTE innerMaskCalculation(const BYTE *raw_intensity, const BYTE maskSize, const int width, int location){
	 
	BYTE border = (maskSize == 3) ? 1 : (maskSize / 3) + 1;
	int ave = 0;

	location = location - (border*width - border);

	for (int i = 0; i < maskSize; i++){
		for (int j = 0; j < maskSize; j++){
			ave += raw_intensity[location + j];
		}
		location += width;
	}
	return ave /= maskSize*maskSize;

}

BYTE* smoothingParalel(const BYTE *raw_intensity, const short maskSize, const int width, const int height){
	
	BYTE * buffer = new BYTE[width*height];
	BYTE clipper = (maskSize == 3) ? 1 : (maskSize / 3) + 1;

#pragma omp parallel
	{
		#pragma omp for schedule(static)  
		for (int i = (width*clipper) + clipper; i <= width*(height - clipper) - clipper; i += width){
			for (int j = 0; j < (width - 2 * clipper); j++){
				buffer[i + j] = innerMaskCalculation(raw_intensity, maskSize, width, (i + j));
			}
		}
	}
	return buffer;

}

BYTE* smoothingSerial(const BYTE *raw_intensity, const short maskSize, const int width, const int height){

	BYTE * buffer = new BYTE[width*height];
	BYTE clipper = (maskSize == 3) ? 1 : (maskSize / 3) + 1;
  
	for (int i = (width*clipper) + clipper; i <= width*(height - clipper) - clipper; i += width){
		for (int j = 0; j < (width - 2 * clipper); j++){
			buffer[i + j] = innerMaskCalculation(raw_intensity, maskSize, width, (i + j));
		}
	}
	return buffer;
}

BYTE * edgeDetectionSerial(const BYTE* intensityBuffer, const int width, const int height, float * aciDegerleri)
{
	int * edge_ver = new int[width*height];
	int * edge_hor = new int[width*height];
	int * sum = new int[width*height];
	int size = width*height;

	BYTE * edgeimage = new BYTE[width*height];
	memset(edge_ver, 0, width*height);
	memset(edge_hor, 0, width*height);
	memset(edgeimage, 0, width*height);
	memset(sum, 0, width*height);

	int biggest = 255;
	int virtualsize = size - width;
	for (int i = width + 1; i < virtualsize; i += width)//resmin içiköþeler ve kenarlar hariç
	{
		for (int j = i; j < i + width - 1; j++)
		{
			edge_hor[j] = intensityBuffer[j + width - 1] + 2 * intensityBuffer[j + width] + intensityBuffer[j + width + 1]
				- intensityBuffer[j - width - 1] - 2 * intensityBuffer[j - width] - intensityBuffer[j - width + 1];
			edge_ver[j] = -intensityBuffer[j + width - 1] + intensityBuffer[j + width + 1] - 2 * intensityBuffer[j - 1] + 2 * intensityBuffer[j + 1]
				- intensityBuffer[j + width - 1] + intensityBuffer[j + width + 1];
			if (edge_ver[j] != 0)
				aciDegerleri[j] = atan2((float)edge_hor[j], edge_ver[j]) * 180 / PI;
			else
				aciDegerleri[j] = atan2((float)edge_hor[j], 1) * 180 / PI;
			if (aciDegerleri[j] < 0)
				aciDegerleri[j] = (360 + aciDegerleri[j]);
			sum[j] = sqrt(pow((double)edge_hor[j], 2) + pow((double)edge_ver[j], 2));
			if (sum[j]>biggest)  biggest = sum[j];

		}

	}
	delete[] edge_hor;
	delete[] edge_ver;

	
	for (int j = 0; j < width*height; j++)//resmin içiköþeler ve kenarlar hariç
	{
		//edgeimage[j] = (sum[j] * 255) / biggest;
		edgeimage[j] = (sum[j] > biggest)? biggest : sum[j];
	}

	delete[] sum;
	return edgeimage;

}

BYTE * edgeDetectionParalel(const BYTE* intensityBuffer, const int width, const int height, float * aciDegerleri)
{
	int * edge_ver = new int[width*height];
	int * edge_hor = new int[width*height];
	int * sum = new int[width*height];
	int size = width*height;

	BYTE * edgeimage = new BYTE[width*height];
	memset(edge_ver, 0, width*height);
	memset(edge_hor, 0, width*height);
	memset(edgeimage, 0, width*height);
	memset(sum, 0, width*height);

	int biggest = 255;
	int virtualsize = size - width;
#pragma omp parallel
{
	#pragma omp for schedule(static) 
	for (int i = width + 1; i < virtualsize; i += width)
		for (int j = i; j < i + width - 1; j++){
			edge_hor[j] = intensityBuffer[j + width - 1] + 2 * intensityBuffer[j + width] + intensityBuffer[j + width + 1]
				- intensityBuffer[j - width - 1] - 2 * intensityBuffer[j - width] - intensityBuffer[j - width + 1];
			edge_ver[j] = -intensityBuffer[j + width - 1] + intensityBuffer[j + width + 1] - 2 * intensityBuffer[j - 1] + 2 * intensityBuffer[j + 1]
				- intensityBuffer[j + width - 1] + intensityBuffer[j + width + 1];
			if (edge_ver[j] != 0)
				aciDegerleri[j] = atan2((float)edge_hor[j], edge_ver[j]) * 180 / PI;
			else
				aciDegerleri[j] = atan2((float)edge_hor[j], 1) * 180 / PI;
			if (aciDegerleri[j] < 0)
				aciDegerleri[j] = (360 + aciDegerleri[j]);
			sum[j] = (int)sqrt(pow((double)edge_hor[j], 2) + pow((double)edge_ver[j], 2));
			if (sum[j]>biggest)  biggest = sum[j];
		}
}
	
	delete[] edge_hor;
	delete[] edge_ver;

	for (int j = 0; j < width*height; j++)
		edgeimage[j] = (sum[j] > biggest) ? biggest : sum[j];

	delete[] sum;
	return edgeimage;

}
