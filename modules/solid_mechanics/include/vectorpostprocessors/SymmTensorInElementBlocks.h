/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SYMMTENSORINELEMENTBLOCKS_H
#define SYMMTENSORINELEMENTBLOCKS_H

#include "ElementVectorPostprocessor.h"
#include "SymmTensor.h"

//Forward Declarations
class SymmTensorInElementBlocks;

template<>
InputParameters validParams<SymmTensorInElementBlocks>();

/**
 * Get all of the elements that are intersected by a line
 */
class SymmTensorInElementBlocks : public ElementVectorPostprocessor
{
public:
  SymmTensorInElementBlocks(const InputParameters & parameters);

  virtual ~SymmTensorInElementBlocks() {}

  /**
   * Clear it out.
   */
  virtual void initialize();

  /**
   * Find the elements
   */
  virtual void execute();

  virtual void finalize();

  virtual void threadJoin(const UserObject & y);

protected:
  /// The elements that intersect the line
  VectorPostprocessorValue & _elem_vars;
  const MaterialProperty<SymmTensor> & _tensor;
  Real _radial_min;
  Real _radial_max;
  Real _axial_min;
  Real _axial_max;
  unsigned int _axial_dir;
  Point _center;
  bool _var_based_domain;
  Real _var_cutoff_value;
  const bool _var_coupled;
  const VariableValue * _var;
  std::vector<Real> _local_elem_data;
  std::vector<Real> _global_elem_data;
  unsigned int _x_dir;
  unsigned int _y_dir;
  
  virtual bool pointInDomain(Point p);
};

#endif
