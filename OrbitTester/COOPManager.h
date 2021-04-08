#pragma once

#include <iostream>
//SH
#include <iomanip>
#include <queue>
#include <mutex>
#include <map>
#include <vector>

#include "cJulian.h"
#include "cSatellite.h"
#include "constForCOOP.h"
#include "MinimalRSO.h"

using namespace std;
using namespace Zeptomoby::OrbitTools;


class COOPManager
{
private:

	PredictionCommand m_command;
	cJulian m_startMomentOfPredictionTimeWindow;
	list<TLEFileInfo> m_TLEFileInfos;

	list<MinimalRSO> m_minimalRSOs;
	map<int, MinimalRSO*> m_mapFromIDToRSO;

public:

	COOPManager();
	virtual ~COOPManager();

	COOPManager& operator=(const COOPManager& rhs);

	void copy(const COOPManager& rhs);
	void clear();

	inline const PredictionCommand& get_prediction_command() const { return m_command; }
	inline const cJulian& get_prediction_epoch() const { return m_startMomentOfPredictionTimeWindow; }
	inline const list<TLEFileInfo>& get_TLE_infos() const { return m_TLEFileInfos; }

	inline const list<MinimalRSO>& get_RSOs() const { return m_minimalRSOs; }

	const MinimalRSO* find_RSO_from_catalog_ID(const int& catalogID) const;

	void read_prediction_command_file(const string& commandFilePath);

	int read_TLE_file(const string& TLEFilePath, const int& numObjects);
	elsetrec convert_TLE_to_elsetrec(char* longstr1, char* longstr2);

	int filter_invalid_TLEs();

	void initialize_minimal_RSOs();
};

