#ifndef GAUDI_DBM_H
#define GAUDI_DBM_H

#include <gdbm.h>
#include <sys/stat.h>

#include <string>
#include <exception>

namespace Gaudi {
  /** Wrapper class for easy access to GDBM files.
   */
  class DBM {
  public:
    /// Read-Write modes for a GDBM file.
    enum ReadWriteType {
      Read = GDBM_READER,           ///< read-only
      Write = GDBM_WRITER,          ///< read/write an existing file
      WriteOrCreate = GDBM_WRCREAT, ///< read/write an existing file or create it
      Create = GDBM_NEWDB           ///< create a new file
    };

    /** Constructor.
     *
     * @param path: path to the database file
     * @param read_write: open mode for gdbm
     * @param mode: create mode for the file (only if the file needs to be created)
     * @param block_size: number of bytes to read in a chunk (only used in creation)
     */
    DBM(const std::string &path,
        ReadWriteType read_write = Read,
        mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH, int block_size = 4096):
      m_path(path), m_bs(block_size), m_rw(read_write), m_mode(mode), m_file(0)
    {}

    /// Destructor.
    ~DBM() {
      if (m_file) {
        gdbm_close(m_file);
      }
    }

    /** Store data in the database.
     */
    bool store(const std::string &k, const std::string &v, bool replace = false) {
      checkFile();
      datum dk = {const_cast<char*>(k.c_str()), k.size()};
      datum dv = {const_cast<char*>(v.c_str()), v.size()};
      return gdbm_store(m_file, dk, dv, replace ? GDBM_REPLACE : GDBM_INSERT) == 0;
    }

    /** Retrieve data from the database.
     */
    std::string fetch(const std::string &k) const {
      checkFile();
      datum dk = {const_cast<char*>(k.c_str()), k.size()};
      datum d = gdbm_fetch(m_file, dk);
      if (!d.dptr) {
        // throw exception("Key not found: " + k);
        return "";
      }
      std::string res(d.dptr, d.dsize);
      free(d.dptr);
      return res;
    }

    /** Check for existence of a key in the database.
     */
    bool exists(const std::string &k) const {
      checkFile();
      datum dk = {const_cast<char*>(k.c_str()), k.size()};
      return gdbm_exists(m_file, dk);
    }

    /** Remove an entry in the database.
     */
    bool remove(const std::string &k) {
      checkFile();
      datum dk = {const_cast<char*>(k.c_str()), k.size()};
      return gdbm_delete(m_file, dk) == 0;
    }

  private:
    /// Check if the database file has been opened and open it if not.
    void checkFile() const {
      if (!m_file){
        m_file = gdbm_open(const_cast<char*>(m_path.c_str()), m_bs, m_rw, m_mode, 0);
      }
    }
    /// Path to the database file.
    std::string m_path;
    int m_bs, m_rw, m_mode;
    /// Internal gdbm file handle.
    mutable GDBM_FILE m_file;
  };
}

#endif /* GAUDI_DBM_H */
