#ifndef HAVE_MATRIX_HPP
#define HAVE_MATRIX_HPP

class Matrix {
public:
	Matrix(size_t rows, size_t columns) :
		_values(new size_t[rows * columns]),
		_row_size(rows),
		_column_size(columns) {	
	}
	
	~Matrix() {
		delete[] _values;
	}
	
	size_t get(size_t row_index, size_t column_index) const {
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
	size_t* _values;
	size_t _row_size;
	size_t _column_size;
};

#endif
