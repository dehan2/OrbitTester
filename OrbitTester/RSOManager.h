#pragma once

#include "OrbitalBall.h"
#include "constForCOOP.h"

#include <list>
#include <string>
#include <array>
#include "cJulian.h"

using namespace std;

class RSOManager
{
private:
	list<OrbitalBall> m_RSOs;
	
	list<elsetrec> m_TLEData;
	list<cSatellite> m_satellites;
	int m_numSegments;

	map<int, OrbitalBall*> m_mapFromIDToOrbitalBall;

	cJulian m_COOPEpoch;

	map<int, double> m_mapFromIDToVelocityUpdatedTime;

	int m_targetCatalogID;
	list<pair<rg_Point3D, tm>> m_wayPoints;

public:
	RSOManager() = default;
	~RSOManager() {	clear();	};

	void clear();

	list<OrbitalBall>& get_RSOs() { return m_RSOs; }
	inline const cJulian& get_epoch() { return m_COOPEpoch; }

	void initialize_RSO_manager(const PredictionCommand& command);

	void load_two_line_element_set_file(const string& filePath, const int& numObjects);
	elsetrec convert_TLE_to_elsetrec(char* longstr1, char* longstr2);
	
	OrbitalBall* find_RSO_from_ID(const int& ID) { return m_mapFromIDToOrbitalBall.at(ID); }
	OrbitalBall* find_RSO_that_has_eccentricity_similar_to_given(const double& targetEccentricity);
	list<array<OrbitalBall*, 2>> find_danger_close_pairs(const double& threshold);

	void save_RSO_infos(const string& filePath);

	void generate_maneuver_plan(const int& targetCatalogID, const double& rotationAngle, const double& startTime, const double& endTime, const int& numSegments);
	void save_maneuver_plan(const string& filePath, const int& targetCatalogID, const double& rotationAngle, const double& startTime, const double& endTime, const int& numSegments);

	pair<double, double> find_time_of_closest_approach_for_RSO_pair(int primaryID, int secondaryID, int targetTime, double searchInterval, double resolution);
};

