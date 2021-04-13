#include "MinimalRSO.h"
#include "COOPManager.h"
#include "constForCOOP.h"

#include <list>
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	COOPManager manager;
	manager.read_prediction_command_file("..\\PredictionCommand.txt");
	manager.read_TLE_file(manager.get_prediction_command().directory + manager.get_prediction_command().tleFile, 0);
	manager.initialize_minimal_RSOs();

	const MinimalRSO* primary = manager.find_RSO_from_catalog_ID(11);
	const MinimalRSO* secondary = manager.find_RSO_from_catalog_ID(19275);



	/*array<int, 10> targetIDs = { 25874, 558, 31019, 39227, 4597, 3669, 44750, 39069, 43379, 43209 };

	for (int i = 0; i < 10; i++)
	{
		const MinimalRSO* targetRSO = manager.find_RSO_from_catalog_ID(targetIDs.at(i));
		cout << "Target RSO: " << targetRSO->get_ID() << endl;

		ofstream fout("ErrorReport_" + to_string(targetIDs.at(i)) + ".txt");

		for (int i = 0; i < 10; i++)
		{
			int resolution = 100 * (i + 1);

			list<double> errors = targetRSO->calculate_linear_approx_error_in_a_period_for_given_resolution(resolution, 5);
			MinimalRSO::sort_list_to_start_from_minimum(errors);
			
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


	/*list<ErrorAnalysisReport> errorReports;
	for (auto& rso : manager.get_RSOs())
	{
		if (errorReports.size() % 100 == 0)
			cout << "RSO[" << errorReports.size() << "]: " << rso.get_ID()<<endl;

		errorReports.push_back(ErrorAnalysisReport());
		ErrorAnalysisReport& currReport = errorReports.back();
		currReport.ID = rso.get_ID();
		currReport.satName = rso.get_TLE_info()->SGP4_80Info.Orbit().SatName();
		currReport.eccentricity = rso.get_TLE_info()->SGP4_80Info.Orbit().Eccentricity();
		currReport.perigee = rso.get_TLE_info()->SGP4_80Info.Orbit().Perigee();

		for (int i = 0; i < 10; i++)
		{
			int resolution = 100 * (i + 1);

			list<double> errors = rso.calculate_linear_approx_error_in_a_period_for_given_resolution(resolution, 5);

			if (errors.size() == resolution)
			{
				errors.sort();
				double maxError = errors.back();
				currReport.errors.at(i) = maxError;
			}
			else
			{
				currReport.errors.at(i) = -1;
			}
		}
	}




	ofstream fout("MaxErrors.txt");
	fout << "ID\tName\tEccentricity\tPerigee\tRes_100\tRes_200\tRes_300\tRes_400\tRes_500\tRes_600\tRes_700\tRes_800\tRes_900\tRes_1000\n";
	for (auto& report : errorReports)
	{
		fout << report.ID << "\t" << report.satName << "\t" << report.eccentricity << "\t" << report.perigee;
		for (int i = 0; i < 10; i++)
		{
			fout << "\t" << report.errors.at(i);
		}
		fout << "\n";
	}
	fout.close();*/


	cout << "Computation end" << endl;
}
