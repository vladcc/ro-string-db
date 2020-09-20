#ifndef STRING_POOL_HPP
#define STRING_POOL_HPP

#include <vector>
#include <string>

class string_pool
{
    public:
    typedef unsigned int uint;
    typedef unsigned char byte;

    inline string_pool(size_t size = 0)
    {_pool.reserve(size);}

	inline uint append(const std::string& str)
    {return append(str.c_str());}

	inline uint append(const char * str)
	{
		uint start = _pool.size();
		for (char ch = *str; ch; ch = *(++str))
			_pool.push_back(ch);
		_pool.push_back('\0');
		return start;
	}
	
    inline const char * get(uint index) const
	{return reinterpret_cast<const char *>(_pool.data() + index);}

    inline void reserve_chars(size_t how_many)
	{_pool.reserve(how_many);}

	inline void shrink_to_fit()
	{_pool.shrink_to_fit();}

	inline size_t size() const
	{return _pool.size();}

    private:
    std::vector<byte> _pool;
};
#endif
