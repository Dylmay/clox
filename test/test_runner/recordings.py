from typing import List, Tuple

from sty import ef, fg, rs

from .test import Test, TestResult


class Recordings:
    __test_cnt: int
    __failed_tests: List[Tuple[Test, TestResult]]

    def __init__(self):
        self.__test_cnt = 0
        self.__failed_tests = []

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

    def print_failed_test_info(self):
        def indent(output: str, indent_by: int) -> str:
            indent_amt = " " * indent_by
            output = indent_amt + output.strip().replace("\n", "\n" + indent_amt)

            return output

        def italic(output: str) -> str:
            return ef.italic + output + rs.ef

        def bold(output: str) -> str:
            return ef.bold + output + rs.dim_bold

        for (test, result) in self.__failed_tests:
            print("")
            print(fg.red + "Failed " + fg.rs + ef.bold + test.name + rs.dim_bold)
            print(italic(f"Program returned {result.retcode}"))

            print(bold(indent("Result:", 2)))
            for fail in result.fails:
                print(indent(fail, 4))

            print(bold(indent("stdout:", 2)))
            if not result.stdout or result.stdout.isspace():
                stdout = "<no stdout>"
            else:
                stdout = result.stdout
            print(italic(indent(stdout, 4)))

            print(bold(indent("stderr:", 2)))
            if not result.stderr or result.stderr.isspace():
                stderr = "<no stderr>"
            else:
                stderr = result.stderr
            print(italic(indent(stderr, 4)))

    def all_passed(self) -> bool:
        return self.total_test_count() == self.passed_test_count()
