file(REMOVE_RECURSE
  "libcvi_tdl.pdb"
  "libcvi_tdl.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/cvi_tdl.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
