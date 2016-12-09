#include "MetaExtension.h"

#include <string>
#include <thread>

#include <chrono>
#include <time.h>

#include "JSONParser.h"

struct RecordData {
    int id;
    string name;
    vector< int > data;

    MD_DECLARE( RecordData, id, name )
};

using Record = Object< RecordData >;

struct Record2Data {
    int id;
    string name;
    Record data;

    MD_DECLARE( Record2Data, id, name, data )
};

using Record2 = Object< Record2Data >;

struct AccountData {
    bool isOpened;
    int value;

    MD_DECLARE( AccountData, isOpened, value );
};

using Account = Object< AccountData >;

struct AccountStorageData {
    int id;
    vector< Account > accounts;

    MD_DECLARE( AccountStorageData, id, accounts );
};

using AccountStorage = Object< AccountStorageData >;

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
    
    int acc = 0;
    for( int i = 0; i < 100; ++i ) {
        Record test;
        test = t->data;
        acc += test->id;
        if( i % 10 ) {
            test().id += 1;
        }
    }

    AccountStorage storage{ 0, vector< Account > {
            Account{ true, 100 },
            Account{ true, 50 }
        }
    };

    string dummy; std::cin >> dummy;

    return 0;
}