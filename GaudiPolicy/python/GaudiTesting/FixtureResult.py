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
import datetime
import subprocess
from typing import List, Union

from GaudiTesting.utils import CodeWrapper


class ProcessTimeoutError(Exception):
    def __init__(self, message, stack_trace):
        super().__init__(message)
        self.stack_trace = stack_trace


class ExceededStreamError(Exception):
    def __init__(self, message, exceeded_stream):
        super().__init__(message)
        self.exceeded_stream = exceeded_stream


class FixtureResult:
    """
    A class to encapsulate the results of a subprocess execution in a test fixture.
    """

    def __init__(
        self,
        completed_process: subprocess.CompletedProcess,
        start_time: datetime,
        end_time: datetime,
        run_exception: Union[ProcessTimeoutError, ExceededStreamError, None],
        command: List[str],
        expanded_command: List[str],
        env: dict,
        cwd: str,
    ):
        self.completed_process = completed_process
        self.start_time = start_time
        self.end_time = end_time
        self.run_exception = run_exception
        self.command = command
        self.expanded_command = expanded_command
        self.runtime_environment = env
        self.cwd = cwd

    def elapsed_time(self):
        return self.end_time - self.start_time

    @staticmethod
    def format_value(value):
        """
        Format a given value to a string representation.
        """
        if isinstance(value, list):
            return repr(value)
        elif isinstance(value, dict):
            return "\n".join(f"{k}={v}" for k, v in value.items())
        return str(value or "")

    def to_dict(self) -> dict:
        """
        Convert the attributes of the FixtureResult into a dictionary.
        """
        properties = {attr: getattr(self, attr) for attr in vars(self)}
        properties.update(
            {
                "stdout": CodeWrapper(
                    self.completed_process.stdout.decode(
                        "utf-8", errors="backslashreplace"
                    ),
                    "console",
                )
                if self.completed_process.stdout
                else None,
                "stderr": CodeWrapper(
                    self.completed_process.stderr.decode(
                        "utf-8", errors="backslashreplace"
                    ),
                    "console",
                )
                if self.completed_process.stderr
                else None,
                "return_code": self.completed_process.returncode
                if self.completed_process.returncode
                else None,
                "elapsed_time": self.elapsed_time(),
                "stack_trace": self.run_exception.stack_trace
                if hasattr(self.run_exception, "stack_trace")
                else None,
                "stream_exceeded": self.run_exception.exceeded_stream
                if hasattr(self.run_exception, "exceeded_stream")
                else None,
            }
        )
        formatted_properties = {
            key: self.format_value(value) for key, value in properties.items()
        }
        return formatted_properties
