from sty import ef, fg, rs


def indent(output: str, indent_by: int) -> str:
    indent_str = " " * indent_by
    output = indent_str + output.strip().replace("\n", "\n" + indent_str)

    return output


def italic(output: str) -> str:
    return ef.italic + output + rs.ef


def bold(output: str) -> str:
    return ef.bold + output + rs.dim_bold


def red(output: str) -> str:
    return fg.red + output + fg.rs


def green(output: str) -> str:
    return fg.green + output + fg.rs
