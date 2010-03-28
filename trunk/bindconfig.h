// TODO: Enclose everything in a namespace

typedef enum {
    kCompClassDevice,
    kCompClassClient,
    kCompClassProtocol,
    kCompClassService
} CompClass;

typedef enum {
    kBindingDisabled,
    kBindingEnabled
} BindingStatus;

BindingStatus bindingStatus(CompClass compClass, char *compStr, char *pathStr);
bool setBindingStatus(CompClass compClass, char *compStr, char *pathStr,
        BindingStatus status);

