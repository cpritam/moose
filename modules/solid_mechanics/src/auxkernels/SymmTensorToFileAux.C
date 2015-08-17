#include "SymmTensorToFileAux.h"

template<>
InputParameters validParams<SymmTensorToFileAux>()
{
  InputParameters params = validParams<VariableToFileAux>();
  params.addRequiredParam<MaterialPropertyName>("tensor", "The material tensor name.");
  return params;
}

SymmTensorToFileAux::SymmTensorToFileAux(const InputParameters & parameters) :
    VariableToFileAux(parameters),
    _tensor(getMaterialProperty<SymmTensor>("tensor"))
{
}

Real
SymmTensorToFileAux::computeValue()
{
  bool print_flag = false;
  Real min = 0.0;
  Real max = 0.0;
  Real r = 0.0;

  switch (_region_type)
  {
    case 0:

      if (_q_point[_qp](0) >= _xmin && _q_point[_qp](0) <= _xmax && _q_point[_qp](1) >= _ymin && _q_point[_qp](1) <= _ymax && _q_point[_qp](2) >= _zmin && _q_point[_qp](2) <= _zmax)
	print_flag = true;

      break;

    case 1:

      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
	if (i != _axial_dir)
	  r += std::pow(_q_point[_qp](i) - _center(i), 2.0);
      r = std::pow(r, 0.5);
      
      if (_axial_dir == 0)
      {
	min = _xmin;
	max = _xmax;
      }
      else if(_axial_dir == 1)
      {
	min = _ymin;
	max = _ymax;
      }
      else
      {
	min = _zmin;
	max = _zmax;
      }

      if (r >= _rmin && r <= _rmax && _q_point[_qp](_axial_dir) >= min && _q_point[_qp](_axial_dir) <= max)
	print_flag = true;

      break;

    default:

      if (_q_point[_qp](0) >= _xmin && _q_point[_qp](0) <= _xmax && _q_point[_qp](1) >= _ymin && _q_point[_qp](1) <= _ymax && _q_point[_qp](2) >= _zmin && _q_point[_qp](2) <= _zmax)
	print_flag = true;
  }
  
  if (print_flag)
    _file_write << _t_step << ' ' << _q_point[_qp](0) << ' ' << _q_point[_qp](1) << ' ' << _q_point[_qp](2) << ' ' << _tensor[_qp].xx() << ' ' << _tensor[_qp].yy() << ' ' << _tensor[_qp].zz() << ' ' << _tensor[_qp].xy() << ' ' << _tensor[_qp].yz() << ' ' << _tensor[_qp].xz() << std::endl;

  return 0;
}
