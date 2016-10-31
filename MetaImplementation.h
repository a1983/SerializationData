#ifndef MetaImplementation_H
#define MetaImplementation_H

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

#endif // MetaImplementation_H