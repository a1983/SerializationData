#include "MetaExtension.h"

#include <string>
#include <thread>

#include <chrono>
#include <time.h>

#include "JSONParser.h"

struct RecordData {
    DECLARE( RecordData, id, name )

    int id;
    string name;
    vector< int > data;
};

using Record = Object< RecordData >;

struct Record2Data {
    DECLARE( Record2Data, id, name, data )

    int id;
    string name;
    Record data;
};

using Record2 = Object< Record2Data >;

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
    t2().id = 42;
    assert( !t.is_equal_to( t2 ) );

    string json = t2.toJson();
    auto t_json = Record2::fromJson( json );
    assert( t_json.is_equal_to( t2 ) );
    assert( t2.is_equal_to( t_json ) );

    t.print();

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    
    int acc = 0;
    for( int i = 0; i < 10000000; ++i ) {
        Record test = t->data;
        acc += test->id;
        if( i % 10 ) {
            test().id += 1;
        }
    }

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

    string dummy; cin >> dummy;

    return 0;
}
