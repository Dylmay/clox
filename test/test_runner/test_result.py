from typing import List

from pydantic.dataclasses import dataclass


@dataclass(frozen=True)
class TestResult:
    stdout: str
    stderr: str
    retcode: int
    fails: List[str]
    success: bool = True


class TestResultBuilder:
    __success: bool
    __stdout: str
    __stderr: str
    __retcode: int
    __fails: List[str]

    def __init__(self, stdout: str, stderr: str, retcode: int) -> None:
        self.__success = True
        self.__stdout = stdout
        self.__stderr = stderr
        self.__retcode = retcode
        self.__fails = []

    def should_return(self, retcode: int) -> "TestResultBuilder":
        if self.__retcode != retcode:
            self.failed(
                f"Return code ({retcode}) does not match expected ({self.__retcode})"
            )
        return self

    def should_fail(self) -> "TestResultBuilder":
        if self.__retcode == 0:
            self.failed("Program did not fail as expected")
        return self

    def error_on_line(self, line_num: int) -> "TestResultBuilder":
        expected_line = f"[line {line_num}]"
        if expected_line not in self.__stderr:
            self.failed(f"fail was not at {expected_line}")
        return self

    def should_output(self, expected: str) -> "TestResultBuilder":
        if expected not in self.__stdout:
            self.failed(f'output did not contain "{expected}"')
        return self

    def should_pass(self) -> "TestResultBuilder":
        if self.__retcode != 0:
            self.failed("Program did not pass as expected")
        return self

    def failed(self, message: str) -> "TestResultBuilder":
        self.__success = False
        self.__fails.append(message)
        return self

    def build(self) -> "TestResult":
        return TestResult(
            success=self.__success,
            stdout=self.__stdout,
            stderr=self.__stderr,
            retcode=self.__retcode,
            fails=self.__fails,
        )
