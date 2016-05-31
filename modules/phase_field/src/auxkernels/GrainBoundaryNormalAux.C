#include "GrainBoundaryNormalAux.h"

template<>
InputParameters validParams<GrainBoundaryNormalAux>()
{
  InputParameters params = validParams<GrainBoundaryAux>();
  params.addRequiredParam<unsigned int>("component", "Component of normal");

  return params;
}

GrainBoundaryNormalAux::GrainBoundaryNormalAux(const InputParameters & parameters) :
  GrainBoundaryAux(parameters),
  _component(getParam<unsigned int>("component"))
{
}

Real
GrainBoundaryNormalAux::computeValue()
{
  _is_nodal = false;
  Real gb_val = GrainBoundaryAux::computeValue();

  if (gb_val > 0.9)
  {
    Point grain2grain_center = _grain_center[_gr_dist[1].index] - _grain_center[_gr_dist[0].index];
    Point norm_vector = grain2grain_center/grain2grain_center.size();
    return norm_vector(_component);
  }
  else
    return 0.0;
}
