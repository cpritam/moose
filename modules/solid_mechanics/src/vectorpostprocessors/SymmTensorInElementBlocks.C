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

#include "SymmTensorInElementBlocks.h"

template<>
InputParameters validParams<SymmTensorInElementBlocks>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  return params;
}

SymmTensorInElementBlocks::SymmTensorInElementBlocks(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    _elem_vars(declareVector("elem_vars")),
    _mesh(_fe_problem.mesh())
{
}

void
SymmTensorInElementBlocks::initialize()
{
  _elem_vars.clear();
}

void
SymmTensorInElementBlocks::execute()
{
  MeshBase::const_element_iterator end_el = _mesh.getMesh().active_local_elements_end();
  //  unsigned int num_elems = _mesh.nElem();
  //  _elem_vars.resize(num_elems);
  
  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != end_el; ++el)
  {
    const Elem * elem = *el;
    _elem_vars.push_back(elem->id());
  }

  /*std::vector<Elem *> elems;

  for (unsigned int i=0; i<num_elems; ++i)
  _elem_vars[i] = intersected_elems[i]->id();*/
}

