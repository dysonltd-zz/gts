#ifndef CONTAINER_H_
#define CONTAINER_H_

#include <algorithm>

template< class ContainerType >
class Container
{
public:
    Container( const ContainerType& container )
    :
    m_container( container )
    {
    }

    Container( const Container& other )
    :
        m_container( other.m_container )
    {
    }

    ~Container()
    {
    }

    template< class ElementType >
    bool contains( const ElementType& element ) const
    {
        if ( std::find( m_container.begin(), m_container.end(), element ) != m_container.end() )
        {
            return true;
        }

        return false;
    }

private:
    Container& operator =( const Container& );

    const ContainerType& m_container;
};

template< class ContainerType >
const Container< ContainerType > container( const ContainerType& baseContainer )
{
    return Container< ContainerType >( baseContainer );
}

#endif /* Container_H_ */
