#ifndef METAEXTENSION_H
#define METAEXTENSION_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include <assert.h>

#include "CoW.h"

#include "MetaImplementation.h"

using namespace std;

template< class R > struct is_iterable {
    typedef char YES[ 1 ];
    typedef char NO[ 2 ];
    template< class T > static YES& check( void*, typename T::iterator* = nullptr );
    template< class T > static NO & check( ... );
    enum {
        value = sizeof( check< R >( nullptr ) ) == sizeof( YES )
    };
};

template< bool is, class T = void >
using if_t = typename enable_if< is, T >::type;

template< class T, class R >
using if_iterable_t = if_t< is_iterable< T >::value, R >;

template< class T, class R >
using if_not_iterable_t = if_t< !is_iterable< T >::value, R >;

template< class T > struct Printer;
template< class T > struct Comparer;
template< class T, class Format > struct Serializer;
class JsonParser;
struct JsonValue;

template< class T >
struct Object : public CoW< T > {
    Object() = default;
    
    Object( const Object& other ) = default;
    Object& operator=( const Object& other ) = default;

    Object( T&& value )
        : CoW< T >( std::move( value ) )
    {
        ;
    }

    template< class M0, class... M >
    Object( M0 arg0, M... args )
        : CoW< T >( T{ arg0, args... } )
    {
        ;
    }
    
    void print( int level = 0 ) const {
        T::visit_all( Printer< T >( const_data(), level ) );
    }

    bool is_equal_to( const T& other ) const {
        return &const_data() == &other || T::visit_if( Comparer< T >( const_data(), other ) );
    }

    string toJson() const {
        Serializer< T, JsonValue > serializer( const_data() );
        T::visit_all( serializer );
        return serializer.result().toString();
    }

    static Object fromJson( string json ) {
        return Object( T::construct( JsonParser( json ).parse() ) );
    }
};

template< class T >
struct Printer {
    Printer( const T& ref, int level ) : ref_( ref ), level_( level ) {}

    void init( string name ) {
        cout << string( level_ * 4, ' ' ) << name << ": " << endl;
    }

    template< class R >
    struct ObjectPrinter {
        static void print( const string& name, const R& value, int level ) {
            cout << string( level * 4 + 2, ' ' ) << name << ": " << value << endl;
        }
    };

    template< class R >
    struct ObjectPrinter< Object< R > > {
        static void print( const string& name, const Object< R >& value, int level ) {
            value.print( level + 1 );
        }
    };

    template< class R, R T::*m >
    void visit( string name ){
        ObjectPrinter< R >::print( name, ref_.*m, level_ );
    }

    const T& ref_;
    int level_;
};

template< class T >
struct Comparer {
    Comparer( const T& lhs, const T& rhs ) : lhs_( lhs ), rhs_( rhs ) {}

    bool init( string ) {
        return true;
    }

    template< class R >
    struct ObjectComparer {
        template< class T >
        static if_not_iterable_t< T, bool > compare( const T& lhs, const T& rhs ) {
            return lhs == rhs;
        }

        template< class T >
        static if_iterable_t< T, bool > compare( const T& lhs, const T& rhs ) {
            T::const_iterator li = begin( lhs ), le = end( lhs );
            T::const_iterator ri = begin( rhs ), re = end( rhs );
            while( li != le ) {
                if( ri == re || *li != *ri ) {
                    return false;
                }
                ++li;
                ++ri;
            }

            return ri == re;
        }
    };

    template< class R >
    struct ObjectComparer< Object< R > > {
        static bool compare( const Object< R >& lhs, const Object< R >& rhs ) {
            return lhs.is_equal_to( rhs );
        }
    };

    template< class R, R T::*m >
    bool visit( string ){
        return ObjectComparer< R >::compare( lhs_.*m, rhs_.*m );
    }

    const T& lhs_;
    const T& rhs_;
};

template< class T, class Format >
struct Serializer {
    Serializer( const T& ref ) : ref_( ref ), format_( Format::createRoot() ) {}

    void init( string ) {
        ;
    }

    template< class R, R T::*m >
    void visit( string name ){
        format_.set( name, ref_.*m );
    }

    Format result() const { return format_; }

    const T& ref_;
    Format format_;
};

#endif // METAEXTENSION_H
