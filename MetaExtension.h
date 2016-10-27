#ifndef METAEXTENSION_H
#define METAEXTENSION_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <assert.h>

using namespace std;

#define EXPAND_VA( x ) x

#define CONCAT_( L, R ) L ## R
#define CONCAT( L, R ) CONCAT_( L, R )

#define GET_FIELD_COUNT_IMPL( _0, _1, _2, _3, _4, N, ... ) N
#define GET_FIELD_COUNT( ... ) EXPAND_VA( GET_FIELD_COUNT_IMPL( __VA_ARGS__, 4, 3, 2, 1, 0 ) )

#define ADD_FIELD( name, field ) m.template visit< decltype( name::field ), &name::field > ( #field )

#define ADD_FIELDS_0( name, op )
#define ADD_FIELDS_1( name, op, field ) ADD_FIELD( name, field )
#define ADD_FIELDS_2( name, op, field, ... ) ADD_FIELD( name, field ) op EXPAND_VA( ADD_FIELDS_1( name, op, __VA_ARGS__ ) )
#define ADD_FIELDS_3( name, op, field, ... ) ADD_FIELD( name, field ) op EXPAND_VA( ADD_FIELDS_2( name, op, __VA_ARGS__ ) )
#define ADD_FIELDS_4( name, op, field, ... ) ADD_FIELD( name, field ) op EXPAND_VA( ADD_FIELDS_3( name, op, __VA_ARGS__ ) )

#define CONSTRUCT_FIELD( name, field ) m.template get< decltype( name::field ) > ( #field )

#define CONSTRUCT_FIELDS_0( name, op )
#define CONSTRUCT_FIELDS_1( name, field ) CONSTRUCT_FIELD( name, field )
#define CONSTRUCT_FIELDS_2( name, field, ... ) CONSTRUCT_FIELD( name, field ), EXPAND_VA( CONSTRUCT_FIELDS_1( name, __VA_ARGS__ ) )
#define CONSTRUCT_FIELDS_3( name, field, ... ) CONSTRUCT_FIELD( name, field ), EXPAND_VA( CONSTRUCT_FIELDS_2( name, __VA_ARGS__ ) )
#define CONSTRUCT_FIELDS_4( name, field, ... ) CONSTRUCT_FIELD( name, field ), EXPAND_VA( CONSTRUCT_FIELDS_3( name, __VA_ARGS__ ) )

#define ADD_FIELDS_( N, name, op, ... ) CONCAT( ADD_FIELDS_, N )( name, op, __VA_ARGS__ )
#define CONSTRUCT_FIELDS_( N, name, ... ) CONCAT( CONSTRUCT_FIELDS_, N )( name, __VA_ARGS__ )

#define DECLARE_( N, name, ... ) \
template< class TT > static bool visit_if( TT&& m ) { \
    return m.init( #name ) && ADD_FIELDS_( N, name, &&, __VA_ARGS__ ); \
} \
template< class TT > static void visit_all( TT&& m ) { \
    m.init( #name ); ADD_FIELDS_( N, name, ;, __VA_ARGS__ ); \
} \
template< class TT > static name construct( TT&& m ) { \
    return name{ CONSTRUCT_FIELDS_( N, name, __VA_ARGS__ ) }; \
}


#define DECLARE( ... ) EXPAND_VA( DECLARE_( GET_FIELD_COUNT( __VA_ARGS__ ), __VA_ARGS__ ) )

template< class T > struct Printer;
template< class T > struct Comparer;
struct JsonSerializer;
struct JsonValue;
template< class T, class Format > struct Serializer;

template< class T >
struct CoW {
    using Type = T;
    T s;

    void print() const {
        T::visit_all( Printer< T >( s ) );
    }

    bool is_equal_to( const T& other ) const {
        return &s == &other || T::visit_if( Comparer< T >( s, other ) );
    }

    string toJson() const {
        Serializer< T, JsonValue > serializer( s );
        T::visit_all( serializer );
        return serializer.result().toString();
    }

    static CoW< T > fromJson( string json ) {
        CoW< T > result;
        result.s = T::construct( JsonSerializer( json ) );
        return result;
    }

    T* operator->() {
        return &s;
    }

    operator const T&() const {
        return s;
    }
};

template< class R > struct is_CoW : false_type {};
template< class R > struct is_CoW< CoW< R > > : true_type {};

template< class T >
struct Printer {
    Printer( const T& ref, int level = 0 ) : ref_( ref ), level_( level ) {}

    void init( string name ) {
        cout << string( level_ * 4, ' ' ) << "Class: " << name << endl;
    }

    template< class R, R T::*m >
    typename enable_if< !is_CoW< R >::value >::type visit( string name ){
        cout << string( level_ * 4 + 2, ' ' ) << name << ": " << ref_.*m << endl;
    }

    template< class R, R T::*m >
    typename enable_if< is_CoW< R >::value >::type visit( string name ){
        cout << string( level_ * 4 + 2, ' ' ) << name << ":" << endl;
        auto&& member = ref_.*m;
        using RType = typename R::Type;
        RType::visit_all( Printer< RType >( member.s, level_ + 1 ) );
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

    template< class R, R T::*m >
    typename enable_if< !is_CoW< R >::value, bool >::type visit( string ){
        return lhs_.*m == rhs_.*m;
    }

    template< class R, R T::*m >
    typename enable_if< is_CoW< R >::value, bool >::type visit( string ){
        auto&& sub_lhs = lhs_.*m;
        auto&& sub_rhs = rhs_.*m;
        using RType = typename R::Type;
        return RType::visit_if( Comparer< RType >( sub_lhs, sub_rhs ) );
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


template< class R > struct is_container {
    typedef char YES[1];
    typedef char NO [2];
    template< class T > static YES& check( typename T::iterator* );
    template< class T > static NO & check( ... );
    enum {
        value = sizeof( check< R >( nullptr ) ) == sizeof( YES )
    };
};

#endif // METAEXTENSION_H
