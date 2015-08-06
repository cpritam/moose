#ifndef INPUTROTORTRANSFROMFILE_H
#define INPUTROTORTRANSFROMFILE_H

#include "InputRotOrTransUserObject.h"
#include "LinearInterpolation.h"

class InputRotOrTransFromFile : public InputRotOrTransUserObject
{
public:
  InputRotOrTransFromFile(const InputParameters & parameters);
  virtual ~InputRotOrTransFromFile(){}
  
  virtual Real getValue(Real) const;

protected:

  virtual void readFromFile();

  std::string _file_name;
  unsigned int _t_data_column_index;
  unsigned int _data_column_index;
  unsigned int _num_column;

  std::vector<Real> _time_data;
  std::vector<Real> _data;
  LinearInterpolation _time_ipol;

};

#endif //INPUTROTORTRANSUSEROBJECT_H
