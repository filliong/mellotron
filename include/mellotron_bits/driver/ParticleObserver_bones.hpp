#ifndef PARTICLE_OBSERVER_BONES_HPP
#define PARTICLE_OBSERVER_BONES_HPP

#include <armadillo>
#include <boost/functional/hash.hpp>
#include <boost/multi_array.hpp>
#include <cmath>
#include <hdf5.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

namespace mellotron {

/*!
 *  \class  ParticleObserver
 *  \author Joey Dumont      <joey.dumont@gmail.com>
 *  \since  2016-09-30
 *  \brief  Observes and outputs the trajectory of a particle in an electromagnetic field.
 *
 * Defines the output of Mellotron. This class observes a single particle as it moves through
 * space. Contains a reference to a Particle object as to query the value of the electric and
 * magnetic field.
 */
template <class FieldModel>
class ParticleObserver
{
public:

  /// Sets the particle to follow.
  /// Also sets an initial size for the data structures that hold information
  /// about the particle.
  ParticleObserver(Particle<FieldModel>& my_particle, const uint my_init_size = 100);

  /// Overloading of the () operator for use with Boost.odeint.
  void operator()(const arma::colvec::fixed<8>& x, double t);

  /// Outputs in a HDF5 file.
  void OutputData();

  /// Writes all the data in a given HDF5 group. Useful to append other write functions in derived classes.
  virtual void WriteAllData(hid_t group_id);

  /// Writes the temporal data points.
  void WriteTimes(hid_t group_id);

  /// Writes gamma.
  void WriteGamma(hid_t group_id);

  /// Writes chi.
  void WriteChi(hid_t group_id);

  /// Writes the state vector (position and momentum).
  void WriteStateVector(hid_t group_id);

  /// Write the electric and magnetic field.
  void WriteElectromagneticField(hid_t group_id);

       Particle<FieldModel>  &  particle;              ///< Particle object that moves through spacetime.

  const int                      init_size;             ///< Initial size of the data structures. Removes some unnecessary arma::resize() calls.

        arma::mat                position;              ///< Record of the particle's position.
        arma::mat                momentum;              ///< Record of the particle's momentum.
        std::vector<double>      gamma;                 ///< Record of the particle's gamma factor.
        std::vector<double>      chi;                   ///< Record of the particle's Lorentz invariant.
        std::vector<double>      times;                 ///< Record of the time values.

        arma::mat                electric_field;        ///< Record of the electric field along the particle's trajectory.
        arma::mat                magnetic_field;        ///< Record of the magnetic field along the particle's trajectory.

        uint                     step_counter;          ///< Counts the number of steps that were taken.

protected:

  /// Utility function that sets the right properties for HDF5 output.
  void SetHDF5Properties(hid_t & dataspace_id, hid_t & plist_id, const int dim, const hsize_t * size, const hsize_t * chunk_size, const uint compression_level);

};

/*!
 *  \class  ParticleObserverLienardWiechert
 *  \author Joey Dumont      <joey.dumont@gmail.com>
 *  \since  2016-09-30
 *  \brief  Observes and outputs the trajectory of a particle in an electromagnetic field.
 *
 * Adds the fields created by the moving charges computed via the Liénard-Wiechert potentials
 * to ParticleObserver.
 */
template <class FieldModel>
class ParticleObserverLienardWiechert : public ParticleObserver<FieldModel>
{
public:

  /// Sets the particle to follow.
  /// We must also declare the radius of the sphere on which
  /// the Liénard-Wiechert field are computed, and the number of points
  /// in theta and phi.
  ParticleObserverLienardWiechert(       Particle<FieldModel>  &  my_particle,
                                  const  double                   my_radius              = 1e4,
                                  const  unsigned int             my_number_points_theta = 50,
                                  const  unsigned int             my_number_points_phi   = 50,
                                  const  unsigned int             my_init_size           = 100);

  /// We redefine the () operator to append the calculation of the emitted radiation.
  void operator()(const arma::colvec::fixed<8>& x, double t);

  /// We interpolate the LW fields such that every spatial point is at the same time.
  void InterpolateLWFieldsOnRetardedTime();

  /// Redeclaration of WriteAllData(hid_t group_id).
  void WriteAllData(hid_t group_id);

  /// Write the Liénard-Wiechert fields to the HDF5 file.
  void WriteLienardWiechertFields(hid_t group_id);


  const double                           radius;                ///< Radius of the detection sphere.
  const unsigned int                     number_points_theta;   ///< Number of discrete points in theta.
  const unsigned int                     number_points_phi;     ///< Number of discrete points in phi.

        arma::vec                        theta;                 ///< Theta values on the sphere [0,\pi).
        arma::vec                        phi;                   ///< Phi values on the sphere [0,2\pi).

        boost::multi_array<double, 4>    electric_field_lw;     ///< Value of the Lienard-Wiechert electric field on the sphere.
        boost::multi_array<double, 4>    magnetic_field_lw;     ///< Value of the Liénard-Wiechert magnetic field on the sphere.
        boost::multi_array<double, 3>    times_lw;              ///< Values of the observation time at which the fields are computed.
};

/*!
 *  \class  ParticleObserverIonized
 *  \author Joey Dumont      <joey.dumont@gmail.com>
 *  \since  2017-06-21
 *  \brief  Observes and outputs the trajectory of an "ionized" particle in an electromagnetic field.
 *
 * This class observes a single particle as it moves through
 * space. Contains a reference to a ParticleIonized object as to query the value of the electric and
 * magnetic field. It only outputs the data if the particle has been "ionized" during the simulation,
 * i.e. that the field surpassed the threshold at some point during the simulation.
 */
template <class FieldModel>
class ParticleObserverIonized : public ParticleObserver<FieldModel>
{
public:
  /// Sets the particle to follow.
  ParticleObserverIonized(ParticleIonized<FieldModel>& my_particle);

  /// Outputs in a HDF5 file.
  void OutputData();

        ParticleIonized<FieldModel>  &  particle_ion;           ///< Particle object that moves through spacetime.

};

} // namespace mellotron

#endif // PARTICLE_OBSERVER_BONES_H
