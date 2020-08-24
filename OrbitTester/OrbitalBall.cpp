#include "OrbitalBall.h"
#include "rg_Point2D.h"
#include <iostream>

#include <vector>

OrbitalBall::OrbitalBall()
{
	m_satellite = nullptr;
}



OrbitalBall::OrbitalBall(int ID, int numSegments, cSatellite* satellite, cJulian* localEpoch)
{
	m_ID = ID;
	m_satellite = satellite;
	m_localEpoch = localEpoch;
	initialize_replica(numSegments);

	m_coordOfPerigee = calculate_coord_of_perigee();
	update_circular_approximation(m_startPointOfLineSegment, m_endPointOfLineSegment);
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
	m_localEpoch = rhs.m_localEpoch;

	//For replica info
	m_startTimeOfLinearApprox = rhs.m_startTimeOfLinearApprox;
	m_secondsPerSegment = rhs.m_secondsPerSegment;
	m_startPointOfLineSegment = rhs.m_startPointOfLineSegment;
	m_endPointOfLineSegment = rhs.m_endPointOfLineSegment;

	m_centerOfCircularApprox = rhs.m_centerOfCircularApprox;
	m_coordOfPerigee = rhs.m_coordOfPerigee;
	m_radiusOfCircularArc = rhs.m_radiusOfCircularArc;
	m_ThetaC = rhs.m_ThetaC;
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



rg_Point3D OrbitalBall::calculate_point_on_Kepler_orbit_at_time(double time) const
{
	cJulian targetTime = *m_localEpoch;
	targetTime.AddSec(time);
	cVector position = m_satellite->PositionEci(targetTime).Position();
	//position.Mul(SPATIAL_SCALE);
	return rg_Point3D(position.m_x, position.m_y, position.m_z);
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
	cJulian julianTime(*m_localEpoch);
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
	double secondsFromTLEEpochToLocalEpoch = m_localEpoch->SpanSec(m_satellite->Orbit().Epoch());

	//cout << "S2T: " << secondsFromPerigeeToTLEEpoch << ", S2L: " << secondsFromTLEEpochToLocalEpoch << endl;

	double seconsFromPerigeeToLocalEpoch = secondsFromPerigeeToTLEEpoch + secondsFromTLEEpochToLocalEpoch;
	return seconsFromPerigeeToLocalEpoch;
}



void OrbitalBall::update_circular_approximation(const rg_Point3D& pt0, const rg_Point3D& pt1)
{
	//2. True anomaly of start point / end point
	double angle0 = m_coordOfPerigee.angle(pt0);
	double angle1 = m_coordOfPerigee.angle(pt1);

	//3. Compute delta
	double r0 = pt0.magnitude();
	double r1 = pt1.magnitude();
	double delta = 0.5 * (pow(r1, 2) - pow(r0, 2)) / (r1 * cos(angle1) - r0 * cos(angle0));
	m_centerOfCircularApprox = m_coordOfPerigee.getUnitVector() * delta;

	//For debug
	double distance0 = m_centerOfCircularApprox.distance(pt0);
	double distance1 = m_centerOfCircularApprox.distance(pt1);
	if (abs(distance0 - distance1) > 1)
		cout << "Wrong center of circular approx!" << endl;

	//4. Compute radius
	double insideSqrt = pow(r0, 2) + pow(delta, 2) - 2 * r0 * delta * cos(angle0);
	m_radiusOfCircularArc = sqrt(insideSqrt);

	//For debug
	if (abs(m_radiusOfCircularArc - distance0) > 1)
		cout << "Wrong radius of circular approx!" << endl;

	//5. Compute ThetaC
	rg_Point3D vec0 = m_startPointOfLineSegment - m_centerOfCircularApprox;
	rg_Point3D vec1 = m_endPointOfLineSegment - m_centerOfCircularApprox;
	m_ThetaC = vec0.angle(vec1);
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
	rg_Point3D vec0 = (m_startPointOfLineSegment - m_centerOfCircularApprox).getUnitVector();
	rg_Point3D vec1 = (m_endPointOfLineSegment - m_centerOfCircularApprox).getUnitVector();

	rg_Point3D zAxis = vec0.crossProduct(vec1).getUnitVector();
	rg_Point3D yAxis = zAxis.crossProduct(vec0).getUnitVector();

	double angle = tau * m_ThetaC;
	rg_Point3D coordOfCircularReplica = m_centerOfCircularApprox + m_radiusOfCircularArc *(cos(angle) * vec0 + sin(angle) * yAxis);
	
	return coordOfCircularReplica;
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
	//180322 - 어떻게 할까...생각해보자

	return maxApproximationError;
}



void OrbitalBall::move_to_next_segment()
{
	m_startTimeOfLinearApprox = m_startTimeOfLinearApprox + m_secondsPerSegment;

	m_startPointOfLineSegment = calculate_point_on_Kepler_orbit_at_time(m_startTimeOfLinearApprox);
	m_endPointOfLineSegment = calculate_point_on_Kepler_orbit_at_time(m_startTimeOfLinearApprox+m_secondsPerSegment);

	m_coord = m_startPointOfLineSegment;
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




	//180322 - SGP4에 맞게 수정필요. 어떻게 할지 생각해보자
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
