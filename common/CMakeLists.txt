target_sources(taller_common
    PRIVATE
    # .cpp files
    liberror.cpp
    resolver.cpp
    resolvererror.cpp
    socket.cpp
    operations_bytes.cpp
    resource_paths.cpp
    PUBLIC
    # .h files
    liberror.h
    queue.h
    resolver.h
    resolvererror.h
    socket.h
    thread.h
    operations_bytes.h
    peer_close_error.h
    resource_paths.h
    )
