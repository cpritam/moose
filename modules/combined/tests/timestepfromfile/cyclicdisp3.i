[Mesh]
  type = GeneratedMesh
  dim = 2
  displacements = 'disp_x disp_y'
  elem_type = QUAD4
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
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
    boundary = bottom
    value = 0
  [../]
  [./move_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = top
    function = t
  [../]
[]

[Materials]
  [./elastic]
    type = Elastic
    block = 0
    disp_y = disp_y
    disp_x = disp_x
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
  [./TimeStepper]
    type = TimePointsFromFile
    file_name = 'cyclic3.txt'
    number_of_columns = 1
    t_data_column_index = 0
    dt_data_column_index = 1
    is_dt_from_file = false
    use_last_time_as_end_time = true
  [../]  
[]

[SolidMechanics]
  [./solid_mech]
    disp_y = disp_y
    disp_x = disp_x
  [../]
[]

