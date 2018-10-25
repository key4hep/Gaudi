# Download Doxygen tags from cppreference.com
set(tags_url http://upload.cppreference.com/mwiki/images/f/f8/cppreference-doxygen-web.tag.xml)

message(STATUS "Downloading Doxygen tags from cppreference.com...")
file(DOWNLOAD ${tags_url} ${DEST_DIR}/cppreference-doxygen-web.tag.xml)

message(STATUS "Doxygen tags downloaded.")
