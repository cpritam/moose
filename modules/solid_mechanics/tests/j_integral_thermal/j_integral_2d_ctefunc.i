[GlobalParams]
  order = FIRST
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
[]

[Mesh]
  file = crack2d.e
  displacements = 'disp_x disp_y'
#  uniform_refine = 3
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]


[AuxVariables]
  [./stress_xx]      # stress aux variables are defined for output; this is a way to get integration point variables to the output file
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]


[Functions]
  [./tempfunc]
    type = ParsedFunction
    value = 10.0*(2*x/504)
  [../]
  [./cte_func]
    type = PiecewiseLinear
    x = '-10 -6 -2 0 2 6 10'
    y = '1.484e-5 1.489e-5 1.494e-5 1.496e-6 1.498e-5 1.502e-5 1.505e-5'

#     x = '-10 10'
#     y = '1.35e-5 1.35e-5'
  [../]
[]

[DomainIntegral]
  integrals = JIntegral
  boundary = 800
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  2d = true
  axis_2d = 2
  radius_inner = '60.0 80.0 100.0 120.0'
  radius_outer = '80.0 100.0 120.0 140.0'
[]

[SolidMechanics]
  [./solid]
  [../]
[]

[AuxKernels]
  [./stress_xx]               # computes stress components for output
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep     # for efficiency, only compute at the end of a timestep
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep
  [../]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep
  [../]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    block = 1
  [../]
[]

[BCs]

  [./crack_y]
    type = PresetBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]

  [./no_y]
    type = PresetBC
    variable = disp_y
    boundary = 400
    value = 0.0
  [../]

  [./no_x1]
    type = PresetBC
    variable = disp_x
    boundary = 900
    value = 0.0
  [../]

[] # BCs

[Materials]
  [./stiffStuff]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y

    youngs_modulus = 207000
    poissons_ratio = 0.3
#    thermal_expansion = 1.35e-5
    formulation = PlaneStrain
    compute_JIntegral = true
    temp = temp
    stress_free_temperature = 0.0
    thermal_expansion_function = cte_func

  [../]
[]


[Executioner]

  type = Transient

  solve_type = 'PJFNK'

  petsc_options = ksp_monitor
  petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31   preonly   lu      1'

  line_search = 'none'

   l_max_its = 50
   nl_max_its = 40

   nl_rel_step_tol= 1e-10
   nl_rel_tol = 1e-10


   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1

[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]

  [./nl_its]
    type = NumNonlinearIterations
  [../]

  [./lin_its]
    type = NumLinearIterations
  [../]
[]


[Outputs]
  file_base = j_integral_2d_ctefunc_out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = false
  [../]
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    pc_side = left
    full = true
  [../]
[]
