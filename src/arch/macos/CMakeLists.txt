#*******************************************************************************
# * Copyright (c) 2016 fortiss GmbH
# * All rights reserved. This program and the accompanying materials
# * are made available under the terms of the Eclipse Public License v1.0
# * which accompanies this distribution, and is available at
# * http://www.eclipse.org/legal/epl-v10.html
# *
# * Contributors:
# *  Alois Zoitl - initial API and implementation and/or initial documentation
# *******************************************************************************/

SET(SOURCE_GROUP ${SOURCE_GROUP}\\macos)

forte_add_architecture(MacOs)

if("${FORTE_ARCHITECTURE}" STREQUAL "MacOs")

  forte_add_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  
  forte_add_sourcefile_hcpp(../posix/forte_thread ../posix/pctimeha forte_sync forte_sem)
  forte_add_sourcefile_cpp(../posix/main.cpp)

  if(FORTE_COM_ETH)
   forte_add_sourcefile_hcpp( ../fdselecthand ../bsdsocketinterf)
   forte_add_sourcefile_h(../gensockhand.h)
   forte_add_sourcefile_h(../posix/sockhand.h)
  endif(FORTE_COM_ETH)

  forte_add_definition("-W -Wall -Wextra -Woverloaded-virtual -Wconversion -DFORTE_LITTLE_ENDIAN -ggdb")
  
  set(FORTE_RTTI_AND_EXCEPTIONS FALSE CACHE BOOL "Enable RTTI and Exceptions")
  mark_as_advanced(FORTE_RTTI_AND_EXCEPTIONS)
  IF(NOT FORTE_RTTI_AND_EXCEPTIONS)
    forte_add_definition("-fno-rtti -fno-exceptions")
  ENDIF(NOT FORTE_RTTI_AND_EXCEPTIONS)
  
  
  set(FORTE_POSIX_GENERATE_MAP_FILE FALSE CACHE BOOL "Enable the generation of map files")
  mark_as_advanced(FORTE_POSIX_GENERATE_MAP_FILE)
  IF(FORTE_POSIX_GENERATE_MAP_FILE)
  	forte_add_link_flags("-Wl,-M -Wl,-Map -Wl,forte.map -Wl,-cref")
  ENDIF(FORTE_POSIX_GENERATE_MAP_FILE)	
  
  forte_add_link_library("-lpthread")  

endif()