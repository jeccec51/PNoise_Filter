#ifndef __PERIODIC_H
#define __PERIODIC_H

#include "exports.h"
#include "Windows.h"
#include <iostream>
#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "Constants.h"

NFILTER_DLL class BandReject_Noise_Filter
{
public:
	BandReject_Noise_Filter(const std::string& strImagePath);
	BandReject_Noise_Filter();
	~BandReject_Noise_Filter();
	bool LoadImageData(void* pImageData, const int& nHeight, const int& nWidth);
	bool Filter_Periodic_Noise();
	cv::Mat GetOutPutImage();
private:
	void ApplyFilter();
	inline bool PeakDetect(const double& d1, const double& d2, const double& d3) 
	{return ((d2 > d1) && (d2 > d3));}
	void ApplyLPF(const cv::Mat& matLPF, cv::Mat& matSpectrum);
	int FindLowPassCutoff(cv::Mat& matMagSpectrum, cv::Mat& matLPF);
	void LowPassFilter();
	void Compute2DSpectrum();
	void ComputePSD(const cv::Mat& matInputImg, cv::Mat& matPSDImage, cv::Mat& matMagSpectrum,
		cv::Mat& matLogPSD, cv::Mat& matLogMagSpectrum, int logFlag);
	cv::Mat FFtShift(const cv::Mat& matInputSpectrum);
	double CalculateAverage(cv::Mat& matImage);
	int m_nInterations;
	bool m_bFileLoadError;
	cv::Mat m_InImage;
	cv::Mat m_GrayImage;
	cv::Mat m_matMAgnitudeSpectrum;
	cv::Mat m_matPSD;
	cv::Mat m_matLogMAgnitudeSpectrum;
	cv::Mat m_matLogPSD;
	cv::Mat m_matLPF;
	cv::Mat m_matOutput;
};
#endif
