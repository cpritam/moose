#ifndef TIMEPOINTSFROMFILE_H
#define TIMEPOINTSFROMFILE_H

#include "TimeStepper.h"
#include "LinearInterpolation.h"

class TimePointsFromFile;

template<>
InputParameters validParams<TimePointsFromFile>();

class TimePointsFromFile : public TimeStepper
{
public:
  TimePointsFromFile(const InputParameters & parameters);
  
  virtual void init();
  virtual void readTimeKnots();
  virtual void preExecute();
  virtual void acceptStep();
  virtual void rejectStep();

protected:
  virtual Real computeInitialDT();
  virtual Real computeDT();
  void removeOldKnots();
  
  std::string _file_name;
  unsigned int _num_column;
  unsigned int _t_column_index;
  unsigned int _dt_column_index;
  bool _is_dt_from_file;
  bool _use_last_time_as_end_time;
  LinearInterpolation _time_ipol;
  Real _growth_factor;
  bool _cutback_occurred;
  Real _min_dt;
  bool _interpolate;

  std::vector<Real> _time_t;
  std::vector<Real> _time_dt;
  std::vector<Real> _time_knots;
};


#endif //DTFROMFILE_H
