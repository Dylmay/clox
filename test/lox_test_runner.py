#!/usr/bin/python

import json
import os
import sys
from argparse import ArgumentParser
from functools import reduce
from typing import List

from pydantic.dataclasses import dataclass
from pydantic import ValidationError
from test_runner.interpreter import Interpreter
from test_runner.recordings import Recordings
from test_runner.test import Test
from test_runner.util import bold, green, indent, italic, red, yellow, JSONSchemaError


def run_test(
    clox_path: str, directory: str, test_file: str, use_valgrind: bool
) -> Recordings:
    with open(f"{directory}/{test_file}", "r", encoding="UTF-8") as test_info:
        try:
            tests = [Test(**t) for t in json.load(test_info)]
        except (TypeError, ValidationError) as exc:
            raise JSONSchemaError(f"{directory}/{test_file} failed to validate: {exc}")

    interpreter = Interpreter(
        interpreter_path=clox_path,
        directory=directory,
        use_valgrind=use_valgrind,
    )
    recordings = Recordings(directory)

    print(bold(directory))
    for (idx, test) in enumerate(tests):
        res = test.run(interpreter)

        idx_str = f"[{idx + 1}/{len(tests)}]"

        if res.success:
            success_msg = green("Passed")
        else:
            success_msg = red("Failed")

        name = bold(test.name)
        desc = italic(test.description)

        print(indent(f"{idx_str}({success_msg}) {name}: {desc}", 2))
        recordings.record_test_result(test, res)

    print(
        italic(
            f"{recordings.passed_test_count()} out of {recordings.total_test_count()} tests passed"
        )
    )

    return recordings


def __run_test_helper(
    clox_path: str,
    directory: str,
    test_file: str,
    use_valgrind: bool,
    tests: List[Recordings],
):
    try:
        test = run_test(clox_path, directory, test_file, use_valgrind)
        tests.append(test)
    except FileNotFoundError:
        print(yellow("[warning]") + f" no {test_file} found within {directory}. Ignoring folder.")
    except JSONSchemaError as exc:
        print(f"{yellow('[warning]')} {exc}. Ignoring folder.")

    for (_, subdirs, _) in os.walk(directory):
        for subdir in subdirs:
            sub_directory = f"{directory}/{subdir}".replace("//", "/")
            __run_test_helper(clox_path, sub_directory, test_file, use_valgrind, tests)


def run_clox_tests(
    clox_path: str, directory: str, test_file: str, use_valgrind: bool
) -> List[Recordings]:
    test_list: List[Recordings] = []

    __run_test_helper(clox_path, directory, test_file, use_valgrind, test_list)

    return test_list


@dataclass
class TestRunnerArgs:
    clox_interpreter: str
    test_directory: str
    tests: str
    valgrind: bool
    fail_info: bool
    verbose: bool
    recursive: bool


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
    parser.add_argument(
        "-r",
        "--recursive",
        action="store_true",
        help="Recursively look through directories for tests",
    )

    parser_args = parser.parse_args()

    return TestRunnerArgs(
        clox_interpreter=parser_args.clox_interpreter[0],
        test_directory=parser_args.test_directory[0],
        tests=parser_args.tests,
        valgrind=parser_args.valgrind,
        fail_info=parser_args.fail_info,
        verbose=parser_args.verbose,
        recursive=parser_args.recursive,
    )


if __name__ == "__main__":
    args = parse_main_args()

    if args.recursive:
        results = run_clox_tests(
            args.clox_interpreter, args.test_directory, args.tests, args.valgrind
        )
    else:
        results = [
            run_test(
                args.clox_interpreter, args.test_directory, args.tests, args.valgrind
            )
        ]

    total_tests = reduce(lambda t, r: t + r.total_test_count(), results, 0)
    passed_tests = reduce(lambda t, r: t + r.passed_test_count(), results, 0)
    print(bold(f"Total: {passed_tests} out of {total_tests} tests passed"))

    if args.fail_info:
        for result in results:
            if not result.all_passed():
                result.print_failed_test_info(indent_by=2)

    if total_tests != passed_tests:
        sys.exit(1)
