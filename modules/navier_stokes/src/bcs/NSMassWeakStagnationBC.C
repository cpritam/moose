#include "NSMassWeakStagnationBC.h"

template<>
InputParameters validParams<NSMassWeakStagnationBC>()
{
  InputParameters params = validParams<NSWeakStagnationBC>();

  return params;
}



NSMassWeakStagnationBC::NSMassWeakStagnationBC(const std::string & name, InputParameters parameters)
    : NSWeakStagnationBC(name, parameters)
{
}




Real NSMassWeakStagnationBC::computeQpResidual()
{
  // rho_s * |u| * (s.n) * phi_i
  return this->rho_static() * std::sqrt(this->velmag2()) * this->sdotn() * _test[_i][_qp];
}




Real NSMassWeakStagnationBC::computeQpJacobian()
{
  // TODO
  return 0.;
}




Real NSMassWeakStagnationBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.;
}




