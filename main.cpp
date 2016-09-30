#include "MetaExtension.h"

#include <string>

#include "JSONParser.h"

struct RecordData {
    DECLARE( RecordData, id, name )

    int id;
    string name;
    vector< int > data;
};

using Record = CoW< RecordData >;

struct Record2Data {
    DECLARE( Record2Data, id, name, data )

    int id;
    string name;
    Record data;
};

using Record2 = CoW< Record2Data >;

int main( int /*argc*/, char* /*argv*/[] ) {
    auto t = Record2::fromJson(
                "{"
                "  \"id\":1,"
                "  \"name\": \"r1\""
                "  \"data\": {"
                "    \"id\": 0,"
                "    \"name\": \"r0\""
                "  }"
                "}" );

    auto t2 = t;

    assert( t.is_equal_to( t ) );
    assert( t.is_equal_to( t2 ) );

    t->id = 3;
    assert( !t.is_equal_to( t2 ) );

    t.print();

    return 0;
}
