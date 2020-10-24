#ifndef __PERIODIC_H
#define __PERIODIC_H

#include "Windows.h"
#include <iostream>
#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <any> 

class BandReject_Noise_Filter
{
public:
	BandReject_Noise_Filter(const std::string& strImagePath);
	BandReject_Noise_Filter();
	~BandReject_Noise_Filter();
	bool LoadImageData(void* pImageData, const int& nHeight, const int& nWidth);
private:
	bool m_bFileLoadError;
	cv::Mat m_InImage;
	cv::Mat m_GrayImage;
};
#endif
