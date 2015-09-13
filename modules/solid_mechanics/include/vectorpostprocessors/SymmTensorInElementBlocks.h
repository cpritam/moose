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

#include "GeneralVectorPostprocessor.h"

//Forward Declarations
class SymmTensorInElementBlocks;

template<>
InputParameters validParams<SymmTensorInElementBlocks>();

/**
 * Get all of the elements that are intersected by a line
 */
class SymmTensorInElementBlocks : public GeneralVectorPostprocessor
{
public:
  SymmTensorInElementBlocks(const InputParameters & parameters);

  virtual ~SymmTensorInElementBlocks() {}

  /**
   * Clear it out.
   */
  virtual void initialize();

  /**
   * Find th elements
   */
  virtual void execute();

protected:
  /// The elements that intersect the line
  VectorPostprocessorValue & _elem_vars;
  MooseMesh & _mesh;
};

#endif
