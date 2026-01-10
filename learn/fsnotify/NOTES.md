inotify
-------

This is a linux-only interface.
- `inotify-tools` has a C library and bash scripts
  - the man pages mentions fsnotify or fanotify that is a different linux interface for the same theing presumably
- [hinotify](https://hackage.haskell.org/package/hinotify) binds directly to the kernel interface, it seems
- [kernel stuff](https://www.kernel.org/pub/linux/kernel/people/rml/inotify/) btw

Cross- Platform
---------------

- [fsnotify](https://hackage.haskell.org/package/fsnotify) is a haskell library
