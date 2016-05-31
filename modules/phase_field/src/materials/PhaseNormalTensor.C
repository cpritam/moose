/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PhaseNormalTensor.h"

template<>
InputParameters validParams<PhaseNormalTensor>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculate normal tensor of a phase based on gradient");
  params.addRequiredCoupledVar("phase", "Phase variable");
  params.addRequiredParam<MaterialPropertyName>("normal_tensor_name", "Name of normal tensor");
  params.addParam<Real>("val_tol", 1e-4, "Variable tolerance");
  params.addParam<Real>("grad_tol", 1e-8, "Gradient tolerance");
  params.addCoupledVar("dphase_x", "Phase derivative aux-variable along x-direction");
  params.addCoupledVar("dphase_y", "Phase derivative aux-variable along y-direction");
  params.addCoupledVar("dphase_z", "Phase derivative aux-variable along z-direction");
  return params;
}

PhaseNormalTensor::PhaseNormalTensor(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _u(coupledValue("phase")),
    _grad_u(coupledGradient("phase")),
    _dphase_x(isCoupled("dphase_x") ? coupledValue("dphase_x") : _zero),
    _dphase_y(isCoupled("dphase_y") ? coupledValue("dphase_y") : _zero),
    _dphase_z(isCoupled("dphase_z") ? coupledValue("dphase_z") : _zero),
    _normal_tensor(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("normal_tensor_name"))),
    _val_tol(getParam<Real>("val_tol")),
    _grad_tol(getParam<Real>("grad_tol"))
{
}

void
PhaseNormalTensor::initQpStatefulProperties()
{
  _normal_tensor[_qp].zero();
}

void
PhaseNormalTensor::computeQpProperties()
{
  const Real magnitude = _grad_u[_qp].norm();

  if (std::abs(_u[_qp]) > _val_tol && magnitude > _grad_tol)
  {
    RealVectorValue vector = _grad_u[_qp]/magnitude;
    _normal_tensor[_qp].vectorOuterProduct(vector, vector);
  }
  else if (std::abs(_u[_qp]) > 0.9 && magnitude < _grad_tol)
  {
    RealVectorValue vector;
    vector(0) = _dphase_x[_qp];
    vector(1) = _dphase_y[_qp];
    vector(2) = _dphase_z[_qp];
    _normal_tensor[_qp].vectorOuterProduct(vector, vector);
  }
  else
    _normal_tensor[_qp].zero();
}
