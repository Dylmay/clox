#!/usr/bin/python

import json
import sys
from argparse import ArgumentParser

from pydantic.dataclasses import dataclass
from sty import ef, fg, rs
from test_runner.interpreter import Interpreter
from test_runner.recordings import Recordings
from test_runner.test import Test


def run_clox_tests(
    clox_path: str, directory: str, test_file: str, use_valgrind: bool
) -> Recordings:
    with open(f"{directory}/{test_file}", "r", encoding="UTF-8") as test_info:
        tests = [Test(**t) for t in json.load(test_info)]

    interpreter = Interpreter(
        interpreter_path=clox_path,
        directory=directory,
        use_valgrind=use_valgrind,
    )
    recordings = Recordings()

    for (idx, test) in enumerate(tests):
        res = test.run(interpreter)

        idx_str = f"[{idx + 1}/{len(tests)}]"

        if res.success:
            success_msg = fg.green + "Passed" + fg.rs
        else:
            success_msg = fg.red + "Failed" + fg.rs

        name = ef.bold + test.name + rs.dim_bold
        desc = ef.italic + test.description + rs.italic

        print(f"{idx_str}({success_msg}) {name}: {desc}")
        recordings.record_test_result(test, res)

    return recordings


@dataclass
class TestRunnerArgs:
    clox_interpreter: str
    test_directory: str
    tests: str
    valgrind: bool
    fail_info: bool
    verbose: bool


def parse_main_args() -> TestRunnerArgs:
    parser = ArgumentParser(description="Python script used to run lox tests")
    parser.add_argument("clox_interpreter", nargs=1, help="Path to the lox interpreter")
    parser.add_argument(
        "test_directory", nargs=1, help="Path to the lox test directory"
    )
    parser.add_argument(
        "-t",
        "--tests",
        nargs=1,
        default="tests.json",
        help="The test layout file to use within the test directory",
    )
    parser.add_argument(
        "-g",
        "--valgrind",
        action="store_true",
        help="Run the tests through valgrind and record any memory leaks",
    )
    parser.add_argument(
        "-i",
        "--fail-info",
        action="store_true",
        help="Outputs information about the tests that have failed",
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Output all test information"
    )

    parser_args = parser.parse_args()

    return TestRunnerArgs(
        clox_interpreter=parser_args.clox_interpreter[0],
        test_directory=parser_args.test_directory[0],
        tests=parser_args.tests,
        valgrind=parser_args.valgrind,
        fail_info=parser_args.fail_info,
        verbose=parser_args.verbose,
    )


if __name__ == "__main__":
    args = parse_main_args()

    results = run_clox_tests(
        args.clox_interpreter, args.test_directory, args.tests, args.valgrind
    )

    print(
        ef.bold
        + f"{results.passed_test_count()} out of {results.total_test_count()} tests passed"
        + rs.dim_bold
    )

    if not results.all_passed():
        if args.fail_info:
            results.print_failed_test_info()

        sys.exit(1)
