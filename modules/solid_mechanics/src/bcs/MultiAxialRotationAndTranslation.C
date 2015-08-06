#include "MultiAxialRotationAndTranslation.h"

template<>
InputParameters validParams<MultiAxialRotationAndTranslation>()
{
  InputParameters params = validParams<PresetNodalBC>();
  params.addRequiredParam<std::vector<UserObjectName> >("rot_trans_user_objects", "List of User object names that provides rotation and translation data");
  params.addParam<Real>("scale_translation", 1, "Scale translation from input");
  params.addParam<Real>("scale_rotation", 1, "Scale rotation from input");
  params.addRequiredParam<unsigned int>("component","The component for the rotational and translational displacement");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

MultiAxialRotationAndTranslation::MultiAxialRotationAndTranslation(const InputParameters & parameters) : 
    PresetNodalBC(parameters),
    _num_uos(getParam<std::vector<UserObjectName> >("rot_trans_user_objects").size()),
    _scale_translation(getParam<Real>("scale_translation")),
    _scale_rotation(getParam<Real>("scale_rotation")),
    _component(getParam<unsigned int>("component"))
{
  if (_num_uos == 0)
    mooseError("Specify atleast one rotational or translational user object");

  _rot_or_trans_uos.resize(_num_uos);

  for (unsigned int i = 0; i < _num_uos; ++i)
    _rot_or_trans_uos[i] = &getUserObjectByName<InputRotOrTransUserObject>(parameters.get< std::vector<UserObjectName> >("rot_trans_user_objects")[i]);
}


void
MultiAxialRotationAndTranslation::initialSetup()
{
  Point axis, origin;
  unsigned int rot_uo_index = 0;
  ColumnMajorMatrix matinv(4,4);

  for (unsigned int i = 0; i < _num_uos; ++i)
    if (_rot_or_trans_uos[i]->isRotation())
    {
      axis = _rot_or_trans_uos[i]->getAxis();
      origin = _rot_or_trans_uos[i]->getOrigin();

      _transformation_matrix.push_back(calculateTransformationMatrices(axis, origin));
      _transformation_matrix[rot_uo_index].inverse(matinv);
      _transformation_matrix_inv.push_back(matinv);
      
      ++rot_uo_index;
    }
}

ColumnMajorMatrix
MultiAxialRotationAndTranslation::calculateTransformationMatrices(Point axis, Point origin)
{
  Real a(axis(0));
  Real b(axis(1));
  Real c(axis(2));
  Real l(a*a+b*b+c*c);
  Real v(b*b+c*c);

  ColumnMajorMatrix transl(4,4);
  transl(0,0) = 1;
  transl(0,1) = 0;
  transl(0,2) = 0;
  transl(0,3) = -origin(0);
  transl(1,0) = 0;
  transl(1,1) = 1;
  transl(1,2) = 0;
  transl(1,3) = -origin(1);
  transl(2,0) = 0;
  transl(2,1) = 0;
  transl(2,2) = 1;
  transl(2,3) = -origin(2);
  transl(3,0) = 0;
  transl(3,1) = 0;
  transl(3,2) = 0;
  transl(3,3) = 1;
ColumnMajorMatrix rotx(4,4);
  rotx(0,0) = 1;
  rotx(0,1) = 0;
  rotx(0,2) = 0;
  rotx(0,3) = 0;
  rotx(1,0) = 0;
  if (std::abs(v) > 0.0)
  {
    rotx(1,1) = c/v;
    rotx(1,2) = -b/v;
    rotx(2,1) = b/v;
    rotx(2,2) = c/v;
  }
  else
  {
    rotx(1,1) = 1.0;
    rotx(1,2) = 0.0;
    rotx(2,1) = 0.0;
    rotx(2,2) = 1.0;
  }
  rotx(1,3) = 0;
  rotx(2,0) = 0;
  rotx(2,3) = 0;
  rotx(3,0) = 0;
  rotx(3,1) = 0;
  rotx(3,2) = 0;
  rotx(3,3) = 1;
  ColumnMajorMatrix roty(4,4);
  roty(0,0) = v/l;
  roty(0,1) = 0;
  roty(0,2) = -a/l;
  roty(0,3) = 0;
  roty(1,0) = 0;
  roty(1,1) = 1;
  roty(1,2) = 0;
  roty(1,3) = 0;
  roty(2,0) = a/l;
  roty(2,1) = 0;
  roty(2,2) = v/l;
  roty(2,3) = 0;
  roty(3,0) = 0;
  roty(3,1) = 0;
  roty(3,2) = 0;
  roty(3,3) = 1;

  return roty * rotx * transl;
}


Real
MultiAxialRotationAndTranslation::computeQpValue()
{
  Point p(*_current_node);
  ColumnMajorMatrix p_old(4,1);
  p_old(0,0) = p(0);
  p_old(1,0) = p(1);
  p_old(2,0) = p(2);
  p_old(3,0) = 1;

  unsigned int rot_index = 0;
  Real value = 0.0;

  for (unsigned int i = 0; i < _num_uos; ++i)
  {
    if (_rot_or_trans_uos[i]->isRotation())
    {
      ColumnMajorMatrix rotz = rotateAroundAxis(_scale_rotation * _rot_or_trans_uos[rot_index]->getValue(_t));
      ColumnMajorMatrix transform = _transformation_matrix_inv[rot_index] * rotz * _transformation_matrix[rot_index];
      ColumnMajorMatrix p_new = transform * p_old;
      value += p_new(_component,0)-p_old(_component,0);
      ++rot_index;
    }
    else
    {
      if (_component == _rot_or_trans_uos[i]->getComponent())
	value += _scale_translation * _rot_or_trans_uos[i]->getValue(_t);
    }
  }

  return value;
}


ColumnMajorMatrix
MultiAxialRotationAndTranslation::rotateAroundAxis(const Real angle)
{
  ColumnMajorMatrix rotz(4,4);
  rotz(0,0) = cos(angle);
  rotz(0,1) = -sin(angle);
  rotz(0,2) = 0;
  rotz(0,3) = 0;
  rotz(1,0) = sin(angle);
  rotz(1,1) = cos(angle);
  rotz(1,2) = 0;
  rotz(1,3) = 0;
  rotz(2,0) = 0;
  rotz(2,1) = 0;
  rotz(2,2) = 1;
  rotz(2,3) = 0;
  rotz(3,0) = 0;
  rotz(3,1) = 0;
  rotz(3,2) = 0;
  rotz(3,3) = 1;

  return rotz;
}
