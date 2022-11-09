from typing import List, Tuple

from .test import Test, TestResult
from .util import bold, indent, italic, red


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
        print(bold("==========================================================="))
        print(bold(self.__directory))
        for (test, result) in self.__failed_tests:
            print("")
            print(indent(red("Failed ") + bold(test.name), indent_by))
            print(italic(indent(f"Program returned {result.retcode}", indent_by)))

            print(bold(indent("Result:", 2 + indent_by)))
            for fail in result.fails:
                print(indent(fail, 4 + indent_by))

            print(bold(indent("stdout:", 2 + indent_by)))
            if not result.stdout or result.stdout.isspace():
                stdout = "<no stdout>"
            else:
                stdout = result.stdout
            print(italic(indent(stdout, 4 + indent_by)))

            print(bold(indent("stderr:", 2 + indent_by)))
            if not result.stderr or result.stderr.isspace():
                stderr = "<no stderr>"
            else:
                stderr = result.stderr
            print(italic(indent(stderr, 4 + indent_by)))

    def all_passed(self) -> bool:
        return self.total_test_count() == self.passed_test_count()
