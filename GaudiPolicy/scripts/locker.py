##########################################################
## stolen and (slighty) adapted from:
##  http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/65203
##

import os, time

if os.name == 'nt':
    import msvcrt
elif os.name == 'posix':
    import fcntl
else:
    raise RuntimeError("Locker only defined for nt and posix platforms")

if os.name == 'nt':
    def lock(file):
        """
        Lock first 10 bytes of a file.
        """
        pos = file.tell() # remember current position
        file.seek(0)
        # By default, python tries about 10 times, then throws an exception.
        # We want to wait forever.
        acquired = False
        while not acquired:
            try:
                msvcrt.locking(file.fileno(),msvcrt.LK_LOCK,10)
                acquired = True
            except IOError, x:
                if x.errno != 36: # 36, AKA 'Resource deadlock avoided', is normal
                    raise
        file.seek(pos) # reset position

    def unlock(file):
        """
        Unlock first 10 bytes of a file.
        """
        pos = file.tell() # remember current position
        file.seek(0)
        msvcrt.locking(file.fileno(),msvcrt.LK_UNLCK,10)
        file.seek(pos) # reset position

elif os.name =='posix':
    def lock(file) :
        # Lock with a simple call to lockf() - this blocks until the lock is aquired
        try:
            fcntl.lockf( file, fcntl.LOCK_EX )
        except IOError, exc_value:
            print "Problem when trying to lock {0}, IOError {1}".format(file, exc_value[0])
            raise
        return

    def unlock(file):
        fcntl.lockf( file, fcntl.LOCK_UN )
        return


import logging
## Lock a file.
#  The file for the lock is created if it doesn't exists and it the "temporary"
#  argument is set to True it will also be deleted when the lock is not needed.
#  The unlocking is done in the destructor (RAII pattern).
class LockFile(object):
    def __init__(self, name, temporary = False):
        self.name = name
        self.temporary = temporary
        self.file = None
        self.log = logging.getLogger("LockFile")
        self.log.info("%s - Locking on %s", time.strftime("%Y-%m-%d_%H:%M:%S"), self.name)
        if not os.path.exists(name):
            mode = "w"
        else:
            self.temporary = False # I do not want to delete a file I didn't create
            mode = "r+"
        try:
            self.file = open(self.name, mode)
            lock(self.file)
        except:
            self.log.warning("Cannot acquire lock on %s", self.name)

    def __del__(self):
        if self.file:
            unlock(self.file)
            self.file.close()
            if self.temporary:
                try:
                    os.remove(self.name)
                except:
                    pass
            self.log.info("%s - Lock on %s released", time.strftime("%Y-%m-%d_%H:%M:%S"), self.name)
            self.file = None # Don't unlock twice!


    # The following methods are needed to allow the use of python's "with" statement, i.e,
    #     with LockFile("myFile") as mylock:
    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.__del__()

