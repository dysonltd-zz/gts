#ifndef CALLBACK_H_
#define CALLBACK_H_

template < class ReturnType >
class Callback
{
public:
    virtual ~Callback() {}
    virtual ReturnType operator() () = 0;
};

template < class ReturnType, class CalleeType >
class CallbackOnClass : public Callback< ReturnType >
{
public:
    typedef ReturnType ( CalleeType::* CalleeMemberFunctionType )();

    CallbackOnClass( CalleeType* const callee, CalleeMemberFunctionType func )
        :
        m_callee( callee ),
        m_func  ( func )
    {
    }


    virtual ReturnType operator() ()
    {
        return ( m_callee->*m_func )();
    }

private:
    CalleeType*              m_callee;
    CalleeMemberFunctionType m_func;

};


template < class ReturnType, class ArgType >
class Callback_1
{
public:
    virtual ~Callback_1() {}
    virtual ReturnType operator() ( ArgType arg ) = 0;
};

template < class ReturnType, class ArgType, class CalleeType >
class Callback_1OnClass : public Callback_1< ReturnType, ArgType >
{
public:
    typedef ReturnType ( CalleeType::* CalleeMemberFunctionType )( ArgType arg );

    Callback_1OnClass( CalleeType* const callee, CalleeMemberFunctionType func )
        :
        m_callee( callee ),
        m_func  ( func )
    {
    }

    virtual ReturnType operator() ( ArgType arg )
    {
        return ( m_callee->*m_func )( arg );
    }

private:
    CalleeType*              m_callee;
    CalleeMemberFunctionType m_func;

};


template < class ReturnType, class CalleeType >
Callback< ReturnType >* const MakeCallback( CalleeType* const callee, ReturnType ( CalleeType::* func )() )
{
    return new CallbackOnClass< ReturnType, CalleeType >( callee, func );
}

template < class ReturnType, class ArgType, class CalleeType >
Callback_1< ReturnType, ArgType >* const MakeCallback( CalleeType* const callee, ReturnType ( CalleeType::* func )( ArgType ) )
{
    return new Callback_1OnClass< ReturnType, ArgType, CalleeType >( callee, func );
}

#endif /* CALLBACK_H_ */