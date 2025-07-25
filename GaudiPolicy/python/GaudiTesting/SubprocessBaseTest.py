#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
import re
import select
import signal
import subprocess
import threading
from datetime import datetime
from pathlib import Path
from string import Template
from typing import Callable, Dict, List, Optional, Union

import pytest

from GaudiTesting.FixtureResult import (
    ExceededStreamError,
    FixtureResult,
    ProcessTimeoutError,
)
from GaudiTesting.utils import file_path_for_class, kill_tree, which

STDOUT_LIMIT = int(os.environ.get("GAUDI_TEST_STDOUT_LIMIT", 1024**2 * 100))


class SubprocessBaseTest:
    """
    A base class for running and managing subprocess executions within a test framework.
    It provides mechanisms for setting up the environment, preparing commands,
    and handling subprocess output and errors.
    """

    command: List[str] = None
    reference: str = None
    environment: List[str] = None
    timeout: int = 600
    returncode: int = 0
    popen_kwargs: Dict = {}
    # By default record environment variables that are likely to affect job execution
    env_to_record: List[str] = [
        r"^(.*PATH|BINARY_TAG|CMTCONFIG|PWD|TMPDIR|DISPLAY|LC_[A-Z]+|LANGUAGE|.*ROOT|.*OPTS|GAUDI_ENV_TO_RECORD(_\d+)?)$"
    ]

    @property
    def cwd(self) -> Optional[Path]:
        cwd = self.popen_kwargs.get("cwd")
        return Path(cwd) if cwd else None

    @classmethod
    def resolve_path(cls, path: Union[Path, str]) -> str:
        """
        Resolve the given path to an absolute path,
        expanding environment variables.
        If path looks relative and does not point to anything
        it is not modified.
        """
        if isinstance(path, Path):
            path = str(path)
        path = os.path.expandvars(path)

        # handle the special case "path/to/file:some_suffix"
        suffix = ""
        if ":" in path:
            path, suffix = path.rsplit(":", 1)
            suffix = f":{suffix}"

        if not os.path.isabs(path):
            base_dir = file_path_for_class(cls).parent
            possible_path = str((base_dir / path).resolve())
            if os.path.exists(possible_path):
                path = possible_path
        return path + suffix

    @classmethod
    def update_env(cls, env: Dict[str, str]) -> None:
        if cls.environment:
            for item in cls.environment:
                key, value = item.split("=", 1)
                env[key] = cls.expand_vars_from(value, env)

    @classmethod
    def _prepare_environment(cls) -> Dict[str, str]:
        env = dict(os.environ)
        cls.update_env(env)
        return env

    @staticmethod
    def expand_vars_from(value: str, env: Dict[str, str]) -> str:
        return Template(value).safe_substitute(env)

    @staticmethod
    def unset_vars(env: Dict[str, str], vars_to_unset: List[str]) -> None:
        for var in vars_to_unset:
            env.pop(var, None)

    @classmethod
    def _determine_program(cls, prog: str) -> str:
        if not any(prog.lower().endswith(ext) for ext in [".exe", ".py", ".bat"]):
            prog += ".exe"
        return which(prog) or cls.resolve_path(prog)

    @classmethod
    def _prepare_command(cls, tmp_path=Path()) -> List[str]:
        """
        Prepare the command to be executed, resolving paths for each part.
        """
        command = [cls._determine_program(cls.command[0])]
        for part in cls.command[1:]:
            if not part.startswith("-"):  # do not try to expand options
                command.append(cls.resolve_path(part))
            else:
                command.append(part)
        return command

    @classmethod
    def _handle_timeout(cls, proc: subprocess.Popen) -> str:
        """
        Handle a process timeout by collecting and returning the stack trace.
        """
        stack_trace = cls._collect_stack_trace(proc)
        cls._terminate_process(proc)
        return stack_trace

    @staticmethod
    def _collect_stack_trace(proc: subprocess.Popen) -> str:
        cmd = [
            "gdb",
            "--pid",
            str(proc.pid),
            "--batch",
            "--eval-command=thread apply all backtrace",
        ]
        gdb = subprocess.Popen(
            cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
        )
        return gdb.communicate()[0].decode("utf-8", errors="backslashreplace")

    @staticmethod
    def _terminate_process(proc: subprocess.Popen) -> None:
        kill_tree(proc.pid, signal.SIGTERM)
        proc.wait(60)
        if proc.poll() is None:
            kill_tree(proc.pid, signal.SIGKILL)

    @classmethod
    def _prepare_execution(cls, tmp_path=None):
        command = cls._prepare_command(tmp_path=tmp_path)
        env = cls._prepare_environment()
        os.makedirs(cls.popen_kwargs.get("cwd", tmp_path), exist_ok=True)
        return command, env

    @classmethod
    def run_program(cls, tmp_path=None) -> FixtureResult:
        """
        Run the specified program and capture its output.
        """
        start_time = datetime.now()
        command, env = cls._prepare_execution(tmp_path=tmp_path)

        proc = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=env,
            **cls.popen_kwargs,
        )

        stdout_chunks, stderr_chunks = [], []
        stdout = stderr = ""
        exceeded_stream = stack_trace = run_exception = None
        streams = {
            proc.stdout.fileno(): (stdout_chunks, "stdout"),
            proc.stderr.fileno(): (stderr_chunks, "stderr"),
        }

        def read_output():
            nonlocal stdout, stderr, exceeded_stream
            while not exceeded_stream and proc.poll() is None:
                readable, _, _ = select.select(streams.keys(), [], [], cls.timeout)
                for fileno in readable:
                    data = os.read(fileno, 1024)
                    chunks, stream_name = streams[fileno]
                    chunks.append(data)
                    if sum(len(chunk) for chunk in chunks) > STDOUT_LIMIT:
                        exceeded_stream = stream_name
                        break

            stdout = b"".join(stdout_chunks)
            stderr = b"".join(stderr_chunks)

        thread = threading.Thread(target=read_output)
        thread.start()
        thread.join(cls.timeout)

        if thread.is_alive():
            stack_trace = cls._handle_timeout(proc)
            run_exception = ProcessTimeoutError("Process timed out", stack_trace)
        elif exceeded_stream:
            run_exception = ExceededStreamError(
                "Stream exceeded size limit", exceeded_stream
            )

        end_time = datetime.now()

        completed_process = subprocess.CompletedProcess(
            args=command,
            returncode=proc.returncode,
            stdout=stdout,
            stderr=stderr,
        )

        # Only record  environment variables that match at least one
        # of the regular expressions in env_to_record.
        # If env_to_record is empty or None, do not record any env variable
        env_to_record = list(cls.env_to_record) if cls.env_to_record else []
        # extend env_to_record with the regular expressions in environment variables
        # named GAUDI_ENV_TO_RECORD, GAUDI_ENV_TO_RECORD_1, GAUDI_ENV_TO_RECORD_2, ...
        env_to_record.extend(
            env[name] for name in env if re.match(r"GAUDI_ENV_TO_RECORD(_\d+)?", name)
        )
        env = {
            key: value
            for key, value in env.items()
            if any(re.match(exp, key) for exp in env_to_record)
        }
        return FixtureResult(
            completed_process=completed_process,
            start_time=start_time,
            end_time=end_time,
            run_exception=run_exception,
            command=cls.command,
            expanded_command=command,
            env=env,
            cwd=cls.popen_kwargs["cwd"],
        )

    @classmethod
    def run_program_for_dbg(cls, tmp_path):
        tmp_path = cls.popen_kwargs.get("cwd", tmp_path)
        command, env = cls._prepare_execution(tmp_path=tmp_path)
        print("Running the command: ", command)
        subprocess.run(
            command,
            env=env,
            cwd=tmp_path,
            **cls.popen_kwargs,
        )

    @pytest.mark.do_not_collect_source
    def test_fixture_setup(
        self,
        record_property: Callable[[str, str], None],
        fixture_result: FixtureResult,
        reference_path: Optional[Path],
    ) -> None:
        """
        Record properties and handle any failures during fixture setup.
        """
        for key, value in fixture_result.to_dict().items():
            if value is not None:
                record_property(key, value)
        if reference_path:
            record_property("reference_file", str(reference_path))

        if fixture_result.run_exception:
            pytest.fail(f"{fixture_result.run_exception}")

    @pytest.mark.do_not_collect_source
    def test_returncode(self, returncode: int) -> None:
        """
        Test that the return code matches the expected value.
        """
        assert returncode == self.returncode
