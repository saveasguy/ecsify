import os
from pathlib import Path
from typing import Iterator

from doit import task_params
from doit.tools import title_with_actions

build_path_param = {
    "name": "build_path",
    "short": "b",
    "long": "build-path",
    "default": "build/Debug",
}

disable_examples_param = {
    "name": "disable_examples",
    "long": "disable-examples",
    "type": bool,
    "default": False,
    "help": "don't build examples",
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
    "default": "on",
}

disable_benchmarks_param = {
    "name": "disable_benchmarks",
    "long": "disable-benchmarks",
    "type": bool,
    "default": False,
    "help": "don't build benchmarks",
}


@task_params(
    [build_path_param, disable_examples_param, test_param, disable_benchmarks_param]
)
def task_config(
    build_path: str, disable_examples: bool, test: str, disable_benchmarks: bool
):
    """configure the project"""
    build_type = Path(build_path).name
    cmd = (
        f"cmake -S . -B {build_path}"
        + " -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        + " -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake"
    )
    if disable_examples:
        cmd += " -DECSIFY_BUILD_EXAMPLES=OFF"
    if test == "off":
        cmd += " -DECSIFY_ENABLE_TESTING=OFF -DECSIFY_ENABLE_COVERAGE=OFF"
    elif test == "coverage":
        cmd += " -DECSIFY_ENABLE_COVERAGE=ON"
    cmd += " -DCMAKE_BUILD_TYPE=" + build_type
    if disable_benchmarks:
        cmd += " -DECSIFY_BUILD_BENCHMARKS=OFF"
    return {
        "actions": [cmd],
        "verbosity": 2,
        "title": title_with_actions,
    }


@task_params([build_path_param])
def task_build(build_path: str):
    """build the project"""
    return {
        "actions": [f"cmake --build {build_path} --parallel"],
        "verbosity": 2,
        "title": title_with_actions,
    }


@task_params([build_path_param])
def task_lint(build_path: str):
    """run C++ linters and static analyzers"""

    def source_files() -> Iterator[str]:
        src_dirs = frozenset(
            (
                Path("benchmarks"),
                Path("examples"),
                Path("include"),
                Path("src"),
                Path("tests"),
            )
        )
        cxx_extensions = frozenset((".h", ".cc"))
        for root, _, files in os.walk("."):
            root = Path(root)
            if not any(root.is_relative_to(dir) for dir in src_dirs):
                continue
            for file in files:
                file_path = root / file
                if file_path.suffix in cxx_extensions:
                    yield str(file_path)
        return

    sources = " ".join(source_files())
    return {
        "actions": [
            f"clang-tidy-18 -p {build_path} {sources}",
            "cpplint --root=.. " + sources,
            "clang-format-18 --dry-run -Werror " + sources,
        ],
        "title": title_with_actions,
    }


@task_params([build_path_param])
def task_test(build_path: str):
    """run tests"""
    return {
        "actions": [f"ctest --test-dir {build_path}/tests"],
        "verbosity": 2,
        "title": title_with_actions,
    }
