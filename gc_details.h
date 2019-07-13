#ifndef GC_DETAILS_H
#define GC_DETAILS_H

// This class defines an element that is stored
// in the garbage collection information list.
template <class T> class PtrDetails {
  public:
    unsigned ref_count; // current reference count
    T *mem_ptr;         // pointer to allocated memory
    bool is_array; // true if pointing to array
    unsigned array_size; // size of array
	//CONSTRUCTOR
    PtrDetails(T *m_ptr, unsigned size = 0) {
      ref_count = 1;
      mem_ptr = m_ptr;
      if (size != 0) {
        is_array = true;
      } else {
    	is_array = false;
      }
      array_size = size;
    }
};
// Overloading operator== allows two class objects to be compared.
template <class T> bool operator==(const PtrDetails<T> &obj1,const PtrDetails<T> &obj2){
    return (obj1.memPtr == obj2.memPtr);
}

#endif