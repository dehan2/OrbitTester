#include "MinimalRSO.h"
#include "COOPManager.h"
#include "constForCOOP.h"

#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main()
{
	COOPManager manager;
	manager.read_prediction_command_file("..\\PredictionCommand.txt");
	manager.read_TLE_file(manager.get_prediction_command().directory + manager.get_prediction_command().tleFile, 0);
	manager.initialize_minimal_RSOs();

	/*list<MissedEntity> missedEntities;
	ifstream fin("Missed.txt");

	if (fin.is_open())
	{
		string line;
		while (getline(fin, line))
		{
			missedEntities.push_back(MissedEntity());
			MissedEntity& entity = missedEntities.back();

			istringstream iss(line);
			iss >> entity.primaryID >> entity.secondaryID >> entity.DCA_COOP >> entity.TCA_COOP >> entity.DCA_STK >> entity.TCA_STK >> entity.timeStr;
		}
	}
	fin.close();

	for (auto& entity : missedEntities)
	{
		const MinimalRSO* primary = manager.find_RSO_from_catalog_ID(entity.primaryID);
		const MinimalRSO* secondary = manager.find_RSO_from_catalog_ID(entity.secondaryID);

		rg_Point3D coord1 = primary->calculate_point_on_Kepler_orbit_at_time(entity.TCA_STK);
		rg_Point3D coord2 = secondary->calculate_point_on_Kepler_orbit_at_time(entity.TCA_STK);
		double distance = coord1.distance(coord2);

		cout << "[" << entity.primaryID << ", " << entity.secondaryID << "] - COOP: [" << entity.DCA_COOP << ", " << entity.TCA_COOP << "], STK: [" << entity.DCA_STK << ", " << entity.TCA_STK << "], found distance: " << distance << endl;
	}*/


	/*ofstream fout("Category.txt");

	for (auto& RSO : manager.get_RSOs())
	{
		int categoryNum = 0;
		double perigee = RSO.get_TLE_info()->SGP4_80Info.Orbit().Perigee();
		double eccentricity = RSO.get_TLE_info()->SGP4_80Info.Orbit().Eccentricity();		
		
		if (perigee < RSO1_prg)
		{
			categoryNum = 1;
		}
		else if (perigee < RSO2_prg)
		{
			if (eccentricity < RSO21_ecc)
				categoryNum = 2;
			else if (eccentricity < RSO22_ecc)
				categoryNum = 3;
			else
				categoryNum = 4;
		}
		else if (perigee < RSO3_prg)
		{
			if (eccentricity < RSO31_ecc)
				categoryNum = 5;
			else if (eccentricity < RSO32_ecc)
				categoryNum = 6;
			else
				categoryNum = 7;
		}
		else
		{
			categoryNum = 8;
		}

		fout << RSO.get_ID() << "\t" << eccentricity << "\t" << perigee << "\t" << categoryNum << "\n";
	}
	fout.close();*/



	/*const MinimalRSO* primary = manager.find_RSO_from_catalog_ID(16202);
	const MinimalRSO* secondary = manager.find_RSO_from_catalog_ID(43379);

	cJulian targetTime(2021, 3, 22, 22, 43, 8.869);
	rg_Point3D coord1 = primary->calculate_point_on_Kepler_orbit_at_time(targetTime);
	rg_Point3D coord2 = secondary->calculate_point_on_Kepler_orbit_at_time(targetTime);
	double distance = coord1.distance(coord2);

	cout << "Found distance: " << distance << endl;*/



	/*array<int, 10> targetIDs = { 25874, 37781, 45839, 39227, 1778, 3669, 44750, 39069, 43379, 43209 };

	for (int i = 0; i < 10; i++)
	{
		int resolution = 100 * (i + 1);
		cout << "Resolution: " << resolution << endl;

		ofstream fout("ErrorReport_" + to_string(resolution) + ".txt");

		for (auto& targetID : targetIDs)
		{
			const MinimalRSO* targetRSO = manager.find_RSO_from_catalog_ID(targetID);
			list<double> errors = targetRSO->calculate_linear_approx_error_in_a_period_for_given_resolution(resolution, 5);

			fout << targetID << "\t";
			for (int j = 0; j < 100; j++)
			{
				double& error = *next(errors.begin(), (i + 1) * j);
				fout << error << "\t";
			}
			fout << "\n";
		}

		fout.close();
	}*/



	/*for (auto& targetID : targetIDs)
	{
		const MinimalRSO* targetRSO = manager.find_RSO_from_catalog_ID(targetID);
		cout << "Target RSO: " << targetRSO->get_ID() << endl;

		ofstream fout("ErrorReport_" + to_string(targetID) + ".txt");

		for (int i = 0; i < 10; i++)
		{
			int resolution = 100 * (i + 1);

			list<double> errors = targetRSO->calculate_linear_approx_error_in_a_period_for_given_resolution(resolution, 5);
			//MinimalRSO::sort_list_to_start_from_minimum(errors);
			
			fout << resolution << "\t";
			for (int j = 0; j < 100; j++)
			{
				double& error = *next(errors.begin(), (i + 1) * j);
				fout << error << "\t";
			}
			fout << "\n";
		}

		fout.close();
	}*/

	for (int r = 0; r < 10; r++)
	{
		int resolution = (r + 1) * 100;
		cout << "Resolution: " << resolution << endl;

		list<ErrorAnalysisReport> errorReports;
		for (auto& rso : manager.get_RSOs())
		{
			if (errorReports.size() % 100 == 0)
				cout << "RSO[" << errorReports.size() << "]: " << rso.get_ID() << endl;

			errorReports.push_back(ErrorAnalysisReport());
			ErrorAnalysisReport& currReport = errorReports.back();
			currReport.ID = rso.get_ID();
			currReport.satName = rso.get_TLE_info()->SGP4_80Info.Orbit().SatName();
			currReport.eccentricity = rso.get_TLE_info()->SGP4_80Info.Orbit().Eccentricity();
			currReport.perigee = rso.get_TLE_info()->SGP4_80Info.Orbit().Perigee();

			list<double> errors = rso.calculate_linear_approx_error_in_a_period_for_given_resolution(resolution, 10);

			if (errors.size() == resolution)
			{
				errors.sort();
				double maxError = errors.back();
				currReport.errors.at(0) = maxError;
			}
			else
			{
				currReport.errors.at(0) = -1;
			}

			for (int i = 0; i < 4; i++)
			{
				double predictedError = rso.predict_maximum_error(resolution, i);
				currReport.errors.at(i + 1) = predictedError;
			}
		}



		string fileName = string("MaxErrors_") + to_string(resolution) + string(".txt");
		ofstream fout(fileName);
		fout << "ID\tName\tEccentricity\tPerigee\tMaxError\tPre_0\tPre_1\tPre_2\tPre_3\n";
		for (auto& report : errorReports)
		{
			fout << report.ID << "\t" << report.satName << "\t" << report.eccentricity << "\t" << report.perigee;
			/*for (int i = 0; i < 10; i++)
			{
				fout << "\t" << report.errors.at(i);
			}*/
			for (int i = 0; i < 5; i++)
			{
				fout << "\t" << report.errors.at(i);
			}
			fout << "\n";
		}
		fout.close();
	}

	cout << "Computation end" << endl;
}
