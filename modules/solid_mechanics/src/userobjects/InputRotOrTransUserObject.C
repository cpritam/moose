#include "InputRotOrTransUserObject.h"

template<>
InputParameters validParams<InputRotOrTransUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  MooseEnum input("rotation translation none", "none");
  params.addRequiredParam<MooseEnum>("input_type", input, "Input time dependent translation or rotation");
  params.addParam<Point>("rotation_axis", 0, "Rotation axis");
  params.addParam<Point>("origin", 0, "Origin");
  MooseEnum units("degrees radians none", "none");
  params.addParam<MooseEnum>("unit_type", units, "Unit of rotation angle");
  params.addParam<unsigned int>("component", 0, "Translational component");

  return params;
}

InputRotOrTransUserObject::InputRotOrTransUserObject(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    _input_type(getParam<MooseEnum>("input_type")),
    _rotation_axis(getParam<Point>("rotation_axis")),
    _origin(getParam<Point>("origin")),
    _unit_type(getParam<MooseEnum>("unit_type")),
    _component(getParam<unsigned int>("component"))
{
  //  std::cout << "Component 1 " << _component << std::endl;
}

bool
InputRotOrTransUserObject::isRotation() const
{
  switch (_input_type)
  {
    case 0:
      return true;
    case 1:
      return false;
    default:
      mooseError("Rotation or translation needs to be specified as input_type");
  }
}

bool
InputRotOrTransUserObject::isTranslation() const
{
  switch (_input_type)
  {
    case 0:
      return false;
    case 1:
      return true;
    default:
      mooseError("Rotation or translation needs to be specified as input_type");
  }
}

Point
InputRotOrTransUserObject::getAxis() const
{
  switch (_input_type)
  {
    case 0:
      return _rotation_axis;
    case 1:
      break;
    default:
      break;
  }
  mooseError("Cannot return rotation axis if rotation is not specified as input_type");
}

Point
InputRotOrTransUserObject::getOrigin() const
{
  switch (_input_type)
  {
    case 0:
      return _origin;
    case 1:
      break;
    default:
      break;
  }

  mooseError("Cannot return origin if rotation is not specified as input_type");
}

unsigned int
InputRotOrTransUserObject::getComponent() const
{
  switch (_input_type)
  {
    case 0:
      break;
    case 1:
      return _component;      
    default:
      break;
  }

  mooseError("Cannot return component if translation is not specified as input_type");
}

Real
InputRotOrTransUserObject::getValue(Real time) const
{
  return 0.0;
}
