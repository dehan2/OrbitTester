#include "MinimalRSO.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "globals.h"
#include <iostream>

MinimalRSO::MinimalRSO(const int& ID, TLEFileInfo* TLEInfo, cJulian* COOPEpoch)
{
	m_ID = ID;
	m_TLEInfo = TLEInfo;
	m_COOPEpoch = COOPEpoch;
}



list<double> MinimalRSO::calculate_linear_approx_error_in_a_period_for_given_resolution(const int& resolution, const int& numSamplesInSegment) const
{
	list<double> maxErrorsAtSegment;
	double period = m_TLEInfo->SGP4_80Info.Orbit().Period();

	int apogeeIndex = 0;
	double farthestDistance = 0;
	for (int i = 0; i < resolution; i++)
	{
		double time = period / resolution * i;
		rg_Point3D coord = calculate_point_on_Kepler_orbit_at_time(time);
		if (coord.magnitude() > farthestDistance)
		{
			apogeeIndex = i;
			farthestDistance = coord.magnitude();
		}	
	}



	for (int i = 0; i < resolution; i++)
	{
		double segmentStartTime = period / resolution * (i+apogeeIndex);
		double segmentEndTime = period / resolution * (i+1 + apogeeIndex);

		rg_Point3D RSOCoordAtStart = calculate_point_on_Kepler_orbit_at_time(segmentStartTime);
		rg_Point3D RSOCoordAtEnd = calculate_point_on_Kepler_orbit_at_time(segmentEndTime);

		double maxErrorAtSegment = 0.0;

		if (RSOCoordAtStart.squaredMagnitude() < 1 || RSOCoordAtEnd.squaredMagnitude() < 1)
		{
			maxErrorAtSegment = -1.0;
			continue;
		}


		for (int j = 1; j < numSamplesInSegment; j++)
		{
			double samplingRatio = double(j) / numSamplesInSegment;
			double currTime = (1 - samplingRatio) * segmentStartTime + samplingRatio * segmentEndTime;
			
			rg_Point3D RSOCoord = calculate_point_on_Kepler_orbit_at_time(currTime);
			if (RSOCoord.squaredMagnitude() < 1)
			{
				maxErrorAtSegment = -1.0;
				break;
			}

			rg_Point3D replicaCoord = (1 - samplingRatio) * RSOCoordAtStart + samplingRatio * RSOCoordAtEnd;

			double approxError = RSOCoord.distance(replicaCoord);
			if (approxError > maxErrorAtSegment)
				maxErrorAtSegment = approxError;
		}

		maxErrorsAtSegment.push_back(maxErrorAtSegment);
	}

	return maxErrorsAtSegment;
}




rg_Point3D MinimalRSO::calculate_point_on_Kepler_orbit_at_time(const double& time) const
{
	cJulian targetTime = *m_COOPEpoch;
	targetTime.AddSec(time);

	return calculate_point_on_Kepler_orbit_at_time(targetTime);
}




rg_Point3D MinimalRSO::calculate_point_on_Kepler_orbit_at_time(const cJulian& time) const
{
	double minFromSatEpochToCOOPEpoch = (time.Date() - m_TLEInfo->SGP4_06Info.jdsatepoch) * 1440;

	double coord[3];
	double velocity[3];
	bool result = sgp4(GRAV_CONST_TYPE, m_TLEInfo->SGP4_06Info, minFromSatEpochToCOOPEpoch, coord, velocity);

	if (result)
		return rg_Point3D(coord[0], coord[1], coord[2]);
	else
		return rg_Point3D();
}





double MinimalRSO::predict_maximum_error(const int& resolution, const int& errorOrder, const bool& useSameR) const
{
	float meanAnomaly = M_PI / resolution;
	
	float eccentricity = m_TLEInfo->SGP4_80Info.Orbit().Eccentricity();

	//The transformation from mean anomaly to true anomaly is cited from https://en.wikipedia.org/wiki/True_anomaly#cite_note-2

	float trueAnomaly = meanAnomaly;
	if (errorOrder >= 1)
		trueAnomaly += 2 * eccentricity * sin(meanAnomaly);
	if (errorOrder >= 2)
		trueAnomaly += 1.25 * pow(eccentricity, 2) * sin(2*meanAnomaly);
	if (errorOrder >= 3)
		trueAnomaly += pow(eccentricity, 3) * (13 / 12 * sin(3 * meanAnomaly) - 0.25 * sin(meanAnomaly));

	float perigee = m_TLEInfo->SGP4_80Info.Orbit().Perigee() + XKMPER_WGS72;

	float semiMajor = m_TLEInfo->SGP4_80Info.Orbit().SemiMajor()* XKMPER_WGS72;
	float rAtStart = semiMajor * (1 - pow(eccentricity, 2)) / (1 + eccentricity * cos(trueAnomaly));

	float predictedMaxError = perigee - rAtStart * cos(trueAnomaly);

	if (useSameR == true)
		predictedMaxError = perigee * (1- cos(trueAnomaly));

	return predictedMaxError;
}






void MinimalRSO::sort_list_to_start_from_minimum(list<double>& listToSort)
{
	auto itForMinimum = listToSort.begin();

	auto it = listToSort.begin();
	while (it != listToSort.end())
	{
		if ((*it) < (*itForMinimum))
			itForMinimum = it;

		it++;
	}

	listToSort.splice(listToSort.end(), listToSort, listToSort.begin(), itForMinimum);
}