/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVacancyRelaxStrain.h"

template<>
InputParameters validParams<ComputeVacancyRelaxStrain>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Computes strain due to vacancy");
  params.addRequiredParam<MaterialPropertyName>("vacancy_strain_tensor", "Name of vacancy strain tensor property");
  params.addRequiredParam<MaterialPropertyName>("reference_tensor", "Name of reference tensor property");
  params.addParam<Real>("prefactor", 1.0, "Scalar prefactor");
  return params;
}

ComputeVacancyRelaxStrain::ComputeVacancyRelaxStrain(const InputParameters & parameters) :
  Material(parameters),
  _cv_relax_strain(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("vacancy_strain_tensor"))),
  _ref_tensor(getMaterialProperty<RealTensorValue>("reference_tensor")),
  _prefactor(getParam<Real>("prefactor"))
{
}

void
ComputeVacancyRelaxStrain::initQpStatefulProperties()
{
  _cv_relax_strain[_qp] = _prefactor * _ref_tensor[_qp];
  //  _cv_relax_strain[_qp].zero();
}

void
ComputeVacancyRelaxStrain::computeQpProperties()
{
  _cv_relax_strain[_qp] = _prefactor * _ref_tensor[_qp];
}

