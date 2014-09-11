#include "LinearIsoElasticPFDamage.h"

template<>
InputParameters validParams<LinearIsoElasticPFDamage>()
{
  InputParameters params = validParams<LinearElasticMaterial>();
  params.addRequiredCoupledVar("c","Concentration");
  params.addParam<Real>("kdamage",1e-6,"Stiffness of damaged matrix");
  return params;
}

LinearIsoElasticPFDamage::LinearIsoElasticPFDamage(const std::string & name,
                                             InputParameters parameters) :
  LinearElasticMaterial(name, parameters),
  _c(coupledValue("c")),
  _kdamage(getParam<Real>("kdamage")),
  _dstress_dc(declareProperty<RankTwoTensor>("dstress_dc")),
  _G0_pos(declareProperty<Real>("G0_pos")),
  _dG0_pos_dstrain(declareProperty<RankTwoTensor>("dG0_pos_dstrain"))
{
  _etens.resize(LIBMESH_DIM);
  _epos.resize(LIBMESH_DIM);
}

void LinearIsoElasticPFDamage::computeQpStress()
{
  _stress[_qp].zero();
  updateVar();
}

void
LinearIsoElasticPFDamage::computeQpElasticityTensor()
{
  _elasticity_tensor[_qp] = _Cijkl;
  
  getScalingFactor();
  _scaling += _kdamage;

  _elasticity_tensor[_qp] *= _scaling;
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}

void
LinearIsoElasticPFDamage::getScalingFactor()
{
  _scaling = 1.0;
}

void
LinearIsoElasticPFDamage::updateVar()
{
  RankTwoTensor stress0pos,stress0neg,stress0;

  Real lambda = _elasticity_tensor[_qp](0,0,1,1);
  Real mu = _elasticity_tensor[_qp](0,1,0,1);

  std::vector<Real> w;
  RankTwoTensor evec;

  _elastic_strain[_qp].symmetricEigenvaluesEigenvectors(w,evec);

  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
    for ( unsigned int j = 0; j < LIBMESH_DIM; j++ )
      for ( unsigned int k = 0; k < LIBMESH_DIM; k++ )
        _etens[i](j,k) = evec(j,i) * evec(k,i);

  Real etr=0.0;
  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
    etr += w[i];

  Real etrpos=(std::abs(etr)+etr)/2.0;
  Real etrneg=(std::abs(etr)-etr)/2.0;

  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
  {
    stress0pos += _etens[i] * ( lambda * etrpos + 2.0 * mu * (std::abs(w[i]) + w[i])/2.0);
    stress0neg += _etens[i] * ( lambda * etrneg + 2.0 * mu * (std::abs(w[i]) - w[i])/2.0);
  }

  Real c = _c[_qp];
  Real xfac = std::pow(1.0-c,2.0) + _kdamage;

  _stress[_qp] = stress0pos * xfac - stress0neg;

  for ( unsigned int i = 0; i < LIBMESH_DIM; i++ )
    _epos[i] = (std::abs(w[i]) + w[i])/2.0;

  Real val = 0.0;
  for (unsigned int i = 0; i < LIBMESH_DIM; i++ )
    val += std::pow(_epos[i],2.0);
  val *= mu;

  _G0_pos[_qp] = lambda * std::pow(etrpos,2.0)/2.0 + val;

  _dG0_pos_dstrain[_qp] = stress0pos;
  _dstress_dc[_qp] = -stress0pos * (2 * (1.0-c));
}
