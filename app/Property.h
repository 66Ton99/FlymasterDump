#pragma once

#include <QObject>
#include <type_traits>

template < class T > class Property;

class AbstractProperty : public QObject {
    Q_OBJECT
public:
    template < class T > const T& as() const;
signals:
    void propertyChanged( AbstractProperty* sender );
};


template < class T > class Property : public AbstractProperty {
public:
    Property() = default;

    Property( const Property< T >& other ) : value_( other() ) {
    }

    Property( Property< T >&& other ) : value_( std::move( other.value_ ) ) {
    }

    Property( const T& value ) : value_( value ) {
    }

    Property( T&& value ) : value_( std::move( value ) ) {
    }

    const T& operator()() const {
        return value_;
    }

    void operator=( const Property< T >& other ) {
        ( *this ) = other();
    }

    void operator=( Property< T >&& other ) {
        ( *this ) = std::move( other() );
    }

    void operator=( const T& value ) {
        if( value == value_ ) {
            return;
        }
        value_ = value;
        emit propertyChanged( this );
    }

    void operator=( T&& value ) {
        if( value == value_ ) {
            return;
        }
        value_ = std::move( value );
        emit propertyChanged( this );
    }

    bool operator==( const Property< T >& other ) const {
        return value_ == other();
    }

    bool operator==( const T& value ) const {
        return value_ == value;
    }

    bool operator!=( const Property< T >& other ) const {
        return !this->operator==( other );
    }

    bool operator!=( const T& value ) const {
        return !this->operator==( value );
    }

private:
    T value_;
};

template < class T >
inline const T& AbstractProperty::as() const {
    return dynamic_cast< const Property< T >* >( this )->operator()();
}

template < typename T >
static inline QMetaObject::Connection connectProperty( const Property< T >& a, Property< T >& b ) {
    // initialize values;
    b = a;
    return connectProperty( a, &b, [&]( AbstractProperty* sender ) { b = sender->as< T >(); } );
}

template < typename T, typename Function >
inline QMetaObject::Connection connectProperty(
    const Property< T >& prop,
    typename QtPrivate::FunctionPointer< Function >::Object* receiver,
    Function slot,
    Qt::ConnectionType type = Qt::ConnectionType::AutoConnection,
    typename std::enable_if< QtPrivate::FunctionPointer< Function >::ArgumentCount == 1 >::type* = 0,
    typename std::enable_if< std::is_member_function_pointer< Function >::value >::type* = 0 ) {
    return QObject::connect( &prop,
                             &Property< T >::propertyChanged,
                             receiver,
                             [slot, receiver]( AbstractProperty* sender ) { ( receiver->*slot )( sender->as< T >() ); },
                             type );
}

template < typename T, typename Function >
inline QMetaObject::Connection connectProperty(
    const Property< T >& prop,
    typename QtPrivate::FunctionPointer< Function >::Object* receiver,
    Function slot,
    Qt::ConnectionType type = Qt::ConnectionType::AutoConnection,
    typename std::enable_if< QtPrivate::FunctionPointer< Function >::ArgumentCount == 1 >::type* = 0,
    typename std::enable_if< !std::is_member_function_pointer< Function >::value >::type* = 0 ) {
    return QObject::connect( &prop,
                             &Property< T >::propertyChanged,
                             receiver,
                             [slot, receiver]( AbstractProperty* sender ) { ( *slot )( sender->as< T >() ); },
                             type );
}

template < typename T, typename Receiver, typename Function >
inline QMetaObject::Connection connectProperty(
    const Property< T >& prop,
    Receiver receiver,
    Function slot,
    Qt::ConnectionType type = Qt::ConnectionType::AutoConnection,
    typename std::enable_if< QtPrivate::FunctionPointer< Function >::ArgumentCount != 1 >::type* = 0 ) {
    return QObject::connect( &prop, &Property< T >::propertyChanged, receiver, slot, type );
}
