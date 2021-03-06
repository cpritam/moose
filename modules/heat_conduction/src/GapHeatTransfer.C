#include "GapHeatTransfer.h"

#include "GapConductance.h"
#include "PenetrationLocator.h"
#include "SystemBase.h"

// libmesh
#include "libmesh/string_to_enum.h"

Threads::spin_mutex slave_flux_mutex;

template<>
InputParameters validParams<GapHeatTransfer>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<IntegratedBC>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");

  // Common
  params.addParam<Real>("min_gap", 1.0e-6, "A minimum gap size");
  params.addParam<Real>("max_gap", 1.0e6, "A maximum gap size");

  // Quadrature based
  params.addParam<bool>("quadrature", false, "Whether or not to do Quadrature point based gap heat transfer.  If this is true then gap_distance and gap_temp should NOT be provided (and will be ignored) however paired_boundary IS then required.");
  params.addParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");

  // Node based options
  params.addCoupledVar("gap_distance", "Distance across the gap");
  params.addCoupledVar("gap_temp", "Temperature on the other side of the gap");

  return params;
}

GapHeatTransfer::GapHeatTransfer(const std::string & name, InputParameters parameters)
   :IntegratedBC(name, parameters),
   _quadrature(getParam<bool>("quadrature")),
   _slave_flux(!_quadrature ? &_sys.getVector("slave_flux") : NULL),
   _gap_conductance(getMaterialProperty<Real>("gap_conductance"+getParam<std::string>("appended_property_name"))),
   _gap_conductance_dT(getMaterialProperty<Real>("gap_conductance"+getParam<std::string>("appended_property_name")+"_dT")),
   _min_gap(getParam<Real>("min_gap")),
   _max_gap(getParam<Real>("max_gap")),
   _gap_temp(0),
   _gap_distance(88888),
   _edge_multiplier(1.0),
   _has_info(false),
   _xdisp_coupled(isCoupled("disp_x")),
   _ydisp_coupled(isCoupled("disp_y")),
   _zdisp_coupled(isCoupled("disp_z")),
   _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
   _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0),
   _gap_distance_value(_quadrature ? _zero : coupledValue("gap_distance")),
   _gap_temp_value(_quadrature ? _zero : coupledValue("gap_temp")),
   _penetration_locator(!_quadrature ? NULL : &getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"),
                                                                               getParam<std::vector<BoundaryName> >("boundary")[0],
                                                                               Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
   _warnings(getParam<bool>("warnings"))
{
  if (_quadrature)
  {
    if (!parameters.isParamValid("paired_boundary"))
      mooseError(std::string("No 'paired_boundary' provided for ") + _name);
  }
  else
  {
    if (!isCoupled("gap_distance"))
      mooseError(std::string("No 'gap_distance' provided for ") + _name);

    if (!isCoupled("gap_temp"))
      mooseError(std::string("No 'gap_temp' provided for ") + _name);
  }
}


Real
GapHeatTransfer::computeQpResidual()
{
  computeGapValues();

  if (!_has_info)
    return 0;

  Real grad_t = (_u[_qp] - _gap_temp) * _edge_multiplier * _gap_conductance[_qp];

  // This is keeping track of this residual contribution so it can be used as the flux on the other side of the gap.
  if (!_quadrature)
  {
    Threads::spin_mutex::scoped_lock lock(slave_flux_mutex);
    const Real slave_flux = computeSlaveFluxContribution(grad_t);
    _slave_flux->add(_var.dofIndices()[_i], slave_flux);
  }

  return _test[_i][_qp]*grad_t;
}

Real
GapHeatTransfer::computeSlaveFluxContribution( Real grad_t )
{
  return _coord[_qp] * _JxW[_qp] * _test[_i][_qp] * grad_t;
}

Real
GapHeatTransfer::computeQpJacobian()
{
  computeGapValues();

  if (!_has_info)
    return 0;

  return _test[_i][_qp] * ((_u[_qp] - _gap_temp) * _edge_multiplier * _gap_conductance_dT[_qp] + _edge_multiplier * _gap_conductance[_qp]) * _phi[_j][_qp];
}

Real
GapHeatTransfer::computeQpOffDiagJacobian( unsigned jvar )
{
  computeGapValues();

  if (!_has_info)
    return 0;

  unsigned coupled_component(0);
  bool active(false);
  if ( _xdisp_coupled && jvar == _xdisp_var )
  {
    coupled_component = 0;
    active = true;
  }
  else if ( _ydisp_coupled && jvar == _ydisp_var )
  {
    coupled_component = 1;
    active = true;
  }
  else if ( _zdisp_coupled && jvar == _zdisp_var )
  {
    coupled_component = 2;
    active = true;
  }

  Real dRdx(0);
  if ( active )
  {
    // Compute dR/du_[xyz]
    // Residual is based on
    //   h_gap = h_conduction() + h_contact() + h_radiation();
    //   grad_t = (_u[_qp] - _gap_temp) * h_gap;
    // So we need
    //   (_u[_qp] - _gap_temp) * (dh_gap/du_[xyz]);
    // Assuming dh_contact/du_[xyz] = dh_radiation/du_[xyz] = 0,
    //   we need dh_conduction/du_[xyz]
    // Given
    //   h_conduction = gapK / gapLength, then
    //   dh_conduction/du_[xyz] = -gapK/gapLength^2 * dgapLength/du_[xyz]
    // Given
    //   gapLength = ((u_x-m_x)^2+(u_y-m_y)^2+(u_z-m_z)^2)^1/2
    // where m_[xyz] is the master coordinate, then
    //   dGapLength/du_[xyz] = 1/2*((u_x-m_x)^2+(u_y-m_y)^2+(u_z-m_z)^2)^(-1/2)*2*(u_[xyz]-m_[xyz])
    //                       = (u_[xyz]-m_[xyz])/gapLength
    // This is the normal vector.

    const Real gapL = gapLength();

    // THIS IS NOT THE NORMAL WE NEED.
    // WE NEED THE NORMAL FROM THE CONSTRAINT, THE NORMAL FROM THE
    // MASTER SURFACE.  HOWEVER, THIS IS TRICKY SINCE THE NORMAL
    // FROM THE MASTER SURFACE WAS COMPUTED FOR A POINT ASSOCIATED
    // WITH A SLAVE NODE.  NOW WE ARE AT A SLAVE INTEGRATION POINT.
    //
    // HOW DO WE GET THE NORMAL WE NEED?
    //
    // Until we have the normal we need,
    //   we'll hope that the one we have is close to the negative of the one we need.
    const Point & normal( _normals[_qp] );

    const Real dgap = dgapLength( -normal(coupled_component) );
    dRdx = -(_u[_qp]-_gap_temp)*_edge_multiplier*_gap_conductance[_qp]/gapL * dgap;
  }
  return _test[_i][_qp] * dRdx * _phi[_j][_qp];
}


Real
GapHeatTransfer::gapLength() const
{
  if (!_has_info)
    return 1.0;

  return GapConductance::gapLength( -_gap_distance, _min_gap, _max_gap );
}

Real
GapHeatTransfer::dgapLength( Real normalComponent ) const
{
  const Real gap_L = gapLength();

  Real dgap(0);

  if ( _min_gap <= gap_L && gap_L <= _max_gap)
  {
    dgap = normalComponent;
  }

  return dgap;
}

void
GapHeatTransfer::computeGapValues()
{
  if (!_quadrature)
  {
    _has_info = true;
    _gap_temp = _gap_temp_value[_qp];
    _gap_distance = _gap_distance_value[_qp];
  }
  else
  {
    Node * qnode = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);
    PenetrationInfo * pinfo = _penetration_locator->_penetration_info[qnode->id()];

    _gap_temp = 0.0;
    _gap_distance = 88888;
    _has_info = false;
    _edge_multiplier = 1.0;

    if (pinfo)
    {
      _gap_distance = pinfo->_distance;
      _has_info = true;

      Elem * slave_side = pinfo->_side;
      std::vector<std::vector<Real> > & slave_side_phi = pinfo->_side_phi;
      _gap_temp = _variable->getValue(slave_side, slave_side_phi);

      Real tangential_tolerance = _penetration_locator->getTangentialTolerance();
      if (tangential_tolerance != 0.0)
      {
        _edge_multiplier = 1.0 - pinfo->_tangential_distance / tangential_tolerance;
        if (_edge_multiplier < 0.0)
        {
          _edge_multiplier = 0.0;
        }
      }
    }
    else
    {
      if (_warnings)
      {
        std::stringstream msg;
        msg << "No gap value information found for node ";
        msg << qnode->id();
        msg << " on processor ";
        msg << processor_id();
        mooseWarning( msg.str() );
      }
    }
  }
}
