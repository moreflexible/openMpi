#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <math.h>
#include <fstream>
#include "omp.h"
#include <chrono>
#include "filter.h"

#define THREAD_COUNT 8   // You can change the number of thread to see the differences between them !!!

using namespace std;

void serialMeanFilter(const LPCTSTR input, const LPCTSTR output, const short maskSize);

void paralelMeanFilter(const LPCTSTR input, const LPCTSTR output, const short maskSize);

void serialEdgeDetection(const LPCTSTR input, const LPCTSTR output);

void paralelEdgeDetection(const LPCTSTR input, const LPCTSTR output);

int _tmain(int argc, _TCHAR* argv[]){
	LPCTSTR inputImage, outputImage;
	
	omp_set_num_threads(THREAD_COUNT);
	omp_set_nested(1);

		// You must change the paths to your directory !!!
	inputImage = L"C://Users/Enes/Desktop/IMAGE_PROCESS/babamlar.bmp";  
	outputImage = L"C://Users/Enes/Desktop/IMAGE_PROCESS/babamSmoothParalel.bmp";
		////*****************************************////
	
	const short maskSize = 7;  // You can change the mask size which must be odd numbers.
	paralelMeanFilter(inputImage, outputImage, maskSize);
	paralelEdgeDetection(outputImage, outputImage);
	
	getchar();
	return 0;
}

void serialMeanFilter(const LPCTSTR input, const LPCTSTR output, const short maskSize){
	int width = 0, height = 0;
	long Size, new_size;

	BYTE* buffer = LoadBMP(&width, &height, &Size, input);

	BYTE* rawIntensity = ConvertBMPToIntensity(buffer, width, height);
	delete []buffer;
	auto starTimer = chrono::high_resolution_clock::now();
	BYTE * serialFilter = smoothingSerial(rawIntensity, maskSize, width, height);
	auto finishTimer = std::chrono::high_resolution_clock::now();
	auto elapsedTime = finishTimer - starTimer;
	printf("Elapsed Time for Serial Smoothing %5d milliseconds\n", chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count());

	BYTE* serialResult = ConvertIntensityToBMP(serialFilter, width, height, &new_size);
	delete[] serialFilter;
	if (SaveBMP(serialResult, width, height, new_size, output))
		cout << " Output Image was successfully saved" << endl;
	else cout << "Error on saving image" << endl;
	delete[] serialResult;
}

void paralelMeanFilter(const LPCTSTR input, const LPCTSTR output, const short maskSize){
	int width = 0, height = 0;
	long Size, new_size;
	
	BYTE* buffer = LoadBMP(&width, &height, &Size, input);

	BYTE* rawIntensity = ConvertBMPToIntensity(buffer, width, height);
	delete[] buffer;
	auto starTimer = chrono::high_resolution_clock::now();
	BYTE * paralelFilter = smoothingParalel(rawIntensity, maskSize, width, height);
	auto finishTimer = std::chrono::high_resolution_clock::now();
	auto elapsedTime = finishTimer - starTimer;
	printf("Elapsed Time for Parallel Smothing %5d milliseconds\n", chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count());
	delete[] rawIntensity;
	BYTE* paralelResult = ConvertIntensityToBMP(paralelFilter, width, height, &new_size);
	if (SaveBMP(paralelResult, width, height, new_size, output))
		cout << " Output Image was successfully saved" << endl;
	else cout << "Error on saving image" << endl;
	delete[] paralelFilter;
	delete[] paralelResult;
}

void serialEdgeDetection(const LPCTSTR input, const LPCTSTR output){
	int width = 0, height = 0;
	long Size, new_size;

	BYTE* buffer = LoadBMP(&width, &height, &Size, input);

	BYTE* rawIntensity = ConvertBMPToIntensity(buffer, width, height);
	delete[]buffer;

	float *angles = new float[width*height];  // Gradient Angles If you wanna use

	auto starTimer = chrono::high_resolution_clock::now();
	BYTE *gradientSerial = edgeDetectionSerial(rawIntensity, width, height, angles);
	auto finishTimer = std::chrono::high_resolution_clock::now();
	auto elapsedTime = finishTimer - starTimer;
	printf("Elapsed Time for Serial Edge Detection %5d milliseconds\n", chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count());
	delete[] rawIntensity;
	BYTE* gradientResultS = ConvertIntensityToBMP(gradientSerial, width, height, &new_size);
	if (SaveBMP(gradientSerial, width, height, new_size, output))
		cout << " Gradient Image Serial was successfully saved" << endl;
	else cout << "Error on saving image" << endl;
	delete[] gradientSerial;
	delete[] gradientResultS;
	delete[] angles;
}

void paralelEdgeDetection(const LPCTSTR input, const LPCTSTR output){
	int width = 0, height = 0;
	long Size, new_size;

	BYTE* buffer = LoadBMP(&width, &height, &Size, input);

	BYTE* rawIntensity = ConvertBMPToIntensity(buffer, width, height);
	delete[]buffer;
	
	float *angles = new float[width*height];  // Gradient Angles If you wanna use

	auto starTimer = chrono::high_resolution_clock::now();
	BYTE *gradientParalel = edgeDetectionParalel(rawIntensity, width, height, angles);
	auto finishTimer = std::chrono::high_resolution_clock::now();
	auto elapsedTime = finishTimer - starTimer;
	printf("Elapsed Time for Paralel Edge Detection %5d milliseconds\n", chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count());
	delete[] rawIntensity;
	BYTE* gradientResultP = ConvertIntensityToBMP(gradientParalel, width, height, &new_size);
	if (SaveBMP(gradientResultP, width, height, new_size, output))
		cout << " Gradient Image Parallel was successfully saved" << endl;
	else cout << "Error on saving image" << endl;
	delete[] gradientParalel;
	delete[] gradientResultP;
	delete[] angles;
}

