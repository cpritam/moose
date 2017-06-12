[Mesh]
  type = FileMesh
  file = 2D_pellet_clad.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./disp_y]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
    use_finite_deform_jacobian = true
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl]
    order = CONSTANT
    family = MONOMIAL
    block = 1
  [../]
  [./temp]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = stress_xx
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = stress_yy
    execute_on = timestep_end
  [../]
  [./intnl]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = intnl
  [../]
  [./temp]
    variable = temp
    type = FunctionAux
    function = '298+1000*t'
  [../]
[]

[UserObjects]
  [./str]
    type = TensorMechanicsHardeningPowerRule
    value_0 = 545.625
    epsilon0 = 0.0055390625
    exponent = 0.18875
    #value_0 = 552.3046875
    #epsilon0 = 0.00644140625
    #exponent = 0.109794921875
  [../]
#  [./str]
#    type = TensorMechanicsHardeningCubic
#    value_0 = 545.0
#    value_residual = 1e5
#    internal_limit = 2.0
#  [../]
  [./Orthotropic]
    type = TensorMechanicsPlasticOrthotropic
    #type = TensorMechanicsPlasticJ2
    b = -0.006015625
    c1 = '1.18652344  1.80126953  2.01367188 1.73205080757 1.73205080757 1.73205080757'
    c2 = '-1.96875    -0.515625    2.265625 0 0 0'
    #b = 0.0012109375
    #c1 = '1.1875 1.4375 2.45898438 1.73205080757 1.73205080757 1.73205080757'
    #c2 = '-1.00195312 -0.96972656  2.2421875 0 0 0'
#    b = 0.0
#    c1 = '1.73205080757 1.73205080757 1.73205080757 1.73205080757 1.73205080757 1.73205080757'
#    c2 = '0 0 0 0 0 0'
    associative = true
    yield_strength = str
    #yield_function_tolerance = 1e-5
    #internal_constraint_tolerance = 1e-9
    yield_function_tolerance = 1e-4
    internal_constraint_tolerance = 1e-4
    use_custom_returnMap = true
    use_custom_cto = true
  [../]
[]

[Materials]
  [./finite_strain_1]
    type = ComputeFiniteStrain
    volumetric_locking_correction = false
    eigenstrain_names = 'thermal_strain_1'
    block = 1
  [../]
  [./finite_strain_2]
    type = ComputeFiniteStrain
    volumetric_locking_correction = false
    eigenstrain_names = 'thermal_strain_2'
    block = 2
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '57e3 38e3'
    block = '1 2'
  [../]
  [./stress_1]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1e-9
    plastic_models = Orthotropic
    debug_fspb = crash
    tangent_operator = elastic
    max_NR_iterations = 100
    perform_finite_strain_rotations = false
    block = 1
    material_axis = cylinder
#    material_axis = none
    axis_of_cylinder = '0 0 1'
  [../]
  [./thermal_expansion_1]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 298
    thermal_expansion_coeff = 0
    temperature = temp
    incremental_form = true
    eigenstrain_name = thermal_strain_1
    block = 1
  [../]
  [./stress_2]
    type = ComputeFiniteStrainElasticStress
    block = 2
  [../]
  [./thermal_expansion_2]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 298
    thermal_expansion_coeff = 1e-4
    temperature = temp
    incremental_form = true
    eigenstrain_name = thermal_strain_2
    block = 2
  [../]
[]

[BCs]
  [./no_disp_x]
    type = PresetBC
    variable = disp_x
    boundary = 2
    value = 0.0
  [../]
  [./no_disp_y]
    type = PresetBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
[]

[Contact]
  [./pellet_clad]
    master = 4
    slave = 3
    disp_x = disp_x
    disp_y = disp_y
    penalty = 1e5
    model = frictionless
    formulation = penalty
    system = constraint
    order = SECOND
  [../]
[]

[Debug]
    show_var_residual_norms = true
[]

[Executioner]
  type = Transient

#  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
#  petsc_options_value = '  201               hypre    boomeramg      10'

#  line_search = 'none'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -ksp_gmres_restart -snes_ksp_ew_rtol0 -snes_ksp_ew_rtolmax -snes_ksp_ew_gamma -snes_ksp_ew_alpha -snes_ksp_ew_alpha2 -snes_ksp_ew_threshold'
  petsc_options_value = 'lu superlu_dist 51 0.5 0.9 1 2 2 0.1'

  nl_rel_tol = 1e-4
  nl_max_its = 10

  l_tol = 1e-4
  l_max_its = 50

  start_time = 0.0
  end_time = 0.2
  dt = 1e-2
  dtmin = 1e-4
#  num_steps = 1
[]

[Outputs]
  exodus = true
  print_perf_log = true
  csv = true
[]

[Preconditioning]
  [./smp]
     type = SMP
     full = true
  [../]
[]
