#Install configuration files with the rest of this package
file( GLOB Configs *.yaml )
install(FILES ${Configs} DESTINATION conf/yaml)
