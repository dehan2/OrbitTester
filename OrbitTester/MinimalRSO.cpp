#include "MinimalRSO.h"
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

	for (int i = 0; i < resolution; i++)
	{
		double segmentStartTime = period / resolution * i;
		double segmentEndTime = period / resolution * (i+1);

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

	double minFromSatEpochToCOOPEpoch = (targetTime.Date() - m_TLEInfo->SGP4_06Info.jdsatepoch) * 1440;
	
	double epoch_80 = m_TLEInfo->SGP4_80Info.Orbit().Epoch().Date();
	double epoch_06 = m_TLEInfo->SGP4_06Info.jdsatepoch;

	
	if(minFromSatEpochToCOOPEpoch < 0)
		return rg_Point3D();


	double coord[3];
	double velocity[3];
	bool result = sgp4(GRAV_CONST_TYPE, m_TLEInfo->SGP4_06Info, minFromSatEpochToCOOPEpoch, coord, velocity);

	if(result)
		return rg_Point3D(coord[0], coord[1], coord[2]);
	else
		return rg_Point3D();
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
