from typing import List, Tuple

from .printer import Printer
from .printer import PrintString as Ps
from .test import Test, TestResult


class Recordings:
    __directory: str
    __test_cnt: int
    __failed_tests: List[Tuple[Test, TestResult]]

    def __init__(self, directory: str):
        self.__test_cnt = 0
        self.__failed_tests = []
        self.__directory = directory

    def record_test_result(self, test: Test, result: TestResult):
        self.__test_cnt += 1
        if not result.success:
            self.__failed_tests.append((test, result))

    def total_test_count(self) -> int:
        return self.__test_cnt

    def failed_test_count(self) -> int:
        return len(self.__failed_tests)

    def passed_test_count(self) -> int:
        return self.total_test_count() - self.failed_test_count()

    def print_failed_test_info(self, indent_by: int = 0):
        Printer.print(
            Ps("===========================================================").bold()
        )
        Printer.print(Ps(self.__directory).bold())
        for (test, result) in self.__failed_tests:
            fail_msg = Ps("Failed ").red() + Ps(test.name).bold()

            Printer.print()
            Printer.print(Ps(fail_msg).indent(indent_by))
            Printer.print(
                Ps(f"Program returned {result.retcode}").italic().indent(indent_by)
            )

            Printer.print(Ps("Result").bold().indent(indent_by))
            for fail in result.fails:
                Printer.print(Ps(fail).indent(4 + indent_by))

            Printer.print(Ps("stdout:").bold().indent(2 + indent_by))
            if not result.stdout or result.stdout.isspace():
                stdout = "<no stdout>"
            else:
                stdout = result.stdout
            Printer.print(Ps(stdout).italic().indent(4 + indent_by))

            Printer.print(Ps("stderr:").bold().indent(2 + indent_by))
            if not result.stderr or result.stderr.isspace():
                stderr = "<no stderr>"
            else:
                stderr = result.stderr
            Printer.print(Ps(stderr).italic().indent(4 + indent_by))

    def all_passed(self) -> bool:
        return self.total_test_count() == self.passed_test_count()
