#!/usr/bin/env python

import json
import os
import sys
from argparse import ArgumentParser
from typing import List

from pydantic import ValidationError
from pydantic.dataclasses import dataclass
from test_runner.interpreter import Interpreter
from test_runner.printer import Printer
from test_runner.printer import PrintString as Ps
from test_runner.recordings import Recordings
from test_runner.test import Test
from test_runner.util import JSONSchemaError


def __get_tests(test_path: str) -> List[Test]:
    # ignore placeholder files
    if os.path.getsize(test_path) == 0:
        return []

    with open(test_path, "r", encoding="UTF-8") as test_info:
        try:
            return [Test(**t) for t in json.load(test_info)]
        except (TypeError, ValidationError, json.JSONDecodeError) as exc:
            raise JSONSchemaError(f"{test_path} failed to validate", exc) from exc


def record_tests(
    clox_path: str, directory: str, test_file: str, use_valgrind: bool
) -> Recordings:
    tests = __get_tests(f"{directory}/{test_file}")

    interpreter = Interpreter(
        interpreter_path=clox_path,
        directory=directory,
        use_valgrind=use_valgrind,
    )
    recordings = Recordings(directory)

    if len(tests) > 0:
        Printer.print(Ps(directory).bold())
        for (idx, test) in enumerate(tests):
            res = test.run(interpreter)

            if res.success:
                success_msg = Ps("Passed").green()
            else:
                success_msg = Ps("Failed").red()

            idx_str = f"[{idx + 1}/{len(tests)}]({success_msg})"
            Printer.print(
                Ps(
                    idx_str
                    + f" {Ps(test.name).bold()}: {Ps(test.description).italic()}"
                ).indent(2)
            )
            recordings.record_test_result(test, res)

        Printer.print(
            Ps(
                f"{recordings.passed_test_count()} out of {recordings.total_test_count()}"
                + " tests passed"
            ).italic()
        )

    return recordings


def run_clox_tests(
    clox_path: str,
    directory: str,
    test_file: str,
    use_valgrind: bool,
) -> List[Recordings]:
    test_list: List[Recordings] = []

    for (cur_folder, _, _) in os.walk(directory):
        try:
            test = record_tests(clox_path, cur_folder, test_file, use_valgrind)
            test_list.append(test)
        except FileNotFoundError:
            Printer.print(
                Ps("[warning]").yellow()
                + f" no {test_file} found within {cur_folder}. Ignoring folder."
            )
        except JSONSchemaError as exc:
            Printer.print(Ps("[warning]").yellow() + f" {exc}. Ignoring folder.")

    return test_list


@dataclass(frozen=True)
class TestRunnerArgs:
    clox_interpreter: str
    test_directory: str
    tests: str
    valgrind: bool
    fail_info: bool
    verbose: bool
    recursive: bool
    quiet: bool


def parse_main_args() -> TestRunnerArgs:
    parser = ArgumentParser(description="Python script used to run lox tests")
    parser.add_argument("clox_interpreter", help="Path to the lox interpreter")
    parser.add_argument("test_directory", help="Path to the lox test directory")
    parser.add_argument(
        "-t",
        "--tests",
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
        "-q",
        "--quiet",
        action="store_true",
        help="Disable printing of test information",
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Output all test information"
    )
    parser.add_argument(
        "-r",
        "--recursive",
        action="store_true",
        help="Recursively look through directories for tests",
    )

    parser_args = parser.parse_args()

    return TestRunnerArgs(
        clox_interpreter=parser_args.clox_interpreter,
        test_directory=parser_args.test_directory,
        tests=parser_args.tests,
        valgrind=parser_args.valgrind,
        fail_info=parser_args.fail_info,
        verbose=parser_args.verbose,
        recursive=parser_args.recursive,
        quiet=parser_args.quiet,
    )


if __name__ == "__main__":
    args = parse_main_args()

    if args.quiet:
        Printer.disable()

    if args.recursive:
        results = run_clox_tests(
            args.clox_interpreter,
            args.test_directory,
            args.tests,
            args.valgrind,
        )
    else:
        results = [
            record_tests(
                args.clox_interpreter,
                args.test_directory,
                args.tests,
                args.valgrind,
            )
        ]

    total_tests = sum(result.total_test_count() for result in results)
    passed_tests = sum(result.passed_test_count() for result in results)
    Printer.print(Ps(f"Total: {passed_tests} out of {total_tests} tests passed").bold())

    if args.fail_info:
        for result in results:
            if not result.all_passed():
                result.print_failed_test_info(indent_by=2)

    if total_tests != passed_tests:
        sys.exit(1)
