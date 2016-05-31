/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVACANCYRELAXSTRAIN_H
#define COMPUTEVACANCYRELAXSTRAIN_H

#include "Material.h"
#include "RankTwoTensor.h"

class ComputeVacancyRelaxStrain;

template<>
InputParameters validParams<ComputeVacancyRelaxStrain>();


class ComputeVacancyRelaxStrain : public Material
{
public:
  ComputeVacancyRelaxStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  MaterialProperty<RankTwoTensor> & _cv_relax_strain;
  const MaterialProperty<RealTensorValue> & _ref_tensor;
  Real _prefactor;

};

#endif
