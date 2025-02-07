from typing import Optional

from pydantic.dataclasses import dataclass

from .interpreter import Interpreter
from .test_result import TestResult, TestResultBuilder


@dataclass(frozen=True)
class Expectation:
    # Failure
    to_fail: bool | None = None
    has_return_code: int | None = None
    on_line: int | None = None
    # Output in stdout
    to_output: list[str | int | float] | None = None
    # Output in stderr
    to_error: list[str] | None = None
    no_leaks: bool | None = None


@dataclass(frozen=True)
class Test:
    name: str
    description: str
    file: str
    expect: Expectation
    to_stdin: Optional[str] = None
    reason: Optional[str] = None

    def run(self, interpreter: Interpreter) -> TestResult:
        result = interpreter.execute(
            self.file, self.expect.no_leaks or False, self.to_stdin or ""
        )

        result_builder = TestResultBuilder(
            stdout=result.stdout,
            stderr=result.stderr,
            retcode=result.returncode,
            filename=self.file,
        )

        if self.expect.has_return_code:
            result_builder.should_return(self.expect.has_return_code)

        if self.expect.to_fail:
            result_builder.should_fail()

            if self.expect.on_line is not None:
                result_builder.error_on_line(self.expect.on_line)
        else:
            result_builder.should_pass()

        if self.expect.to_output is not None:
            for expected in self.expect.to_output:
                result_builder.should_output(str(expected))

        if self.expect.to_error is not None:
            for expected in self.expect.to_error:
                result_builder.should_error(expected)

        return result_builder.build()
