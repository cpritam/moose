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
  params.addParam<Real>("grad_magnitude_tol", 1e-8, "Tolerance to check magnitude of gradient");
  return params;
}

PhaseNormalTensor::PhaseNormalTensor(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _u(coupledValue("phase")),
    _grad_u(coupledGradient("phase")),
    _normal_tensor(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("normal_tensor_name"))),
    _val_tol(getParam<Real>("val_tol")),
    _grad_magnitude_tol(getParam<Real>("grad_magnitude_tol"))
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

  if (std::abs(_u[_qp]) > _val_tol)
  {
    if (magnitude < _grad_magnitude_tol)
      mooseError("Small gradient magnitude: Modify mesh density and/or element type");
      
    RealVectorValue vector = _grad_u[_qp]/magnitude;
    _normal_tensor[_qp].vectorOuterProduct(vector, vector);
  }
  else
    _normal_tensor[_qp].zero();
}
