#pragma once

#include "constForCOOP.h"
#include "cJulian.h"
#include "rg_Point3D.h"
#include <list>

using namespace std;

class MinimalRSO
{
private:
	int m_ID;
	TLEFileInfo* m_TLEInfo;
	cJulian* m_COOPEpoch;


public:
	MinimalRSO() = default;
	MinimalRSO(const int& ID, TLEFileInfo* TLEInfo, cJulian* COOPEpoch);
	~MinimalRSO() = default;

	const int get_ID() const { return m_ID; }
	const TLEFileInfo* get_TLE_info() const { return m_TLEInfo; }

	list<double> calculate_linear_approx_error_in_a_period_for_given_resolution(const int& resolution, const int& numSamplesInSegment) const;
	rg_Point3D calculate_point_on_Kepler_orbit_at_time(const double& time) const;
	rg_Point3D calculate_point_on_Kepler_orbit_at_time(const cJulian& time) const;

	static void sort_list_to_start_from_minimum(list<double>& listToSort);
};

