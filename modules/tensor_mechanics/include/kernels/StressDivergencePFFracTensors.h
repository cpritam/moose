#ifndef STRESSDIVERGENCEPFFRACTENSORS_H
#define STRESSDIVERGENCEPFFRACTENSORS_H

#include "StressDivergenceTensors.h"

class StressDivergencePFFracTensors;

template<>
InputParameters validParams<StressDivergencePFFracTensors>();

class StressDivergencePFFracTensors : public StressDivergenceTensors
{
public:
  StressDivergencePFFracTensors(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MaterialProperty<RankTwoTensor> * _d_stress_dc;
  std::string _pff_jac_prop_name;

  const bool _c_coupled;
  const unsigned int _c_var;

private:

};

#endif //STRESSDIVERGENCEPFFRACTENSORS_H
