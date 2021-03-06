#include "RankFourAux.h"

template<>
InputParameters validParams<RankFourAux>()
{
  InputParameters params = validParams<AuxKernel>();

  //add stuff here
  params.addRequiredParam<std::string>("rank_four_tensor", "The rank four material tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>("index_i", "index_i >= 0 & index_i <= 2", "The index i of ijkl for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_j", "index_j >= 0 & index_j <= 2", "The index j of ijkl for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_k", "index_k >= 0 & index_k <= 2", "The index k of ijkl for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>("index_l", "index_l >= 0 & index_l <= 2", "The index l of ijkl for the tensor to output (0, 1, 2)");

  return params;
}

RankFourAux::RankFourAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _tensor(getMaterialProperty<ElasticityTensorR4>(getParam<std::string>("rank_four_tensor"))),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j")),
    _k(getParam<unsigned int>("index_k")),
    _l(getParam<unsigned int>("index_l"))
{
}

Real
RankFourAux::computeValue()
{
  return _tensor[_qp](_i, _j, _k, _l);
}
