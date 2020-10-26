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
		if (m_InImage.channels() > 1)
		{
			cvtColor(m_InImage, m_GrayImage, cv::COLOR_RGB2GRAY);
		}
		else
		{
			m_GrayImage = m_InImage;
		}
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

bool BandReject_Noise_Filter::Filter_Periodic_Noise()
{
	bool bFilterError = false;
	for (m_nInterations = 0; m_nInterations < InterationParms::kMaxIterations; m_nInterations++)
	{
		Compute2DSpectrum();
	}
	return bFilterError;
}

void BandReject_Noise_Filter::Compute2DSpectrum()
{
	cv::Mat matSpatialImage = m_GrayImage;
	cv::Rect fftRegion = Rect(0, 0, matSpatialImage.cols & -2, matSpatialImage.rows & -2);
	ComputePSD(matSpatialImage, m_matPSD, m_matMAgnitudeSpectrum,
		m_matLogPSD, m_matLogMAgnitudeSpectrum, 1);
	m_matPSD = FFtShift(m_matPSD);
	m_matMAgnitudeSpectrum = FFtShift(m_matMAgnitudeSpectrum);
	m_matLogPSD = FFtShift(m_matLogPSD);
	m_matLogMAgnitudeSpectrum = FFtShift(m_matLogMAgnitudeSpectrum);
	cv::normalize(m_matPSD, m_matPSD, 0, 255, NORM_MINMAX);
	cv::normalize(m_matMAgnitudeSpectrum, m_matMAgnitudeSpectrum, 0, 255, NORM_MINMAX);
	cv::normalize(m_matLogPSD, m_matLogPSD, 0, 1, NORM_MINMAX);
	cv::normalize(m_matLogMAgnitudeSpectrum, m_matLogMAgnitudeSpectrum, 0, 1, NORM_MINMAX);
}

void BandReject_Noise_Filter::ComputePSD(const cv::Mat& matSpatial, cv::Mat& matPSDImage,
	cv::Mat& matMagSpectrum, cv::Mat& matLogPSD, cv::Mat& matLogMagSpectrum, int nLogFlag)
{
	//Function returns both the power spectral density and magnitude spectrum of incoming image, 
	//logarithmic and raw version
	cv::Mat planes[2] = { cv::Mat_<float>(matSpatial.clone()),
		cv::Mat::zeros(matSpatial.size(), CV_64F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);
	cv::dft(complexI, complexI);
	cv::split(complexI, planes);            // planes[0] = Re(DFT(I)), planes[1] = Im(DFT(I))
	cv::Mat matMagnitudeSpectrum;
	cv::magnitude(planes[0], planes[1], matMagSpectrum);
	planes[0].at<float>(0) = 0;
	planes[1].at<float>(0) = 0;
	// compute the PSD = sqrt(Re(DFT(I))^2 + Im(DFT(I))^2)^2
	magnitude(planes[0], planes[1], matPSDImage);        //imgPSD = sqrt(Power spectrum density)
	pow(matPSDImage, 2, matPSDImage);                         //it needs ^2 in order to get PSD
	// logPSD = log(1 + PSD)
	matLogPSD = matPSDImage + Scalar::all(1);
	log(matLogPSD, matLogPSD);
	log(matMagSpectrum, matLogMagSpectrum);
}

cv::Mat BandReject_Noise_Filter::FFtShift(const cv::Mat& matInputSpectrum)
{
	/*Shifts the FFt quadrants for better visualization*/
	cv::Mat matOutputSpectrum = matInputSpectrum.clone();
	int nCenterX = matOutputSpectrum.cols / 2;
	int nCenterY = matOutputSpectrum.rows / 2;
	Mat matQuadrant0(matOutputSpectrum, Rect(0, 0, nCenterX, nCenterY));
	Mat matQuadrant1(matOutputSpectrum, Rect(nCenterX, 0, nCenterX, nCenterY));
	Mat matQuadrant2(matOutputSpectrum, Rect(0, nCenterY, nCenterX, nCenterY));
	Mat matQuadrant3(matOutputSpectrum, Rect(nCenterX, nCenterY, nCenterX, nCenterY));
	Mat matTmp;
	matQuadrant0.copyTo(matTmp);
	matQuadrant3.copyTo(matQuadrant0);
	matTmp.copyTo(matQuadrant3);
	matQuadrant1.copyTo(matTmp);
	matQuadrant2.copyTo(matQuadrant1);
	matTmp.copyTo(matQuadrant2);
	return matOutputSpectrum;
}
