# openMpi

  In this repository, we try to process images with OpenMPI library. As you know OpenMPI is a multithread library so we decided to do use this library for our works which takes long times. 
  We made serial and paralel functions for "Mean Filters" and serial and paralel functions for "Edge Detection".

The code is written in "Visual C++".

USING THE CODE 

0) First enable the OpenMPI support for your platform.
1) The Input images have to be ".bmp" format and the output image must be saved as ".bmp" format.
2) In the main code, you must be change your input image and output image (saved image) paths to yours.
3) The defined MACRO (THREAD_COUNT) can be change to see the speed differences on the image processing.
4) "Mask Size" of the smooth(mean) filter can be changed but must be odd numbers and greater than 3".
5) In the "Edge Detection" , you can use the parameter of function (float * angles) for your needs (ex: Canny Edge Detection).
