#ifndef CoW_H
#define CoW_H

#include <atomic>

template< class T >
class CoW {
    class Holder {
        std::atomic< int > ref_;

    public:
        T data_;

        Holder() : ref_( 1 ) {
        }

        Holder( const T& data ) : ref_( 1 ), data_( data ) {
        }

        ~Holder()
        {
        }

        void ref() {
            ++ref_;
        }

        bool deref() {
            return --ref_ > 0;
        }

        bool is_detached() const {
            return ref_ == 1;
        }

        Holder* copy() {
            return new Holder{ data_ };
        }
    };

public:
    using Type = T;

    CoW()
        : holder_( new Holder() )
    {
        ;
    }

    ~CoW() {
        deref();
    }

    CoW( const CoW& other )
        : holder_( other.holder_ )
    {
        ref();
    }

    CoW& operator=( const CoW& other ) {
        if( holder_ != other.holder_ ) {
            deref();
            holder_ = other.holder_;
            ref();
        }
        return *this;
    }

    CoW( const T& value )
        : holder_( new Holder( value ) )
    {
        ;
    }

    operator const T&() const {
        return const_data();
    }

    const T* operator->() const {
        return &const_data();
    }

    T& operator()() {
        return mutable_data();
    }

    T& mutable_data() {
        detach();
        return holder_->data_;
    }

    const T& const_data() const {
        return holder_->data_;
    }

    int is_detached() const {
        return holder_->is_detached();
    }

private:
    void detach() {
        if( !is_detached() ) {
            Holder* new_holder = holder_->copy();
            deref();
            holder_ = new_holder;
        }
    }

    void ref() {
        holder_->ref();
    }

    void deref() {
        if( !holder_->deref() ) {
            delete holder_;
        }
    }

    Holder* holder_;
};

#endif // CoW_H