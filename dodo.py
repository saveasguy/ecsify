import os
from pathlib import Path

from doit import task_params
from doit.tools import title_with_actions

build_path_param = {
    "name": "build_path",
    "short": "b",
    "long": "build-path",
    "default": "build",
}

disable_examples_param = {
    "name": "disable_examples",
    "long": "disable-examples",
    "type": bool,
    "default": False,
    "help": "do not build examples",
}

test_param = {
    "name": "test",
    "long": "test",
    "type": str,
    "choices": (
        ("off", "disable testing"),
        ("on", "enable testing"),
        ("coverage", "enable testing with code coverage"),
    ),
    "default": "coverage",
}


def _build_type(build_path: str) -> str:
    for file in Path(build_path).iterdir():
        if file.is_dir() and file.name == "Debug":
            return "Debug"
    return "Release"


@task_params([build_path_param, disable_examples_param, test_param])
def task_config(build_path: str, disable_examples: bool, test: str):
    """configure the project"""
    build_type = _build_type(build_path)
    cmd = (
        f"cmake -S . -B {build_path}/{build_type}"
        + " -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        + " -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake"
    )
    if disable_examples:
        cmd += " -DECSIFY_BUILD_EXAMPLES=OFF"
    if test == "off":
        cmd += " -DECSIFY_ENABLE_TESTING=OFF"
    elif test == "on":
        cmd += " -DECSIFY_ENABLE_COVERAGE=OFF"
    else:
        cmd += ' -DCMAKE_CXX_FLAGS="-coverage"'
    cmd += " -DCMAKE_BUILD_TYPE=" + build_type
    return {
        "actions": [cmd],
        "verbosity": 2,
        "title": title_with_actions,
    }


@task_params([build_path_param])
def task_build(build_path: str):
    """build the project"""
    return {
        "actions": [f"cmake --build {build_path}/{_build_type(build_path)} --parallel"],
        "task_dep": ["config"],
        "verbosity": 2,
        "title": title_with_actions,
    }


@task_params([build_path_param])
def task_lint(build_path: str):
    """run C++ linters and static analyzers"""

    def source_files():
        for root, _, files in os.walk("."):
            root_path = Path(root)
            for file in files:
                file_path = root_path / file
                if file_path.suffix in {".h", ".cc"}:
                    yield str(file_path)
        return

    sources = " ".join(source_files())
    return {
        "actions": [
            f"clang-tidy -p {build_path}/{_build_type(build_path)} {sources}",
            "cpplint --root=.. " + sources,
            "clang-format --dry-run -Werror " + sources,
        ],
        "task_dep": ["config"],
        "title": title_with_actions,
    }


@task_params([build_path_param])
def task_test(build_path: str):
    """run tests"""
    return {
        "actions": [f"ctest --test-dir {build_path}/{_build_type(build_path)}/tests"],
        "task_dep": ["build"],
        "verbosity": 2,
        "title": title_with_actions,
    }
