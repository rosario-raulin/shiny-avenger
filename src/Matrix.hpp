#ifndef HAVE_MATRIX_HPP
#define HAVE_MATRIX_HPP

#include <boost/scoped_array.hpp>
#include <cstring>

class Matrix {
public:
	Matrix(size_t rows, size_t columns) :
		_values(new size_t[rows * columns]),
		_row_size(rows),
		_column_size(columns) {
		// we initialize everything to 0
		memset(&(_values[0]), 0, rows * columns * sizeof(size_t));
	}
	
	Matrix(const Matrix& other) :
		_values(new size_t[other._row_size * other._column_size]),
		_row_size(other._row_size),
		_column_size(other._column_size) {
		memcpy(&(_values[0]), &(other._values[0]), sizeof(size_t) * _row_size * _column_size);
	}
	
	size_t get(size_t row_index, size_t column_index) const {
		return _values[row_index * _column_size + column_index];
	}
	
	size_t& get(size_t row_index, size_t column_index) {
		return _values[row_index * _column_size + column_index];
	}
	
	void set(size_t row_index, size_t column_index, size_t value) {
		_values[row_index * _column_size + column_index] = value;
	}
	
	size_t getRowSize() const {
		return _row_size;
	}
	
	size_t getColumnSize() const {
		return _column_size;
	}
	
private:
	boost::scoped_array<size_t> _values;
	size_t _row_size;
	size_t _column_size;
};

#endif
