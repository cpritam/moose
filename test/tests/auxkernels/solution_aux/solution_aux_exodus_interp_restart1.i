[Mesh]
  # This test uses SolutionUserObject which doesn't work with ParallelMesh.
  type = FileMesh
  file = cubesource.e
  distribution = serial
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  [../]
[]

[AuxVariables]
  [./nn]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./nn]
    type = SolutionAux
    variable = nn
    solution = soln
  [../]
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = cubesource.e
    system_variables = source_nodal
    execute_on = 'initial timestep_begin'
  [../]
[]
[Problem]
  type = FEProblem
  use_legacy_uo_initialization = false
[]

[BCs]
  [./stuff]
    type = DirichletBC
    variable = u
    boundary = '1 2'
    value = 0.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  l_max_its = 800
  nl_rel_tol = 1e-10
  num_steps = 5
  end_time = 5
  dt = 0.5
[]

[Outputs]
  exodus = true
  checkpoint = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
