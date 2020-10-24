#include "PeriodicFilter.h"
#include <numeric>

using namespace cv;
using namespace std;


BandReject_Noise_Filter::BandReject_Noise_Filter(const std::string& imPath)
{
	m_bFileLoadError = true;
	m_InImage = cv::imread(imPath, cv::IMREAD_GRAYSCALE);
	if (m_InImage.empty())
	{
		std::cout << "Error opening image" << endl;
	}
	else
	{
		m_bFileLoadError = false;
	}
}

BandReject_Noise_Filter::BandReject_Noise_Filter()
{
	m_bFileLoadError = true;
}


BandReject_Noise_Filter::~BandReject_Noise_Filter()
{
}

bool BandReject_Noise_Filter::LoadImageData(void* pImageData, const int& nHeight,
	const int& nWidth)
{
	m_bFileLoadError = true;
	float* pImData = static_cast<float*>(pImageData);
	if (pImData != nullptr &&
		nHeight > 0 &&
		nWidth > 0 &&
		sizeof(pImageData) == sizeof(float)
		)
	{
		cv::Mat imageBuffer = cv::Mat(nWidth, nHeight, CV_32F, pImData);
		m_InImage = imageBuffer;
		m_bFileLoadError = false;
	}
	return m_bFileLoadError;
}
