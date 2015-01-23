#ifndef HAVE_MATRIX_HPP
#define HAVE_MATRIX_HPP

#include <memory>

template<class T>
class Matrix {
public:
	Matrix(std::size_t rows, std::size_t columns) :
		_rows(rows),
		_columns(columns),
		_values(new T[rows * columns]()){
	}
	
	T& get(std::size_t row, std::size_t column) const {
		return _values[row * _columns + column];
	}
	
	T& get(std::size_t row, std::size_t column) {
		return _values[row * _columns + column];
	}
	
	template<class ValueType>
	void set(std::size_t row, std::size_t column, ValueType&& value) {
		_values[row * _columns + column] = std::forward<ValueType>(value);
	}
	
	std::size_t rows() const {
		return _rows;
	}
	
	std::size_t columns() const {
		return _columns;
	}
	
private:
	const std::size_t _rows;
	const std::size_t _columns;
	std::unique_ptr<T[]> _values;
};

#endif
