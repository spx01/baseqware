import telnetlib

game_port = 49152
game_host = "127.0.0.1"

body = """#pragma once

namespace sdk {{
    enum class CID {{
{}
    }};// namespace CID
}}// namespace sdk
"""
dest = "include/sdk/classes.h"


def main():
    tn = telnetlib.Telnet(game_host, game_port)
    tn.write(b"sv_dump_class_info; echo ##DONE##\n")
    lines = tn.read_until(b"##DONE##", 1).decode("ascii").splitlines()[2:-1]
    lines = list(map(lambda x: f"        {x.lstrip().split('(')[0]},", lines))
    lines[0] = lines[0][:-1] + " = 0,"
    with open(dest, "wt") as f:
        f.write(body.format("\n".join(lines)))


if __name__ == "__main__":
    main()
