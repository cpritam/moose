/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiAxialRotationAndTranslationAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "GeneralUserObject.h"
#include "MultiAxialRotationAndTranslation.h"

template<>
InputParameters validParams<MultiAxialRotationAndTranslationAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where the rotational displacement will be applied");
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addRequiredParam<NonlinearVariableName>("disp_y", "The y displacement");
  params.addRequiredParam<NonlinearVariableName>("disp_z", "The z displacement");
  params.addRequiredParam<std::vector<UserObjectName> >("rot_trans_user_objects", "List of User object names that provides rotation and translation data");
  params.addParam<Real>("scale_translation", 1, "Scale translation from input");
  params.addParam<Real>("scale_rotation", 1, "Scale rotation from input");

  return params;
}

MultiAxialRotationAndTranslationAction::MultiAxialRotationAndTranslationAction(const InputParameters & params) :
  Action(params),
  _boundary(getParam<std::vector<BoundaryName> >("boundary")),
  _disp_x(getParam<NonlinearVariableName>("disp_x")),
  _disp_y(getParam<NonlinearVariableName>("disp_y")),
  _disp_z(getParam<NonlinearVariableName>("disp_z")),
  _kernel_name("MultiAxialRotationAndTranslation")
{
}

void
MultiAxialRotationAndTranslationAction::act()
{
  std::vector<unsigned int> dim_vec;

  dim_vec.push_back(0);
  dim_vec.push_back(1);
  dim_vec.push_back(2);

  std::vector<NonlinearVariableName> vars;
  vars.push_back(_disp_x);
  vars.push_back(_disp_y);
  vars.push_back(_disp_z);

  std::string short_name(_name);
  // Chop off "BCs/DisplacementAboutAxis/"
  short_name.erase(0, 5+_kernel_name.size());

  for (std::vector<unsigned int>::iterator it = dim_vec.begin(); it != dim_vec.end(); ++it)
  {
    std::stringstream name;
    name << "BCs/";
    name << short_name;
    name << "_";
    name << *it;

    InputParameters params = _factory.getValidParams(_kernel_name);

    params.set<std::vector<BoundaryName> >("boundary") = _boundary;
    params.set< std::vector<UserObjectName> >("rot_trans_user_objects") = getParam< std::vector<UserObjectName> >("rot_trans_user_objects");
    params.set<Real>("scale_translation") = getParam<Real>("scale_translation");
    params.set<Real>("scale_rotation") = getParam<Real>("scale_rotation");
    params.set<unsigned int>("component") = *it;
    params.set<NonlinearVariableName>("variable") = vars[*it];
    _problem->addBoundaryCondition(_kernel_name, name.str(), params);
  }
}
