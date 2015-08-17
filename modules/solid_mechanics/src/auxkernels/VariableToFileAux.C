#include "VariableToFileAux.h"

template<>
InputParameters validParams<VariableToFileAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("out_file_name", "File name to output stress data");
  MooseEnum type("box cylinder", "box");
  params.addParam<MooseEnum>("region_type", type, "Box or cylindrical region demarcated for output");
  params.addParam<Real>("xmin", 0.0, "Minimum x coordinate value - define domain");
  params.addParam<Real>("xmax", 0.0, "Maximum x coordinate value - define domain");
  params.addParam<Real>("ymin", 0.0, "Minimum y coordinate value - define domain");
  params.addParam<Real>("ymax", 0.0, "Maximum y coordinate value - define domain");
  params.addParam<Real>("zmin", 0.0, "Minimum z coordinate value - define domain");
  params.addParam<Real>("zmax", 0.0, "Maximum z coordinate value - define domain");
  params.addParam<Real>("rmin", 0.0, "Minium radius");
  params.addParam<Real>("rmax", 0.0, "Maximum radius");
  params.addParam<Point>("center", 0, "Reference center point"); 
  params.addParam<unsigned int>("axial_dir", 0, "Axial direction in case of cylindrical region");

  return params;
}

VariableToFileAux::VariableToFileAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _out_file_name(getParam<std::string>("out_file_name")),
    _region_type(getParam<MooseEnum>("region_type")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax")),
    _rmin(getParam<Real>("rmin")),
    _rmax(getParam<Real>("rmax")),
    _center(getParam<Point>("center")),
    _axial_dir(getParam<unsigned int>("axial_dir"))
{
  _file_write.open(_out_file_name.c_str(), std::ofstream::out);
}

VariableToFileAux::~VariableToFileAux()
{
  _file_write.close();
}    

Real
VariableToFileAux::computeValue()
{
  return 0.0;
}
