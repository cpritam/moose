/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECONSTMATERIALTENSOR_H
#define COMPUTECONSTMATERIALTENSOR_H

#include "Material.h"
#include "RankTwoTensor.h"

class ComputeConstMaterialTensor;

template<>
InputParameters validParams<ComputeConstMaterialTensor>();


class ComputeConstMaterialTensor : public Material
{
public:
  ComputeConstMaterialTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  RankTwoTensor _const_tensor;
  MaterialProperty<RankTwoTensor> & _const_property_tensor;
};

#endif
