require 'mkmf'

$LDFLAGS << ' -lz'

cpp_include 'tunepimp/tp_c.h'
if have_library('tunepimp', 'tp_New')
  create_makefile('tunepimp')
end
