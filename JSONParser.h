#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <assert.h>
#include <list>
#include <map>
#include <string>

template< class T > struct CoW;

class JsonLexer {
public:
    enum TokenType {
        TT_UNKNOWN,
        TT_ERROR,
        TT_EOF,

        TT_STRING,
        TT_NUMBER,

        TT_TRUE,
        TT_FALSE,
        TT_NULL,

        TT_LEFT_CURLY = '{',
        TT_RIGHT_CURLY = '}',

        TT_LEFT_SQUARE = '[',
        TT_RIGHT_SQUARE = ']',

        TT_COLON = ':',
        TT_COMMA = ','
    };

    struct Token {
        TokenType type;
        std::string::const_iterator b;
        std::string::const_iterator e;

        std::string toString() const { return std::string( b, e ); }
        std::string toUnquotedString() const { return std::string( b + 1, e - 1 ); }
    };

    JsonLexer( const std::string& s )
        : b_ ( s.begin() )
        , i_ ( s.begin() )
        , e_ ( s.end() )
    {}

    Token next() {
        skipSpaces();

        if( i_ == e_ ) {
            return Token { TT_EOF };
        }

        const char cur = *i_;
        switch( cur ) {
        case TT_LEFT_CURLY :
        case TT_RIGHT_CURLY :
        case TT_LEFT_SQUARE :
        case TT_RIGHT_SQUARE :
        case TT_COLON :
        case TT_COMMA :
            ++i_;
            return Token { TokenType( cur ) };
        case '"' :
            return tryGetString();
        case 't' :
            return tryGet( "true", TT_TRUE );
        case 'f' :
            return tryGet( "false", TT_FALSE );
        case 'n' :
            return tryGet( "null", TT_NULL );
        default :
            if( cur >= 0 || cur <= 9 || cur == '-' ) {
                return tryGetNumber();
            }
        }

        return Token { TT_ERROR };
    }

    void skipSpaces() {
        while( i_ != e_ && isspace( *i_ ) ) ++i_;
    }

    Token tryGetString() {
        std::string::const_iterator begin = i_;
        while( ++i_ != e_ ) {
            if( *i_ == '"' ) {
                return Token { TT_STRING, begin, ++i_ };
            }
        }
        return Token{ TT_ERROR, begin, i_ };
    }

    Token tryGet( const char* text, TokenType token ) {
        std::string::const_iterator begin = i_ + 1;
        const char* c = text + 1;
        do {
            if( i_ == e_ ) {
                return Token{ TT_ERROR, begin, i_ };
            }

            if( *i_ != *c ) {
                return Token{ TT_ERROR, begin, i_ };
            }
        } while( ++i_, ++c );

        return Token{ token, begin, i_++ };
    }

    void consumeDigits()
    {
        while( ++i_ != e_ ) {
            if( !isdigit( *i_ ) ) {
                break;
            }
        }
    }

    bool consumeDigitsWithPoint() {
        consumeDigits();
        if( *i_ == '.' ) {
            ++i_;
            if( !isdigit( *i_ ) ) {
                return false;
            }
            consumeDigits();
        }
        return true;
    }

    Token tryGetNumber() {
        std::string::const_iterator begin = i_;

        if( !consumeDigitsWithPoint() ) {
            return Token{ TT_ERROR, begin, i_ };
        }

        if( *i_ == 'e' || *i_ == 'E' ) {
            ++i_;
            if( *i_ == '-' || *i_ == '+' ) {
                ++i_;
            }
            if( !isdigit( *i_ ) ) {
                return Token{ TT_ERROR, begin, i_ };
            }
            consumeDigitsWithPoint();
        }

        return Token{ TT_NUMBER, begin, i_ };
    }

    bool hasNext() const {
        return i_ != e_;
    }

    int currentPos() const {
        return i_ - b_;
    }

private:
    std::string::const_iterator b_;
    std::string::const_iterator i_;
    std::string::const_iterator e_;
};

struct JsonValue {
    enum Type {
        Invalid,
        Null,
        Boolean,
        Number,
        String,
        Array,
        Object
    };

    JsonValue( Type type = Invalid )
        : type_( type )
    {}

    JsonValue( Type type, std::string s )
        : type_( type )
        , string_( s )
    {}

    Type type_ = Invalid;

    std::string string_;
    std::list< JsonValue > array_;
    std::map< string, JsonValue > object_;

    bool isValid() const { return type_ != Invalid; }

    void insert( std::string key, JsonValue value ) {
        assert( object_.find( key ) == object_.end() );
        object_[ key ] = value;
    }

    void append( JsonValue value ) {
        array_.push_back( value );
    }

    template< class T >
    struct ObjectGetter;

    template<>
    struct ObjectGetter< int > { static int get( JsonValue& value ) {
        return std::stoi( value.string_ );
    } };

    template<>
    struct ObjectGetter< std::string > { static std::string get( JsonValue& value ) {
        return value.string_;
    } };

    template< class T >
    struct ObjectGetter< CoW< T > > { static CoW< T > get( JsonValue& value ) {
        CoW< T > result;
        result.s = T::construct( JsonDeserializer< T >( value ) );
        return result;
    } };

    template< class T >
    T get( std::string key ){
        return ObjectGetter< T >::get( object_[ key ] );
    }
};

class JsonParser {
public:
    JsonParser( std::string json )
        : json_( json )
    {}

    JsonValue parse() {
        JsonLexer lexer( json_ );
        if( lexer.hasNext() ) {
            JsonLexer::Token next = lexer.next();
            switch( next.type ) {
                case JsonLexer::TT_LEFT_CURLY: return tryGetObject( lexer );
                case JsonLexer::TT_LEFT_SQUARE: return tryGetArray( lexer );
            }
        }
        return JsonValue();
    }

    JsonValue tryGetObject( JsonLexer& lexer ) {
        if( !lexer.hasNext() ) {
            return JsonValue();
        }

        JsonValue object{ JsonValue::Object };

        do {
            JsonLexer::Token next = lexer.next();
            switch( next.type ) {
                case JsonLexer::TT_RIGHT_CURLY:
                    return object;

                case JsonLexer::TT_COMMA:
                    // check object has key value pair

                    if( lexer.hasNext() )
                        next = lexer.next();

                    if( next.type != JsonLexer::TT_STRING ) {
                        return JsonValue();
                    }

                case JsonLexer::TT_STRING: {
                    if( !lexer.hasNext() || lexer.next().type != JsonLexer::TT_COLON ) {
                        return JsonValue();
                    }

                    JsonValue value = tryGetValue( lexer );
                    if( !value.isValid() ) {
                        return value;
                    }

                    std::string key( next.toUnquotedString() );
                    object.insert( key, value );
                    break;
                }
            }
        } while( lexer.hasNext() );

        return JsonValue{};
    }

    JsonValue tryGetArray( JsonLexer& lexer ) {
        if( !lexer.hasNext() ) {
            return JsonValue();
        }

        JsonValue array{ JsonValue::Array };

        do {
            JsonLexer::Token next = lexer.next();

            switch( next.type ) {
                case JsonLexer::TT_RIGHT_SQUARE:
                    return array;

                case JsonLexer::TT_COMMA:
                    // check object has key value pair

                    if( lexer.hasNext() )
                        next = lexer.next();

                default: {
                    JsonValue value = tryGetValue( next, lexer );
                    if( !value.isValid() ) {
                        return value;
                    }

                    array.append( value );
                    break;
                }
            }
        } while( lexer.hasNext() );

        return JsonValue{};
    }

    JsonValue tryGetValue( JsonLexer& lexer ) {
        if( !lexer.hasNext() ) {
            return JsonValue();
        }

        return tryGetValue( lexer.next(), lexer );
    }

    JsonValue tryGetValue( JsonLexer::Token next, JsonLexer& lexer ) {
        switch( next.type ) {
            case JsonLexer::TT_NULL:   return JsonValue( JsonValue::Null );
            case JsonLexer::TT_FALSE:  return JsonValue( JsonValue::Boolean );
            case JsonLexer::TT_TRUE:   return JsonValue( JsonValue::Boolean, "1" );
            case JsonLexer::TT_NUMBER: return JsonValue( JsonValue::Number, next.toString() );
            case JsonLexer::TT_STRING: return JsonValue( JsonValue::String, next.toUnquotedString() );
            case JsonLexer::TT_LEFT_CURLY: return tryGetObject( lexer );
            case JsonLexer::TT_LEFT_SQUARE: return tryGetArray( lexer );
        default:
            break;
        }

        return JsonValue();
    }

    bool checkCounter() {
        return true;
    }

private:
    std::string json_;
};

template< class T >
struct JsonDeserializer {
    JsonDeserializer( std::string json )
        : json_( JsonParser( json ).parse() )
    {}

    JsonDeserializer( JsonValue json )
        : json_( json )
    {}

    template< class R, R T::*m >
    R get( std::string key ) {
        return json_.get< R >( key );
    }

    JsonValue json_;
};

template< class T >
struct JsonSerializer {
    JsonSerializer( std::string json )
        : json_( JsonParser( json ).parse() )
    {}

    JsonSerializer( JsonValue json )
        : json_( json )
    {}

    template< class R, R T::*m >
    R get( std::string key ) {
        return json_.get< R >( key );
    }

    JsonValue json_;
};

#endif // JSONPARSER_H
