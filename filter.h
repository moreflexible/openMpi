#include <windows.h>

BYTE* LoadBMP(int* width, int* height, long* size, LPCTSTR bmpfile);
bool  SaveBMP(BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile);
BYTE* ConvertBMPToIntensity(const BYTE* Buffer, int width, int height);
BYTE* ConvertIntensityToBMP(const BYTE* Buffer, int width, int height, long* newsize);
BYTE* smoothingParalel(const BYTE *raw_intensity, const short maskSize, const int width, const int height);
BYTE* smoothingSerial(const BYTE *raw_intensity, const short maskSize, const int width, const int height);
BYTE * edgeDetectionSerial(const BYTE* intensityBuffer, const int width, const int height, float * aciDegerleri);
BYTE * edgeDetectionParalel(const BYTE* intensityBuffer, const int width, const int height, float * aciDegerleri);
