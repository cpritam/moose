#ifndef MULTIAXIALROTATIONANDTRANSLATIONACTION_H
#define MULTIAXIALROTATIONANDTRANSLATIONACTION_H

#include "Action.h"

class MultiAxialRotationAndTranslationAction;

template<>
InputParameters validParams<MultiAxialRotationAndTranslationAction>();

class MultiAxialRotationAndTranslationAction : public Action
{
public:
  MultiAxialRotationAndTranslationAction(const InputParameters & params);
  virtual void act();

private:
  const std::vector<BoundaryName> _boundary;
  const std::string _disp_x;
  const std::string _disp_y;
  const std::string _disp_z;

protected:
  std::string _kernel_name;
};

#endif //MULTIAXIALROTATIONANDTRANSLATIONACTION_H
