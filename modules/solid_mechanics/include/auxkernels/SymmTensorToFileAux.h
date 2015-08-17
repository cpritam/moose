#ifndef SYMMTENSORTOFILEAUX_H
#define SYMMTENSORTOFILEAUX_H

#include "VariableToFileAux.h"
#include "SymmTensor.h"

class SymmTensorToFileAux;

template<>
InputParameters validParams<SymmTensorToFileAux>();

class SymmTensorToFileAux : public VariableToFileAux
{
public:
  SymmTensorToFileAux(const InputParameters & parameters);
  virtual ~SymmTensorToFileAux() {}

protected:
  Real computeValue();
  const MaterialProperty<SymmTensor> & _tensor;
};

#endif //SYMMTENSORTOFILEAUX_H
