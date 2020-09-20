#ifndef SORT_VECTOR_HPP
#define SORT_VECTOR_HPP

#include "generic_compar.ipp"

#include <vector>
#include <stdexcept>
#include <algorithm>

#define throw_str(str) "sort_vector: " str

template <typename T, typename TContextLookup = void*>
class sort_vector
{
	/*
	   This vector implementation supports context lookups. I.e., the user
	   can pass a pointer to something, which they can then access from within
	   a comparison function. This is needed when your lookup requires
	   indirection. This vector is *not* kept sorted on insertion. Rather data
	   is appended to it, and when done, the user calls seal(). This calls
	   shrink_to_fit(), calls sort(), and marks the vector as sorted. If
	   append() is called after seal(), the vector is marked as unsorted again.
	   Calling lookup() or equal_range() on an unsorted vector results in an
	   exception.
	*/
    public:
    struct equal_range_ctx_compars
    {
		equal_range_ctx_compars(
			gen_comp_less_ctx_lower_bound<T, TContextLookup>& lower_bound_cmp,
			gen_comp_less_ctx_upper_bound<T, TContextLookup>& upper_bound_cmp,
			TContextLookup context_val
		) :
			lower_bound_cmp(lower_bound_cmp),
			upper_bound_cmp(upper_bound_cmp),
			context_val(context_val)
		{}
		
		void change_context(TContextLookup new_ctx_val)
		{context_val = new_ctx_val;}
		
		void set_context()
		{
			lower_bound_cmp.set_context(context_val);
			upper_bound_cmp.set_context(context_val);
		}
		
		gen_comp_less_ctx_lower_bound<T, TContextLookup>& lower_bound_cmp;
		gen_comp_less_ctx_upper_bound<T, TContextLookup>& upper_bound_cmp;
		
		private:
		TContextLookup context_val;
	};
    /*
       equal_range_ctx_compars is needed because stl implements upper and lower
       bound in terms of only operator <. This means that the places of the
       element from your container and the element you pass to the algorithm
       switch. This, in turn, means the user would need two different comparison
       functions - one for each bound. The difference between
       gen_comp_less_ctx_lower_bound and gen_comp_less_ctx_upper_bound is that
       gen_comp_less_ctx_upper_bound switches the order of the elements on
       behalf of the user, so only a single function can be used for finding
       both bounds.
    */
    
    sort_vector(gen_comp_less<T, TContextLookup> compar) :
        _compar(compar),
        _sorted(false)
    {}

    void append(const T& what)
    {
        _vect.push_back(what);
        _sorted = false;
    }

    void seal()
    {
        _vect.shrink_to_fit();
        std::sort(_vect.begin(), _vect.end(), _compar);
        _sorted = true;
    }
	
	bool equal_range(const T& dummy,
		std::pair<size_t, size_t>& out,
		equal_range_ctx_compars& compars
	)
	{return _equal_range(dummy, out, compars);}
	/*
	   Performs a lower bound and an upper using the comparison classes in
	   compars. Returns true if there is a valid range. The start and end of
	   the range are returned in out. Note that equal range can only be
	   performed as a context lookup. The value to look for is given to compars
	   before passing it to the function. However, the const T& dummy is still
	   needed, because the stl bound functions expect it. This has to be
	   provided by the user, since no assumptions are made about T being default
	   constructable.
	*/
	
    bool lookup(const T& what, const T ** out)
    {return _lookup(what, out, _compar);}
	/*
	   Does a binary search for what. Returns true if it's found and *out points
	   to the result. Note that this function uses lower_bound(), which in turn
	   returns the first element which is not less than what. In order to know
	   if it's actually equal to what, the comparison function from _compar is
	   called.
	*/
	
    bool lookup(const T& dummy,
		const T ** out,
		gen_comp_less_ctx_lower_bound<T, TContextLookup>& lower_bound_cmp
	)
    {return _lookup(dummy, out, lower_bound_cmp);}
	/*
	   Same as above, but the value of dummy is ignored and lower_bound_cmp
	   is used instead of _compar.
	*/

    void reserve(size_t how_many)
    {_vect.reserve(how_many);}

    const T& get(int index) const
    {return _vect[index];}

    size_t size() const
    {return _vect.size();}

    private:
    bool _equal_range(const T& what,
		std::pair<size_t, size_t>& out,
		equal_range_ctx_compars& compars
	)
	{
		if (_sorted)
		{
			auto begin = _vect.begin();
			auto end = _vect.end();
			
			compars.set_context();
			auto lower = std::lower_bound(begin, end, what,
				compars.lower_bound_cmp
			);
			auto upper = std::upper_bound(begin, end, what,
				compars.upper_bound_cmp
			);
			
			out.first = (lower - begin);
			out.second = (upper - begin);
			
			return (lower != upper);
		}
		else
			_throw(throw_str("equal_range on unsorted data"));
			
		return false; // make gcc happy
	}
    
    bool _lookup(const T& what,
		const T ** out,
		gen_comp_less<T, TContextLookup>& compar
	)
    {
		if (_sorted)
		{
			bool ret = false;
			
			auto begin = _vect.begin();
			auto end = _vect.end();

			auto found = std::lower_bound(begin, end, what, compar);
			if (found != end && (compar.three_way_cmp(*found, what) == 0))
			{
				*out = &(*found);
				ret = true;
			}
			
			return ret;
		}
		else
			_throw(throw_str("lookup on unsorted data"));
			
		return false; // make gcc happy
    }

	void _throw(const char * str)
	{throw std::runtime_error(str);}

	std::vector<T> _vect;
	gen_comp_less<T, TContextLookup> _compar;
    bool _sorted;
};

#undef throw_str
#endif
