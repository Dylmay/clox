import subprocess
from subprocess import CompletedProcess

from pydantic.dataclasses import dataclass


@dataclass(frozen=True)
class Interpreter:
    interpreter_path: str
    directory: str
    use_valgrind: bool

    def execute(
        self,
        test_file: str,
        force_valgrind: bool = False,
        to_stdin: str = "",
    ) -> CompletedProcess[str]:
        cmd = [self.interpreter_path, f"{self.directory}/{test_file}"]

        if self.use_valgrind or force_valgrind:
            valgrind_cmd = [
                "valgrind",
                "--leak-check=full",
                "-s",
            ]

            cmd = valgrind_cmd + cmd

        return subprocess.run(
            cmd,
            capture_output=True,
            check=False,
            input=to_stdin,
            text=True,
        )
