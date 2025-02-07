from pydantic.dataclasses import dataclass


@dataclass(frozen=True)
class TestResult:
    stdout: str
    stderr: str
    retcode: int
    fails: list[str]
    filename: str
    success: bool = True


class TestResultBuilder:
    def __init__(self, stdout: str, stderr: str, retcode: int, filename: str) -> None:
        self.__success: bool = True
        self.__stdout: str = stdout
        self.__stderr: str = stderr
        self.__retcode: int = retcode
        self.__fails: list[str] = []
        self.__filename: str = filename

    def should_return(self, retcode: int) -> "TestResultBuilder":
        if self.__retcode != retcode:
            self.failed(
                f"Return code ({self.__retcode}) does not match expected ({retcode})"
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
            self.failed(f'stdout did not contain "{expected}"')
        return self

    def should_error(self, expected: str) -> "TestResultBuilder":
        if expected not in self.__stderr:
            self.failed(f'stderr did not contain "{expected}"')
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
            filename=self.__filename,
        )
