#ifndef GBCONCENTRATIONIC_H
#define GBCONCENTRATIONIC_H

#include "InitialCondition.h"

class GBConcentrationIC;

template<>
InputParameters validParams<GBConcentrationIC>();

class GBConcentrationIC : public InitialCondition
{
public:
  GBConcentrationIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  void readGrainCenter();
  Real computeGBValue(const Point & p);

  std::string _file_name;
  Real _gb_thickness;
  unsigned int _mesh_dim;
  Real _gb_conc;
  Real _matrix_conc;
  unsigned int _num_grains;

  std::vector<Point> _grain_center;

  struct DistancePoint
  {
    Real val;
    unsigned int index;
  };

  // Sort the temp_centerpoints into order of magnitude                                                                                                                                             
  struct DistancePointComparator
  {
    bool operator () (const DistancePoint & a, const DistancePoint & b)
    {
      return a.val < b.val;
    }
  } _customLess;

  std::vector<DistancePoint> _gr_dist;
};

#endif
