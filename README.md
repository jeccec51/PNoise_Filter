# PNoise_Filter
Example to remove periodic noise in an image using traditional imaging techniques

Usage
To clone this project use the following command 
git clone https://github.com/jeccec51/PNoise_Filter.git
The OpenCV headers and dlls are provided along with this project. If you want to change the OpenCV version, you may need to build OpenCV and then change the linker settings to accommodate OpenCV libs and dlls. 
This project is an introductory example, to demonstrate how OpenCV libraries can be employed to remove periodic noise pattern from images. This project is developed in windows platform, with visual studio 2019 community addition. This is developed as a shared DLL.
To need to include PeriodicNoiseFilter.h in your main project, and then you can use the class BandReject_Noise_Filter I your code. 
This class has two options to provide the input image.
1.	Image path can be provided, the constructor will read the image and store in its member variables after pre-processing
2.	Can provide the floating-point image buffer, along with its metadata like height and width. Constructor will load the data, and store in its member variables after proper usage. 
3.	The Filter_Periodic_Noise() function will perform the noise filtering and store the output in its member variables
4.	The GetOutPutImage() function will return the filtered image. You can use this in your project either to process further or to displa/store.

Theory is explained in the docx version of readme
