[Mesh]
  type = FileMesh
  file = hollow_cylinder.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./var_out]
     family = MONOMIAL
     order = CONSTANT
  [../]
[]

[AuxKernels]
  [./var_out]
     type = SymmTensorToFileAux
     variable = var_out
     out_file_name = stress_cyl.out
     region_type = cylinder
     tensor = stress
     center = '0 0 12.9'
     rmin = 2
     rmax = 2.5
     zmin = 4
     zmax = 4.5
     axial_dir = 2
     execute_on = timestep_end
  [../]
[]

[BCs]
  [./fix_x]
    type = PresetBC
    variable = disp_x
    boundary = 2
    value = 0
  [../]
  [./fix_y]
    type = PresetBC
    variable = disp_y
    boundary = 2
    value = 0
  [../]
  [./fix_z]
    type = PresetBC
    variable = disp_z
    boundary = 2
    value = 0
  [../]
  [./move_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 1
    function = 't'
  [../]
[]

[Materials]
  [./elastic]
    type = Elastic
    block = 1
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    poissons_ratio = 0.3
    youngs_modulus = 210000
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly       lu           1'
  line_search = none
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-15
  l_max_its = 50
  nl_max_its = 10
  num_steps = 1
  [./TimeStepper]
    type = TimePointsFromFile
    file_name = 'cyclic.txt'
    number_of_columns = 2
    t_data_column_index = 0
    dt_data_column_index = 1
    is_dt_from_file = true
    use_last_time_as_end_time = true
  [../]  
[]

[SolidMechanics]
  [./solid_mech]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Outputs]
  output_initial = true
  print_linear_residuals = false
  print_perf_log = true
  [./exo]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]