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

	/*const MinimalRSO* targetRSO1 = manager.find_RSO_from_catalog_ID(467);
	const MinimalRSO* targetRSO2 = manager.find_RSO_from_catalog_ID(404);
	const MinimalRSO* targetRSO3 = manager.find_RSO_from_catalog_ID(336);

	list<double> errors1 = targetRSO1->calculate_linear_approx_error_in_a_period_for_given_resolution(100, 5);
	MinimalRSO::sort_list_to_start_from_minimum(errors1);
	
	list<double> errors2 = targetRSO2->calculate_linear_approx_error_in_a_period_for_given_resolution(100, 5);
	MinimalRSO::sort_list_to_start_from_minimum(errors2);

	list<double> errors3 = targetRSO3->calculate_linear_approx_error_in_a_period_for_given_resolution(100, 5);
	MinimalRSO::sort_list_to_start_from_minimum(errors3);
	
	cout << "Err1: [ ";
	for (auto& err : errors1)
		cout << err << " ";
	cout << "]" << endl;

	cout << "Err2: [ ";
	for (auto& err : errors2)
		cout << err << " ";
	cout << "]" << endl;

	cout << "Err1: [ ";
	for (auto& err : errors3)
		cout << err << " ";
	cout << "]" << endl;*/


	list<ErrorAnalysisReport> errorReports;
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

		/*cout << "Errors: [";
		for (int i = 0; i < 10; i++)
		{
			cout << " " << currReport.errors.at(i);
		}
		cout << "]" << endl;*/
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
	fout.close();


	cout << "Computation end" << endl;
}
