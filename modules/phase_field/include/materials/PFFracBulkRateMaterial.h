#ifndef PFFRACBULKRATEMATERIAL_H
#define PFFRACBULKRATEMATERIAL_H

#include "Material.h"
#include "Function.h"

/**
 * Phase-field fracture
 * This class obtains critical energy release rate (gc) value
 * Used by PFFRacBulkRate
 */

class PFFracBulkRateMaterial;

template<>
InputParameters validParams<PFFracBulkRateMaterial>();

class PFFracBulkRateMaterial : public Material
{
public:
  PFFracBulkRateMaterial(const std::string & name,
                        InputParameters parameters);

protected:
  virtual void initQpStatefulProperties(){}
  virtual void computeQpProperties();
  /**
   * This function obtains the value of gc
   * Must be overidden by the user for heterogeneous gc
   */
  virtual void getProp(){}

  ///Input parameter for homogeneous gc
  Real _gc;

  ///Material property where the values are stored
  MaterialProperty<Real> &_gc_prop;
  MaterialProperty<Real> &_gc_prop_old;

private:

};

#endif //PFFRACBULKRATEMATERIAL_H
