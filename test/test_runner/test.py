from typing import List, Optional

from pydantic.dataclasses import dataclass

from .interpreter import Interpreter
from .test_result import TestResult, TestResultBuilder


@dataclass(frozen=True)
class Expectation:
    # Failure
    to_fail: Optional[bool] = None
    has_return_code: Optional[int] = None
    on_line: Optional[int] = None
    # Output in stdout
    to_output: Optional[List[str]] = None
    # Output in stderr
    to_err: Optional[List[str]] = None
    no_leaks: Optional[bool] = None


@dataclass(frozen=True)
class Test:
    name: str
    description: str
    reason: str
    file: str
    expect: Expectation

    def run(self, interpreter: Interpreter) -> TestResult:
        result = interpreter.execute(self.file, self.expect.no_leaks or False)

        result_builder = TestResultBuilder(
            stdout=result.stdout.decode("UTF-8"),
            stderr=result.stderr.decode("UTF-8"),
            retcode=result.returncode,
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
                result_builder.should_output(expected)

        return result_builder.build()
