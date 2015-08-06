[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
  xmin = 0
  xmax = 4
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
#  nx = 16
#  ny = 4
#  nz = 4
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

[BCs]
  [./fix_x]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./fix_y]
    type = PresetBC
    variable = disp_y
    boundary = left
    value = 0
  [../]
  [./fix_z]
    type = PresetBC
    variable = disp_z
    boundary = left
    value = 0
  [../]
  [./MultiAxialRotationAndTranslation]
    [./coupledbc]
      disp_x = disp_x
      disp_y = disp_y
      disp_z = disp_z
      boundary = right
      rot_trans_user_objects = 'rot'
    [../]
  [../]
[]

[UserObjects]
  [./trans]
    type = InputRotOrTransFromFile
    file_name = cyclic_data.txt
    t_data_column_index = 0
    data_column_index = 1
    number_of_columns = 3
    input_type = translation
    component = 1
  [../]
  [./rot]
    type = InputRotOrTransFromFile
    file_name = cyclic_data.txt
    t_data_column_index = 0
    data_column_index = 2
    number_of_columns = 3
    input_type = rotation
    rotation_axis = '1 0 0'
    origin = '4 0.5 0.5'
    unit_type = radians
  [../]
[]

[Materials]
  [./elastic]
    type = Elastic
    block = 0
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
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-15
  l_max_its = 50
  nl_max_its = 10
  dtmin = 1e-3
  [./TimeStepper]
    type = TimePointsFromFile
    file_name = 'cyclic_data.txt'
    number_of_columns = 3
    t_data_column_index = 0
    is_dt_from_file = false
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

