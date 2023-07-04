# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.0.2-6/components/bootloader/subproject"
  "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader"
  "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader-prefix"
  "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader-prefix/tmp"
  "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader-prefix/src"
  "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Bitcom/BLEMESH/BLEMESH-V1/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
