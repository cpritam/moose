/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PHASENORMALTENSOR_H
#define PHASENORMALTENSOR_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"

class PhaseNormalTensor;

template<>
InputParameters validParams<PhaseNormalTensor>();

/**
 * Calculate phase normal tensor based on gradient
 */
class PhaseNormalTensor : public DerivativeMaterialInterface<Material>
{
public:
  PhaseNormalTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  const VariableValue & _u;
  const VariableGradient & _grad_u;
  const VariableValue & _dphase_x;
  const VariableValue & _dphase_y;
  const VariableValue & _dphase_z;

  MaterialProperty<RankTwoTensor> & _normal_tensor;

  Real _val_tol;
  Real _grad_tol;
};

#endif
