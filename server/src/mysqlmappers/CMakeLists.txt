add_library(mysqlmappers STATIC abstractmysqlmapper.h)
set_target_properties(mysqlmappers PROPERTIES LINKER_LANGUAGE CXX)
# aux_source_directory(. utilslist)
# add_library(fxutils ${utilslist})
