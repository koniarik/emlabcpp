
from enum import Enum
import sys

def load_file(file):
    with open(file, "r") as fd:
        while line := fd.readline():
            yield line

class snippet(Enum):
    START = 1
    END = 2

# Features todo:
# - proper skip of non-cpp stuff
# - move project-specific-config into the readme file
#       seems like <!--- --> is the way to go

def find_snippets(lines):
    gen_stuff = False
    section_skip = 0
    for line in lines:
        if "```cpp" in line or "<!--cpp" in line:
            yield snippet.START
            gen_stuff = True
            continue
        if "```cmake" in line:
            yield "// " + line
            section_skip += 1
            continue
        elif "```" in line or "-->" in line:
            if section_skip > 0:
                section_skip -= 1
                yield "// " + line
                continue
            yield snippet.END
            gen_stuff = False
            continue
        elif gen_stuff:
            if "error:" in line:
                yield "// " + line
            else:
                yield line
        else:
            yield "// " + line

def gen_cpp(lines):
    yield f"""

    #include <emlabcpp/algorithm.h>
    #include <emlabcpp/protocol.h>
    #include <emlabcpp/physical_quantity.h>
    #include <emlabcpp/defer.h>
    #include <emlabcpp/enumerate.h>
    #include <emlabcpp/static_circular_buffer.h>
    #include <emlabcpp/enum.h>
    #include <emlabcpp/match.h>
    #include <gtest/gtest.h>

    namespace emlabcpp{{

    TEST(readme, readme){{

        struct a_t{{}};
        struct b_t{{}};

    """

    for l in lines:
        if l == snippet.START:
            yield f"""
                {{
            """
        elif l == snippet.END:
            yield f"""
                }}
            """
        else:
            yield l

    yield f"""
    }}

    }}
    """

inpt_md = sys.argv[1]
outpt_cpp = sys.argv[2]

with open(outpt_cpp, "w") as fd:
    for out in gen_cpp(find_snippets(load_file(inpt_md))):
        fd.write(out)
