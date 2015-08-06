/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef MULTIAXIALROTATIONANDTRANSLATION_H
#define MULTIAXIALROTATIONANDTRANSLATION_H

#include "PresetNodalBC.h"
#include "InputRotOrTransUserObject.h"

//Forward Declarations
class MultiAxialRotationAndTranslation;

template<>
InputParameters validParams<MultiAxialRotationAndTranslation>();

class MultiAxialRotationAndTranslation : public PresetNodalBC
{
public:
  MultiAxialRotationAndTranslation(const InputParameters & parameters);

protected:
  virtual Real computeQpValue();
  virtual void initialSetup();
  ColumnMajorMatrix rotateAroundAxis(const Real angle);
  ColumnMajorMatrix calculateTransformationMatrices(Point, Point);

  unsigned int _num_uos;
  Real _scale_translation;
  Real _scale_rotation;
  unsigned int _component;
  std::vector<const InputRotOrTransUserObject *> _rot_or_trans_uos;
  std::vector<ColumnMajorMatrix> _transformation_matrix;
  std::vector<ColumnMajorMatrix> _transformation_matrix_inv;
};

#endif //MULTIAXIALROTATIONANDTRANSLATION
