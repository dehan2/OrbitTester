 #pragma once

#include "constForCOOP.h"
#include "coreLib.h"
#include "orbitLib.h"
#include "rg_Point3D.h"
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

//For new STK - 201012
#include "sgp4ext.h"
#include "sgp4unit.h"

#include "Arc3D.h"

//Orbit 2D based on Keplerian orbital position
class OrbitalBall
{
protected:
	int		m_ID = -1;
	double	m_time = 0.0;
	rg_Point3D	m_coord;
	rg_Point3D	m_velocity;
	cSatellite* m_satellite = nullptr;
	int m_numSegments = 0;
	cJulian* m_COOPEpoch = nullptr;
	elsetrec* m_TLEData = nullptr;

	bool isSGP4Available = true;

	//For replica info
	double m_startTimeOfLinearApprox;
	double m_secondsPerSegment;
	rg_Point3D m_startPointOfLineSegment;
	rg_Point3D m_endPointOfLineSegment;

	rg_Point3D m_centerOfCircularApprox;
	rg_Point3D m_coordOfPerigee;
	double m_radiusOfCircularArc;
	double m_ThetaC;

	//For new circular replica
	Arc3D m_arcForCircularReplica;

public:
	OrbitalBall();
	OrbitalBall(int ID, int numSegments, cSatellite* m_satellite, cJulian* localEpoch, elsetrec* TLEData);
	OrbitalBall(const OrbitalBall& rhs);
	
	virtual ~OrbitalBall();

	OrbitalBall& operator=(const OrbitalBall& rhs);

	virtual void copy(const OrbitalBall& rhs);
	virtual void clear();

	inline int get_ID() const { return m_ID; }
	inline double get_time() const { return m_time; }
	inline rg_Point3D get_coord() const { return m_coord; }
	inline rg_Point3D get_velocity() const { return m_velocity; }
	inline cSatellite* get_satellite() const { return m_satellite; }
	inline int get_num_segments() const { return m_numSegments; }
	inline cJulian* get_COOP_epoch() const { return m_COOPEpoch; }
	inline const bool& is_SGP4_available() const { return isSGP4Available; }

	//Replica related functions
	inline double get_next_segment_transition_time() const { return m_startTimeOfLinearApprox + m_secondsPerSegment; }

	inline void set_ID(int ID) { m_ID = ID; }
	inline void set_time(double time) { m_time = time; }
	inline void set_coord(const rg_Point3D& coord) { m_coord = coord; }
	inline void set_velocity(const rg_Point3D& velocity) { m_velocity = velocity; }

	void initialize_replica(int numSegments);
	void change_num_segments(int numSegments);

	rg_Point3D calculate_point_on_Kepler_orbit_at_time_old(double time) const;
	rg_Point3D calculate_point_on_Kepler_orbit_at_time(double time) const; //Updated STK Version - 201012
	rg_Point3D calculate_replica_position_at_time(double time) const;
	double calculate_segment_start_time(double time) const;

	inline double calculate_seconds_per_segment() const { return m_satellite->Orbit().Period() / m_numSegments; }
	double calculate_seconds_from_perigee(const cJulian& t) const;
	double calculate_seconds_from_local_epoch(const cJulian& t) const { return t.SpanSec(*m_COOPEpoch); }
	double calculate_seconds_from_perigee_to_local_epoch() const;
	
	//New version
	void update_circular_approximation(const rg_Point3D& pt0, const rg_Point3D& pt1, const rg_Point3D& pt2);






	//For Circular Approximation
	void update_circular_approximation(const rg_Point3D& pt0, const rg_Point3D& pt1);
	rg_Point3D calculate_coord_of_perigee();
	rg_Point3D calculate_coord_of_circular_replica_at_time(const double& time);
	rg_Point3D calculate_coord_of_circular_replica_at_tau(const double& tau);

	pair<double, double> calculate_max_L2K_and_L2C_error_for_current_line_segment(int numSample);
	double calculate_max_L2K_error_for_current_line_segment(int numSample);
	double calculate_max_L2C_error_for_current_line_segment(int numSample);
	double calculate_max_C2K_error_for_current_line_segment(int numSample);

	float calculate_max_approximation_error();

	inline void initialize_velocity() { m_velocity = (m_endPointOfLineSegment - m_startPointOfLineSegment) / m_secondsPerSegment; }
	void	move_to_next_segment();

	rg_Point3D calculate_position_at_time(double time) const;

	float calculate_positional_error();
};

