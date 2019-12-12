gaudi_subdir(GaudiConfiguration)


gaudi_install_python_modules()

# Ideally I would use '--cover-min-percentage=100', but the version of nose we
# have is a bit old
gaudi_add_test(nose
               COMMAND nosetests -v --with-doctest --with-coverage --cover-package=GaudiConfig2
                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose
               PASSREGEX "TOTAL .* 100%"
               FAILREGEX "FAILED")
