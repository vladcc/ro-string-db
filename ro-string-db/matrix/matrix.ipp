#ifndef MATRIX_IPP
#define MATRIX_IPP

#include <vector>

template <typename T>
class matrix
{
    public:
		typedef unsigned int uint;
		
        inline matrix(uint rows, uint cols) :
			_memory(rows*cols),
			_height(rows),
			_width(cols)
        {}

        inline T& get(uint row, uint col)
        {return _memory[_index(row, col)];}
        
        inline void place(uint row, uint col, const T& what)
        {_memory[_index(row, col)] = what;}

		inline uint get_rows()
		{return _height;}
		
		inline uint get_cols()
		{return _width;}

	protected:
		inline std::vector<T>& expose_memory()
		{return _memory;}

    private:
        inline uint _index(uint row, uint col)
        {return row*_width+col;}

        std::vector<T> _memory;
        uint _height;
        uint _width;
};
#endif
