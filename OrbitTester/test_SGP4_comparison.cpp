#include "MinimalRSO.h"
#include "COOPManager.h"
#include "constForCOOP.h"

#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "rg_Point3D.h"

using namespace std;

void load_STK_coord_file(const string& filePath, list<RSOCoords>& coords);
int translate_month_string(const string& monthStr);

int main()
{
	/*array<list<RSOCoords>, 10> STKCoords;
	array<int, 10> targetIDs =
	{
		25874, 37781, 45839, 39227, 1778,
		3669, 44750, 39069, 43379, 43209
	};


	string directory("STK_SGP4\\");

	for (int i = 0; i < 10; i++)
	{
		string fileName = string("no") + to_string(i + 1) + string("_") + to_string(targetIDs.at(i)) + string(".csv");
		load_STK_coord_file(directory + fileName, STKCoords.at(i));
	}*/


	COOPManager manager;
	manager.read_prediction_command_file("..\\PredictionCommand.txt");
	manager.read_TLE_file(manager.get_prediction_command().directory + manager.get_prediction_command().tleFile, 0);
	manager.initialize_minimal_RSOs();



	/*ofstream fout("ComparisonResult.txt");
	ofstream fout2("Deviations.txt");

	for (int i = 0; i < 10; i++)
	{
		cout << "Process " << targetIDs.at(i) << endl;

		fout << i + 1 << "\t" << targetIDs.at(i)<<"\t";

		fout2 << i + 1 << "\t" << targetIDs.at(i) << "\t";

		const MinimalRSO* targetRSO = manager.find_RSO_from_catalog_ID(targetIDs.at(i));
		list<float> errors;

		for (auto& STKCoord : STKCoords.at(i))
		{
			cJulian targetTime(STKCoord.year, STKCoord.mon, STKCoord.day, STKCoord.hour, STKCoord.min, STKCoord.sec);
			rg_Point3D coordFromCOOP = targetRSO->calculate_point_on_Kepler_orbit_at_time(targetTime);
			double difference = coordFromCOOP.distance(STKCoord.coord);
			errors.push_back(difference);

			fout2 << difference << "\t";
		}
		errors.sort();

		float min = errors.front();
		float max = errors.back();

		fout << errors.size() << "\t" << min << "\t" << max << "\t";

		float mean = 0.0f;
		for (auto& error : errors)
			mean += error;

		mean /= errors.size();

		fout << mean << "\t";

		float stdev = 0.0f;
		for (auto& error : errors)
			stdev += pow(error - mean, 2);

		stdev /= (errors.size() - 1);

		stdev = sqrt(stdev);

		fout << stdev << "\n";

		fout2 << "\n";

	}

	fout.close();
	fout2.close();*/



	cout << "Computation end" << endl;
}




void load_STK_coord_file(const string& filePath, list<RSOCoords>& coords)
{
	ifstream fin;
	fin.open(filePath);

	if (fin.is_open())
	{
		char lineData[256];
		fin.getline(lineData, 256);

		while (!fin.eof())
		{
			fin.getline(lineData, 256);

			coords.push_back(RSOCoords());
			RSOCoords& currCoord = coords.back();

			char* context;
			string delimiter = " :,";

			string token = strtok_s(lineData, delimiter.c_str(), &context);
			currCoord.day = stoi(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			currCoord.mon = translate_month_string(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			currCoord.year = stoi(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			currCoord.hour = stoi(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			currCoord.min = stoi(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			currCoord.sec = stof(token);

			float x, y, z, vx, vy, vz;

			token = strtok_s(NULL, delimiter.c_str(), &context);
			x = stof(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			y = stof(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			z = stof(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			vx = stof(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			vy = stof(token);

			token = strtok_s(NULL, delimiter.c_str(), &context);
			vz = stof(token);

			currCoord.coord = rg_Point3D(x, y, z);
			currCoord.velocity = rg_Point3D(vx, vy, vz);
		}
	}
	fin.close();
}


int translate_month_string(const string& monthStr)
{
	if (monthStr.compare("Jan") == 0)
	{
		return 1;
	}
	else if (monthStr.compare("Feb") == 0)
	{
		return 2;
	}
	else if (monthStr.compare("Mar") == 0)
	{
		return 3;
	}
	else if (monthStr.compare("Apr") == 0)
	{
		return 4;
	}
	else if (monthStr.compare("May") == 0)
	{
		return 5;
	}
	else if (monthStr.compare("Jun") == 0)
	{
		return 6;
	}
	else if (monthStr.compare("Jul") == 0)
	{
		return 7;
	}
	else if (monthStr.compare("Aug") == 0)
	{
		return 8;
	}
	else if (monthStr.compare("Sep") == 0)
	{
		return 9;
	}
	else if (monthStr.compare("Oct") == 0)
	{
		return 10;
	}
	else if (monthStr.compare("Nov") == 0)
	{
		return 11;
	}
	else if (monthStr.compare("Dec") == 0)
	{
		return 12;
	}
	else //Unknown
	{
		return -1;
	}
}
