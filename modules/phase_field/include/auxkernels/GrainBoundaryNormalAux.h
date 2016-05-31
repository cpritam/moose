#ifndef GRAINBOUNDARYNORMALAUX_H
#define GRAINBOUNDARYNORMALAUX_H

#include "GrainBoundaryAux.h"

class GrainBoundaryNormalAux;

template<>
InputParameters validParams<GrainBoundaryNormalAux>();

class GrainBoundaryNormalAux : public GrainBoundaryAux
{
public:
  GrainBoundaryNormalAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  unsigned int _component;
};

#endif
