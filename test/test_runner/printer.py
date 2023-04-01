from textwrap import indent

from sty import Style, ef, fg, rs


class PrintString(str):
    __start_attributes: list[str | Style]
    __end_attributes: list[str | Style]
    __indent_str: str

    def __init__(self, _):
        self.__start_attributes = []
        self.__end_attributes = []
        self.__indent_str = ""

    def indent(self, indent_by: int = 0, indent_str: str = " ") -> "PrintString":
        self.__indent_str = indent_str * indent_by
        return self

    def italic(self) -> "PrintString":
        return self.add_attribute(ef.italic, rs.ef)

    def bold(self) -> "PrintString":
        return self.add_attribute(ef.bold, rs.dim_bold)

    def red(self) -> "PrintString":
        return self.add_attribute(fg.red, fg.rs)

    def green(self) -> "PrintString":
        return self.add_attribute(fg.green, fg.rs)

    def yellow(self) -> "PrintString":
        return self.add_attribute(fg.yellow, fg.rs)

    def add_attribute(
        self, attribute: str | Style, end_attribute: str | Style
    ) -> "PrintString":
        self.__start_attributes.append(attribute)
        self.__end_attributes.append(end_attribute)
        return self

    def build(self) -> str:
        if len(self.__indent_str) > 1:
            output_string = indent(self.strip(), self.__indent_str)
        else:
            output_string = self

        start_attrs = "".join(self.__start_attributes)
        end_attrs = "".join(self.__end_attributes)

        return start_attrs + output_string + end_attrs

    def __str__(self) -> str:
        return self.build()

    def __repr__(self) -> str:
        return self.build()


class Printer:
    __is_enabled = True

    @classmethod
    def disable(cls):
        cls.__is_enabled = False

    @classmethod
    def enable(cls):
        cls.__is_enabled = True

    @classmethod
    def print(cls, statement: str = ""):
        if cls.__is_enabled:
            print(statement)
