#ifndef INPUTROTORTRANSUSEROBJECT_H
#define INPUTROTORTRANSUSEROBJECT_H

#include "GeneralUserObject.h"

class InputRotOrTransUserObject : public GeneralUserObject
{
public:
  InputRotOrTransUserObject(const InputParameters & parameters);

  virtual ~InputRotOrTransUserObject(){}

  virtual void initialSetup() {}

  virtual void residualSetup() {}

  virtual void timestepSetup() {}

  virtual void execute() {}

  virtual void initialize() {}

  virtual void finalize() {}

  virtual bool isRotation() const;
  virtual bool isTranslation() const;
  virtual Point getAxis() const;
  virtual Point getOrigin() const;
  virtual unsigned int getComponent() const;
  virtual Real getValue(Real) const;

protected:

  MooseEnum _input_type;
  Point _rotation_axis;
  Point _origin;
  MooseEnum _unit_type;
  unsigned int _component;
};

#endif //INPUTROTORTRANSUSEROBJECT_H
