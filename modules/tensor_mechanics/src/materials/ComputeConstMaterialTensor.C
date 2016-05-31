/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeConstMaterialTensor.h"

template<>
InputParameters validParams<ComputeConstMaterialTensor>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Computes constant material tensor");
  params.addRequiredParam<MaterialPropertyName>("const_tensor_name", "Name of constant tensor property");
  params.addRequiredParam<std::vector<Real> >("tensor_values", "Vector of values defining the constant tensor");
  return params;
}

ComputeConstMaterialTensor::ComputeConstMaterialTensor(const InputParameters & parameters) :
  Material(parameters),
  _const_property_tensor(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("const_tensor_name")))
{
  _const_tensor.fillFromInputVector(getParam<std::vector<Real> >("tensor_values"));
}

void
ComputeConstMaterialTensor::initQpStatefulProperties()
{
  _const_property_tensor[_qp] = _const_tensor;
}

void
ComputeConstMaterialTensor::computeQpProperties()
{
  _const_property_tensor[_qp] = _const_tensor;
}

