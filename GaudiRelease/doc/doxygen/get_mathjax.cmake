# Download MathJax
set(MathJax_url https://github.com/mathjax/MathJax/archive/master.tar.gz)
set(MathJax_dst ${TARFILE_DEST}/MathJax.tar.gz)

message(STATUS "Downloading MathJax...")
file(DOWNLOAD ${MathJax_url} ${MathJax_dst})

message(STATUS "Unpacking MathJax...")
file(MAKE_DIRECTORY ${INSTALL_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${TARFILE_DEST}/MathJax.tar.gz
                WORKING_DIRECTORY ${INSTALL_DIR})
file(RENAME ${INSTALL_DIR}/MathJax-master ${INSTALL_DIR}/mathjax)

message(STATUS "MathJax installed.")
