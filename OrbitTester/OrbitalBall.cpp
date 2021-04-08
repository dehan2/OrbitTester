#include "OrbitalBall.h"
#include "rg_Point2D.h"
#include <iostream>

#include <vector>
#include "rg_Circle2D.h"
#include "Circle3D.h"

OrbitalBall::OrbitalBall()
{
	m_satellite = nullptr;
}



OrbitalBall::OrbitalBall(int ID, int numSegments, cSatellite* satellite, cJulian* localEpoch, elsetrec* TLEData)
{
	m_ID = ID;
	m_satellite = satellite;
	m_COOPEpoch = localEpoch;
	m_TLEData = TLEData;

	double secFromSatEpochToPredictionStartTime = (localEpoch->Date() - TLEData->jdsatepoch) * 1440;

	double coordArray[3];
	double velocityArray[3];
	isSGP4Available = sgp4(GRAV_CONST_TYPE, *m_TLEData, secFromSatEpochToPredictionStartTime, coordArray, velocityArray);

	if (isSGP4Available)
	{
		initialize_replica(numSegments);

		m_coordOfPerigee = calculate_coord_of_perigee();
		update_circular_approximation(m_startPointOfLineSegment, m_endPointOfLineSegment);

	}
}



OrbitalBall::OrbitalBall(const OrbitalBall& rhs)
{
	copy(rhs);
}



OrbitalBall::~OrbitalBall()
{
}



OrbitalBall& OrbitalBall::operator=(const OrbitalBall& rhs)
{
	if (this == &rhs)
		return *this;

	copy(rhs);
	return *this;
}



void OrbitalBall::copy(const OrbitalBall& rhs)
{
	m_ID = rhs.m_ID;
	m_time = rhs.m_time;
	m_coord = rhs.m_coord;
	m_velocity = rhs.m_velocity;
	m_satellite = rhs.m_satellite;
	m_numSegments = rhs.m_numSegments;
	m_COOPEpoch = rhs.m_COOPEpoch;
	m_TLEData = rhs.m_TLEData;

	m_arcForCircularReplica = rhs.m_arcForCircularReplica;

	//For replica info
	m_startTimeOfLinearApprox = rhs.m_startTimeOfLinearApprox;
	m_secondsPerSegment = rhs.m_secondsPerSegment;
	m_startPointOfLineSegment = rhs.m_startPointOfLineSegment;
	m_endPointOfLineSegment = rhs.m_endPointOfLineSegment;

	m_centerOfCircularApprox = rhs.m_centerOfCircularApprox;
	m_coordOfPerigee = rhs.m_coordOfPerigee;
	m_radiusOfCircularArc = rhs.m_radiusOfCircularArc;
	m_ThetaC = rhs.m_ThetaC;

	isSGP4Available = rhs.isSGP4Available;
}



void OrbitalBall::clear()
{
}



void OrbitalBall::initialize_replica(int numSegments)
{
	m_numSegments = numSegments;
	m_secondsPerSegment = calculate_seconds_per_segment();

	m_startTimeOfLinearApprox = calculate_segment_start_time(m_time);
	m_startPointOfLineSegment = calculate_point_on_Kepler_orbit_at_time(m_startTimeOfLinearApprox);
	m_endPointOfLineSegment = calculate_point_on_Kepler_orbit_at_time(m_startTimeOfLinearApprox + m_secondsPerSegment);
	m_coord = calculate_replica_position_at_time(m_time);
	initialize_velocity();
}



void OrbitalBall::change_num_segments(int numSegments)
{
	initialize_replica(numSegments);
}



rg_Point3D OrbitalBall::calculate_point_on_Kepler_orbit_at_time_old(double time) const
{
	cJulian targetTime = *m_COOPEpoch;
	targetTime.AddSec(time);
	cVector position = m_satellite->PositionEci(targetTime).Position();
	//position.Mul(SPATIAL_SCALE);
	return rg_Point3D(position.m_x, position.m_y, position.m_z);
}



rg_Point3D OrbitalBall::calculate_point_on_Kepler_orbit_at_time(double time) const
{
	cJulian targetTime = *m_COOPEpoch;
	targetTime.AddSec(time);

	double secFromSatEpochToCOOPEpoch = (targetTime.Date() - m_TLEData->jdsatepoch) * 1440;

	double coord[3];
	double velocity[3];
	sgp4(GRAV_CONST_TYPE, *m_TLEData, secFromSatEpochToCOOPEpoch, coord, velocity);

	return rg_Point3D(coord[0], coord[1], coord[2]);
}



rg_Point3D OrbitalBall::calculate_replica_position_at_time(double time) const
{
	double timeOnLineSegment = time - m_startTimeOfLinearApprox;
	rg_Point3D positionAtTime = m_startPointOfLineSegment + m_velocity * timeOnLineSegment;

	//cout <<"Replica ["<<m_ID<<"] - SS: " << segmentStartTime << ", SPS: " << secondsPerSegment << ", TOS: " << timeOnLineSegment << endl;
	return positionAtTime;
}



double OrbitalBall::calculate_segment_start_time(double time) const
{
	cJulian julianTime(*m_COOPEpoch);
	julianTime.AddSec(time);
	double secondsFromPerigee = calculate_seconds_from_perigee(julianTime);

	double nextSegmentTransitionTimeFromPerigee = 0.0;
	while (nextSegmentTransitionTimeFromPerigee < secondsFromPerigee)
		nextSegmentTransitionTimeFromPerigee += m_secondsPerSegment;

	//cout << "Next segment transition time: " << nextSegmentTransitionTimeFromPerigee << endl;

	double nextSegmentTransitionTimeFromLocalEpoch = nextSegmentTransitionTimeFromPerigee - calculate_seconds_from_perigee_to_local_epoch();
	return nextSegmentTransitionTimeFromLocalEpoch - m_secondsPerSegment;
}



double OrbitalBall::calculate_seconds_from_perigee_to_local_epoch() const
{
	double meanAnomaly = m_satellite->Orbit().MeanAnomaly();
	double meanMotionPerSec = m_satellite->Orbit().MeanMotion() * 60;
	double secondsFromPerigeeToTLEEpoch = meanAnomaly / meanMotionPerSec;
	double secondsFromTLEEpochToLocalEpoch = m_COOPEpoch->SpanSec(m_satellite->Orbit().Epoch());

	//cout << "S2T: " << secondsFromPerigeeToTLEEpoch << ", S2L: " << secondsFromTLEEpochToLocalEpoch << endl;

	double seconsFromPerigeeToLocalEpoch = secondsFromPerigeeToTLEEpoch + secondsFromTLEEpochToLocalEpoch;
	return seconsFromPerigeeToLocalEpoch;
}




//////////////////////////////////////////////////////////////////////////
// NEW VERSION
//////////////////////////////////////////////////////////////////////////



void OrbitalBall::update_circular_approximation(const rg_Point3D& pt0, const rg_Point3D& pt1, const rg_Point3D& pt2)
{
	rg_Point3D vec1 = pt0 - pt1;
	rg_Point3D vec2 = pt2 - pt1;


	//////////////////////////////////////////////////////////////////////////
	//Cotangent circle solution from https://stackoverflow.com/questions/13977354/build-circle-from-3-points-in-3d-space-implementation-in-c-or-c
	//////////////////////////////////////////////////////////////////////////
	
	// triangle "edges"
	const rg_Point3D t = pt1 - pt0;
	const rg_Point3D u = pt2 - pt0;
	const rg_Point3D v = pt2 - pt1;

	// triangle normal
	const rg_Point3D w = t.crossProduct(u);
	const double wsl = w.squaredMagnitude();//w.getSqrLength();
	
	if (wsl < 10e-14)
	{
		cout << "Error: too small wsl: " << wsl << endl;
		//return false; // area of the triangle is too small (you may additionally check the points for colinearity if you are paranoid)
	}

	// helpers
	const double iwsl2 = 1.0 / (2.0 * wsl);
	const double tt = t.squaredMagnitude();
	const double uu = u.squaredMagnitude();

	// result circle
	rg_Point3D circleCenter_3D = pt0 + (u * tt * (u * v) - t * uu * (t * v)) * iwsl2;
	double   radius = sqrt(tt * uu * (v.squaredMagnitude()) * iwsl2 * 0.5);
	rg_Point3D circAxis = w / sqrt(wsl);
	rg_Point3D zVec = (pt0 - circleCenter_3D).crossProduct(pt2 - circleCenter_3D).getUnitVector();

	m_arcForCircularReplica = Arc3D(Circle3D(circleCenter_3D, radius, zVec), pt0, pt2);
}






//////////////////////////////////////////////////////////////////////////
// OLD VERSION
//////////////////////////////////////////////////////////////////////////

void OrbitalBall::update_circular_approximation(const rg_Point3D& pt0, const rg_Point3D& pt1)
{
	double midTimeOnSegment = m_startTimeOfLinearApprox + m_secondsPerSegment / 2;
	rg_Point3D midPt = calculate_point_on_Kepler_orbit_at_time(midTimeOnSegment);
	update_circular_approximation(pt0, midPt, pt1);
}







rg_Point3D OrbitalBall::calculate_coord_of_perigee()
{
	double secondsFromPerigeeToLocalEpoch = calculate_seconds_from_perigee_to_local_epoch();
	rg_Point3D perigeeCoord = calculate_point_on_Kepler_orbit_at_time(-secondsFromPerigeeToLocalEpoch);
	return perigeeCoord;
}



rg_Point3D OrbitalBall::calculate_coord_of_circular_replica_at_time(const double& time)
{
	double tau = (time - m_startTimeOfLinearApprox) / m_secondsPerSegment;
	rg_Point3D coordOfCircularReplica = calculate_coord_of_circular_replica_at_tau(tau);
	return coordOfCircularReplica;
}



rg_Point3D OrbitalBall::calculate_coord_of_circular_replica_at_tau(const double& tau)
{
	rg_Point3D coordOfCircularReplica = m_arcForCircularReplica.evaluate_point_on_arc(tau);


	/*rg_Point3D vec0 = (m_startPointOfLineSegment - m_centerOfCircularApprox).getUnitVector();
	rg_Point3D vec1 = (m_endPointOfLineSegment - m_centerOfCircularApprox).getUnitVector();

	rg_Point3D zAxis = vec0.crossProduct(vec1).getUnitVector();
	rg_Point3D yAxis = zAxis.crossProduct(vec0).getUnitVector();

	double angle = tau * m_ThetaC;
	rg_Point3D coordOfCircularReplica = m_centerOfCircularApprox + m_radiusOfCircularArc *(cos(angle) * vec0 + sin(angle) * yAxis);*/
	
	return coordOfCircularReplica;
}



pair<double, double> OrbitalBall::calculate_max_L2K_and_L2C_error_for_current_line_segment(int numSample)
{
	double maxL2K = calculate_max_L2K_error_for_current_line_segment(numSample);
	double maxL2C = calculate_max_L2C_error_for_current_line_segment(numSample);
	return make_pair(maxL2K, maxL2C);
}



double OrbitalBall::calculate_max_L2K_error_for_current_line_segment(int numSample)
{
	double maxDistance = 0;

	for (int i = 1; i < numSample; i++)
	{
		double targetTime = m_startTimeOfLinearApprox + m_secondsPerSegment * (double)i / numSample;
		rg_Point3D coordOnKepler = calculate_point_on_Kepler_orbit_at_time(targetTime);
		rg_Point3D coordOfReplica = calculate_replica_position_at_time(targetTime);

		double distanceL2K = coordOnKepler.distance(coordOfReplica);

		if (distanceL2K > maxDistance)
			maxDistance = distanceL2K;
	}

	return maxDistance;
}



double OrbitalBall::calculate_max_L2C_error_for_current_line_segment(int numSample)
{
	double maxDistance = 0;

	for (int i = 1; i < numSample; i++)
	{
		double targetTime = m_startTimeOfLinearApprox + m_secondsPerSegment * (double)i / numSample;
		rg_Point3D coordOfReplica = calculate_replica_position_at_time(targetTime);
		rg_Point3D coordOfCircularReplica = calculate_coord_of_circular_replica_at_time(targetTime);

		double distanceL2C = coordOfReplica.distance(coordOfCircularReplica);
		
		if (distanceL2C > maxDistance)
			maxDistance = distanceL2C;
	}

	return maxDistance;
}



double OrbitalBall::calculate_max_C2K_error_for_current_line_segment(int numSample)
{
	double maxDistance = 0;

	for (int i = 1; i < numSample; i++)
	{
		double targetTime = m_startTimeOfLinearApprox + m_secondsPerSegment * (double)i / numSample;
		rg_Point3D coordOfRSO = calculate_point_on_Kepler_orbit_at_time(targetTime);
		rg_Point3D coordOfCircularReplica = calculate_coord_of_circular_replica_at_time(targetTime);

		double distanceC2K = coordOfRSO.distance(coordOfCircularReplica);

		if (distanceC2K > maxDistance)
			maxDistance = distanceC2K;
	}

	return maxDistance;
}



double OrbitalBall::calculate_seconds_from_perigee(const cJulian& t) const
{
	double secondsFromLocalEpoch = calculate_seconds_from_local_epoch(t);
	double secondsFromPerigee = secondsFromLocalEpoch + calculate_seconds_from_perigee_to_local_epoch();
	return secondsFromPerigee;
}



float OrbitalBall::calculate_max_approximation_error()
{
	float maxApproximationError = 0;
	//180322 - ��� �ұ�...�����غ���

	return maxApproximationError;
}



void OrbitalBall::move_to_next_segment()
{
	m_startTimeOfLinearApprox = m_startTimeOfLinearApprox + m_secondsPerSegment;

	m_startPointOfLineSegment = calculate_point_on_Kepler_orbit_at_time(m_startTimeOfLinearApprox);
	m_endPointOfLineSegment = calculate_point_on_Kepler_orbit_at_time(m_startTimeOfLinearApprox+m_secondsPerSegment);

	m_coord = m_startPointOfLineSegment;
	m_time = m_startTimeOfLinearApprox;
	initialize_velocity();

	update_circular_approximation(m_startPointOfLineSegment, m_endPointOfLineSegment);
}



rg_Point3D OrbitalBall::calculate_position_at_time(double time) const
{
	rg_Point3D position = m_coord + (time - m_time) * m_velocity;
	return position;
}



float OrbitalBall::calculate_positional_error()
{
	float dt = calculate_seconds_per_segment();	//Time interval

	pair<rg_Point3D, rg_Point3D> lineSegment = { get_coord(), calculate_position_at_time(m_time + dt) };

	/*float r = ((lineSegment.first + lineSegment.second) / 2).magnitude();
	float L = lineSegment.first.distance(lineSegment.second);	//Length of line segments

	float positionValue = calculate_position_value(m_meanAnomaly);
	float meanAnomalyAtStart = floor(positionValue) * 2 * M_PI / m_numSegments;
	float meanAnomalyAtEnd = ceil(positionValue) * 2 * M_PI / m_numSegments;
	float trueAnomalyAtStart = m_orbit.calculate_true_anomaly_from_mean_anomaly(meanAnomalyAtStart);
	float trueAnomalyAtEnd = m_orbit.calculate_true_anomaly_from_mean_anomaly(meanAnomalyAtEnd);

	float w = (trueAnomalyAtEnd - trueAnomalyAtStart) / dt;	//Angular velocity
	float T = (positionValue - ceil(positionValue))*dt;	//Elapsed time from segment start

	const float a = acos(L / (2 * r));
	const float l = L * T / dt;
	const float q = 2 * r*sin(w / 2 * T);

	const float esqr = pow(l, 2) + pow(q, 2) - 2 * l*q*sin(w / 2 * T + a);

	if (esqr > 0)
	{
		return sqrt(esqr);
	}
	else if (abs(esqr) < 0.0001)
	{
		return 0;
	}
	else
	{
		return 0;
		//return -1;
	}*/




	//180322 - SGP4�� �°� �����ʿ�. ��� ���� �����غ���
	/*float dt = m_orbit.calculate_period()/m_segmentPoints.size();	//Time interval

	pair<rg_Point3D, rg_Point3D> lineSegment;

	float r = ((lineSegment.first + lineSegment.second) / 2).magnitude();
	float L = lineSegment.first.distance(lineSegment.second);	//Length of line segments

	float positionValue = calculate_position_value(m_meanAnomaly);
	float meanAnomalyAtStart = floor(positionValue) * 2 * M_PI / m_numSegments;
	float meanAnomalyAtEnd = ceil(positionValue) * 2 * M_PI / m_numSegments;
	float trueAnomalyAtStart = m_orbit.calculate_true_anomaly_from_mean_anomaly(meanAnomalyAtStart);
	float trueAnomalyAtEnd = m_orbit.calculate_true_anomaly_from_mean_anomaly(meanAnomalyAtEnd);
	
	float w = (trueAnomalyAtEnd - trueAnomalyAtStart) / dt;	//Angular velocity
	float T = (positionValue - ceil(positionValue))*dt;	//Elapsed time from segment start

	const float a = acos(L / (2 * r));
	const float l = L*T / dt;
	const float q = 2 * r*sin(w / 2 * T);

	const float esqr = pow(l, 2) + pow(q, 2) - 2 * l*q*sin(w / 2 * T + a);

	if (esqr > 0)
	{
		return sqrt(esqr);
	}
	else if (abs(esqr) < 0.0001)
	{
		return 0;
	}
	else
	{
		return 0;
		//return -1;
	}*/

	return 0;
}
