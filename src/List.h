#ifndef _LIST_H_
#define _LIST_H_

/**
 * Implements an ArrayList.
 */
class List {
  
  public:
    List();
  
    /** The contents of the list are not destroyed. */
    ~List();
  
    /** The number of elements currently in the list. */
    int getNumElements();
  
    /** Add the element onto the end of the list */
    List *add(void *element);
  
    /**
     * Adds another List to the end of this List, in place.
     * The original List (i.e., this) is returned.
     */
    List *add(List *list);
  
    void *toArray();
  
    void *get(int index);
  
    void *remove(int index);
  
    void *replace(int index, void *newElement);

    /**
     * Resets the number of elements to zero.
     */
    void clear();
  
    /**
     * The list is returned to the same state as if it had just been created.
     */
    void revertToNewState();
  
  private:
    void growArrayList();
  
    const static int DEFAULT_LENGTH = 1;
    int maxLength;
    void **arrayList;
    int numElements;
};

#endif // _LIST_H_
