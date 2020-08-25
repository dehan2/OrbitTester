// OrbitTester.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <fstream>
#include <array>
#include <iomanip>
#include "RSOManager.h"
#include "constForDVDCOOP.h"

const int numEccentricitySegments = 10;

using namespace std;


void read_prediction_command_file(const string& filePath, list<PredictionCommand>& commands);


void record_approx_errors(RSOManager& manager, int numTimeSegments);
void record_L2C_L2K_ratio(RSOManager& manager, const string& fileName, int numTimeSegments);
void record_max_approx_error_ratio(RSOManager& manager, const string& fileName);
void record_max_approx_errors_for_line_segments(RSOManager& manager, const string& fileName, int numLineSegments);


int main()
{
	list<PredictionCommand> commands;
	read_prediction_command_file("..\\PredictionCommand.txt", commands);


    RSOManager manager;
	manager.initialize_RSO_manager(commands.front());

	//record_approx_errors(manager, 1000);
	//record_L2C_L2K_ratio(manager, "L2CRatio.txt", 1000);
	record_max_approx_errors_for_line_segments(manager, "MaxL2KRatio.txt", commands.front().numLineSegments);
}



void read_prediction_command_file(const string& filePath, list<PredictionCommand>& commands)
{
	ifstream fin;	
	fin.open(filePath);

	if (fin.is_open()) 	
	{
		while (!fin.eof()) 
		{
			char lineData[256];
			fin.getline(lineData, 256);    

			if (lineData[0] != '#')
			{
				PredictionCommand command;

				char* context;
				string delimiter = " \t";
				
				string token = strtok_s(lineData, delimiter.c_str(), &context);
				command.directory = token;
				
				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.tleFile = token;

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.numObject = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.numLineSegments = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.predictionTimeWindow = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.year = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.month = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.day = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.hour = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.min = stoi(token);

				token = strtok_s(NULL, delimiter.c_str(), &context);
				command.sec = stoi(token);

				commands.push_back(command);
			}
		}
	}

	fin.close();
}



void record_approx_errors(RSOManager& manager, int numTimeSegments)
{
	for (int eccentricity = 1; eccentricity < numEccentricitySegments; eccentricity++)
	{
		OrbitalBall* ball = manager.find_RSO_that_has_eccentricity_similar_to_given(1.0/numEccentricitySegments * eccentricity);

		string name = ball->get_satellite()->Orbit().SatName();
		string ID = ball->get_satellite()->Orbit().SatId();
		double RSOEccentricity = ball->get_satellite()->Orbit().Eccentricity();
		double period = ball->get_satellite()->Orbit().Period();
		double perigee = ball->get_satellite()->Orbit().Perigee();

		cout << "RSO " << eccentricity << ": " << ID << ", " << name << ", " << RSOEccentricity << ", " << period << endl;

		string fileName = "Sat" + to_string(eccentricity) + ".csv";
		ofstream approxErrorFile(fileName);
		approxErrorFile << fixed << setprecision(2);
		approxErrorFile << "# " << name << " (" << ID << "), eccentricity: " << RSOEccentricity << ", perigee(km): " << perigee << "\n";

		double nextSegmentTransitionTime = ball->get_next_segment_transition_time();
		for (int time = 0; time < numTimeSegments; time++)
		{
			double targetTime = period * time / numTimeSegments;

			while (targetTime > nextSegmentTransitionTime)
			{
				//cout << "Segment Changed: " << nextSegmentTransitionTime << endl;
				ball->move_to_next_segment();
				nextSegmentTransitionTime = ball->get_next_segment_transition_time();
			}

			rg_Point3D coordOnKepler = ball->calculate_point_on_Kepler_orbit_at_time(targetTime);
			rg_Point3D coordOfReplica = ball->calculate_replica_position_at_time(targetTime);
			rg_Point3D coordOfCircularReplica = ball->calculate_coord_of_circular_replica_at_time(targetTime);

			double distanceL2K = coordOnKepler.distance(coordOfReplica);
			double distanceC2K = coordOnKepler.distance(coordOfCircularReplica);
			double distanceL2C = coordOfReplica.distance(coordOfCircularReplica);

			approxErrorFile << time << "\t" << distanceL2K << "\t" << distanceC2K << "\t" << distanceL2C << "\n";
		}
		cout << "Error recording finish" << endl;
		approxErrorFile.close();
	}
	cout << "Error estimation finish" << endl;
}



void record_L2C_L2K_ratio(RSOManager& manager, const string& fileName, int numTimeSegments)
{
	array<OrbitalBall*, numEccentricitySegments-1> RSOs;
	array<vector<double>, numEccentricitySegments-1> L2CRatios;
	for (int eccentricity = 1; eccentricity < numEccentricitySegments; eccentricity++)
	{
		OrbitalBall* ball = manager.find_RSO_that_has_eccentricity_similar_to_given(1.0/numEccentricitySegments * eccentricity);

		string name = ball->get_satellite()->Orbit().SatName();
		string ID = ball->get_satellite()->Orbit().SatId();
		double RSOEccentricity = ball->get_satellite()->Orbit().Eccentricity();
		double period = ball->get_satellite()->Orbit().Period();
		double perigee = ball->get_satellite()->Orbit().Perigee();

		RSOs.at(eccentricity - 1) = ball;
		L2CRatios.at(eccentricity - 1) = vector<double>(numTimeSegments);

		cout << "RSO " << eccentricity << ": " << ID << ", " << name << ", " << RSOEccentricity << ", " << period << endl;

		double nextSegmentTransitionTime = ball->get_next_segment_transition_time();
		for (int time = 0; time < numTimeSegments; time++)
		{
			double targetTime = period * time / numTimeSegments;

			while (targetTime > nextSegmentTransitionTime)
			{
				//cout << "Segment Changed: " << nextSegmentTransitionTime << endl;
				ball->move_to_next_segment();
				nextSegmentTransitionTime = ball->get_next_segment_transition_time();
			}

			rg_Point3D coordOnKepler = ball->calculate_point_on_Kepler_orbit_at_time(targetTime);
			rg_Point3D coordOfReplica = ball->calculate_replica_position_at_time(targetTime);
			rg_Point3D coordOfCircularReplica = ball->calculate_coord_of_circular_replica_at_time(targetTime);

			double distanceL2K = coordOnKepler.distance(coordOfReplica);
			double distanceC2K = coordOnKepler.distance(coordOfCircularReplica);
			double distanceL2C = coordOfReplica.distance(coordOfCircularReplica);

			double L2CRatio = distanceL2C / distanceL2K;
			L2CRatios.at(eccentricity - 1).at(time) = L2CRatio;
		}
		cout << "Error recording finish" << endl;
	}
	cout << "Error estimation finish" << endl;

	ofstream L2CRatioRecord(fileName);

	for (int eccentricity = 0; eccentricity < numEccentricitySegments-1; eccentricity++)
	{
		OrbitalBall* ball = RSOs.at(eccentricity);

		string name = ball->get_satellite()->Orbit().SatName();
		string ID = ball->get_satellite()->Orbit().SatId();
		double RSOEccentricity = ball->get_satellite()->Orbit().Eccentricity();
		double period = ball->get_satellite()->Orbit().Period();
		double perigee = ball->get_satellite()->Orbit().Perigee();
		double apogee = ball->get_satellite()->Orbit().Apogee();
		L2CRatioRecord << "# " << name << " (" << ID << "), e: " << RSOEccentricity << ", p: " << perigee << ", a: " << apogee << "\n";
	}


	for (int time = 0; time < numTimeSegments; time++)
	{
		L2CRatioRecord << time << "\t";
		for (int eccentricity = 0; eccentricity < numEccentricitySegments-1; eccentricity++)
		{
			L2CRatioRecord << L2CRatios.at(eccentricity).at(time) << "\t";
		}
		L2CRatioRecord << "\n";
	}
	L2CRatioRecord.close();
	cout << "L2C ratio recording finish" << endl;
}



void record_max_approx_error_ratio(RSOManager& manager, const string& fileName)
{

}



void record_max_approx_errors_for_line_segments(RSOManager& manager, const string& fileName, int numLineSegments)
{
	array<OrbitalBall*, numEccentricitySegments - 1> RSOs;
	array<vector<double>, numEccentricitySegments - 1> L2KErrors;
	array<vector<double>, numEccentricitySegments - 1> L2CErrors;
	
	for (int eccentricity = 1; eccentricity < numEccentricitySegments; eccentricity++)
	{
		OrbitalBall* ball = manager.find_RSO_that_has_eccentricity_similar_to_given(1.0 / numEccentricitySegments * eccentricity);

		string name = ball->get_satellite()->Orbit().SatName();
		string ID = ball->get_satellite()->Orbit().SatId();
		double RSOEccentricity = ball->get_satellite()->Orbit().Eccentricity();
		double period = ball->get_satellite()->Orbit().Period();
		double perigee = ball->get_satellite()->Orbit().Perigee();
		double apogee = ball->get_satellite()->Orbit().Apogee();

		RSOs.at(eccentricity - 1) = ball;
		L2KErrors.at(eccentricity - 1) = vector<double>();
		L2CErrors.at(eccentricity - 1) = vector<double>();

		cout << "RSO " << eccentricity << ": " << ID << ", " << name << ", " << RSOEccentricity << ", " << period << endl;

		string fileName = "Sat" + to_string(eccentricity) + ".csv";
		ofstream approxErrorFile(fileName);
		approxErrorFile << fixed << setprecision(2);
		approxErrorFile << "# " << name << " (" << ID << "), e: " << RSOEccentricity << ", p: " << perigee << ", a: "<<apogee<<"\n";

		double nextSegmentTransitionTime = ball->get_next_segment_transition_time();
		for (int segment = 0; segment < numLineSegments; segment++)
		{
			pair<double, double> maxErrors = ball->calculate_max_L2K_and_L2C_error_for_current_line_segment(10);
			L2KErrors.at(eccentricity - 1).push_back(maxErrors.first);
			L2CErrors.at(eccentricity - 1).push_back(maxErrors.second);
			double ratio = maxErrors.first / maxErrors.second;

			ball->move_to_next_segment();
			nextSegmentTransitionTime = ball->get_next_segment_transition_time();

			approxErrorFile << fixed << setprecision(5);
			approxErrorFile << segment << "\t"<<maxErrors.first << "\t" << maxErrors.second << "\t" << ratio << "\n";
		}
		cout << "Error recording finish" << endl;
		approxErrorFile.close();
	}
	cout << "Error estimation finish" << endl;

	ofstream outputFile(fileName);

	for (int eccentricity = 0; eccentricity < numEccentricitySegments - 1; eccentricity++)
	{
		OrbitalBall* ball = RSOs.at(eccentricity);

		string name = ball->get_satellite()->Orbit().SatName();
		string ID = ball->get_satellite()->Orbit().SatId();
		double RSOEccentricity = ball->get_satellite()->Orbit().Eccentricity();
		double period = ball->get_satellite()->Orbit().Period();
		double perigee = ball->get_satellite()->Orbit().Perigee();
		double apogee = ball->get_satellite()->Orbit().Apogee();
		outputFile << "# " << name << " (" << ID << "), e: " << RSOEccentricity << ", p: " << perigee << ", a: " << apogee << "\n";
	}


	for (int segment = 0; segment < numLineSegments; segment++)
	{
		outputFile << segment << "\t";
		for (int eccentricity = 0; eccentricity < numEccentricitySegments - 1; eccentricity++)
		{
			double ratio = L2KErrors.at(eccentricity).at(segment) / L2CErrors.at(eccentricity).at(segment);
			outputFile << ratio << "\t";
		}
		outputFile << "\n";
	}
	outputFile.close();
	cout << "L2K/L2C ratio recording finish" << endl;
}

