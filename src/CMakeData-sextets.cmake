# To determine what to put in this file, run the following from src:
#	#!/bin/sh
#	
#	NETWORK_TEST=(-name '*EzSockets*')
#	
#	echo 'list(APPEND SMDATA_SEXTETS_SRC'
#	find Sextets '(' -name '*.cpp' -a -not ${NETWORK_TEST[@]} ')' -exec echo '  "{}"' \; | sort
#	echo ')'
#	echo
#	echo 'list(APPEND SMDATA_SEXTETS_HPP'
#	find Sextets '(' -name '*.h' -a -not ${NETWORK_TEST[@]} ')' -exec echo '  "{}"' \; | sort
#	echo ')'
#	echo
#	echo 'if(WITH_NETWORKING)'
#	echo '  list(APPEND SMDATA_SEXTETS_SRC'
#	find Sextets '(' -name '*.cpp' -a ${NETWORK_TEST[@]} ')' -exec echo '    "{}"' \; | sort
#	echo '  )'
#	echo '  list(APPEND SMDATA_SEXTETS_HPP'
#	find Sextets '(' -name '*.h' -a ${NETWORK_TEST[@]} ')' -exec echo '    "{}"' \; | sort
#	echo '  )'
#	echo 'endif()'
#	echo
#	echo 'source_group("Sextets Support Library" FILES ${SMDATA_SEXTETS_SRC} ${SMDATA_SEXTETS_HPP})'

list(APPEND SMDATA_SEXTETS_SRC
  "Sextets/Data.cpp"
  "Sextets/IO/NoopPacketWriter.cpp"
  "Sextets/IO/PacketReaderEventGenerator.cpp"
  "Sextets/IO/RageFilePacketWriter.cpp"
  "Sextets/IO/StdCFilePacketReader.cpp"
  "Sextets/PacketBuffer.cpp"
  "Sextets/Packet.cpp"
)

list(APPEND SMDATA_SEXTETS_HPP
  "Sextets/Data.h"
  "Sextets/IO/NoopPacketWriter.h"
  "Sextets/IO/PacketReaderEventGenerator.h"
  "Sextets/IO/PacketReader.h"
  "Sextets/IO/PacketWriter.h"
  "Sextets/IO/RageFilePacketWriter.h"
  "Sextets/IO/StdCFilePacketReader.h"
  "Sextets/PacketBuffer.h"
  "Sextets/Packet.h"
)

if(NOT WIN32)
  list(APPEND SMDATA_SEXTETS_SRC
    "Sextets/IO/SelectFilePacketReader.cpp"
  )
  list(APPEND SMDATA_SEXTETS_HPP
    "Sextets/IO/SelectFilePacketReader.h"
  )
endif()

if(WITH_NETWORKING)
  list(APPEND SMDATA_SEXTETS_SRC
    "Sextets/IO/EzSocketsPacketReader.cpp"
  )
  list(APPEND SMDATA_SEXTETS_HPP
    "Sextets/IO/EzSocketsPacketReader.h"
  )
endif()

source_group("Sextets Support Library" FILES ${SMDATA_SEXTETS_SRC} ${SMDATA_SEXTETS_HPP})

