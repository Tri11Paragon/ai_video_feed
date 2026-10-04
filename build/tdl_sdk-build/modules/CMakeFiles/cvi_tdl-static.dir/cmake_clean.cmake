file(REMOVE_RECURSE
  "libcvi_tdl.a"
  "libcvi_tdl.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/cvi_tdl-static.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
