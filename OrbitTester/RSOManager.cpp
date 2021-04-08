#include "RSOManager.h"
#include "rg_TMatrix3D.h"

#include <iomanip>
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
	m_COOPEpoch = cJulian(command.year, command.month, command.day, command.hour, command.min, command.sec);

	auto itForTLEData = m_TLEData.begin();
	for (auto& satellite : m_satellites)
	{
		m_RSOs.push_back(OrbitalBall(index, m_numSegments, &satellite, &m_COOPEpoch, &*itForTLEData));
		m_mapFromIDToOrbitalBall[stoi(m_RSOs.back().get_satellite()->Orbit().SatId())] = &m_RSOs.back();
		index++;
		itForTLEData++;
	}

#ifdef CHECK_TLE
	cout << "Data check..." << endl;
	auto dangerClosePairs = find_danger_close_pairs(1.0);
	for (auto& dangerClosePair : dangerClosePairs)
	{
		auto primary = dangerClosePair.at(0);
		auto secondary = dangerClosePair.at(1);
		double interRSODistance = primary->get_coord().distance(secondary->get_coord());
		cout << "Short distance pair [" << primary->get_satellite()->Orbit().SatId() << ", " << secondary->get_satellite()->Orbit().SatId() << "] with d: " << interRSODistance << "km" << endl;
	}
#endif
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
		m_satellites.push_back(satSGP4);

		char c_secondLine[130];
		char c_thirdLine[130];

		strcpy(c_secondLine, secondLine.c_str());
		strcpy(c_thirdLine, thirdLine.c_str());
		elsetrec currData = convert_TLE_to_elsetrec(c_secondLine, c_thirdLine);
		m_TLEData.push_back(currData);
	}

	fin.close();
}



elsetrec RSOManager::convert_TLE_to_elsetrec(char* longstr1, char* longstr2)
{
	const double deg2rad = pi / 180.0;         //   0.0174532925199433
	const double xpdotp = 1440.0 / (2.0 * pi);  // 229.1831180523293

	double sec, mu, radiusearthkm, tumin, xke, j2, j3, j4, j3oj2;
	double startsec, stopsec, startdayofyr, stopdayofyr, jdstart, jdstop;
	int startyear, stopyear, startmon, stopmon, startday, stopday,
		starthr, stophr, startmin, stopmin;
	int cardnumb, numb, j;
	long revnum = 0, elnum = 0;
	char classification, intldesg[11];
	int year = 0;
	int mon, day, hr, minute, nexp, ibexp;

	getgravconst(GRAV_CONST_TYPE, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2);

	elsetrec satrec;
	satrec.error = 0;

	// set the implied decimal points since doing a formated read
	// fixes for bad input data values (missing, ...)
	for (j = 10; j <= 15; j++)
		if (longstr1[j] == ' ')
			longstr1[j] = '_';

	if (longstr1[44] != ' ')
		longstr1[43] = longstr1[44];
	longstr1[44] = '.';
	if (longstr1[7] == ' ')
		longstr1[7] = 'U';
	if (longstr1[9] == ' ')
		longstr1[9] = '.';
	for (j = 45; j <= 49; j++)
		if (longstr1[j] == ' ')
			longstr1[j] = '0';
	if (longstr1[51] == ' ')
		longstr1[51] = '0';
	if (longstr1[53] != ' ')
		longstr1[52] = longstr1[53];
	longstr1[53] = '.';
	longstr2[25] = '.';
	for (j = 26; j <= 32; j++)
		if (longstr2[j] == ' ')
			longstr2[j] = '0';
	if (longstr1[62] == ' ')
		longstr1[62] = '0';
	if (longstr1[68] == ' ')
		longstr1[68] = '0';

	sscanf(longstr1, "%2d %5ld %1c %10s %2d %12lf %11lf %7lf %2d %7lf %2d %2d %6ld ",
		&cardnumb, &satrec.satnum, &classification, intldesg, &satrec.epochyr,
		&satrec.epochdays, &satrec.ndot, &satrec.nddot, &nexp, &satrec.bstar,
		&ibexp, &numb, &elnum);

	if (longstr2[52] == ' ')
		sscanf(longstr2, "%2d %5ld %9lf %9lf %8lf %9lf %9lf %10lf %6ld \n",
			&cardnumb, &satrec.satnum, &satrec.inclo,
			&satrec.nodeo, &satrec.ecco, &satrec.argpo, &satrec.mo, &satrec.no,
			&revnum);
	else
		sscanf(longstr2, "%2d %5ld %9lf %9lf %8lf %9lf %9lf %11lf %6ld \n",
			&cardnumb, &satrec.satnum, &satrec.inclo,
			&satrec.nodeo, &satrec.ecco, &satrec.argpo, &satrec.mo, &satrec.no,
			&revnum);

	// ---- find no, ndot, nddot ----
	satrec.no = satrec.no / xpdotp; //* rad/min
	satrec.nddot = satrec.nddot * pow(10.0, nexp);
	satrec.bstar = satrec.bstar * pow(10.0, ibexp);

	// ---- convert to sgp4 units ----
	satrec.a = pow(satrec.no * tumin, (-2.0 / 3.0));
	satrec.ndot = satrec.ndot / (xpdotp * 1440.0);  //* ? * minperday
	satrec.nddot = satrec.nddot / (xpdotp * 1440.0 * 1440);

	// ---- find standard orbital elements ----
	satrec.inclo = satrec.inclo * deg2rad;
	satrec.nodeo = satrec.nodeo * deg2rad;
	satrec.argpo = satrec.argpo * deg2rad;
	satrec.mo = satrec.mo * deg2rad;

	satrec.alta = satrec.a * (1.0 + satrec.ecco) - 1.0;
	satrec.altp = satrec.a * (1.0 - satrec.ecco) - 1.0;

	// ----------------------------------------------------------------
	// find sgp4epoch time of element set
	// remember that sgp4 uses units of days from 0 jan 1950 (sgp4epoch)
	// and minutes from the epoch (time)
	// ----------------------------------------------------------------

	// ---------------- temp fix for years from 1957-2056 -------------------
	// --------- correct fix will occur when year is 4-digit in tle ---------
	if (satrec.epochyr < 57)
		year = satrec.epochyr + 2000;
	else
		year = satrec.epochyr + 1900;

	days2mdhms(year, satrec.epochdays, mon, day, hr, minute, sec);
	jday(year, mon, day, hr, minute, sec, satrec.jdsatepoch);

	// ---------------- initialize the orbit at sgp4epoch -------------------
	sgp4init(GRAV_CONST_TYPE, OPS_MODE, satrec.satnum, satrec.jdsatepoch - 2433281.5, satrec.bstar,
		satrec.ecco, satrec.argpo, satrec.inclo, satrec.mo, satrec.no,
		satrec.nodeo, satrec);

	return satrec;
}



OrbitalBall* RSOManager::find_RSO_that_has_eccentricity_similar_to_given(const double& targetEccentricity)
{
	OrbitalBall* ballWithClosestEccentricity = nullptr;
	double minEccentricityDifference = DBL_MAX;
	for (auto& ball : m_RSOs)
	{
		double eccentricity = ball.get_satellite()->Orbit().Eccentricity();
		double difference = abs(eccentricity - targetEccentricity);
		if (ball.is_SGP4_available() && (difference < minEccentricityDifference))
		{
			ballWithClosestEccentricity = &ball;
			minEccentricityDifference = difference;
		}
	}

	return ballWithClosestEccentricity;
}



list<array<OrbitalBall*, 2>> RSOManager::find_danger_close_pairs(const double& threshold)
{
	list<array<OrbitalBall*, 2>> dangerClosePairs;
	for (list<OrbitalBall>::iterator it1 = m_RSOs.begin(); it1 != m_RSOs.end(); it1++)
	{
		for (list<OrbitalBall>::iterator it2 = next(it1); it2 != m_RSOs.end(); it2++)
		{
			float distance = (*it1).get_coord().distance((*it2).get_coord());
			if (distance < threshold)
			{
				dangerClosePairs.push_back({ &(*it1), &(*it2) });
			}
		}
	}
	return dangerClosePairs;
}



void RSOManager::save_RSO_infos(const string& filePath)
{
	ofstream RSOInfoFile(filePath);
	RSOInfoFile << "% RSO Information - Total "<<m_RSOs.size()<<" objects\n%\n";
	RSOInfoFile << "% CatalogID\tCatalogName\n";
	for (auto& rso : m_RSOs)
	{
		RSOInfoFile << setfill('0') << setw(5) << stoi(rso.get_satellite()->Orbit().SatId());
		RSOInfoFile << "\t" <<rso.get_satellite()->Orbit().SatName() << "\n";
	}
	RSOInfoFile.close();
}




tm convert_seconds_to_tmStruct(const cJulian& epoch, const double& targetTime)
{
	cJulian julianTime(epoch);
	julianTime.AddSec(targetTime);

	time_t timet = julianTime.ToTime();
	tm gmt;
#ifdef _WIN32
	gmtime_s(&gmt, &timet);
#else
	gmtime_r(&timet, &gmt);
#endif
	return gmt;
}


string make_time_string(tm moment)
{
	string monthStr;
	switch (moment.tm_mon)
	{
	case 0:
		monthStr = string("Jan.");
		break;
	case 1:
		monthStr = string("Feb.");
		break;
	case 2:
		monthStr = string("Mar.");
		break;
	case 3:
		monthStr = string("Apr.");
		break;
	case 4:
		monthStr = string("May.");
		break;
	case 5:
		monthStr = string("Jun.");
		break;
	case 6:
		monthStr = string("Jul.");
		break;
	case 7:
		monthStr = string("Aug.");
		break;
	case 8:
		monthStr = string("Sep.");
		break;
	case 9:
		monthStr = string("Oct.");
		break;
	case 10:
		monthStr = string("Nov.");
		break;
	case 11:
		monthStr = string("Dec.");
		break;
	default:
		break;
	}

	string timeStr = to_string(moment.tm_hour) += string(":") += to_string(moment.tm_min) += string(":") += to_string(moment.tm_sec) += string(", ") += monthStr += string(" ") += to_string(moment.tm_mday) += string(", ") += to_string(moment.tm_year + 1900);
	return timeStr;
}



void RSOManager::generate_maneuver_plan(const int& targetCatalogID, const double& rotationAngle, const double& startTime, const double& endTime, const int& numSegments)
{
	m_targetCatalogID = targetCatalogID;
	m_wayPoints.clear();

	OrbitalBall* targetRSO = m_mapFromIDToOrbitalBall.at(targetCatalogID);
	rg_Point3D initLocation = targetRSO->calculate_point_on_Kepler_orbit_at_time(startTime);

	rg_TMatrix3D rotationMat;
	rotationMat.rotateArbitraryAxis(initLocation, rotationAngle);

	m_wayPoints.push_back(make_pair(initLocation, convert_seconds_to_tmStruct(m_COOPEpoch, startTime)));

	double secondsPerSegment = (endTime - startTime) / numSegments;

	for (int i = 1; i <= numSegments; i++)
	{
		double currTime = startTime + i * secondsPerSegment;
		rg_Point3D rotatedCoord = rotationMat * targetRSO->calculate_point_on_Kepler_orbit_at_time(currTime);
		m_wayPoints.push_back(make_pair(rotatedCoord, convert_seconds_to_tmStruct(m_COOPEpoch, currTime)));
	}
}




void RSOManager::save_maneuver_plan(const string& filePath, const int& targetCatalogID, const double& rotationAngle, const double& startTime, const double& endTime, const int& numSegments)
{
	ofstream maneuverPlanFile(filePath);

	maneuverPlanFile << "% Maneuver Plan - Generated by rotating the trajectory of RSO " << targetCatalogID << " by " << rotationAngle << " degrees.\n";
	
	tm startTimeInTM = convert_seconds_to_tmStruct(m_COOPEpoch, startTime);
	tm endTimeInTM = convert_seconds_to_tmStruct(m_COOPEpoch, endTime);

	maneuverPlanFile << "% Period: " << make_time_string(startTimeInTM) << " to " << make_time_string(endTimeInTM) << ", Num samples: " << numSegments << "\n% \n";

	int index = 1;
	for (auto& wayPoint : m_wayPoints)
	{
		const rg_Point3D& coord = wayPoint.first;
		const tm& time = wayPoint.second;
		maneuverPlanFile << index++ << "\t" << coord.getX() << "\t" << coord.getY() << "\t" << coord.getZ() << "\t" << time.tm_year + 1900 << "\t" << time.tm_mon + 1 << "\t" << time.tm_mday << "\t" << time.tm_hour << "\t" << time.tm_min << "\t" << time.tm_sec << "\n";
	}

	maneuverPlanFile.close();
}



pair<double, double> RSOManager::find_time_of_closest_approach_for_RSO_pair(int primaryID, int secondaryID, int targetTime, double searchInterval, double resolution)
{
	double startTime = targetTime - searchInterval;
	double endTime = targetTime + searchInterval;

	OrbitalBall* primary = m_mapFromIDToOrbitalBall.at(primaryID);
	OrbitalBall* secondary = m_mapFromIDToOrbitalBall.at(secondaryID);

	double TCA = DBL_MAX;
	double minDistance = DBL_MAX;

	double currTime = startTime;
	while (currTime <= endTime)
	{
		rg_Point3D primaryCoord = primary->calculate_point_on_Kepler_orbit_at_time(currTime);
		rg_Point3D secondaryCoord = secondary->calculate_point_on_Kepler_orbit_at_time(currTime);
		double distance = primaryCoord.distance(secondaryCoord);

		if (distance < minDistance)
		{
			TCA = currTime;
			minDistance = distance;
		}

		currTime += resolution;
	}

	return make_pair(TCA, minDistance);
}
