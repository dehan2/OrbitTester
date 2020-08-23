// OrbitTester.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <fstream>
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

