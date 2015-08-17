#ifndef VARIABLETOFILEAUX_H
#define VARIABLETOFILEAUX_H

#include "AuxKernel.h"

class VariableToFileAux;

template<>
InputParameters validParams<VariableToFileAux>();

class VariableToFileAux : public AuxKernel
{
public:
  VariableToFileAux(const InputParameters & parameters);
  virtual ~VariableToFileAux();

protected:
  std::string _out_file_name;
  MooseEnum _region_type;
  Real _xmin;
  Real _xmax;
  Real _ymin;
  Real _ymax; 
  Real _zmin;
  Real _zmax;
  Real _rmin;
  Real _rmax;
  Point _center;
  unsigned int _axial_dir;

  Real computeValue();
  std::ofstream _file_write;
};

#endif //VARIABLETOFILEAUX_H
