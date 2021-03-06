/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
// MOOSE includes
#include "MooseError.h"
#include "SolutionUserObject.h"
#include "RotationMatrix.h"

// libMesh includes
#include "libmesh/equation_systems.h"
#include "libmesh/mesh_function.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"

template<>
InputParameters validParams<SolutionUserObject>()
{
  // Get the input parameters from the parent class
  InputParameters params = validParams<GeneralUserObject>();

  // Add required parameters
  params.addRequiredParam<MeshFileName>("mesh", "The name of the mesh file (must be xda or exodusII file).");
  params.addParam<std::vector<std::string> >("system_variables", std::vector<std::string>(),
                                             "The name of the nodal and elemental variables from the file you want to use for values");
  params.addDeprecatedParam<std::vector<std::string> >("nodal_variables", "Nodal variables", "Use 'system_variables' for all variable names, both nodal and elemental");
  params.addDeprecatedParam<std::vector<std::string> >("elemental_variables", "Elemental variables", "Use 'system_variables' for all variable names, both nodal and elemental");

  // When using XDA files the following must be defined
  params.addParam<FileName>("es", "The name of the file holding the equation system info in xda format (xda only).");
  params.addParam<std::string>("system", "nl0", "The name of the system to pull values out of (xda only).");

  // When using ExodusII a specific time is extracted
  params.addParam<int>("timestep", -1, "Index of the single timestep used (exodusII only).  If not supplied, time interpolation will occur.");

  // Add ability to perform coordinate transformation: scale, factor
  params.addParam<std::vector<Real> >("coord_scale", "This name has been deprecated.  Please use scale instead");
  params.addParam<std::vector<Real> >("coord_factor", "This name has been deprecated.  Please use translation instead");
  params.addParam<std::vector<Real> >("scale", std::vector<Real>(LIBMESH_DIM,1), "Scale factor for points in the simulation");
  params.addParam<std::vector<Real> >("scale_multiplier", std::vector<Real>(LIBMESH_DIM,1), "Scale multiplying factor for points in the simulation");
  params.addParam<std::vector<Real> >("translation", std::vector<Real>(LIBMESH_DIM,0), "Translation factors for x,y,z coordinates of the simulation");
  params.addParam<RealVectorValue>("rotation0_vector", RealVectorValue(0, 0, 1), "Vector about which to rotate points of the simulation.");
  params.addParam<Real>("rotation0_angle", 0.0, "Anticlockwise rotation angle (in degrees) to use for rotation about rotation0_vector.");
  params.addParam<RealVectorValue>("rotation1_vector", RealVectorValue(0, 0, 1), "Vector about which to rotate points of the simulation.");
  params.addParam<Real>("rotation1_angle", 0.0, "Anticlockwise rotation angle (in degrees) to use for rotation about rotation1_vector.");
  params.addParam<bool>("legacy_read", false, "Utilize the legacy call to EquationsSystems::read, this may be required for older XDA/XDR files");

  // following lines build the default_transformation_order
  MooseEnum t1("rotation0 translation scale rotation1 scale_multiplier", "translation");
  MooseEnum t2("rotation0 translation scale rotation1 scale_multiplier", "scale");
  std::vector<MooseEnum> default_transformation_order;
  default_transformation_order.push_back(t1);
  default_transformation_order.push_back(t2);
  params.addParam<std::vector<MooseEnum> >("transformation_order", default_transformation_order, "The order to perform the operations in.  Define R0 to be the rotation matrix encoded by rotation0_vector and rotation0_angle.  Similarly for R1.  Denote the scale by s, the scale_multiplier by m, and the translation by t.  Then, given a point x in the simulation, if transformation_order = 'rotation0 scale_multiplier translation scale rotation1' then form p = R1*(R0*x*m - t)/s.  Then the values provided by the SolutionUserObject at point x in the simulation are the variable values at point p in the mesh.");
  // Return the parameters
  return params;
}

SolutionUserObject::SolutionUserObject(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    _file_type(MooseEnum("xda=0 exodusII=1 xdr=2")),
    _mesh_file(getParam<MeshFileName>("mesh")),
    _es_file(getParam<FileName>("es")),
    _system_name(getParam<std::string>("system")),
    _system_variables(getParam<std::vector<std::string> >("system_variables")),
    _exodus_time_index(getParam<int>("timestep")),
    _interpolate_times(false),
    _mesh(NULL),
    _es(NULL),
    _system(NULL),
    _mesh_function(NULL),
    _exodusII_io(NULL),
    _serialized_solution(NULL),
    _es2(NULL),
    _system2(NULL),
    _mesh_function2(NULL),
    _serialized_solution2(NULL),
    _interpolation_time(0.0),
    _interpolation_factor(0.0),
    _exodus_times(NULL),
    _exodus_index1(-1),
    _exodus_index2(-1),
    _scale(getParam<std::vector<Real> >("scale")),
    _scale_multiplier(getParam<std::vector<Real> >("scale_multiplier")),
    _translation(getParam<std::vector<Real> >("translation")),
    _rotation0_vector(getParam<RealVectorValue>("rotation0_vector")),
    _rotation0_angle(getParam<Real>("rotation0_angle")),
    _r0(RealTensorValue()),
    _rotation1_vector(getParam<RealVectorValue>("rotation1_vector")),
    _rotation1_angle(getParam<Real>("rotation1_angle")),
    _r1(RealTensorValue()),
    _transformation_order(getParam<std::vector<MooseEnum> >("transformation_order")),
    _initialized(false),
    _legacy_read(getParam<bool>("legacy_read"))
{

  // Handle deprecated parameters
  if (isParamValid("nodal_variables"))
  {
    std::vector<std::string> vars = getParam<std::vector<std::string> >("nodal_variables");
    _system_variables.insert(_system_variables.end(), vars.begin(), vars.end());
  }

  if (isParamValid("elemental_variables"))
  {
    std::vector<std::string> vars = getParam<std::vector<std::string> >("elemental_variables");
    _system_variables.insert(_system_variables.end(), vars.begin(), vars.end());
  }

  if (_legacy_read)
    mooseWarning("The input parameter 'legacy_read' is deprecated.\nThis option is for legacy support and will be removed on 10/1/2014.\nThe xda/xdr files being read should be regenerated and the flag removed.");

  // Extract coordinate transformation
  if (parameters.isParamValid("coord_scale"))
  {
    mooseWarning("Parameter name coord_scale is deprecated.  Please use scale instead.");
    _scale = getParam<std::vector<Real> >("coord_scale");
  }

  if (parameters.isParamValid("coord_factor"))
  {
    mooseWarning("Parameter name coord_factor is deprecated.  Please use translation instead.");
    _translation = getParam<std::vector<Real> >("coord_factor");
  }

  // form rotation matrices with the specified angles
  Real halfPi = std::acos(0.0);
  Real a;
  Real b;

  a = std::cos(halfPi*_rotation0_angle/90);
  b = std::sin(halfPi*_rotation0_angle/90);
  // the following is an anticlockwise rotation about z
  RealTensorValue rot0_z(
  a, -b, 0,
  b, a, 0,
  0, 0, 1);
  // form the rotation matrix that will take rotation0_vector to the z axis
  RealTensorValue vec0_to_z = RotationMatrix::rotVecToZ(_rotation0_vector);
  // _r0 is then: rotate points so vec0 lies along z; then rotate about angle0; then rotate points back
  _r0 = vec0_to_z.transpose()*(rot0_z*vec0_to_z);

  a = std::cos(halfPi*_rotation1_angle/90);
  b = std::sin(halfPi*_rotation1_angle/90);
  // the following is an anticlockwise rotation about z
  RealTensorValue rot1_z(
  a, -b, 0,
  b, a, 0,
  0, 0, 1);
  // form the rotation matrix that will take rotation1_vector to the z axis
  RealTensorValue vec1_to_z = RotationMatrix::rotVecToZ(_rotation1_vector);
  // _r1 is then: rotate points so vec1 lies along z; then rotate about angle1; then rotate points back
  _r1 = vec1_to_z.transpose()*(rot1_z*vec1_to_z);
}

SolutionUserObject::~SolutionUserObject()
{
  delete _es;
  delete _mesh;
  delete _serialized_solution;
  delete _mesh_function;

  if (_exodusII_io)
    delete _exodusII_io;

  if (_es2)
    delete _es2;

  if (_mesh_function2)
    delete _mesh_function2;

  if (_serialized_solution2)
    delete _serialized_solution2;
}

void
SolutionUserObject::readXda()
{
  // Check that the required files exist
  MooseUtils::checkFileReadable(_es_file);
  MooseUtils::checkFileReadable(_mesh_file);

  // Read the libmesh::mesh from the xda file
  _mesh->read(_mesh_file);

  // Create the libmesh::EquationSystems
  _es = new EquationSystems(*_mesh);

  // Use the legacy read
  if (_legacy_read)
    _es->read(_es_file);

  // Use new read syntax (binary)
  else if (_file_type ==  "xdr")
    _es->read(_es_file, DECODE, EquationSystems::READ_HEADER | EquationSystems::READ_DATA | EquationSystems::READ_ADDITIONAL_DATA);

  // Use new read syntax
  else if (_file_type ==  "xda")
    _es->read(_es_file, READ, EquationSystems::READ_HEADER | EquationSystems::READ_DATA | EquationSystems::READ_ADDITIONAL_DATA);

  // This should never occur, just incase produce an error
  else
    mooseError("Faild to determine proper read method for XDA/XDR equation system file: " << _es_file);

  // Update and store the EquationSystems name locally
  _es->update();
  _system = &_es->get_system(_system_name);
}

void
SolutionUserObject::readExodusII()
{
  // Define a default system name
  if (_system_name == "")
    _system_name = "SolutionUserObjectSystem";

  // Interpolate between times rather than using values from a set timestep
  if (_exodus_time_index == -1)
    _interpolate_times = true;  // Read the file

  // Read the Exodus file
  _exodusII_io = new ExodusII_IO (*_mesh);
  _exodusII_io->read(_mesh_file);
  _exodus_times = &_exodusII_io->get_time_steps();

  // Check that the number of time steps is valid
  int num_exo_times = _exodus_times->size();
  if (num_exo_times == 0)
    mooseError("In SolutionUserObject, exodus file contains no timesteps.");

  // Account for parallel mesh
  if (dynamic_cast<ParallelMesh *>(_mesh))
  {
    _mesh->allow_renumbering(true);
    _mesh->prepare_for_use(/*false*/);
  }
  else
  {
    _mesh->allow_renumbering(false);
    _mesh->prepare_for_use(/*true*/);
  }

  // Create EquationSystems object for solution
  _es = new EquationSystems(*_mesh);
  _es->add_system<ExplicitSystem> (_system_name);
  _system = &_es->get_system(_system_name);

  // Get the variable name lists as set; these need to be sets to perform set_intersection
  const std::vector<std::string> & all_nodal(_exodusII_io->get_nodal_var_names());
  const std::vector<std::string> & all_elemental(_exodusII_io->get_elem_var_names());

  // Storage for the nodal and elemental variables to consider
  std::vector<std::string> nodal, elemental;

  // Build nodal/elemental variable lists, limit to variables listed in 'system_variables', if provided
  if (!_system_variables.empty())
  {
    for (std::vector<std::string>::const_iterator it = _system_variables.begin(); it != _system_variables.end(); ++it)
    {
      if (std::find(all_nodal.begin(), all_nodal.end(), *it) != all_nodal.end())
        nodal.push_back(*it);
      if (std::find(all_elemental.begin(), all_elemental.end(), *it) != all_elemental.end())
        elemental.push_back(*it);
    }
  }
  else
  {
    nodal = all_nodal;
    elemental = all_elemental;
  }

  // Add the variables to the system
  for (std::vector<std::string>::const_iterator it = nodal.begin(); it != nodal.end(); ++it)
    _system->add_variable(*it, FIRST);

  for (std::vector<std::string>::const_iterator it = elemental.begin(); it != elemental.end(); ++it)
    _system->add_variable(*it, CONSTANT, MONOMIAL);

  // Initilize the equations systems
  _es->init();

  // Interpolate times
  if (_interpolate_times)
  {
    // Create a second equation system
    _es2 = new EquationSystems(*_mesh);
    _es2->add_system<ExplicitSystem> (_system_name);
    _system2 = &_es2->get_system(_system_name);

    // Add the variables to the system
    for (std::vector<std::string>::const_iterator it = nodal.begin(); it != nodal.end(); ++it)
      _system2->add_variable(*it, FIRST);

    for (std::vector<std::string>::const_iterator it = elemental.begin(); it != elemental.end(); ++it)
      _system2->add_variable(*it, CONSTANT, MONOMIAL);

    // Initialize
    _es2->init();

    // Update the times for interpolation (initially start at 0)
    updateExodusBracketingTimeIndices(0.0);

    // Copy the solutions from the first system
    for (std::vector<std::string>::const_iterator it = nodal.begin(); it != nodal.end(); ++it)
    {
      _exodusII_io->copy_nodal_solution(*_system, *it, *it, _exodus_index1+1);
      _exodusII_io->copy_nodal_solution(*_system2, *it, *it, _exodus_index2+1);
    }

    for (std::vector<std::string>::const_iterator it = elemental.begin(); it != elemental.end(); ++it)
    {
      _exodusII_io->copy_elemental_solution(*_system, *it, *it, _exodus_index1+1);
      _exodusII_io->copy_elemental_solution(*_system2, *it, *it, _exodus_index2+1);
    }

    // Update the systems
    _system->update();
    _es->update();
    _system2->update();
    _es2->update();
  }

  // Non-interpolated times
  else
  {
    if (_exodus_time_index > num_exo_times)
      mooseError("In SolutionUserObject, timestep = "<<_exodus_time_index<<", but there are only "<<num_exo_times<<" time steps.");

    // Copy the values from the ExodusII file
    for (std::vector<std::string>::const_iterator it = nodal.begin(); it != nodal.end(); ++it)
      _exodusII_io->copy_nodal_solution(*_system, *it, *it,  _exodus_time_index);

    for (std::vector<std::string>::const_iterator it = elemental.begin(); it != elemental.end(); ++it)
      _exodusII_io->copy_elemental_solution(*_system, *it, *it, _exodus_time_index);

    // Update the equations systems
    _system->update();
    _es->update();
  }
}

Real
SolutionUserObject::directValue(const Node * node, const std::string & var_name) const
{
  // Get the libmesh variable and system numbers
  unsigned int var_num = _system->variable_number(var_name);
  unsigned int sys_num = _system->number();

  // Get the node id and associated dof
  dof_id_type node_id = node->id();
  dof_id_type dof_id = _system->get_mesh().node(node_id).dof_number(sys_num, var_num, 0);

  // Return the desried value for the dof
  return directValue(dof_id);
}

Real
SolutionUserObject::directValue(const Elem * elem, const std::string & var_name) const
{
  // Get the libmesh variable and system numbers
  unsigned int var_num = _system->variable_number(var_name);
  unsigned int sys_num = _system->number();

  // Get the element id and associated dof
  dof_id_type elem_id = elem->id();
  dof_id_type dof_id = _system->get_mesh().elem(elem_id)->dof_number(sys_num, var_num, 0);

  // Return the desired value
  return directValue(dof_id);
}

void
SolutionUserObject::initialize()
{
}

void
SolutionUserObject::finalize()
{
}

void
SolutionUserObject::timestepSetup()
{
  // Update time interpolatation for ExodusII solution
  if (_file_type == 1 && _interpolate_times)
    updateExodusTimeInterpolation(_t);
}

void
SolutionUserObject::execute()
{
}

void
SolutionUserObject::initialSetup()
{

  // Make sure this only happens once
  if (_initialized)
    return;

  // Several aspects of SolutionUserObject won't work if the FEProblem's MooseMesh is
  // a ParallelMesh:
  // .) ExodusII_IO::copy_nodal_solution() doesn't work in parallel.
  // .) We don't know if directValue will be used, which may request
  //    a value on a Node we don't have.
  _fe_problem.mesh().errorIfParallelDistribution("SolutionUserObject");


  // Create a libmesh::Mesh object for storing the loaded data.  Since
  // SolutionUserObject is restricted to only work with SerialMesh
  // (see above) we can force the Mesh used here to be a SerialMesh.
  _mesh = new SerialMesh(_communicator);

  // ExodusII mesh file supplied
  if (MooseUtils::hasExtension(_mesh_file, "e", /*strip_exodus_ext =*/ true))
  {
    _file_type = "exodusII";
    readExodusII();
  }

  // XDA mesh file supplied
  else if (MooseUtils::hasExtension(_mesh_file, "xda"))
  {
    _file_type = "xda";
    readXda();
  }

  else if (MooseUtils::hasExtension(_mesh_file, "xdr"))
  {
    _file_type = "xdr";
    readXda();
  }

  // Produce an error for an unknown file type
  else
    mooseError("In SolutionUserObject, invalid file type (only .xda, .xdr, and .e supported)");

  // Intilize the serial solution vector
  _serialized_solution = NumericVector<Number>::build(_communicator).release();
  _serialized_solution->init(_system->n_dofs(), false, SERIAL);

  // Pull down a full copy of this vector on every processor so we can get values in parallel
  _system->solution->localize(*_serialized_solution);

  // Vector of variable numbers to apply the MeshFunction to
  std::vector<unsigned int> var_nums;

  // If no variables were given, use all of them
  if (_system_variables.empty())
  {
    _system->get_all_variable_numbers(var_nums);
    for (std::vector<unsigned int>::const_iterator it = var_nums.begin(); it != var_nums.end(); ++it)
      _system_variables.push_back(_system->variable_name(*it));
  }

  // Otherwise, gather the numbers for the variabels given
  else
  {
    for (std::vector<std::string>::const_iterator it = _system_variables.begin(); it != _system_variables.end(); ++it)
      var_nums.push_back(_system->variable_number(*it));
  }

  // Create the MeshFunction for working with the solution data
  _mesh_function = new MeshFunction(*_es, *_serialized_solution, _system->get_dof_map(), var_nums);
  _mesh_function->init();

  // Build second MeshFunction for interpolation
  if (_interpolate_times)
  {
    // Need to pull down a full copy of this vector on every processor so we can get values in parallel
    _serialized_solution2 = NumericVector<Number>::build(_communicator).release();
    _serialized_solution2->init(_system2->n_dofs(), false, SERIAL);
    _system2->solution->localize(*_serialized_solution2);

    // Create the MeshFunction for the second copy of the data
    _mesh_function2 = new MeshFunction(*_es2, *_serialized_solution2, _system2->get_dof_map(), var_nums);
    _mesh_function2->init();

  }

  // Populate the data maps that indicate if the variable is nodal and the MeshFunction variable index
  for (unsigned int i = 0; i < _system_variables.size(); ++i)
  {
    std::string name = _system_variables[i];
    FEType type = _system->variable_type(name);
    if (type.order == CONSTANT)
      _local_variable_nodal[name] = false;
    else
      _local_variable_nodal[name] = true;

    _local_variable_index[name] = i;
  }

  // Set initialization flag
  _initialized = true;
}

MooseEnum
SolutionUserObject::getSolutionFileType()
{
  return _file_type;
}

void
SolutionUserObject::updateExodusTimeInterpolation(Real time)
{
  if (time != _interpolation_time)
  {
    if (updateExodusBracketingTimeIndices(time))
    {

      for (std::vector<std::string>::const_iterator it = _system_variables.begin(); it != _system_variables.end(); ++it)
      {
        if (_local_variable_nodal[*it])
          _exodusII_io->copy_nodal_solution(*_system, *it, _exodus_index1+1);
        else
          _exodusII_io->copy_elemental_solution(*_system, *it, *it, _exodus_index1+1);
      }

      _system->update();
      _es->update();
      _system->solution->localize(*_serialized_solution);

      for (std::vector<std::string>::const_iterator it = _system_variables.begin(); it != _system_variables.end(); ++it)
      {
        if (_local_variable_nodal[*it])
          _exodusII_io->copy_nodal_solution(*_system2, *it, _exodus_index2+1);
        else
          _exodusII_io->copy_elemental_solution(*_system2, *it, *it, _exodus_index2+1);
      }

      _system2->update();
      _es2->update();
      _system2->solution->localize(*_serialized_solution2);
    }
    _interpolation_time = time;
  }
}

bool
SolutionUserObject::updateExodusBracketingTimeIndices(Real time)
{
  if (_file_type != 1)
    mooseError("In SolutionUserObject, getTimeInterpolationData only applicable for exodusII file type");

  int old_index1 = _exodus_index1;
  int old_index2 = _exodus_index2;

  int num_exo_times = _exodus_times->size();

  if (time  < (*_exodus_times)[0])
  {
    _exodus_index1 = 0;
    _exodus_index2 = 0;
    _interpolation_factor = 0.0;
  }
  else
  {
    for (int i=0; i<num_exo_times-1; ++i)
    {
      if (time  <= (*_exodus_times)[i+1])
      {
        _exodus_index1 = i;
        _exodus_index2 = i+1;
        _interpolation_factor = (time-(*_exodus_times)[i])/((*_exodus_times)[i+1]-(*_exodus_times)[i]);
        break;
      }
      else if (i==num_exo_times-2)
      {
        _exodus_index1 = num_exo_times-1;
        _exodus_index2 = num_exo_times-1;
        _interpolation_factor = 1.0;
        break;
      }
    }
  }

  bool indices_modified(false);

  if (_exodus_index1 != old_index1 ||  _exodus_index2 != old_index2)
    indices_modified = true;

  return indices_modified;
}


Real
SolutionUserObject::pointValue(Real t, const Point & p, const std::string & var_name) const
{
  // Create copy of point
  Point pt(p);

  // do the transformations
  for (unsigned int trans_num = 0 ; trans_num < _transformation_order.size() ; ++trans_num)
  {
    if (_transformation_order[trans_num] == "rotation0")
      pt = _r0*pt;
    else if (_transformation_order[trans_num] == "translation")
      for (unsigned int i=0; i<LIBMESH_DIM; ++i)
        pt(i) -= _translation[i];
    else if (_transformation_order[trans_num] == "scale")
      for (unsigned int i=0; i<LIBMESH_DIM; ++i)
        pt(i) /= _scale[i];
    else if (_transformation_order[trans_num] == "scale_multiplier")
      for (unsigned int i=0; i<LIBMESH_DIM; ++i)
        pt(i) *= _scale_multiplier[i];
    else if (_transformation_order[trans_num] == "rotation1")
      pt = _r1*pt;
  }

  // Extract the value at the current point
  Real val = evalMeshFunction(pt, var_name, 1);

  // Interplolate
  if (_file_type == 1 && _interpolate_times)
  {
    mooseAssert(t == _interpolation_time, "Time passed into value() must match time at last call to timestepSetup()");
    Real val2 = evalMeshFunction(pt, var_name, 2);
    val = val + (val2 - val)*_interpolation_factor;
  }

  return val;
}

Real
SolutionUserObject::directValue(dof_id_type dof_index) const
{
  Real val = (*_serialized_solution)(dof_index);
  if (_file_type==1 && _interpolate_times)
  {
    Real val2 = (*_serialized_solution2)(dof_index);
    val = val + (val2 - val)*_interpolation_factor;
  }
  return val;
}

Real
SolutionUserObject::evalMeshFunction(const Point & p, std::string var_name, unsigned int func_num) const
{
  // Storage for mesh function output
  DenseVector<Number> output;

  // Extract a value from the _mesh_function
  if (func_num == 1)
    (*_mesh_function)(p, 0.0, output);

  // Extract a value from _mesh_function2
  else if (func_num == 2)
    (*_mesh_function2)(p, 0.0, output);

  else
    mooseError("The func_num must be 1 or 2");

  // Extract the variable index for the MeshFunction(s), must use iterator b/c of const
  std::map<std::string, unsigned int>::const_iterator it = _local_variable_index.find(var_name);
  return output(it->second);
}

const std::vector<std::string> &
SolutionUserObject::variableNames() const
{
  return _system_variables;
}

bool
SolutionUserObject::isVariableNodal(const std::string & var_name) const
{
  // Use iterator method, [] is not marked const
  std::map<std::string, bool>::const_iterator it = _local_variable_nodal.find(var_name);
  return it->second;
}
