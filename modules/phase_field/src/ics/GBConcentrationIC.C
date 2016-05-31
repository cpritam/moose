#include "GBConcentrationIC.h"

template<>
InputParameters validParams<GBConcentrationIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addParam<std::string>("file_name", "Name of file with grain center");
  params.addRequiredParam<Real>("gb_thickness", "Grain boundary thickness");
  params.addParam<unsigned int>("mesh_dimension", 2, "Mesh Dimension");
  params.addRequiredParam<Real>("gb_conc", "Concentration in GB");
  params.addRequiredParam<Real>("matrix_conc", "Concentration in matrix");
  return params;
}

GBConcentrationIC::GBConcentrationIC(const InputParameters & parameters) :
  InitialCondition(parameters),
  _file_name(getParam<std::string>("file_name")),
  _gb_thickness(getParam<Real>("gb_thickness")),
  _mesh_dim(getParam<unsigned int>("mesh_dimension")),
  _gb_conc(getParam<Real>("gb_conc")),
  _matrix_conc(getParam<Real>("matrix_conc")),
  _num_grains(0)
{
  readGrainCenter();
}

Real
GBConcentrationIC::value(const Point & p)
{
  Real gb_val = computeGBValue(p);
  return (1.0 -  gb_val) * _matrix_conc + gb_val * _gb_conc;
}


void
GBConcentrationIC::readGrainCenter()
{
  MooseUtils::checkFileReadable(_file_name);
  
  std::ifstream file;
  file.open(_file_name.c_str());

  Real coord;
  unsigned int index = 0;
  
  while (file >> coord)
  {
    if (index == 0)
    {
      _num_grains++;
      _grain_center.resize(_num_grains);
    }
   
    _grain_center[_num_grains - 1](index) = coord;
    index++;
    
    if (index == _mesh_dim)
      index = 0;
  }

  file.close();

  if (index > 0)
    mooseError("Incomplete data in file");

  if (_num_grains < 2)
    mooseError("Atleast 2 center points need to be specified");

  _gr_dist.resize(_num_grains);
}

Real
GBConcentrationIC::computeGBValue(const Point & coord)
{
  for (unsigned int i = 0; i < _num_grains; ++i)
  {
    _gr_dist[i].val = (coord - _grain_center[i]).norm();
    _gr_dist[i].index = i;
  }
  
  std::sort(_gr_dist.begin(), _gr_dist.end(), _customLess);

  Point grain2grain = _grain_center[_gr_dist[1].index] - _grain_center[_gr_dist[0].index];
  Point node2grain = coord - _grain_center[_gr_dist[0].index];

  Real grain2grain_dis = grain2grain.norm();
  grain2grain /= grain2grain_dis;

  Real node2grain_dis = std::abs(node2grain.contract(grain2grain));

  Real rel_dis = grain2grain_dis/2.0 - node2grain_dis;

  return rel_dis < _gb_thickness/2.0 ? 1.0 : std::exp(-2.0 * rel_dis/_gb_thickness);
}
