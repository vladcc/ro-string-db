#ifndef GENERIC_COMPAR_HPP
#define GENERIC_COMPAR_HPP

/*
	The point of this class is to express the various operators with a three
	way comparison function, so you can implement an actual binary search with
	context using stl. In reality, the whole ordeal amounts to copying two
	pointers, provided TContext is a pointer type.
*/

template <typename TValue, typename TContext = void*>
struct generic_compar
{
	typedef int(*fcompar)(const TValue& lhs_element,
		const TValue& rhs_element_or_context_val,
		TContext ctx
	);	
	
	generic_compar(fcompar compar =
		[](const TValue& lhs, const TValue& rhs, TContext ctx)
		{
			return ((lhs > rhs)-(lhs < rhs));
		},
		TContext context = TContext()
	) :
		_context(context),
		_compar(compar)
	{}
	
    virtual bool operator()(const TValue& lhs, const TValue& rhs) = 0;

    inline int three_way_cmp(const TValue& lhs,
		const TValue& rhs,
		TContext context
	)
	{return _compar(lhs, rhs, context);}
    
    inline int three_way_cmp(const TValue& lhs, const TValue& rhs)
	{return three_way_cmp(lhs, rhs, _context);}
    
    inline void set_context(TContext ctx)
    {_context = ctx;}
    
    protected:
    TContext _context;
    
    private:
    fcompar _compar;
};

template <typename TValue, typename TContext = void*>
struct gen_comp_less : public generic_compar<TValue, TContext>
{
    gen_comp_less() : generic_compar<TValue, TContext>() {}
    gen_comp_less(typename generic_compar<TValue, TContext>::fcompar compar,
		TContext context = TContext()
	) :
        generic_compar<TValue, TContext>(compar, context)
    {}

    inline bool operator()(const TValue& lhs, const TValue& rhs) override
    {return (this->three_way_cmp(lhs, rhs, this->_context) < 0);}
};

template <typename TValue, typename TContext = void*>
struct gen_comp_less_ctx_lower_bound : public gen_comp_less<TValue, TContext>
{
/*
	https://en.cppreference.com/w/cpp/algorithm/lower_bound
	
	The range [first, last) must be partitioned with respect to the expression
	element < value or comp(element, value)
	
	Finds the first element that *is not* less than the value.
	
	while (element < value)
		next_element();
	
	(element < value) == (comp(element, value) < 0)
*/
    gen_comp_less_ctx_lower_bound() : gen_comp_less<TValue, TContext>() {}
    gen_comp_less_ctx_lower_bound(
		typename generic_compar<TValue, TContext>::fcompar
		compar,
		TContext context = TContext()
	) :
        gen_comp_less<TValue, TContext>(compar, context)
    {}

    inline bool operator()(const TValue& lhs_elem,
		const TValue& rhs_val
	) override
    {return (this->three_way_cmp(lhs_elem, rhs_val, this->_context) < 0);}
};

template <typename TValue, typename TContext = void*>
struct gen_comp_less_ctx_upper_bound : public gen_comp_less<TValue, TContext>
{
/*
	https://en.cppreference.com/w/cpp/algorithm/upper_bound
	
	The range [first, last) must be partitioned with respect to the expression
	!(value < element) or !comp(value, element)
	
	Finds the first element which *is* greater than the value.
	
	while (!(value < element))
		next_element();
	
	!(value < element) == !(element > value) 
	!(element > value) == !(comp(element, value) > 0)
*/
    gen_comp_less_ctx_upper_bound() : gen_comp_less<TValue, TContext>() {}
    gen_comp_less_ctx_upper_bound(
		typename generic_compar<TValue, TContext>::fcompar compar,
		TContext context = TContext()
	) :
        gen_comp_less<TValue, TContext>(compar, context)
    {}

    inline bool operator()(const TValue& lhs_val,
		const TValue& rhs_elem
	) override
    {return (this->three_way_cmp(rhs_elem, lhs_val, this->_context) > 0);}
};

template <typename TValue, typename TContext = void*>
struct gen_comp_greater : public generic_compar<TValue, TContext>
{
    gen_comp_greater() : generic_compar<TValue, TContext>() {}
    gen_comp_greater(typename generic_compar<TValue, TContext>::fcompar compar,
		TContext context = TContext()
	) :
        generic_compar<TValue, TContext>(compar, context)
    {}

    inline bool operator()(const TValue& lhs, const TValue& rhs) override
    {return (this->three_way_cmp(lhs, rhs, this->_context) > 0);}
};


template <typename TValue, typename TContext = void*>
struct gen_comp_eq : public generic_compar<TValue, TContext>
{
    gen_comp_eq() : generic_compar<TValue, TContext>() {}
    gen_comp_eq(typename generic_compar<TValue, TContext>::fcompar compar,
		TContext context = TContext()
	) :
        generic_compar<TValue, TContext>(compar, context)
    {}
    
    inline bool operator()(const TValue& lhs, const TValue& rhs) override
    {return (this->three_way_cmp(lhs, rhs, this->_context) == 0);}
};

template <typename TValue, typename TContext = void*>
struct gen_comp_neq : public generic_compar<TValue, TContext>
{
    gen_comp_neq() : generic_compar<TValue, TContext>() {}
    gen_comp_neq(typename generic_compar<TValue, TContext>::fcompar compar,
		TContext context = TContext()
	) :
		generic_compar<TValue, TContext>(compar, context)
    {}

    inline bool operator()(const TValue& lhs, const TValue& rhs) override
    {return (this->three_way_cmp(lhs, rhs, this->_context) != 0);}
};
#endif
