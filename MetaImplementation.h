#ifndef MetaImplementation_H
#define MetaImplementation_H

#define MD_EXP( x ) x

#define MD_CONCAT_( L, R ) L ## R
#define MD_CONCAT( L, R ) MD_CONCAT_( L, R )

#define MD_GET_FIELD_COUNT_IMPL( _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ... ) N
#define MD_GET_FIELD_COUNT( ... ) MD_EXP( MD_GET_FIELD_COUNT_IMPL( __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 ) )

#define MD_FOR_EACH_0( fi, fe, ... )
#define MD_FOR_EACH_1( fi, fe, n, f, ... ) fe( n, f )
#define MD_FOR_EACH_2( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_1( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_3( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_2( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_4( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_3( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_5( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_4( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_6( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_5( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_7( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_6( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_8( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_7( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_9( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_8( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_10( fi, fe, n, f, ... ) fi( n, f ) MD_EXP( MD_FOR_EACH_9( fi, fe, n, __VA_ARGS__ ) )
#define MD_FOR_EACH_N( N, fi, fe, n, ... ) MD_CONCAT( MD_FOR_EACH_, N )MD_EXP( ( fi, fe, n, __VA_ARGS__ ) )

#define MD_VISIT_ALL_E( name, field ) m.template visit< decltype( name::field ), &name::field > ( #field )
#define MD_VISIT_ALL_I( name, field ) MD_VISIT_ALL_E( name, field );
#define MD_VISIT_ALL( N, name, ... ) MD_FOR_EACH_N( N, MD_VISIT_ALL_I, MD_VISIT_ALL_E, name, __VA_ARGS__ )

#define MD_VISIT_IF_I( name, field ) MD_VISIT_ALL_E( name, field ) &&
#define MD_VISIT_IF( N, name, ... ) MD_FOR_EACH_N( N, MD_VISIT_IF_I, MD_VISIT_ALL_E, name, __VA_ARGS__ )

#define MD_CONSTRUCT_FIELD_E( name, field ) m.template get< decltype( name::field ) > ( #field )
#define MD_CONSTRUCT_FIELD_I( name, field ) MD_CONSTRUCT_FIELD_E( name, field ),
#define MD_CONSTRUCT_FIELD_N( N, name, ... ) MD_FOR_EACH_N( N, MD_CONSTRUCT_FIELD_I, MD_CONSTRUCT_FIELD_E, name, __VA_ARGS__ )


#define MD_DECLARE_N( N, name, ... ) \
template< class TT > static bool visit_if( TT&& m ) { \
    return m.init( #name ) && MD_VISIT_IF( N, name, __VA_ARGS__ ); \
} \
template< class TT > static void visit_all( TT&& m ) { \
    m.init( #name ); MD_VISIT_ALL( N, name, __VA_ARGS__ ); \
} \
template< class TT > static name construct( TT&& m ) { \
    return name{ MD_CONSTRUCT_FIELD_N( N, name, __VA_ARGS__ ) }; \
}

#define MD_DECLARE( ... ) MD_EXP( MD_DECLARE_N( MD_GET_FIELD_COUNT( __VA_ARGS__ ), __VA_ARGS__ ) )

#endif // MetaImplementation_H