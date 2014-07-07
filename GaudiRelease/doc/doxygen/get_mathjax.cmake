# Download MathJax
message(STATUS "Downloading MathJax...")
file(DOWNLOAD https://github.com/mathjax/MathJax/archive/master.tar.gz
     ${TARFILE_DEST}/MathJax.tar.gz)
message(STATUS "Unpacking MathJax...")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${TARFILE_DEST}/MathJax.tar.gz
                WORKING_DIRECTORY ${INSTALL_DIR})
file(RENAME ${INSTALL_DIR}/MathJax-master ${INSTALL_DIR}/mathjax)
file(WRITE MathJax-installed "")
message(STATUS "MathJax installed.")
