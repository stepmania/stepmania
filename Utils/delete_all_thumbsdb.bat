forfiles -p.. -s -mthumbs.db -c"cmd /c attrib -s -h @FILE"
forfiles -p.. -s -mthumbs.db -c"cmd /c del @FILE"
