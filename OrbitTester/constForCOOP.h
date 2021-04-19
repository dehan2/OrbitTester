#pragma once

#include <ctime>
#include <cwchar>
#include <list>
#include <array>
#include "coreLib.h"
#include "orbitLib.h"
#include "sgp4ext.h"
#include "sgp4unit.h"

#define DISABLE_COLLISION
//#define CHECK_TLE

//enum ORBIT_SOURCE_TYPE { NONE, TLE };

//All parameters uses km for length, kg for mass, and second for time unit.

//const float GRAVITY_G = 6.6742E-11; // gravitational constant https://en.wikipedia.org/wiki/Gravitational_constant
//const float GRAVITY_M = 5.9722E24; // mass of the earth https://en.wikipedia.org/wiki/Earth_mass
const float GRAVITY_MU = 3.986E5;	//G*M, standard gravitational parameter https://en.wikipedia.org/wiki/Standard_gravitational_parameter

//const cJulian SIMULATION_EPOCH(2018, 3, 22, 0, 0, 0); //00:00:00 22. Mar. 2018 
//const cJulian SIMULATION_EPOCH(2018, 7, 12, 0, 0, 0); 
//const cJulian SIMULATION_EPOCH(2018, 7, 20, 6, 0, 0); 

const double SATELLITE_SIZE = 1.0E-10;
const int BASIC_NUM_APPROXIMATION =5;
const int MAX_APPROXIMATION_LEVEL = 30;

const int BEGINNING_ID_OF_ORBIT_BALL = 1;

const float ORBIT_BOUNDING_TETRAHEDRON_SCALE = 4;

const float INITIAL_SEARCH_INTERVAL = 1;
const int MAX_SEARCH_ITERATION = 100;
const float INTERVAL_EPSILON = 0.01;

class OrbitalBall;

struct SegmentTransitionEvent
{
	double time;
	OrbitalBall* ball;
};


struct compare_segment_transition_events_in_ascending_order {
	bool operator()(const SegmentTransitionEvent& lhs, const SegmentTransitionEvent& rhs)
	{
		return lhs.time > rhs.time;
	}
};


#ifndef _WIN32
#include <limits>
#define DBL_MAX std::numeric_limits<double>::max()
#endif

#define SCAN_MINIMAL_PROXIMITY

const int OPTIMAL_MANEUVER_SEARCH_RANGE = 10;
const int MANEUVER_ANGLE_INCREMENT = 1;


struct PredictionCommand
{
	string directory, tleFile;
	int numObject, numLineSegments, predictionTimeWindow;
	int year, month, day, hour, min, sec;
};


//SGP4_06 Parameters
// opsmode = 'a' best understanding of how afspc code works, 'i' imporved sgp4 resulting in smoother behavior
// GRAV_CONST_TYPE = wgs72old / wgs72 / wgs84;

//SGP4_06 Parameters
const char OPS_MODE = 'i';
const char TYPE_RUN = 'm';
const char TYPE_INPUT = 'e';
const gravconsttype GRAV_CONST_TYPE = wgs84;

const bool FILTER_DATA = true;

//const int TEMPORAL_SCALE = 10;

const int TIME_FRAGMENT_SCALE = 1e3;
const double REVERSE_TIME_FRAGMENT_SCALE = 1e-3;

struct TLEFileInfo
{
	elsetrec SGP4_06Info;
	cSatellite SGP4_80Info;
};


struct OutlierInfo
{
	int ID;
	int primary;
	int secondary;
	int year, mon, day, hour, min;
	float sec, distance;
	string tag;
};



struct ErrorAnalysisReport
{
	int ID;
	string satName;
	float eccentricity;
	float perigee;
	array<float, 10> errors;
};


struct MissedEntity
{
	int primaryID;
	int secondaryID;
	double DCA_COOP;
	double TCA_COOP;
	double DCA_STK;
	double TCA_STK;
	string timeStr;
};



//Conjunction Category parameter
const int RSO1_prg = 500;
const int RSO2_prg = 750;
const int RSO3_prg = 1200;

const double RSO21_ecc = 0.0015;
const double RSO22_ecc = 0.008;

const double RSO31_ecc = 0.003;
const double RSO32_ecc = 0.009;