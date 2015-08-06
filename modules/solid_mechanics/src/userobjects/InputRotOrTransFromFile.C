#include "InputRotOrTransFromFile.h"

template<>
InputParameters validParams<InputRotOrTransFromFile>()
{
  InputParameters params = validParams<InputRotOrTransUserObject>();
  params.addRequiredParam<std::string>("file_name", "Name of file with rotation/translation history");
  params.addRequiredParam<unsigned int>("t_data_column_index", "Column index of time data in file");
  params.addRequiredParam<unsigned int>("data_column_index", "Column index of data in file");
  params.addRequiredParam<unsigned int>("number_of_columns", "Number of columns in file");

  return params;
}

InputRotOrTransFromFile::InputRotOrTransFromFile(const InputParameters & parameters) :
    InputRotOrTransUserObject(parameters),
    _file_name(getParam<std::string>("file_name")),
    _t_data_column_index(getParam<unsigned int>("t_data_column_index")),
    _data_column_index(getParam<unsigned int>("data_column_index")),
    _num_column(getParam<unsigned int>("number_of_columns"))
{
  readFromFile();
}

Real
InputRotOrTransFromFile::getValue(Real time) const
{
  Real val = _time_ipol.sample(time);

  switch (_input_type)
  {
    case 0:
      switch (_unit_type)
      {
       case 0:
	 val = val * libMesh::pi/180.0;
	 break;
       case 1:
	 break;
       default:
	 break;
      }
    case 1:
      break;      
    default:
      break;
  }

  return val;
}

void
InputRotOrTransFromFile::readFromFile()
{
  MooseUtils::checkFileReadable(_file_name);
  
  std::ifstream file_read;
  file_read.open(_file_name.c_str());

  unsigned int column = 0;
  Real data;

  while (file_read >> data)
  {
    if (column == _t_data_column_index)
      _time_data.push_back(data);

    if (column == _data_column_index)
      _data.push_back(data);

    if (++column == _num_column)
      column = 0;
  }

  file_read.close();

  unsigned int nsize = _time_data.size();
  if (nsize == 0)
    mooseError("Invalid data provided in file. Try changing column index");

  _time_ipol.setData(_time_data, _data);
}



