/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SymmTensorInElementBlocks.h"

template<>
InputParameters validParams<SymmTensorInElementBlocks>()
{
  InputParameters params = validParams<ElementVectorPostprocessor>();
  params.addParam<Real>("radial_min", 0.0, "Minimum radial distance");
  params.addParam<Real>("radial_max", 1e6, "Maximum radial distance");
  params.addParam<Real>("axial_min", -1e6, "Minimum axial distance");
  params.addParam<Real>("axial_max", 1e6, "Maximum axial distance");
  params.addParam<unsigned int>("axial_dir", 2, "Axial direction");
  params.addParam<Point>("center", 0, "Center Point");

  return params;
}

SymmTensorInElementBlocks::SymmTensorInElementBlocks(const InputParameters & parameters) :
    ElementVectorPostprocessor(parameters),
    _elem_vars(declareVector("elem_vars")),
    _tensor(getMaterialProperty<SymmTensor>("stress")),
    _radial_min(getParam<Real>("radial_min")),
    _radial_max(getParam<Real>("radial_max")),
    _axial_min(getParam<Real>("axial_min")),
    _axial_max(getParam<Real>("axial_max")),
    _axial_dir(getParam<unsigned int>("axial_dir")),
    _center(getParam<Point>("center")),
    _local_elem_data(0),
    _global_elem_data(0),
    _x_dir(0),
    _y_dir(1)
{
  if (_axial_dir == 0)
  {
    _x_dir = 1;
    _y_dir = 2;
  }

  if (_axial_dir == 1)
  {
    _x_dir = 0;
    _y_dir = 2;
  }
}

void
SymmTensorInElementBlocks::initialize()
{
  _local_elem_data.clear();
  _global_elem_data.clear();
  _elem_vars.clear();
}

void
SymmTensorInElementBlocks::execute()
{  
  unsigned int elem_id = _current_elem->id();
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    if (pointInDomain(_q_point[qp]))
    {
      _local_elem_data.push_back(1.0 * elem_id);
      _local_elem_data.push_back(1.0 * qp);
      _local_elem_data.push_back(_q_point[qp](0));
      _local_elem_data.push_back(_q_point[qp](1));
      _local_elem_data.push_back(_q_point[qp](2));
      _local_elem_data.push_back(_tensor[qp].xx());
      _local_elem_data.push_back(_tensor[qp].yy());
      _local_elem_data.push_back(_tensor[qp].zz());
      _local_elem_data.push_back(_tensor[qp].xy());
      _local_elem_data.push_back(_tensor[qp].yz());
      _local_elem_data.push_back(_tensor[qp].zx());
    }
  }
}

void
SymmTensorInElementBlocks::finalize()
{
  _global_elem_data.insert(_global_elem_data.end(),_local_elem_data.begin(), _local_elem_data.end());
  
  std::vector<Real> comm(_global_elem_data.size());

  for (unsigned int i = 0; i < _global_elem_data.size(); ++i)
    comm[i] = _global_elem_data[i];

  _communicator.allgather(comm);

  _elem_vars.resize(comm.size());
  
  for (unsigned int i = 0; i < comm.size(); ++i)
    _elem_vars[i] = comm[i];
}

void
SymmTensorInElementBlocks::threadJoin(const UserObject & y)
{
  const SymmTensorInElementBlocks & elem_data = static_cast<const SymmTensorInElementBlocks &>(y);
  _global_elem_data.insert(_global_elem_data.end(),elem_data._local_elem_data.begin(), elem_data._local_elem_data.end());
}

bool
SymmTensorInElementBlocks::pointInDomain(Point p)
{
  Real rdis = 0;
  rdis += std::pow(p(_x_dir)-_center(_x_dir), 2.0);
  rdis += std::pow(p(_y_dir)-_center(_y_dir), 2.0);

  rdis = std::pow(rdis, 0.5);
  Real zdis = p(_axial_dir)-_center(_axial_dir);
  
  if (rdis > _radial_max || rdis < _radial_min || zdis > _axial_max || zdis < _axial_min)
    return false;

  return true;
}
