#ifndef GRAINBOUNDARYAUX_H
#define GRAINBOUNDARYAUX_H

#include "AuxKernel.h"

class GrainBoundaryAux;

template<>
InputParameters validParams<GrainBoundaryAux>();

class GrainBoundaryAux : public AuxKernel
{
public:
  GrainBoundaryAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  void readGrainCenter();

  std::string _file_name;
  Real _gb_thickness;
  unsigned int _mesh_dim;
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
