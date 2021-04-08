#include "COOPManager.h"
#include <iostream>
#include <fstream>


COOPManager::COOPManager()
{

}



COOPManager::~COOPManager()
{
	clear();
}


COOPManager& COOPManager::operator=(const COOPManager& rhs)
{
	if (this == &rhs)
		return *this;

	copy(rhs);
	return *this;
}



void COOPManager::copy(const COOPManager& rhs)
{
	//Do nothing
}



void COOPManager::clear()
{

}



const MinimalRSO* COOPManager::find_RSO_from_catalog_ID(const int& catalogID) const
{
	auto it = m_mapFromIDToRSO.find(catalogID);
	if (it != m_mapFromIDToRSO.end())
	{
		return (*it).second;
	}
	else
		return nullptr;
}




void COOPManager::read_prediction_command_file(const string& commandFilePath)
{
	ifstream fin;
	fin.open(commandFilePath);

	if (fin.is_open())
	{
		while (!fin.eof())
		{
			char lineData[256];
			fin.getline(lineData, 256);

			if (lineData[0] != '#')
			{
				char* context;
				string delimiter = " \t";

				string token = strtok_s(lineData, delimiter.c_str(), &context);
				m_command.directory = token;

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.tleFile = token;

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.numObject = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.numLineSegments = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.predictionTimeWindow = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.year = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.month = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.day = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.hour = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.min = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				m_command.sec = stoi(token);
			}
		}
	}

	fin.close();

	m_startMomentOfPredictionTimeWindow = cJulian(m_command.year, m_command.month, m_command.day, m_command.hour, m_command.min, m_command.sec);
}



int COOPManager::read_TLE_file(const string& TLEFilePath, const int& numObjects)
{
	// Space track의 3LE data를 읽는 함수
	ifstream fin;
	fin.open(TLEFilePath.c_str());
	string delimiter = " ";

	int numLoadedRSOs = 0;

	int targetNumRSOs = numObjects;
	//만일 numObjects 값이 0이면 읽을 수 있는 만큼 최대한 읽기
	if (numObjects == 0)
		targetNumRSOs = INT_MAX;

	for (int i = 0; i < targetNumRSOs; i++)
	{
		// 각 RSO당 세 줄의 입력데이터를 읽는다
		string firstLine, secondLine, thirdLine;
		getline(fin, firstLine);
		if (firstLine.length() > 0)
		{
			getline(fin, secondLine);
			getline(fin, thirdLine);

			size_t pos = 0;
			pos = firstLine.find(delimiter);
			firstLine = firstLine.substr(pos + delimiter.length());

			// SGP4_80 library 객체로 tle data 전달
			// SGP4_80은 1980년도 버전의 propagation 함수이다
			cTle TLELines(firstLine, secondLine, thirdLine);
			cSatellite SGP4_80Info(TLELines);

			char c_secondLine[130];
			char c_thirdLine[130];

			// SGP4_06 library 객체로 tle data 전달
			// SGP4_06은 2006년도 버전의 propagation 함수이다
			strcpy(c_secondLine, secondLine.c_str());
			strcpy(c_thirdLine, thirdLine.c_str());
			elsetrec SGP4_06Info = convert_TLE_to_elsetrec(c_secondLine, c_thirdLine);

			m_TLEFileInfos.push_back({ SGP4_06Info, SGP4_80Info });
			numLoadedRSOs++;
		}
		else
		{
			break;
		}
	}

	fin.close();

	// SGP4를 통해 유효하지 않은 RSO들을 filter out 하는 부분
	int numFilteredRSOs = filter_invalid_TLEs();

	return numLoadedRSOs;
}



elsetrec COOPManager::convert_TLE_to_elsetrec(char* longstr1, char* longstr2)
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



int COOPManager::filter_invalid_TLEs()
{
	list<rg_Point3D> coordOfRSOs;
	auto itForTLEData = m_TLEFileInfos.begin();
	
	int numFilteredRSOs = 0;

	while (itForTLEData != m_TLEFileInfos.end())
	{
		double secFromSatEpochToPredictionStartTime = (m_startMomentOfPredictionTimeWindow.Date() - (*itForTLEData).SGP4_06Info.jdsatepoch) * 1440;

		double coordArray[3];
		double velocityArray[3];

		// sgp4 function의 return 값을 통해 TLE의 유효성을 검사할 수 있음
		bool sgp4Result = sgp4(GRAV_CONST_TYPE, (*itForTLEData).SGP4_06Info, secFromSatEpochToPredictionStartTime, coordArray, velocityArray);
		if (sgp4Result == true)
		{

			//Added by JHCha 210302 - To filter out TBA (to be assigned) RSOs
			
			bool isToBeAssigned = false;

			if ((*itForTLEData).SGP4_80Info.Orbit().SatId().c_str()[0] == 'T')
			{
				//std::cout << "found: " << (*itForTLEData).SGP4_80Info.Orbit().SatId() << endl;
				isToBeAssigned = true;
			}
				

			if (isToBeAssigned == false)
			{
				itForTLEData++;
			}
			else
			{
				cout << "RSO " << (*itForTLEData).SGP4_80Info.Orbit().SatId() << "(" << (*itForTLEData).SGP4_80Info.Name() << ") is overlapped to other RSO" << endl;

				itForTLEData = m_TLEFileInfos.erase(itForTLEData);
			}
		}
		else
		{
			cout << "RSO " << (*itForTLEData).SGP4_80Info.Orbit().SatId() << "(" << (*itForTLEData).SGP4_80Info.Name() << ") is not fit for SGP4" << endl;
			itForTLEData = m_TLEFileInfos.erase(itForTLEData);
			++numFilteredRSOs;
		}
	}

	return numFilteredRSOs;
}



void COOPManager::initialize_minimal_RSOs()
{
	for (auto& TLEInfo : m_TLEFileInfos)
	{
		int ID = stoi(TLEInfo.SGP4_80Info.Orbit().SatId());
		m_minimalRSOs.push_back(MinimalRSO(ID, &TLEInfo, &m_startMomentOfPredictionTimeWindow));
		m_mapFromIDToRSO[ID] = &m_minimalRSOs.back();
	}
}