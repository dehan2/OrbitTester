#include "RSOManager.h"

#include <fstream>
#include <iostream>


void RSOManager::clear()
{

}



void RSOManager::initialize_RSO_manager(const PredictionCommand& command)
{
	string filePath = command.directory + command.tleFile;
	load_two_line_element_set_file(filePath, command.numObject);

	int index = BEGINNING_ID_OF_ORBIT_BALL;
	m_numSegments = command.numLineSegments;
	m_epoch = cJulian(command.year, command.month, command.day, command.hour, command.min, command.sec);

	for (auto& satellite : m_satellites)
	{
		m_RSOs.push_back(OrbitalBall(index, m_numSegments, &satellite, &m_epoch));
		m_mapFromIDToOrbitalBall[stoi(m_RSOs.back().get_satellite()->Orbit().SatId())] = &m_RSOs.back();
		index++;
	}
}



void RSOManager::load_two_line_element_set_file(const string& filePath, const int& numObjects)
{
	ifstream fin;
	fin.open(filePath.c_str());
	string delimiter = " ";

	for (int i = 0; i < numObjects; i++)
	{
		string firstLine, secondLine, thirdLine;
		getline(fin, firstLine);
		getline(fin, secondLine);
		getline(fin, thirdLine);

		size_t pos = 0;
		pos = firstLine.find(delimiter);
		firstLine = firstLine.substr(pos + delimiter.length());
		cTle tleSGP4(firstLine, secondLine, thirdLine);
		cSatellite satSGP4(tleSGP4);

		m_TLEData.push_back(tleSGP4);
		m_satellites.push_back(satSGP4);
	}

	fin.close();
}




OrbitalBall* RSOManager::find_RSO_that_has_eccentricity_similar_to_given(const double& targetEccentricity)
{
	OrbitalBall* ballWithClosestEccentricity = nullptr;
	double minEccentricityDifference = DBL_MAX;
	for (auto& ball : m_RSOs)
	{
		double eccentricity = ball.get_satellite()->Orbit().Eccentricity();
		double difference = abs(eccentricity - targetEccentricity);
		if (difference < minEccentricityDifference)
		{
			ballWithClosestEccentricity = &ball;
			minEccentricityDifference = difference;
		}
	}

	return ballWithClosestEccentricity;
}
