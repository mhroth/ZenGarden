#ifndef _REMOTE_OBJECT_H_
#define _REMOTE_OBJECT_H_

class RemoteObject {
  
  public:
    RemoteObject(char *newTag);
    ~RemoteObject();
    
    char *getTag();
    
  protected:
    char *tag;
};

#endif // _REMOTE_OBJECT_H_
