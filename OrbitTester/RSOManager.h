#pragma once

#include "OrbitalBall.h"
#include <list>
#include <string>

using namespace std;

class RSOManager
{
private:
	list<OrbitalBall> m_orbitalBalls;
	list<cTle> m_TLEData;
	list<cSatellite> m_satellites;
	int m_numSegments;

	map<int, OrbitalBall*> m_mapFromIDToOrbitalBall;

	cJulian m_epoch;

	map<int, double> m_mapFromIDToVelocityUpdatedTime;

public:
	RSOManager() = default;
	~RSOManager() {	clear();	};

	void clear();

	void initialize_RSO_manager(const PredictionCommand& command);
	void load_two_line_element_set_file(const string& filePath, const int& numObjects);
	void initialize_orbital_simulator(int numSegments, cJulian& epoch, const double& timeWindow);
	
	OrbitalBall* find_RSO_that_has_eccentricity_similar_to_given(const double& targetEccentricity);
};
