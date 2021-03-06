/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TABLESOUTPUTBASE_H
#define TABLESOUTPUTBASE_H

// MOOSE includes
#include "PetscOutput.h"
#include "FormattedTable.h"

class TableOutput;

template<>
InputParameters validParams<TableOutput>();

/**
 * Base class for scalar variables and postprocessors output objects
 *
 * This class populates three FormattedTable objects that may then be used
 * by child classes for creating custom output objects:
 * _all_data_table - includes the data from both postprocessors and scalar aux variables
 * _postprocessor_table - includes the data from only the postprocessors
 * _scalar_table - includes the data from only the scalar aux variables
 *
 * @see CSV Console
 */
class TableOutput :
  public PetscOutput
{
public:

  /**
   * Class constructor.
   */
  TableOutput(const std::string & name, InputParameters);

  /**
   * Destructor
   */
  virtual ~TableOutput();

protected:

  //@{
  /**
   * Produces an error, it is not possible to output nodal and elemental data to a table
   *
   * The call to this function is disable by suppressing the input parameter: output_nodal_variables
   */
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  //@}

  /**
   * Populates the tables with scalar aux variables
   *
   * If an aux variable contains multiple components the output name for the
   * variable is appended with the component number (e.g., aux_0, aux_1, ...)
   */
  virtual void outputScalarVariables();

  /**
   * Populates the tables with postprocessor values
   */
  virtual void outputPostprocessors();

  /**
   * Populates the tables with VectorPostprocessor values
   */
  virtual void outputVectorPostprocessors();

  /// Flag for allowing all table data to become restartable
  bool _tables_restartable;

  /// Table containing postprocessor data
  FormattedTable & _postprocessor_table;

  /// Formatted tables for outputting vector postprocessor data.  One per VectorPostprocessor
  std::map<std::string, FormattedTable> _vector_postprocessor_tables;

  /// Table containing scalar aux variables
  FormattedTable & _scalar_table;

  /// Table containing postprocessor values and scalar aux variables
  FormattedTable & _all_data_table;

};

#endif /* TABLEOUTPUT_H */
