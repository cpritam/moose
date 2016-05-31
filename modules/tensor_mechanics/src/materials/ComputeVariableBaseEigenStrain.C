/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVariableBaseEigenStrain.h"

template<>
InputParameters validParams<ComputeVariableBaseEigenStrain>()
{
  InputParameters params = validParams<ComputeStressFreeStrainBase>();
  params.addClassDescription("Computes Eigenstrain based on material property tensor base");
  params.addRequiredParam<MaterialPropertyName>("base_tensor_property_name", "Name of base tensor property");
  params.addParam<MaterialPropertyName>("prefactor", 1.0, "Name of material defining the variable dependence");
  params.addParam<MaterialPropertyName>("offset_tensor", "Name of offset tensor property");
  return params;
}

ComputeVariableBaseEigenStrain::ComputeVariableBaseEigenStrain(const InputParameters & parameters) :
    ComputeStressFreeStrainBase(parameters),
    _base_tensor(getMaterialProperty<RealTensorValue>("base_tensor_property_name")),
    _prefactor(getMaterialProperty<Real>("prefactor")),
    _has_offset_tensor(isParamValid("offset_tensor"))
{
  if (_has_offset_tensor)
    _offset_tensor = &getMaterialProperty<RankTwoTensor>("offset_tensor");
}

void
ComputeVariableBaseEigenStrain::computeQpStressFreeStrain()
{
  RankTwoTensor base_rank_two_tensor = _base_tensor[_qp];
  _stress_free_strain[_qp] = base_rank_two_tensor * _prefactor[_qp];

  if (_has_offset_tensor)
    _stress_free_strain[_qp] += (*_offset_tensor)[_qp];
}

