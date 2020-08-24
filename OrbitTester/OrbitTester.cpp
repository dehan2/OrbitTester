// OrbitTester.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <fstream>
#include <array>
#include <iomanip>
#include "RSOManager.h"
#include "constForDVDCOOP.h"

using namespace std;



void read_prediction_command_file(const string& filePath, list<PredictionCommand>& commands);

int main()
{
	list<PredictionCommand> commands;
	read_prediction_command_file("..\\PredictionCommand.txt", commands);


    RSOManager manager;
	manager.initialize_RSO_manager(commands.front());

	/*cout << "Eccentricity" << endl;
	for (auto& RSO : manager.get_RSOs())
	{
		cout << RSO.get_satellite()->Orbit().Eccentricity() << endl;
	}
	cout << "Eccentricity print finish" << endl;*/

	for (int eccentricity = 1; eccentricity < 10; eccentricity++)
	{
		OrbitalBall* ball = manager.find_RSO_that_has_eccentricity_similar_to_given(0.1 * eccentricity);
		
		string name = ball->get_satellite()->Orbit().SatName();
		string ID = ball->get_satellite()->Orbit().SatId();
		double RSOEccentricity = ball->get_satellite()->Orbit().Eccentricity();
		double period = ball->get_satellite()->Orbit().Period();
		double perigee = ball->get_satellite()->Orbit().Perigee();

		cout << "RSO " << eccentricity << ": " << ID << ", " << name << ", " << RSOEccentricity <<", "<<period<< endl;
		
		string fileName = "Sat" + to_string(eccentricity) + ".csv";
		ofstream approxErrorFile(fileName);
		approxErrorFile << fixed << setprecision(2);
		approxErrorFile << "# "<<name<<" ("<< ID << "), eccentricity: " << RSOEccentricity << ", perigee(km): " << perigee << "\n";

		double nextSegmentTransitionTime = ball->get_next_segment_transition_time();
		for (int time = 0; time < 1000; time++)
		{
			double targetTime = period * time / 1000;

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

