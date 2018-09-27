#!/usr/bin/python

import sys
import os

(src, dst_h, dst_cpp) = sys.argv[1:4]

out_fd_h = open(dst_h, "a+")
out_fd_cpp = open(dst_cpp, "a+")

with open(src, "r") as fd:
    name = os.path.basename(src).replace(".", "_")
    symbol_name_header_data = "extern unsigned char %s_data[];\n" % (name)
    symbol_name_header_sz_data = "extern int %s_size;\n" % (name)

    out_fd_h.write(symbol_name_header_data)
    out_fd_h.write(symbol_name_header_sz_data)

    sz = os.stat(src).st_size

    symbol_name_cpp_data = "unsigned char %s_data[] = {\n" % (name)
    contents = fd.read()
    for i in xrange(len(contents)):
        symbol_name_cpp_data += ("0x%x" % ord(contents[i])) + ", "
        if i > 0 and i % 16 == 0:
            symbol_name_cpp_data += "\n"
    symbol_name_cpp_data += "};\n"

    symbol_name_cpp_data += "int %s_size = %d;\n" % (name, sz)

    out_fd_cpp.write(symbol_name_cpp_data)
