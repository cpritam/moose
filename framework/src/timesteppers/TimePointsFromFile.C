#include "TimePointsFromFile.h"

template<>
InputParameters validParams<TimePointsFromFile>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<std::string>("file_name", "Name of file from where time knot points are read");
  params.addRequiredParam<unsigned int>("number_of_columns", "Number of columns in the file");
  params.addRequiredParam<unsigned int>("t_data_column_index", "Index of column with t data");
  params.addParam<unsigned int>("dt_data_column_index", 0, "Index of column with dt data");
  params.addRequiredParam<bool>("is_dt_from_file", "if dt specified in file then true, else calculated from t points");
  params.addRequiredParam<bool>("use_last_time_as_end_time", "If true uses last time point as end time, else uses end time specified in executioner block");
  params.addParam<Real>("growth_factor", 2, "Maximum ratio of new to previous timestep sizes following a step that required the time step to be cut due to a failed solve.");
  params.addParam<Real>("min_dt", 0, "The minimal dt to take.");
  params.addParam<bool>("interpolate", true, "Whether or not to interpolate DT between times.  This is true by default for historical reasons.");

  return params;
}

TimePointsFromFile::TimePointsFromFile(const InputParameters & parameters) :
    TimeStepper(parameters),
    _file_name(getParam<std::string>("file_name")),
    _num_column(getParam<unsigned int>("number_of_columns")),
    _t_column_index(getParam<unsigned int>("t_data_column_index")),
    _dt_column_index(getParam<unsigned int>("dt_data_column_index")),
    _is_dt_from_file(getParam<bool>("is_dt_from_file")),
    _use_last_time_as_end_time(getParam<bool>("use_last_time_as_end_time")),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_occurred(false),
    _min_dt(getParam<Real>("min_dt")),
    _interpolate(getParam<bool>("interpolate"))
{  
  readTimeKnots();

  _time_knots = _time_t;
  _time_ipol.setData(_time_t, _time_dt);

  if (_use_last_time_as_end_time)
    _end_time = _time_t[_time_t.size()-1];
}

void
TimePointsFromFile::readTimeKnots()
{
  MooseUtils::checkFileReadable(_file_name);

  std::ifstream file_read;
  file_read.open(_file_name.c_str());

  Real t, dt, data;
  unsigned int column = 0;

  while (file_read >> data)
  {
    if (column == _t_column_index)
      _time_t.push_back(data);

    if (column == _dt_column_index && _is_dt_from_file)
      _time_dt.push_back(data);

    if (++column == _num_column)
      column = 0;
  }

  file_read.close();

  unsigned int nsize = _time_t.size();
  if (nsize == 0)
    mooseError("Invalid data provided in file. Try changing column index");

  if (!_is_dt_from_file)
  {
    _time_dt.resize(nsize);

    for (unsigned int i = 0; i < nsize-1; ++i)
      _time_dt[i] = _time_t[i+1]- _time_t[i];
    
    if (nsize > 1)
      _time_dt[nsize-1] = _time_dt[nsize-2];
    else
      mooseError("Cannot evaluate dt when size of time_t is 1: Provide in time file");
  }
  else if (_time_t.size() - 1 == _time_dt.size())
      _time_dt.push_back(_time_t[_time_t.size()-1]);
}

void
TimePointsFromFile::init()
{
}

void
TimePointsFromFile::removeOldKnots()
{
  while ((_time_knots.size() > 0) && (*_time_knots.begin() <= _time || std::abs(*_time_knots.begin() - _time) < 1e-10))
    _time_knots.erase(_time_knots.begin());
}

void
TimePointsFromFile::preExecute()
{
  TimeStepper::preExecute();
  removeOldKnots();
}

Real
TimePointsFromFile::computeInitialDT()
{
  return computeDT();
}

Real
TimePointsFromFile::computeDT()
{
  Real local_dt = 0;

  if (_interpolate)
    local_dt = _time_ipol.sample(_time);
  else // Find where we are
  {
    unsigned int i=0;
    for (; i < _time_t.size(); i++)
      if (MooseUtils::relativeFuzzyGreaterEqual(_time, _time_t[i]))
        break;

    // Use the last dt after the end
    if (i == _time_t.size())
      local_dt = _time_dt.back();
    else
      local_dt = _time_dt[i];
  }

  // sync to time knot
  if ((_time_knots.size() > 0) && (_time + local_dt >= (*_time_knots.begin())))
    local_dt = (*_time_knots.begin()) - _time;
  // honor minimal dt
  if (local_dt < _min_dt)
    local_dt = _min_dt;

  _cutback_occurred = false;

  return local_dt;
}

void
TimePointsFromFile::acceptStep()
{
  removeOldKnots();
}

void
TimePointsFromFile::rejectStep()
{
  _cutback_occurred = true;
  TimeStepper::rejectStep();
}
