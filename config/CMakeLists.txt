#Install configuration files with the rest of this package
file( GLOB Configs *.xml )
install(FILES ${Configs} DESTINATION conf/xml)
