#ifndef GC_POINTER_H
#define GC_POINTER_H

#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"

using namespace std;

template <class T, int size = 0> class Pointer{
  private:
    // refContainer maintains the garbage collection list.
    static list<PtrDetails<T>> refContainer;
    T *addr = nullptr;
    bool is_array = false; 
    unsigned array_size = 0;
    static bool first;
    typename list<PtrDetails<T>>::iterator findPtrInfo(T *ptr);
  
  public:
    // Define an iterator type for Pointer<T>.
    typedef Iter<T> GCiterator;
    Pointer(){
        Pointer(NULL);
    }
    Pointer(T*);
    Pointer(const Pointer &);
    ~Pointer();
    // Collect garbage. Returns true if at least
    // one object was freed.
    static bool collect();
    T* operator=(T *t);
    // Overload assignment of Pointer to Pointer.
    Pointer &operator=(Pointer &rv);
    // Return a reference to the object pointed to by this Pointer.
    T& operator*(){
        return *addr;
    }
    // Return the address being pointed to.
    T* operator->() { return addr; }
    // Return a reference to the object at the
    // index specified by i.
    T& operator[](int i){ return addr[i];}
    // Conversion function to T *.
    operator T *() { return addr; }
    // Return an Iter to the start of the allocated memory.
    Iter<T> begin(){
        int _size;
        if (is_array) {
          _size = array_size;
        } else {
          _size = 1;
        }
        return Iter<T>(addr, addr, addr + _size);
    }
    // Return an Iter to one past the end of an allocated array.
    Iter<T> end(){
      int _size;
      if (is_array) {
        _size = array_size;
      } else {
        _size = 1;
      }
      return Iter<T>(addr + _size, addr, addr + _size);
    }
    // Return the size of refContainer for this type of Pointer.
    static int refContainerSize() { return refContainer.size(); }
    // A utility function that displays refContainer.
    static void showlist();
    // Clear refContainer when program exits.
    static void shutdown();
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size> list<PtrDetails<T>> Pointer<T, size>::refContainer;
template <class T, int size> bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects. -> see class interface
template<class T,int size> Pointer<T,size>::Pointer(T *t){
    // Register shutdown() as an exit function.
    if (first)
        atexit(shutdown);
    first = false;

    typename list<PtrDetails<T>>::iterator p = findPtrInfo(t);
    if(p == refContainer.end()) {
        PtrDetails<T> newPtr(t, size);
        if(size > 0) {
            is_array = true;
            array_size = size;
        }
        refContainer.push_back(newPtr);
    } else {
        p->ref_count++;
    }
    this->addr = t;
}
// Copy constructor.
template< class T, int size> Pointer<T,size>::Pointer(const Pointer &ob) {
    typename list<PtrDetails<T>>::iterator p = findPtrInfo(ob.addr);
    p->ref_count++;
    this->is_array = ob.is_array;
    this->array_size = ob.array_size;   
	this->addr = ob.addr;
}

// Destructor for Pointer.
template <class T, int size> Pointer<T, size>::~Pointer() {
    typename list<PtrDetails<T>>::iterator p = findPtrInfo(addr);
    p->ref_count--;
    if(p->ref_count == 0)collect();
}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size> bool Pointer<T, size>::collect(){
    bool memFreed = false;
    typename list<PtrDetails<T>>::iterator p = refContainer.begin();
    do {
        for(; p != refContainer.end(); p++){
            if(p->ref_count > 0)continue;
            if(p->mem_ptr) {
                if(p->is_array){
                    delete[] p->mem_ptr;
                }
                else delete p->mem_ptr;
                refContainer.erase(p--);
            }
        }
    } while (p != refContainer.end());
    return memFreed;
}

// Overload assignment of pointer to Pointer.
template <class T, int size> T* Pointer<T, size>::operator=(T *t){
    typename list<PtrDetails<T>>::iterator p = findPtrInfo(addr);
    p->ref_count--;
    if(p->ref_count == 0) collect();
    p = findPtrInfo(t);
    if (p == refContainer.end()){
        PtrDetails<T> newPtr(t, size);
        refContainer.push_back(newPtr);
    } else {
        p->ref_count++;
    }
    if (size > 0) {
        is_array = true;
        array_size = size;
    }
    this->addr = t;
    return addr;
}
// Overload assignment of Pointer to Pointer.
template <class T, int size> Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv){
    if (*this == rv) {
      return *this;
    }
    typename list<PtrDetails<T>>::iterator p = findPtrInfo(addr);
    p->ref_count--;
    if (p->ref_count == 0) {
      collect();
    }
    p = findPtrInfo(rv.addr);
    p->ref_count++;
	this->addr = rv.addr;
    this->is_array = rv.is_array;
    this->array_size = rv.array_size;
    return *this;
}

// A utility function that displays refContainer.
template <class T, int size> void Pointer<T, size>::showlist(){
    typename list<PtrDetails<T> >::iterator p;
    cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    cout << "mem_ptr ref_count value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        cout << "[" << (void *)p->mem_ptr << "]"
             << " " << p->ref_count << " ";
        if (p->mem_ptr)
            cout << " " << *p->mem_ptr;
        else
            cout << "---";
        cout << endl;
    }
    cout << endl;
}
// Find a pointer in refContainer.
template <class T, int size> typename list<PtrDetails<T>>::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename list<PtrDetails<T>>::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->mem_ptr == ptr)
            return p;
	return p;
}
// Clear refContainer when program exits.
template <class T, int size> void Pointer<T, size>::shutdown(){
    if (refContainerSize() == 0)
        return; // list is empty
    typename list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++){
        // Set all reference counts to zero
        p->ref_count = 0;
    }
    collect();
}

#endif